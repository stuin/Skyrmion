#include <array>
#include <deque>
#include <map>
#include <thread>

#include "../UpdateList.h"
#include "../AudioList.h"
#include "../NetworkList.h"
#include "../../input/Settings.h"
#include "../../util/TimingStats.hpp"
#include "SharedUpdateList.hpp"
#include "DirectFileIO.hpp"

#define SOKOL_IMPL
#define SOKOL_GLCORE
#define SOKOL_TRACE_HOOKS
#include "../../include/sokol/sokol_gfx.h"//
#include "../../include/sokol_gp/sokol_gp.h"//
#include "../../include/sokol/sokol_app.h"//
#include "../../include/sokol/sokol_audio.h"//
#include "../../include/sokol/sokol_glue.h"//
#include "../../include/sokol/sokol_time.h"//
#include "../../include/sokol/sokol_log.h"//

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "../../include/sokol_gp/thirdparty/stb_image.h"//

#define SOKOL_IMGUI_IMPL
#include "../../include/imgui/imgui.h"//
#include "../../include/sokol/util/sokol_imgui.h"//
#include "../../include/sokol/util/sokol_gfx_imgui.h"//

#include <GLFW/glfw3.h>

#define DTOR 0.0174532925199
#define RTOD 57.2957795131

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            460
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

/*
 * Manages layers of nodes through update cycle
 */

//Static variables
LayerData UpdateList::layers[MAXLAYER];
int UpdateList::maxLayer = 0;
int UpdateList::maxULayer = 0;
bool UpdateList::running = false;
std::vector<UNode *> UpdateList::deleted1;
std::vector<UNode *> UpdateList::deleted2;

//Rendering
Node *UpdateList::camera = NULL;
FloatRect UpdateList::cameraRect;
FloatRect UpdateList::screenRect;
skColor UpdateList::backgroundColor;

//Event handling
std::array<std::vector<UNode *>, EVENT_MAX> UpdateList::listeners;
std::deque<Event> UpdateList::event_queue;
std::array<Event, EVENT_MAX> UpdateList::event_previous;
std::vector<int> UpdateList::watchedKeycodes;
std::vector<bool> UpdateList::watchedKeycodesPrevious;
bool UpdateList::remapKeycode = false;

//System timers
TimingStats DebugTimers::updateTimes;
TimingStats DebugTimers::updateLiteralTimes;
TimingStats DebugTimers::frameTimes;
TimingStats DebugTimers::frameNodeTimes;
TimingStats DebugTimers::frameBufferTimes;

//Skyrmion Resource Data
std::vector<ResourceData> UpdateList::resourceData;
std::vector<BufferData> UpdateList::bufferData;
std::vector<ShaderUniform> UpdateList::shaderUniforms;

//Sokol textures
std::vector<sg_image> textureSet;
std::vector<sg_view> bufferSet;
std::vector<sg_shader> shaderSet;

std::thread updates;

//Load image from file
static Vector2i load_image(std::string filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if(!data) {
    	textureSet.push_back(img);
        return Vector2i(0, 0);
    }
    sg_image_desc image_desc = {0};
    image_desc.width = width;
    image_desc.height = height;
    image_desc.data.mip_levels[0].ptr = data;
    image_desc.data.mip_levels[0].size = (size_t)(width * height * 4);
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    textureSet.push_back(img);
    return Vector2i(width, height);
}

//Load texture from file and add to set
int UpdateList::loadResource(std::string filename) {
	if(filename.length() > 0 && filename[0] != '_') {
		if(filename.substr(filename.length()-4) == ".png") {
			resourceData.emplace_back(filename, SK_INVALID);

			Vector2i size = load_image(filename);
			if(size.x > 0) {
				resourceData[resourceData.size() - 1].type = SK_TEXTURE;
				resourceData[resourceData.size() - 1].size = size;
			}
		} else if(filename.substr(filename.length()-3) == ".fs") {
			//Load shader
			//filename = TextFormat(filename.c_str(), GLSL_VERSION);
			//shaderSet.push_back(LoadShader(0, filename.c_str()));
			resourceData.emplace_back(filename, SK_SHADER, Vector2i(0, 0), shaderSet.size()-1);
			textureSet.emplace_back();
		} else {
			textureSet.emplace_back();
			resourceData.emplace_back(filename, SK_INVALID);
		}
	} else {
		textureSet.emplace_back();
		resourceData.emplace_back(filename, SK_INVALID);
	}
	return textureSet.size() - 1;
}

//Replace blank texture with render buffer
sint UpdateList::createResource(sint texture, Vector2i size, sint index, int type) {
	if(texture == 0)
		texture = UpdateList::getResourceCount();
	while(texture >= resourceData.size()) {
		textureSet.emplace_back();
		resourceData.emplace_back(UNKNOWNSPACE, SK_INVALID);
	}
	//if(resourceData[texture].type == SK_TEXTURE && resourceData[texture].index != 0)
	//	return resourceData[texture].index;
	if(resourceData[texture].type != SK_INVALID)
		throw new std::invalid_argument(BUFFERERROR);

	//Mark resource location
	resourceData[texture].size = size;
	resourceData[texture].index = index;
	resourceData[texture].type = type;
	resourceData[texture].filename = UNKNOWNRESOURCE;
	return texture;
}

//Draw ImGui texture
void UpdateList::drawImGuiTexture(sint texture, Vector2i size) {
	if(texture >= resourceData.size() || !resourceData[texture].isTexture())
		throw new std::invalid_argument(TEXTUREERROR);
	sg_view_desc view_desc = {};
	view_desc.texture.image = textureSet[texture];
	sg_view tex_view = sg_make_view(view_desc);
	ImGui::Image(simgui_imtextureid(tex_view), ImVec2(size.x, size.y));
}

//Pick color from texture
skColor UpdateList::pickColor(sint texture, Vector2i position) {
	if(texture >= resourceData.size() || !resourceData[texture].isTexture())
		return skColor(0,0,0,0);

	ResourceData data = resourceData[texture];
	sint index = (position.x+position.y*data.size.x)*4;
	int width, height, channels;
	uint8_t* image = stbi_load(data.filename.c_str(), &width, &height, &channels, 4);
	if(image == NULL)
		return skColor(0,0,0,0);
	skColor color(image + index);
	stbi_image_free(image);
	return color;
}

void UpdateList::sendUniformValues(sint uIndex) {
	ShaderUniform uniform = shaderUniforms[uIndex];
	sint rIndex = uniform.texture;
	sint sIndex = resourceData[uniform.shader].index;
	//std::cout << rIndex << ":" << uIndex << ":" << sIndex << "\n";

	//Run direct texture replacement
	if(uniform.type == SKU_DIRECT_TEXTURE) {
		//UpdateTexture(textureSet[sIndex], uniform.iValues.data());
		std::cout << "INFO: TEXTURE UNIFORM: " << uniform.iValues << "\n";
		return;
	}

	//To use as global variable
	if(uniform.shader == 0)
		return;

	//Finalize unknown shader uniform
	if(uniform.location == -1) {
		//uniform.location = GetShaderLocation(shaderSet[sIndex], uniform.name.c_str());
		if(resourceData[uniform.texture].filename == UNKNOWNRESOURCE)
			resourceData[uniform.texture].filename = uniform.name;
	}

	//Send values to shader
	/*switch(uniform.type) {
	case SKU_FLOAT:
		SetShaderValue(shaderSet[sIndex], uniform.location, &(uniform.fValues[0]), SHADER_UNIFORM_FLOAT);
		break;
	case SKU_INT:
		SetShaderValue(shaderSet[sIndex], uniform.location, &(uniform.iValues[0]), SHADER_UNIFORM_INT);
		break;
	case SKU_FLOAT3_VECTOR:
		SetShaderValueV(shaderSet[sIndex], uniform.location, uniform.fValues.data(), SHADER_UNIFORM_VEC3, uniform.fValues.size() / 3);
		break;
	case SKU_INT3_VECTOR:
		SetShaderValueV(shaderSet[sIndex], uniform.location, uniform.iValues.data(), SHADER_UNIFORM_IVEC3, uniform.iValues.size() / 3);
		break;
	case SKU_FLOAT2:
		SetShaderValueV(shaderSet[sIndex], uniform.location, uniform.fValues.data(), SHADER_UNIFORM_VEC2, 1);
		break;
	case SKU_TEXTURE:
		SetShaderValueTexture(shaderSet[sIndex], uniform.location, textureSet[uniform.iValues[0]]);
		break;
	}*/

	//if(uniform.isFloat())
	//	std::cout << "INFO: SHADER UNIFORM: " << uniform.location << ": " << uniform.fValues << "\n";
	//else
	//	std::cout << "INFO: SHADER UNIFORM: " << uniform.location << ": " << uniform.iValues << "\n";

	//Notify nodes of uniform update
	event_previous[EVENT_BUFFER] = {};
	event_queue.emplace_back(EVENT_BUFFER, true, rIndex);
}

static const std::map<int, int> blendModeMap = {
	{SK_BLEND_ALPHA, SGP_BLENDMODE_BLEND},
	{SK_BLEND_ALPHA_MULT, SGP_BLENDMODE_BLEND_PREMULTIPLIED},
	{SK_BLEND_ADD, SGP_BLENDMODE_ADD},
	{SK_BLEND_MULT, SGP_BLENDMODE_MUL},
};

void sgp_set_color(skColor color) {
	sgp_set_color(color.red, color.green, color.blue, color.alpha);
}

void UpdateList::drawNode(Node *source, sint passthrough) {
	FloatRect rect = source->getRect();
	RenderComponent *rendering = source->getRenderComponent(false);

	//Check for valid rendering data
	if(rendering == NULL) {
		sgp_set_color(COLOR_PURPLE);
		sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
		sgp_reset_color();
		return;
	}

	//Check if rendering from/to passthrough buffer
	Vector2f scale = source->getScale();
	if(rendering->getType() == RENDER_PASSTHROUGH_BUFFER && passthrough != 0) {
		rendering = rendering->getSubComponent();
		rect.width /= scale.x;
		rect.height /= scale.y;
		scale = Vector2i(1,1);
	}

	sgp_set_blend_mode((sgp_blend_mode)blendModeMap.at(rendering->getBlendMode()));
	sgp_set_color(rendering->getColor());

	sint texture = source->getTexture();
	Vector2f flip = Vector2f(scale.x < 0 ? -1 : 1, scale.y < 0 ? -1 : 1);
	Vector2f scaleA = scale.abs();

	switch(rendering->getType()) {
	case RENDER_TEXTURE_SINGLE: case RENDER_PASSTHROUGH_BUFFER:
		if(resourceData[texture].isTexture()) {
			Vector2i size = resourceData[texture].size;
			sgp_rect src = {(float)0, (float)0, size.x*flip.x, size.y*flip.y};
			sgp_rect dst = {rect.left, rect.top, rect.width, rect.height};
			sgp_set_image(0, textureSet[texture]);
			sgp_draw_textured_rect(0, dst, src);
			sgp_reset_image(0);
			break;
		}
	case RENDER_COLOR_SINGLE:
		sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
		break;
	case RENDER_TEXTURE_RECT: {
		TextureRect tex = *rendering->getTextureRect();
		if(tex.pwidth != 0 && tex.pheight != 0) {
			Vector2i origin = Vector2i(abs(tex.pwidth)*scaleA.x/2, abs(tex.pheight)*scaleA.y/2);
			sgp_rect dst = {tex.px*scaleA.x+rect.left+origin.x, tex.py*scaleA.y+rect.top+origin.y, tex.pwidth*scale.x, tex.pheight*scale.y};
			sgp_rect src = {(float)tex.tx, (float)tex.ty, flip.x*tex.twidth, flip.y*tex.theight};
			if(resourceData[texture].isTexture())
				sgp_set_image(0, textureSet[texture]);
			if(tex.rotation != 0) {
				sgp_push_transform();
				sgp_rotate_at(DTOR*tex.rotation, dst.x + dst.w/2.0, dst.y + dst.h/2.0);
				sgp_draw_textured_rect(0, dst, src);
				sgp_pop_transform();
			} else {
				sgp_draw_textured_rect(0, dst, src);
			}
			if(resourceData[texture].isTexture())
				sgp_reset_image(0);
		}
		} break;
	case RENDER_TEXTURE_ARRAY: case RENDER_COLOR_TEXTURE_ARRAY: {
		std::vector<TextureRect> *textureRects = rendering->getTextureRects();
		if(resourceData[texture].isTexture())
			sgp_set_image(0, textureSet[texture]);
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			if(tex.pwidth != 0 && tex.pheight != 0) {
				Vector2i origin = Vector2i(abs(tex.pwidth)*scaleA.x/2, abs(tex.pheight)*scaleA.y/2);
				sgp_rect dst = {tex.px*scaleA.x+rect.left+origin.x, tex.py*scaleA.y+rect.top+origin.y, tex.pwidth*scale.x, tex.pheight*scale.y};
				sgp_rect src = {(float)tex.tx, (float)tex.ty, flip.x*tex.twidth, flip.y*tex.theight};
				if(rendering->getType() == RENDER_COLOR_TEXTURE_ARRAY)
					sgp_set_color(rendering->getColor(i));
				if(tex.rotation != 0) {
					sgp_push_transform();
					sgp_rotate_at(DTOR*tex.rotation, dst.x + dst.w/2.0, dst.y + dst.h/2.0);
					sgp_draw_textured_rect(0, dst, src);
					sgp_pop_transform();
				} else {
					sgp_draw_textured_rect(0, dst, src);
				}
				if(rendering->getType() == RENDER_COLOR_TEXTURE_ARRAY)
					sgp_set_color(rendering->getColor());
			}
		}
		if(resourceData[texture].isTexture())
			sgp_reset_image(0);
		} break;
	case RENDER_COLOR_RECT: {
		sgp_point points[] = {
			{rect.left, rect.top},
			{rect.left+rect.width, rect.top},
			{rect.left+rect.width, rect.top+rect.height},
			{rect.left, rect.top+rect.height}
		};
		sgp_draw_lines_strip(points, 4);

		if(rendering->getColor(1) != COLOR_EMPTY) {
			sgp_set_color(rendering->getColor(1));
			sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
		}
		} break;
	case RENDER_COLOR_ARRAY: case RENDER_GRADIENT_ARRAY: {
		std::vector<skColor> *colors = rendering->getColors();
		uint width = rendering->getSize();
		uint height = colors->size() / rendering->getSize();
		float tWidth = (float)rect.width/(width);
		float tHeight = (float)rect.height/(height);
		//std::cout << width << "," << height << ": " << tWidth << "," << tHeight << "\n";
		for(sint y = 0; y < height-1; y++) {
			for(sint x = 0; x < width-1; x++) {
				sgp_rect dst = {rect.left + tWidth*x, rect.top + tHeight*y, tWidth, tHeight};
				sgp_set_color((*colors)[x + y*width]);
				if(rendering->getType() == RENDER_COLOR_ARRAY)
					sgp_draw_filled_rect(dst.x, dst.y, dst.w, dst.h);
				else {
					//Color color2 = rayColor((*colors)[(x+1) + y*width]);
					//Color color3 = rayColor((*colors)[x + (y+1)*width]);
					//Color color4 = rayColor((*colors)[(x+1) + (y+1)*width]);
					//DrawRectangleGradientEx(dst, color1, color3, color4, color2);
				}
			}
		}
		} break;
	case RENDER_STRING:
		//if(source->getString() != NULL && resourceData[texture].type == SK_FONT)
		//	DrawTextEx(fontSet[resourceData[texture].index], source->getString(), Vector2{rect.left, rect.top}, rendering->getSize(), 1, color);
		//else if(source->getString() != NULL)
		//	DrawTextEx(GetFontDefault(), source->getString(), Vector2{rect.left, rect.top}, rendering->getSize(), 1, color);
		break;
	}

	sgp_reset_color();
	sgp_reset_blend_mode();
}

//Thread safe draw nodes in list
void UpdateList::draw(FloatRect cameraRect) {
    // Clear the frame buffer.
    sgp_set_color(backgroundColor);
    sgp_clear();
    sgp_reset_color();

    sgp_project(cameraRect.left, cameraRect.left+cameraRect.width, cameraRect.top, cameraRect.top+cameraRect.height);

    uint64_t lastTime = stm_now();

	//Render each node in order
	for(int layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(!layers[layer].hidden) {
			//if(layers[layer].shader != 0)
			//	BeginShaderMode(shaderSet[resourceData[layers[layer].shader].index]);
			while(source != NULL) {
				if(!source->isHidden() &&
					(layers[layer].global || source->getRect().intersects(cameraRect))) {

					drawNode(source);
				}
				source = (Node*)source->getNext();
			}
			//EndShaderMode();
		}
	}

	DebugTimers::frameNodeTimes.addDelta(stm_sec(stm_since(lastTime)));
	sgp_reset_project();
}

void finilizeBuffer(BufferData data) {
	sint texture = data.texture;
	std::string *colorLabel = new std::string("color-buffer-");
	*colorLabel += std::to_string(texture);
	std::string *depthLabel = new std::string("depth-buffer-");
	*depthLabel += std::to_string(texture);

	sg_image_desc color_img_desc = {0};
    color_img_desc.usage.storage_image = true;
    color_img_desc.width = data.size.x;
    color_img_desc.height = data.size.y;
    color_img_desc.pixel_format = (sg_pixel_format)sapp_color_format();
    color_img_desc.label = colorLabel->c_str();
    sg_image color_img = sg_make_image(&color_img_desc);

    //sg_image_desc depth_img_desc = color_img_desc;
    //depth_img_desc.pixel_format = (sg_pixel_format)sapp_depth_format();
    //depth_img_desc.label = depthLabel->c_str();
    //sg_image depth_img = sg_make_image(&depth_img_desc);

	sg_view_desc simg_view_desc = {
        .storage_image = {
            .image = color_img,
        },
    };
    bufferSet.push_back(sg_make_view(&simg_view_desc));

	textureSet[texture] = color_img;
}

void UpdateList::drawBuffer(sint bIndex) {
	BufferData data = bufferData[bIndex];
	sint rIndex = data.texture;
	//std::cout << "INFO: BUFFER: " << rIndex << "\n";

	//Create buffer object
	if(resourceData[rIndex].type == SK_INVALID_BUFFER) {
		finilizeBuffer(data);
		resourceData[rIndex].type = SK_BUFFER;
	}

	uint64_t lastTime = stm_now();

	// Begin recording draw commands for a frame buffer of size (width, height).
	ResourceData buffer = resourceData[data.texture];
    sgp_begin(buffer.size.x, buffer.size.y);

    //Clear buffer
    if(data.color != COLOR_NONE) {
    	skColor color = data.color;
    	sgp_set_color(color);
    	sgp_clear();
    }
    //if(data.shader != 0)
	//	BeginShaderMode(shaderSet[resourceData[data.shader].index]);

    //sgp_viewport(0,0, buffer.size.x, buffer.size.if(data.shader != 0)
	//	BeginShaderMode(shaderSet[resourceData[data.shader].index]);y);
    sgp_project(0, buffer.size.x, 0, buffer.size.y);

    //Render specific linked node
	if(data.source != NULL) {
		FloatRect sourceRect = data.source->getRect();
		sgp_project(sourceRect.left, buffer.size.x, sourceRect.top, buffer.size.y);

		if(data.source->getRenderComponent(false)->getType() == RENDER_PASSTHROUGH_BUFFER)
			drawNode(data.source, 1);
		else
			drawNode(data.source);
	} else {
		sgp_project(0, buffer.size.x, 0, buffer.size.y);
	}

	//Render nodes in included layers
	for(int layer = 0; layer <= maxLayer; layer++) {
		if(data.layers[layer]) {
			Node *source = layers[layer].root;
			while(source != NULL) {
				if(!source->isHidden())
					drawNode(source);
				source = (Node*)source->getNext();
			}
		}
	}

	sgp_reset_project();

	sg_bindings bindings = {};
	bindings.views[0] = bufferSet[bIndex];

    sg_begin_pass({ .compute = true });
    sg_apply_bindings(bindings);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();

    //Notify nodes of buffer update
	event_previous[EVENT_BUFFER] = {};
	event_queue.emplace_back(EVENT_BUFFER, true, rIndex);

	DebugTimers::frameBufferTimes.addDelta(stm_sec(stm_since(lastTime)));
}

void event(const sapp_event* event) {
	bool down = false;
	switch(event->type) {
	case SAPP_EVENTTYPE_KEY_DOWN: case SAPP_EVENTTYPE_KEY_UP:
		//Keyboard
		down = event->type == SAPP_EVENTTYPE_KEY_DOWN;
		if(down && ImGui::GetIO().WantCaptureKeyboard && !UpdateList::remapKeycode)
			break;
		UpdateList::queueEvent(EVENT_KEYPRESS, down,
			event->key_code, event->frame_count, 0);
		if(down)
			UpdateList::remapKeycode = false;
		break;
	case SAPP_EVENTTYPE_MOUSE_DOWN: case SAPP_EVENTTYPE_MOUSE_UP:
		//Mouse button
		down = event->type == SAPP_EVENTTYPE_MOUSE_DOWN;
		if(down && ImGui::GetIO().WantCaptureMouse && !UpdateList::remapKeycode)
			break;
		UpdateList::queueEvent(EVENT_KEYPRESS, down,
			event->mouse_button + MOUSE_OFFSET);
		UpdateList::queueEvent(EVENT_MOUSE, event->type == SAPP_EVENTTYPE_MOUSE_DOWN,
			event->mouse_button, event->mouse_x, event->mouse_y);
		if(down)
			UpdateList::remapKeycode = false;
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		//Mouse movement
		UpdateList::queueEvent(EVENT_MOUSE, event->modifiers & 0x100,
			0, event->mouse_x, event->mouse_y);
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		//Mouse Scrolling
		UpdateList::queueEvent(EVENT_KEYPRESS, event->scroll_y > 0,
			MOUSE_OFFSET+5);
		UpdateList::queueEvent(EVENT_KEYPRESS, event->scroll_y < 0,
			MOUSE_OFFSET+6);
		UpdateList::queueEvent(EVENT_SCROLL, event->scroll_y < 0,
			0, event->scroll_x, event->scroll_y);
		break;
	case SAPP_EVENTTYPE_TOUCHES_BEGAN: case SAPP_EVENTTYPE_TOUCHES_ENDED:
		//Touch
		UpdateList::queueEvent(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN,
			MOUSE_OFFSET+7);
		for(int i = 0; i < event->num_touches; i++) {
			UpdateList::queueEvent(EVENT_TOUCH, event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN,
				MOUSE_OFFSET+7, event->touches[i].pos_x, event->touches[i].pos_y);
		}
		break;
	case SAPP_EVENTTYPE_TOUCHES_MOVED:
		//Touch movement
		for(int i = 0; i < event->num_touches; i++) {
			UpdateList::queueEvent(EVENT_TOUCH, true,
				0, event->touches[i].pos_x, event->touches[i].pos_y);
		}
		break;
	case SAPP_EVENTTYPE_RESIZED:
		UpdateList::queueEvent(EVENT_RESIZE, false,
			event->framebuffer_width/event->window_width, event->window_width, event->window_height);
		break;
	case SAPP_EVENTTYPE_UNFOCUSED: case SAPP_EVENTTYPE_FOCUSED:
		UpdateList::queueEvent(EVENT_FOCUS, event->type == SAPP_EVENTTYPE_UNFOCUSED, 0);
		break;
	case SAPP_EVENTTYPE_ICONIFIED: case SAPP_EVENTTYPE_SUSPENDED: case SAPP_EVENTTYPE_QUIT_REQUESTED:
		UpdateList::queueEvent(EVENT_SUSPEND, true, 0);
		break;
	case SAPP_EVENTTYPE_RESTORED: case SAPP_EVENTTYPE_RESUMED:
		UpdateList::queueEvent(EVENT_SUSPEND, false, 0);
		break;
	default:
		break;
	}

	simgui_handle_event(event);
	sapp_consume_event();
}

//Joystick input handled separately
void UpdateList::queueEvents() {
	//Joystick input
	for(int joystickId = 0; joystickId < 4 && glfwJoystickPresent(GLFW_JOYSTICK_1 + joystickId); joystickId++) {
		int axisCount = 0;
		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + joystickId, &axisCount);

		for(int axisId = 0; axisId+1 < axisCount; axisId += 2) {
			float x = axes[axisId];
			float y = axes[axisId+1];
			x = (std::abs(x) > JOYSTICK_DEADZONE) ? x : 0;
			y = (std::abs(y) > JOYSTICK_DEADZONE) ? y : 0;
			event_queue.emplace_back(EVENT_JOYSTICK, x != 0 || y != 0, axisId/2, x, y);
		}

		//Joystick buttons
		for(sint i = 0; i < watchedKeycodes.size(); i++) {
			int code = watchedKeycodes[i];
			if(code >= JOYSTICK_OFFSET && joystickId == (code-JOYSTICK_OFFSET)/50) {
				int buttonId = (code-JOYSTICK_OFFSET)%50;
				int axisId = (buttonId-33)/2;
				bool negative = (buttonId-33)%2 == 0;
				int buttonCount = 0;
				const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1 + joystickId, &buttonCount);

				if(buttonId<33 && buttonId < buttonCount)
					event_queue.emplace_back(EVENT_KEYPRESS, buttons[buttonId] == GLFW_PRESS, code);
				else if(buttonId>32 && axisId < axisCount && negative)
					event_queue.emplace_back(EVENT_KEYPRESS, axes[axisId] < -JOYSTICK_DEADZONE, code);
				else if(buttonId>32 && axisId < axisCount && !negative)
					event_queue.emplace_back(EVENT_KEYPRESS, axes[axisId] > JOYSTICK_DEADZONE, code);
			}
		}
	}
}

void UpdateList::frame(void) {
    DebugTimers::frameTimes.addDelta(sapp_frame_duration());

    #ifdef PLATFORM_WEB
		UpdateList::update(delta);
	#endif

    UpdateList::queueEvents();
	NetworkList::processNetworking();

	//Update shader uniforms
	for(sint i = 0; i < shaderUniforms.size(); i++) {
		if(shaderUniforms[i].update) {
			sendUniformValues(i);
			shaderUniforms[i].update = false;
		}
	}

	//Reload buffer textures
	for(sint i = 1; i < bufferData.size(); i++) {
		if(bufferData[i].redraw) {
			//drawBuffer(i);
			bufferData[i].redraw = false;
		}
	}

	// Get current window size.
    int width = sapp_width(), height = sapp_height();
	sgp_begin(cameraRect.width, cameraRect.height);
    sgp_viewport(0,0,cameraRect.width, cameraRect.height);

    //Start imgui frame
    simgui_frame_desc_t simguidesc = { };
    simguidesc.width = width;
    simguidesc.height = height;
    simguidesc.delta_time = sapp_frame_duration();
    simguidesc.dpi_scale = sapp_dpi_scale();
    simgui_new_frame(&simguidesc);

    //Find camera position
	screenRect = FloatRect(0,0,width,height);
	if(camera != NULL)
		cameraRect = camera->getRect();
	else
		cameraRect = screenRect;

    //Main draw function
    draw(cameraRect);

    //Render imgui debug
    if(listeners[EVENT_IMGUI].size() > 0) {
	    sgimgui_draw();
	    if(ImGui::BeginMainMenuBar()) {
	        sgimgui_draw_menu("sokol-gfx");

	        //Render menu bar
        	for(UNode *node : listeners[EVENT_IMGUI])
				node->recieveEvent(Event(EVENT_IMGUI, true, 0));
	        ImGui::EndMainMenuBar();
	    }
	    //Render individual windows
	    for(UNode *node : listeners[EVENT_IMGUI])
			node->recieveEvent(Event(EVENT_IMGUI, false, 0));
	}

    // Begin a render pass.
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);
    // Dispatch all draw commands to Sokol GFX.
    sgp_flush();
    // Finish a draw command queue, clearing it.
    sgp_end();

    //imgui render
    simgui_render();

    // End render pass.
    sg_end_pass();
    // Commit Sokol render.
    sg_commit();

    //Loop through list to delete nodes from memory
	std::vector<UNode *>::iterator dit = deleted2.begin();
	while(dit != deleted2.end()) {
		UNode *node = *dit;
		dit = deleted2.erase(dit);
		delete node;
	}
}

void UpdateList::init(void) {
	WindowConfig config = windowConfig();
	std::cout << config.windowSize << "\n";
	screenRect = FloatRect(0,0, config.windowSize.x, config.windowSize.y);

	// initialize Sokol GFX
    sg_desc sgdesc = { };
    sgdesc.environment = sglue_environment();
    sgdesc.logger.func = slog_func;
    sg_setup(&sgdesc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // initialize Sokol GP
    sgp_desc sgpdesc = { };
    sgp_setup(&sgpdesc);
    if(!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }

    glfwInit();

    // init sokol-audio
    saudio_desc saudiodesc = { };
    saudiodesc.stream_cb = AudioList::stream_cb;
    saudiodesc.logger.func = slog_func;
    saudiodesc.num_channels = 2;
    saudio_setup(saudiodesc);

    //initialize imgui
    simgui_desc_t simguidesc = { };
    simgui_setup(&simguidesc);

    //imgui gfx debug
    sgimgui_desc_t sgimgui = { };
    sgimgui_setup(&sgimgui);

    //Set layer names
    for(sint layer = 0; layer < config.layerNames.size(); layer++)
		layers[layer].name = config.layerNames[layer];
	maxLayer = config.layerNames.size()-1;

    //Load textures
    bufferSet.emplace_back();
    bufferData.emplace_back();
	shaderSet.emplace_back();
	for(std::string file : config.textureFiles)
		UpdateList::loadResource(file);

	//WIP: Show loading screen

	#ifdef _DEBUG
	    setupDebugTools();
	#endif

    //Start update thread and initialize
	updates = std::thread(initialize);
	while(!UpdateList::running)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::cout << "Starting Rendering\n";

	/*for(sg_image &image : textureSet) {
    	if(sg_query_image_state(image)) {
    		fprintf(stderr, "failed to load images");
        	exit(-1);
    	}
    }*/
}

void UpdateList::startEngine() {
	std::cout << "SKYRMION: Update thread starting\n";

	//Initial node update
	for(int layer = 0; layer <= maxLayer; layer++) {
		UNode *source = layers[layer].root;

		while(source != NULL) {
			source->update(-1);
			source = source->getNext();
		}
	}
	for(int layer = 0; layer <= maxLayer; layer++) {
		UNode *source = layers[layer].root;

		while(source != NULL) {
			source->update(-1);
			source = source->getNext();
		}
	}
	UpdateList::running = true;

	stm_setup();
	uint64_t lastTime = stm_now();
	while(UpdateList::running) {

		//Calculate delta times
		double delta = stm_sec(stm_laptime(&lastTime));
		DebugTimers::updateTimes.addDelta(delta);

		//Update nodes
		UpdateList::update(delta);
		DebugTimers::updateLiteralTimes.addDelta(stm_sec(stm_since(lastTime)));

		std::this_thread::sleep_for(
			std::chrono::milliseconds(10-(int)stm_ms(stm_since(lastTime))));
	}

	std::cout << "SKYRMION: Update thread ending\n";
}

void UpdateList::cleanup(void) {
	std::cout << "Cleanup Rendering\n";
	running = false;
	updates.join();

	AudioList::cleanupAudio();

	sgimgui_shutdown();
	simgui_shutdown();
	saudio_shutdown();
	glfwTerminate();
    sgp_shutdown();
    sg_shutdown();
}

void UpdateList::stopEngine() {
	running = false;
	sapp_quit();
}

bool UpdateList::isRunning() {
	return running;
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    WindowConfig config = windowConfig();

   	sapp_desc desc = {0};
   	desc.width = config.windowSize.x;
   	desc.height = config.windowSize.y;
   	desc.init_cb = UpdateList::init;
   	desc.frame_cb = UpdateList::frame;
   	desc.event_cb = event;
   	desc.cleanup_cb = UpdateList::cleanup;
   	desc.window_title = config.windowTitle.c_str();
   	desc.logger.func = slog_func;
    return desc;
}
