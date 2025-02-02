#include <array>
#include <deque>
#include <map>
#include <thread>

#include <stdio.h>
#include <stdlib.h>

#include "../UpdateList.h"
#include "../../debug/TimingStats.hpp"

#define SOKOL_IMPL
#define SOKOL_GLCORE
#define SOKOL_TRACE_HOOKS
#include "../../include/sokol/sokol_gfx.h"//
#include "../../include/sokol_gp/sokol_gp.h"//
#include "../../include/sokol/sokol_app.h"//
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

#define DTOR 0.0174532925199
#define RTOD 57.2957795131

#define TEXTUREERROR "Texture does not exist"
#define BUFFERERROR "Cannot replace texture with render buffer"
#define FILEERROR "Failed to read file"

/*
 * Manages layers of nodes through update cycle
 */

//Static variables
LayerData UpdateList::layers[MAXLAYER];
std::vector<Node *> UpdateList::deleted1;
std::vector<Node *> UpdateList::deleted2;
Layer UpdateList::maxLayer = 0;
bool UpdateList::running = false;

//Rendering
Node *UpdateList::camera = NULL;
FloatRect UpdateList::cameraRect;
FloatRect UpdateList::screenRect;

//Event handling
std::deque<Event> event_queue;
std::array<std::vector<Node *>, EVENT_MAX> listeners;

//Textures stored in this file
std::vector<TextureData> textureData;
std::vector<sg_image> textureSet;
std::vector<sg_attachments> bufferSet;
std::vector<Node *> reloadBuffer;

std::thread updates;
sgimgui_t sgimgui;

//Engine compatible file read/write
char *IO::openFile(std::string filename) {
	char *source = NULL;
	FILE *fp = fopen(filename.c_str(), "r");
	if(fp != NULL) {
	    //Go to the end of the file
	    if(fseek(fp, 0L, SEEK_END) == 0) {
	        //Get the size of the file and go back to the start
	        long bufsize = ftell(fp);
	        if(bufsize == -1 || fseek(fp, 0L, SEEK_SET) != 0)
	        	throw new std::invalid_argument(FILEERROR);

	        //Allocate our buffer to that size
	        source = (char*)malloc(sizeof(char) * (bufsize + 1));

	        //Read the entire file into memory
	        size_t newLen = fread(source, sizeof(char), bufsize, fp);
	        if(ferror(fp) != 0)
	            throw new std::invalid_argument(FILEERROR);
	        source[newLen++] = '\0';
	    }
	    fclose(fp);
	}
	return source;
}
void IO::closeFile(char *file) {
	free(file);
}

//Add node to update cycle
void UpdateList::addNode(Node *next) {
	Layer layer = next->getLayer();
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(layer > maxLayer)
		maxLayer = layer;
	if(layers[layer].root == NULL) {
		layers[layer].root = next;
		layers[layer].count = 1;
	} else {
		layers[layer].root->addNode(next);
		layers[layer].count++;
	}
}

void UpdateList::addNodes(std::vector<Node *> nodes) {
	for(Node *node : nodes)
		addNode(node);
}

//Get node in specific layer
Node *UpdateList::getNode(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].root;
}

//Remove all nodes in layer
void UpdateList::clearLayer(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);

	Node *source = layers[layer].root;
	while(source != NULL) {
		source->setDelete();
		source = source->getNext();;
	}
}

//Subscribe node to cetain event type
void UpdateList::addListener(Node *item, int type) {
	listeners[type].push_back(item);
}

void UpdateList::watchKeycode(int keycode) {

}

//Send custom event
void UpdateList::queueEvent(Event event) {
	event.type += EVENT_MAX;
	event_queue.push_back(event);
}

//Send signal message to all nodes in layer
void UpdateList::sendSignal(Layer layer, int id, Node *sender) {
	Node *source = layers[layer].root;
	while(source != NULL) {
		source->recieveSignal(id, sender);
		source = source->getNext();
	}
}

//Send signal message to all nodes in game
void UpdateList::sendSignal(int id, Node *sender) {
	for(Layer layer = 0; layer <= maxLayer; layer++)
		sendSignal(layer, id, sender);
}

//Set camera to follow node
Node *UpdateList::setCamera(Node *follow, Vector2f size, Vector2f position) {
	if(camera != NULL) {
		camera->setSize(Vector2i(size.x,size.y));
		camera->setParent(follow);
	} else
		camera = new Node(0, Vector2i(size.x,size.y), true, follow);
	cameraRect = FloatRect(position.x, position.y, size.x, size.y);
	camera->setPosition(position);
	return camera;
}

//Get camera and screen sizes
FloatRect UpdateList::getCameraRect() {
	return cameraRect;
}
FloatRect UpdateList::getScreenRect() {
	return screenRect;
}
Vector2f UpdateList::getScaleFactor() {
	return cameraRect.getSize() / screenRect.getSize();
}

//Do not update nodes
void UpdateList::pauseLayer(Layer layer, bool pause) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	layers[layer].paused = pause;
}

//Do not render nodes
void UpdateList::hideLayer(Layer layer, bool hidden) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	layers[layer].hidden = hidden;
}

//Update nodes outside of camera bounds
void UpdateList::globalLayer(Layer layer, bool global) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	layers[layer].global = global;
}

//Check if layer is paused
bool UpdateList::isLayerPaused(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].paused;
}

//Check if layer is marked hidden
bool UpdateList::isLayerHidden(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].hidden;
}

//Get all data for layer
LayerData &UpdateList::getLayerData(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer];
}

//Get number of occupied layers
int UpdateList::getLayerCount() {
	return maxLayer + 1;
}

//Load image from file
static sg_image load_image(std::string filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename.c_str(), &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if(!data) {
    	textureSet.push_back(img);
    	textureData.push_back(filename);
        return img;
    }
    sg_image_desc image_desc = {0};
    image_desc.width = width;
    image_desc.height = height;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = (size_t)(width * height * 4);
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    textureSet.push_back(img);
    textureData.emplace_back(filename, Vector2i(width, height));
    return img;
}

//Load texture from file and add to set
int UpdateList::loadTexture(std::string filename) {
	if(filename.length() > 0 && filename[0] != '#')
		load_image(filename);
	else {
		textureSet.emplace_back();
		textureData.emplace_back(filename);
	}
	return textureSet.size() - 1;
}

//Replace blank texture with render buffer
int UpdateList::createBuffer(sint texture, Vector2i size) {
	if(texture >= textureData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	if(texture < textureData.size() && textureData[texture].valid && textureData[texture].buffer != 0)
		return textureData[texture].buffer;
	if(texture < textureData.size() && textureData[texture].valid)
		throw new std::invalid_argument(BUFFERERROR);

	//Create and add buffer
	sint i = bufferSet.size();
	textureData[texture].size = size;
	textureData[texture].buffer = i;
	return i;
}

//Schedule reload call before next draw
void UpdateList::scheduleReload(Node *source) {
	reloadBuffer.push_back(source);
}

//Get size of texture
Vector2i UpdateList::getTextureSize(sint texture) {
	if(texture >= textureData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	return textureData[texture].size;
}

//Get all texture data
TextureData &UpdateList::getTextureData(sint texture) {
	if(texture >= textureData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	return textureData[texture];
}

//Draw ImGui texture
void UpdateList::drawImGuiTexture(sint texture, Vector2i size) {
	if(texture >= textureData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	ImGui::Image(simgui_imtextureid(textureSet[texture]), ImVec2(size.x, size.y));
}

//Pick color from texture
skColor UpdateList::pickColor(sint texture, Vector2i position) {
	if(texture >= textureData.size() || !textureData[texture].valid)
		return skColor(0,0,0,0);

	TextureData data = textureData[texture];
	sint index = (position.x+position.y*data.size.x)*4;
	int width, height, channels;
	uint8_t* image = stbi_load(data.filename.c_str(), &width, &height, &channels, 4);
	skColor color(image + index);
	stbi_image_free(image);
	return color;
}

//Update all nodes in list
void UpdateList::update(double time) {
	UpdateList::processEvents();
	deleted2.insert(deleted2.end(), deleted1.begin(), deleted1.end());
	deleted1.clear();

	//Check collisions and updates
	for(Layer layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(!layers[layer].paused) {
			//Check first node for deletion
			if(source != NULL && source->isDeleted()) {
				deleted1.push_back(source);
				source = source->getNext();
				layers[layer].root = source;
			}

			//For each node in layer order
			while(source != NULL) {
				if(layers[layer].global || camera == NULL || source->getRect().intersects(camera->getRect())) {
					//Check each selected collision layer
					int collisionLayer = 0;
					for(int i = 0; i < (int)source->getCollisionLayers().count(); i++) {
						while(!source->getCollisionLayer(collisionLayer))
							collisionLayer++;

						//Check collision box of each node
						Node *other = layers[collisionLayer].root;
						while(other != NULL && !other->isDeleted()) {
							if(other != source && source->getRect().intersects(other->getRect()))
								source->collide(other, time);
							other = other->getNext();
						}
						collisionLayer++;
					}

					//Update each object
					source->update(time);
				}

				//Check next node for removing from list
				while(source->getNext() != NULL && source->getNext()->isDeleted()) {
					deleted1.push_back(source->getNext());
					source->deleteNext();
					layers[source->getLayer()].count--;
				}

				source = source->getNext();
			}
		}
	}
}

static const std::map<int, int> blendModeMap = {
	{SK_BLEND_ALPHA, SGP_BLENDMODE_BLEND},
	{SK_BLEND_ALPHA_MULT, SGP_BLENDMODE_BLEND_PREMULTIPLIED},
	{SK_BLEND_ADD, SGP_BLENDMODE_ADD},
	{SK_BLEND_MULT, SGP_BLENDMODE_MUL},
};

void UpdateList::drawNode(Node *source) {
	sgp_set_color(source->getColor().red, source->getColor().green, source->getColor().blue, source->getColor().alpha);
	sgp_set_blend_mode((sgp_blend_mode)blendModeMap.at(source->getBlendMode()));

	sint texture = source->getTexture();
	FloatRect rect = source->getDrawRect();
	std::vector<TextureRect> *textureRects = source->getTextureRects();

	if(textureRects->size() == 0) {
		if(texture < textureData.size() && textureData[texture].valid) {
			//Default square texture
			sgp_set_image(0, textureSet[texture]);
			sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
			sgp_reset_image(0);
		} else {
			sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
		}
	} else {
		//Tilemapped or partial texture
		if(texture < textureData.size() && textureData[texture].valid)
			sgp_set_image(0, textureSet[texture]);
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			Vector2f scale = source->getScale();
			if(tex.pwidth != 0 && tex.pheight != 0) {
				sgp_rect dst = {tex.px*scale.x + rect.left, tex.py*scale.y + rect.top, tex.pwidth*scale.x, tex.pheight*scale.y};
				sgp_rect src = {(float)tex.tx, (float)tex.ty, (float)tex.twidth, (float)tex.theight};
				if(tex.rotation != 0) {
					sgp_push_transform();
					sgp_rotate_at(DTOR*tex.rotation, dst.x + dst.w/2.0, dst.y + dst.h/2.0);
					sgp_draw_textured_rect(0, dst, src);
					sgp_pop_transform();
				} else {
					sgp_draw_textured_rect(0, dst, src);
				}
			}
		}
		if(texture < textureData.size() && textureData[texture].valid)
			sgp_reset_image(0);
	}
}

//Thread safe draw nodes in list
void UpdateList::draw(FloatRect cameraRect) {
    // Clear the frame buffer.
    skColor background = backgroundColor();
    sgp_set_color(background.red, background.green, background.blue, background.alpha);
    sgp_clear();

    sgp_project(cameraRect.left, cameraRect.left+cameraRect.width, cameraRect.top, cameraRect.top+cameraRect.height);

	//Render each node in order
	for(Layer layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(!layers[layer].hidden) {
			while(source != NULL) {
				if(!source->isHidden() &&
					(layers[layer].global || source->getRect().intersects(cameraRect))) {

					drawNode(source);
				}
				source = source->getNext();
			}
		}
	}
}

void UpdateList::drawBuffer(Node *source) {
	// Begin recording draw commands for a frame buffer of size (width, height).
	TextureData buffer = textureData[source->getBuffer()];
    sgp_begin(buffer.size.x, buffer.size.y);
    sgp_viewport(0,0, buffer.size.x, buffer.size.y);

    if(source->getColor() != COLOR_NONE) {
    	sgp_set_color(source->getColor().red, source->getColor().green, source->getColor().blue, source->getColor().alpha);
    	sgp_clear();
    }
    drawNode(source);

    sg_pass pass = {0};
    pass.attachments = bufferSet[buffer.buffer];
    sg_begin_pass(&pass);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

void UpdateList::startEngine() {
	std::cout << "SKYRMION: Update thread starting\n";

    #ifdef _DEBUG
		setupDebugTools();
	#endif

	//Initial node update
	for(Layer layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

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

//Process window events on update thread
void UpdateList::processEvents() {
	//Remove deleted nodes
	for(int type = 0; type < EVENT_MAX; type++) {
		for(auto it = listeners[type].begin(); it != listeners[type].end();) {
			if((*it)->isDeleted())
				it = listeners[type].erase(it);
			else
				++it;
		}
	}

	//Send event to marked listeners
	int count = event_queue.size();
	for(int i = 0; i < count; i++) {
		Event event = event_queue.front();
		event_queue.pop_front();

		for(Node *node : listeners[event.type % EVENT_MAX])
			node->recieveEvent(event);
	}
}

void UpdateList::queueEvents() {

}

void event(const sapp_event* event) {
	switch(event->type) {
	case SAPP_EVENTTYPE_KEY_DOWN: case SAPP_EVENTTYPE_KEY_UP:
		//Keyboard
		event_queue.emplace_back(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_KEY_DOWN,
			event->key_code, event->frame_count, 0);
		break;
	case SAPP_EVENTTYPE_MOUSE_DOWN: case SAPP_EVENTTYPE_MOUSE_UP:
		//Mouse button
		event_queue.emplace_back(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_MOUSE_DOWN,
			event->mouse_button + MOUSE_OFFSET);
		event_queue.emplace_back(EVENT_MOUSE, event->type == SAPP_EVENTTYPE_MOUSE_DOWN,
			event->mouse_button, event->mouse_x, event->mouse_y);
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		//Mouse movement
		event_queue.emplace_back(EVENT_MOUSE, event->modifiers & 0x100,
			0, event->mouse_x, event->mouse_y);
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		//Mouse Scrolling
		event_queue.emplace_back(EVENT_KEYPRESS, event->scroll_y > 0,
			MOUSE_OFFSET+5);
		event_queue.emplace_back(EVENT_KEYPRESS, event->scroll_y < 0,
			MOUSE_OFFSET+6);
		event_queue.emplace_back(EVENT_SCROLL, event->scroll_y < 0,
			0, event->scroll_x, event->scroll_y);
		break;
	case SAPP_EVENTTYPE_TOUCHES_BEGAN: case SAPP_EVENTTYPE_TOUCHES_ENDED:
		//Touch
		event_queue.emplace_back(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN,
			MOUSE_OFFSET+7);
		for(int i = 0; i < event->num_touches; i++) {
			event_queue.emplace_back(EVENT_TOUCH, event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN,
				MOUSE_OFFSET+7, event->touches[i].pos_x, event->touches[i].pos_y);
		}
		break;
	case SAPP_EVENTTYPE_TOUCHES_MOVED:
		//Touch movement
		for(int i = 0; i < event->num_touches; i++) {
			event_queue.emplace_back(EVENT_TOUCH, true,
				0, event->touches[i].pos_x, event->touches[i].pos_y);
		}
		break;
	case SAPP_EVENTTYPE_RESIZED:
		event_queue.emplace_back(EVENT_RESIZE, false,
			event->framebuffer_width/event->window_width, event->window_width, event->window_height);
		break;
	case SAPP_EVENTTYPE_UNFOCUSED: case SAPP_EVENTTYPE_FOCUSED:
		event_queue.emplace_back(EVENT_FOCUS, event->type == SAPP_EVENTTYPE_UNFOCUSED, 0);
		break;
	case SAPP_EVENTTYPE_ICONIFIED: case SAPP_EVENTTYPE_SUSPENDED: case SAPP_EVENTTYPE_QUIT_REQUESTED:
		event_queue.emplace_back(EVENT_SUSPEND, true, 0);
		break;
	case SAPP_EVENTTYPE_RESTORED: case SAPP_EVENTTYPE_RESUMED:
		event_queue.emplace_back(EVENT_SUSPEND, false, 0);
		break;
	default:
		break;
	}

	simgui_handle_event(event);
	sapp_consume_event();
}

void UpdateList::frame(void) {
    DebugTimers::frameTimes.addDelta(sapp_frame_duration());
    uint64_t lastTime = stm_now();

    //Reload buffer textures
	for(Node *node : reloadBuffer)
		drawBuffer(node);
	reloadBuffer.clear();

	// Get current window size.
    int width = sapp_width(), height = sapp_height();

	// Begin recording draw commands for a frame buffer of size (width, height).
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

    //imgui gfx debug
    #if _DEBUG
    sgimgui_draw(&sgimgui);
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("sokol-gfx")) {
            ImGui::MenuItem("Capabilities", 0, &sgimgui.caps_window.open);
            ImGui::MenuItem("Frame Stats", 0, &sgimgui.frame_stats_window.open);
            ImGui::MenuItem("Buffers", 0, &sgimgui.buffer_window.open);
            ImGui::MenuItem("Images", 0, &sgimgui.image_window.open);
            ImGui::MenuItem("Samplers", 0, &sgimgui.sampler_window.open);
            ImGui::MenuItem("Shaders", 0, &sgimgui.shader_window.open);
            ImGui::MenuItem("Pipelines", 0, &sgimgui.pipeline_window.open);
            ImGui::MenuItem("Attachments", 0, &sgimgui.attachments_window.open);
            ImGui::MenuItem("Calls", 0, &sgimgui.capture_window.open);
            ImGui::EndMenu();
        }
        skyrmionImguiMenu();
        if(listeners[EVENT_IMGUI].size() > 0) {
        	if(ImGui::BeginMenu("Game")) {
	        	for(Node *node : listeners[EVENT_IMGUI])
					node->recieveEvent(Event(EVENT_IMGUI, true, 0));
				ImGui::EndMenu();
			}
        }
        ImGui::EndMainMenuBar();
    }
    skyrmionImgui();
    for(Node *node : listeners[EVENT_IMGUI])
		node->recieveEvent(Event(EVENT_IMGUI, false, 0));
    #endif

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
	std::vector<Node *>::iterator dit = deleted2.begin();
	while(dit != deleted2.end()) {
		Node *node = *dit;
		dit = deleted2.erase(dit);
		delete node;
	}

	DebugTimers::frameLiteralTimes.addDelta(stm_sec(stm_since(lastTime)));
}

void UpdateList::init(void) {
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

    //initialize imgui
    simgui_desc_t simguidesc = { };
    simgui_setup(&simguidesc);

    //imgui gfx debug
    sgimgui_desc_t gimgui_desc = { };
    sgimgui_init(&sgimgui, &gimgui_desc);

    //Set layer names
    for(Layer layer = 0; layer < layerNames().size(); layer++)
    	layers[layer].name = layerNames()[layer];

    #ifdef _DEBUG
    addDebugTextures();
    #endif

    //Load textures
    bufferSet.emplace_back();
	for(std::string file : textureFiles())
		UpdateList::loadTexture(file);

    //Start update thread and initialize
	updates = std::thread(initialize);

	while(!UpdateList::running)
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	//Prepare buffer textures
	for(sint texture = 0; texture < textureData.size(); texture++) {
		TextureData &data = textureData[texture];
		if(data.buffer != 0) {
			sg_image_desc color_img_desc = {0};
		    color_img_desc.render_target = true;
		    color_img_desc.width = data.size.x;
		    color_img_desc.height = data.size.y;
		    color_img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
		    color_img_desc.label = "color-image";
		    sg_image color_img = sg_make_image(&color_img_desc);

		    sg_image_desc depth_img_desc = color_img_desc;
		    depth_img_desc.pixel_format = SG_PIXELFORMAT_DEPTH;
		    depth_img_desc.label = "depth-image";
		    sg_image depth_img = sg_make_image(&depth_img_desc);

			sg_attachments_desc a_desc = {0};
		    a_desc.colors[0].image = color_img;
		    a_desc.depth_stencil.image = depth_img;
		    a_desc.label = "offscreen-pass";
		    bufferSet.push_back(sg_make_attachments(&a_desc));

			textureSet[texture] = color_img;
			data.valid = true;
		}
	}


    std::cout << "Starting Rendering\n";

	/*for(sg_image &image : textureSet) {
    	if(sg_query_image_state(image)) {
    		fprintf(stderr, "failed to load images");
        	exit(-1);
    	}
    }*/
}

void UpdateList::cleanup(void) {
	std::cout << "Cleanup Rendering\n";
	running = false;
	updates.join();
	sgimgui_discard(&sgimgui);
	simgui_shutdown();
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

   	sapp_desc desc = {0};
   	desc.init_cb = UpdateList::init;
   	desc.frame_cb = UpdateList::frame;
   	desc.event_cb = event;
   	desc.cleanup_cb = UpdateList::cleanup;
   	desc.window_title = windowTitle()->c_str();
   	desc.logger.func = slog_func;
    return desc;
}
