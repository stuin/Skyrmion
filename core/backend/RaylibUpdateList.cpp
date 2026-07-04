#include <array>
#include <map>
#include <thread>

#include "../UpdateList.h"
#include "../../input/Settings.h"
#include "../../util/TimingStats.hpp"
#include "SharedUpdateList.hpp"

#include "../../include/imgui/imgui.h"//
#include "../../include/rlImGui/rlImGui.h"//
#include "../../include/raylib/src/rlgl.h"//

#ifdef PLATFORM_WEB
	#include <emscripten/emscripten.h>
#endif

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
UNode * UpdateList::uLayers[MAXLAYER] = {NULL};
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
Camera2D raycamera;

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

//Raylib resources
std::vector<Texture2D> textureSet;
std::vector<RenderTexture2D> bufferSet;
std::vector<Font> fontSet;
std::vector<Shader> shaderSet;

std::thread updates;

//Engine compatible file read/write
char *IO::openFile(std::string filename) {
	return LoadFileText(filename.c_str());
}
void IO::closeFile(char *file) {
	UnloadFileText(file);
}
void IO::writeFile(std::string filename, char *text) {
	SaveFileText(filename.c_str(), text);
}
void IO::writeFile(std::string filename, std::string text) {
	char *out = (char*)malloc(text.length());
	strcpy(out, text.c_str());

	IO::writeFile(filename, out);
	free(out);
}

//Load texture from file and add to set
int UpdateList::loadResource(std::string filename) {
	if(filename.length() > 0 && filename[0] != '_') {
		if(filename.substr(filename.length()-4) == ".png") {
			//Load texture
			Texture2D texture = LoadTexture(filename.c_str());
			textureSet.push_back(texture);
			resourceData.emplace_back(filename, SK_TEXTURE, Vector2i(texture.width, texture.height));
			if(texture.id == 0 && texture.width == 0)
				resourceData[resourceData.size() - 1].type = SK_INVALID;
		} else if(filename.substr(filename.length()-4) == ".ttf") {
			//Load font
			fontSet.push_back(LoadFont(filename.c_str()));
			resourceData.emplace_back(filename, SK_FONT, Vector2i(0, 10), fontSet.size()-1);
			textureSet.emplace_back();
		} else if(filename.substr(filename.length()-3) == ".fs") {
			//Load shader
			filename = TextFormat(filename.c_str(), GLSL_VERSION);
			shaderSet.push_back(LoadShader(0, filename.c_str()));
			resourceData.emplace_back(filename, SK_SHADER, Vector2i(0, 0), shaderSet.size()-1);
			textureSet.emplace_back();
		}
	} else {
		textureSet.emplace_back();
		resourceData.emplace_back(filename, SK_INVALID);
	}
	return textureSet.size() - 1;
}

//Replace blank texture with custom resource
sint UpdateList::createResource(sint texture, Vector2i size, sint index, int type) {
	if(texture == 0)
		texture = UpdateList::getResourceCount();
	while(texture >= resourceData.size()) {
		//throw new std::invalid_argument(TEXTUREERROR);
		textureSet.emplace_back();
		resourceData.emplace_back(UNKNOWNSPACE, SK_INVALID);
	}
	//if(resourceData[texture].type == SK_TEXTURE && resourceData[texture].index != 0)
	//	return resourceData[texture].index;
	//if(resourceData[texture].type != SK_INVALID)
	//	throw new std::invalid_argument(BUFFERERROR);

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
	rlImGuiImageSize(&(textureSet[texture]), size.x, size.y);
}

//Pick color from texture
skColor UpdateList::pickColor(sint texture, Vector2i position) {
	if(texture >= resourceData.size() || !resourceData[texture].isTexture())
		return skColor(0,0,0,0);

	Color color = GetImageColor(LoadImageFromTexture(textureSet[texture]), position.x, position.y);
	//std::cout << resourceData[texture].type << "\n";
	return skColor(color.r, color.g, color.b, color.a);
}

//Send values to shader uniform
void UpdateList::sendUniformValues(sint uIndex) {
	ShaderUniform uniform = shaderUniforms[uIndex];
	sint rIndex = uniform.texture;
	sint sIndex = resourceData[uniform.shader].index;
	//std::cout << rIndex << ":" << uIndex << ":" << sIndex << "\n";

	//Run direct texture replacement
	if(uniform.type == SKU_DIRECT_TEXTURE) {
		UpdateTexture(textureSet[sIndex], uniform.iValues.data());
		std::cout << "INFO: TEXTURE UNIFORM: " << uniform.iValues << "\n";
		return;
	}

	//To use as global variable
	if(uniform.shader == 0)
		return;

	//Finalize unknown shader uniform
	if(uniform.location == -1) {
		uniform.location = GetShaderLocation(shaderSet[sIndex], uniform.name.c_str());
		if(resourceData[uniform.texture].filename == UNKNOWNRESOURCE)
			resourceData[uniform.texture].filename = uniform.name;
	}

	//Send values to shader
	switch(uniform.type) {
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
	}

	//if(uniform.isFloat())
	//	std::cout << "INFO: SHADER UNIFORM: " << uniform.location << ": " << uniform.fValues << "\n";
	//else
	//	std::cout << "INFO: SHADER UNIFORM: " << uniform.location << ": " << uniform.iValues << "\n";

	//Notify nodes of uniform update
	event_previous[EVENT_BUFFER] = {};
	event_queue.emplace_back(EVENT_BUFFER, true, rIndex);
}

static const std::map<int, int> blendModeMap = {
	{SK_BLEND_ALPHA, BLEND_ALPHA},
	{SK_BLEND_ALPHA_MULT, BLEND_ALPHA_PREMULTIPLY},
	{SK_BLEND_ADD, BLEND_ADDITIVE},
	{SK_BLEND_MULT, BLEND_MULTIPLIED},
	{SK_BLEND_MAX, BLEND_CUSTOM}
};

Color rayColor(skColor color) {
	return {color.r(), color.g(), color.b(), color.a()};
}

void UpdateList::drawNode(Node *source, sint passthrough) {
	FloatRect rect = source->getRect();
	RenderComponent *rendering = source->getRenderComponent(false);

	//Check for valid rendering data
	if(rendering == NULL) {
		Rectangle dst = {rect.left, rect.top, (float)rect.width, (float)rect.height};
		DrawRectangleRec(dst, PURPLE);
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

	if(rendering->getBlendMode() == SK_BLEND_MAX)
		rlSetBlendFactors(1, 1, RL_MAX);
	BeginBlendMode(blendModeMap.at(rendering->getBlendMode()));


	Color color = rayColor(rendering->getColor());
	sint texture = rendering->getTexture();

	Vector2f flip = Vector2f(scale.x < 0 ? -1 : 1, scale.y < 0 ? -1 : 1);
	Vector2f scaleA = scale.abs();

	switch(rendering->getType()) {
	case RENDER_TEXTURE_SINGLE: case RENDER_PASSTHROUGH_BUFFER:
		if(resourceData[texture].isTexture()) {
			Vector2i size = resourceData[texture].size;
			Rectangle src = {(float)0, (float)0, size.x*flip.x, size.y*flip.y};
			Rectangle dst = {rect.left, rect.top, rect.width, rect.height};
			DrawTexturePro(textureSet[texture], src, dst, Vector2{0, 0}, 0, color);
			break;
		}
	case RENDER_COLOR_SINGLE:
		DrawRectangleRec({rect.left, rect.top, (float)rect.width, (float)rect.height}, color);
		break;
	case RENDER_TEXTURE_RECT: {
		TextureRect tex = rendering->getTextureRect();
		if(tex.pwidth != 0 && tex.pheight != 0) {
			Vector2 origin = Vector2{abs(tex.pwidth)*scaleA.x/2, abs(tex.pheight)*scaleA.y/2};
			Rectangle dst = {tex.px*scaleA.x+rect.left+origin.x, tex.py*scaleA.y+rect.top+origin.y, tex.pwidth*scale.x, tex.pheight*scale.y};
			Rectangle src = {(float)tex.tx, (float)tex.ty, flip.x*tex.twidth, flip.y*tex.theight};
			if(resourceData[texture].isTexture())
				DrawTexturePro(textureSet[texture], src, dst, origin, (float)tex.rotation, WHITE);
			else
				DrawRectanglePro(dst, origin, (float)tex.rotation, PURPLE);
		}
		} break;
	case RENDER_TEXTURE_ARRAY: {
		std::vector<TextureRect> *textureRects = rendering->getTextureRects();
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			if(tex.pwidth != 0 && tex.pheight != 0) {
				Vector2 origin = Vector2{abs(tex.pwidth)*scaleA.x/2, abs(tex.pheight)*scaleA.y/2};
				Rectangle dst = {tex.px*scaleA.x+rect.left+origin.x, tex.py*scaleA.y+rect.top+origin.y, tex.pwidth*scale.x, tex.pheight*scale.y};
				Rectangle src = {(float)tex.tx, (float)tex.ty, flip.x*tex.twidth, flip.y*tex.theight};
				if(resourceData[texture].isTexture())
					DrawTexturePro(textureSet[texture], src, dst, origin, (float)tex.rotation, WHITE);
				else
					DrawRectanglePro(dst, origin, (float)tex.rotation, PURPLE);
			}
		}
		} break;
	case RENDER_COLOR_RECT: {
		Color colorI = rayColor(rendering->getColor(1));
		Rectangle dst = {rect.left, rect.top, (float)rect.width, (float)rect.height};
		if(rendering->getColor(1) != COLOR_EMPTY)
			DrawRectangleRec(dst, colorI);
		DrawRectangleLinesEx(dst, rendering->getSize(), color);
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
				Rectangle dst = {rect.left + tWidth*x, rect.top + tHeight*y, tWidth, tHeight};
				Color color1 = rayColor((*colors)[x + y*width]);
				if(rendering->getType() == RENDER_COLOR_ARRAY)
					DrawRectangleRec(dst, color1);
				else {
					Color color2 = rayColor((*colors)[(x+1) + y*width]);
					Color color3 = rayColor((*colors)[x + (y+1)*width]);
					Color color4 = rayColor((*colors)[(x+1) + (y+1)*width]);
					DrawRectangleGradientEx(dst, color1, color3, color4, color2);
				}
			}
		}
		} break;
	case RENDER_STRING:
		if(source->getString() != NULL && resourceData[texture].type == SK_FONT)
			DrawTextEx(fontSet[resourceData[texture].index], source->getString(), Vector2{rect.left, rect.top}, rendering->getSize(), 1, color);
		else if(source->getString() != NULL)
			DrawTextEx(GetFontDefault(), source->getString(), Vector2{rect.left, rect.top}, rendering->getSize(), 1, color);
		break;
	}

	EndBlendMode();
}

//Thread safe draw nodes in list
void UpdateList::draw(FloatRect cameraRect) {
	ClearBackground(rayColor(backgroundColor));

	raycamera.target = Vector2{cameraRect.left, cameraRect.top};
	raycamera.zoom = screenRect.width / cameraRect.width;

	BeginMode2D(raycamera);

	double lastTime = GetTime();

	//Render each node in order
	for(int layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(!layers[layer].hidden) {
			if(layers[layer].shader != 0)
				BeginShaderMode(shaderSet[resourceData[layers[layer].shader].index]);
			while(source != NULL) {
				if(!source->isHidden() &&
					(layers[layer].global || source->getRect().intersects(cameraRect))) {

					drawNode(source, 0);
				}
				source = (Node*)source->getNext();
			}
			EndShaderMode();
		}
	}

	DebugTimers::frameNodeTimes.addDelta(GetTime()-lastTime);
	EndMode2D();
}

void UpdateList::drawBuffer(sint bIndex) {
	BufferData data = bufferData[bIndex];
	sint rIndex = data.texture;
	//std::cout << "INFO: BUFFER: " << rIndex << "\n";

	//Create buffer object
	if(resourceData[rIndex].type == SK_INVALID_BUFFER) {
		bufferSet.push_back(LoadRenderTexture(data.size.x, data.size.y));
		textureSet[rIndex] = bufferSet[bIndex].texture;
		resourceData[rIndex].type = SK_BUFFER;
	}

	double lastTime = GetTime();
	BeginTextureMode(bufferSet[bIndex]);

	//Clear buffer
	if(data.color != COLOR_NONE)
		ClearBackground(Color{data.color.r(), data.color.g(),
			data.color.b(), data.color.a()});
	if(data.shader != 0)
		BeginShaderMode(shaderSet[resourceData[data.shader].index]);

	//Render specific linked node
	if(data.source != NULL) {
		FloatRect sourceRect = data.source->getRect();
		raycamera.target = Vector2{sourceRect.left, sourceRect.top};
		raycamera.zoom = 1;

		BeginMode2D(raycamera);
		if(data.source->getRenderComponent(false)->getType() == RENDER_PASSTHROUGH_BUFFER)
			drawNode(data.source, 1);
		else
			drawNode(data.source);
	} else {
		raycamera.target = Vector2{0, 0};
		raycamera.zoom = 1;

		BeginMode2D(raycamera);
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

	EndMode2D();
	EndShaderMode();
	EndTextureMode();

	//Notify nodes of buffer update
	event_previous[EVENT_BUFFER] = {};
	event_queue.emplace_back(EVENT_BUFFER, true, rIndex);

	DebugTimers::frameBufferTimes.addDelta(GetTime()-lastTime);
}

//Audio systems
void UpdateList::setVolume(int volume) {
	if(!IsAudioDeviceReady())
		InitAudioDevice();

	SetMasterVolume(volume/100.0);
}

//Background music
Music backgroundMusic;
void UpdateList::musicStream(std::string filename, int volume) {
	if(!IsAudioDeviceReady())
		InitAudioDevice();

	backgroundMusic = LoadMusicStream(filename.c_str());
	SetMusicVolume(backgroundMusic, volume/100.0);
	PlayMusicStream(backgroundMusic);
}

void UpdateList::processAudio() {
	//Play audio
	if(IsAudioDeviceReady() && IsMusicStreamPlaying(backgroundMusic))
		UpdateMusicStream(backgroundMusic);
}

//Register keycode for polling
void UpdateList::watchKeycode(int keycode) {
	watchedKeycodes.push_back(keycode);
	watchedKeycodesPrevious.push_back(false);
}

void UpdateList::startRemap() {
	remapKeycode = true;
}

bool UpdateList::checkKeycode(int code, bool down) {
	//Check keypress by input type
	if(code < MOUSE_OFFSET)
		down = IsKeyDown(code);
	else if(code < MOUSE_OFFSET+7)
		down = IsMouseButtonDown(code-MOUSE_OFFSET);
	else if(code == MOUSE_OFFSET+7)
		down = GetMouseWheelMoveV().y>0;
	else if(code == MOUSE_OFFSET+8)
		down = GetMouseWheelMoveV().y<0;
	else if(code == MOUSE_OFFSET+9)
		down = GetTouchPointCount()>0;
	else if(code >= JOYSTICK_OFFSET) {
		int joystickId = (code-JOYSTICK_OFFSET)/JOYSTICK_NEXT;
		int buttonId = (code-JOYSTICK_OFFSET)%JOYSTICK_NEXT;
		int axisId = (buttonId-33)/2;
		bool negative = (buttonId-33)%2 == 0;
		if(buttonId<33 && IsGamepadAvailable(joystickId))
			down = IsGamepadButtonDown(joystickId, buttonId);
		else if(IsGamepadAvailable(joystickId) && axisId < GetGamepadAxisCount(joystickId) && negative)
			down = GetGamepadAxisMovement(joystickId, axisId) < -JOYSTICK_DEADZONE;
		else if(IsGamepadAvailable(joystickId) && axisId < GetGamepadAxisCount(joystickId) && !negative)
			down = GetGamepadAxisMovement(joystickId, axisId) > JOYSTICK_DEADZONE;
	}
	return down;
}

//Add events to queue on draw thread
void UpdateList::queueEvents() {
	//Keyboard
	for(sint i = 0; i < watchedKeycodes.size(); i++) {
		int code = watchedKeycodes[i];
		bool down = checkKeycode(code, watchedKeycodesPrevious[i]);

		if(down && ImGui::GetIO().WantCaptureKeyboard && code < MOUSE_OFFSET && !remapKeycode)
			continue;
		if(down && ImGui::GetIO().WantCaptureMouse && code >= MOUSE_OFFSET && code < JOYSTICK_OFFSET && !remapKeycode)
			continue;

		//Only send changed keys
		if(down != watchedKeycodesPrevious[i]) {
			event_queue.emplace_back(EVENT_KEYPRESS, down, code);
			watchedKeycodesPrevious[i] = down;
		}
	}

	//Remapping, check every keycode
	if(remapKeycode) {
		for(const auto& [key, code] : Settings::EVENT_KEYMAP) {
			if(code > 0 && checkKeycode(code, false)) {
				event_queue.emplace_back(EVENT_KEYPRESS, true, code);
				remapKeycode = false;
			}
		}
	}

	//Mouse
	bool pressed = false;
	for(int button = 0; button < 7; button++) {
		if(IsMouseButtonDown(button) && !ImGui::GetIO().WantCaptureMouse) {
			event_queue.emplace_back(EVENT_MOUSE, true, button, GetMouseX(), GetMouseY());
			pressed = true;
		}
	}
	if(!pressed)
		event_queue.emplace_back(EVENT_MOUSE, false, 0, GetMouseX(), GetMouseY());
	if(GetMouseWheelMoveV().y != 0 || GetMouseWheelMoveV().x != 0)
		event_queue.emplace_back(EVENT_SCROLL, GetMouseWheelMoveV().y<0, 0, GetMouseWheelMoveV().x, GetMouseWheelMoveV().y);

	//Touch
	for(int touch = 0; touch < GetTouchPointCount(); touch++)
		event_queue.emplace_back(EVENT_TOUCH, true, touch, GetTouchPosition(touch).x, GetTouchPosition(touch).y);
	if(GetTouchPointCount() == 0)
		event_queue.emplace_back(EVENT_TOUCH, false, 0, 0, 0);

	//Joystick
	int joystickId = 0;
	while(joystickId < 4 && IsGamepadAvailable(joystickId)) {
		for(int axisId = 0; axisId+1 < GetGamepadAxisCount(joystickId); axisId += 2) {
			float x = GetGamepadAxisMovement(joystickId, axisId);
			float y = GetGamepadAxisMovement(joystickId, axisId+1);
			x = (std::abs(x) > JOYSTICK_DEADZONE) ? x : 0;
			y = (std::abs(y) > JOYSTICK_DEADZONE) ? y : 0;
			event_queue.emplace_back(EVENT_JOYSTICK, x != 0 || y != 0, axisId/2+(joystickId+1)*4, x, y);
		}
		joystickId++;
	}

	//Window
	if(IsWindowResized())
		event_queue.emplace_back(EVENT_RESIZE, false, GetRenderWidth()/GetScreenWidth(), GetScreenWidth(), GetScreenHeight());
	event_queue.emplace_back(EVENT_FOCUS, !IsWindowFocused(), 0);
	event_queue.emplace_back(EVENT_SUSPEND, IsWindowHidden() || IsWindowMinimized(), 0);
}

void UpdateList::frame(void) {
	double delta = GetFrameTime();
	DebugTimers::frameTimes.addDelta(delta);

	#ifdef PLATFORM_WEB
		UpdateList::update(delta);
	#endif

	// Get current window size.
	int width = GetRenderWidth();
	int height = GetRenderHeight();
	BeginDrawing();

	UpdateList::queueEvents();
	UpdateList::processNetworking();

	//Start imgui frame
	rlImGuiBegin();

	//Update shader uniforms
	for(sint i = 0; i < shaderUniforms.size(); i++) {
		if(shaderUniforms[i].update) {
			sendUniformValues(i);
			shaderUniforms[i].update = false;
		}
	}

	//Reload buffer textures
	for(sint i = 0; i < bufferData.size(); i++) {
		if(bufferData[i].redraw) {
			drawBuffer(i);
			bufferData[i].redraw = false;
		}
	}

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
		if(ImGui::BeginMainMenuBar()) {
			//Render menu bar
			for(UNode *node : listeners[EVENT_IMGUI])
				node->recieveEvent(Event(EVENT_IMGUI, true, 0));
			ImGui::EndMainMenuBar();
		}
		//Render individual windows
		for(UNode *node : listeners[EVENT_IMGUI])
			node->recieveEvent(Event(EVENT_IMGUI, false, 0));
	}

	rlImGuiEnd();

	//Loop through list to delete nodes from memory
	std::vector<UNode *>::iterator dit = deleted2.begin();
	while(dit != deleted2.end()) {
		UNode *node = *dit;
		dit = deleted2.erase(dit);
		delete node;
	}
	//DebugTimers::frameLiteralTimes.addDelta(GetTime()-lastTime);

	EndDrawing();
}

void testThread() {
	std::cout << "SKYRMION: Test Thread\n";
}

void UpdateList::init() {
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	SetExitKey(0);

	WindowConfig config = windowConfig();
	std::cout << config.windowSize << "\n";
	screenRect = FloatRect(0,0, config.windowSize.x, config.windowSize.y);

	InitWindow(config.windowSize.x, config.windowSize.y, config.windowTitle.c_str());

	//initialize imgui
	rlImGuiSetup(true);

	//Set layer names
	for(sint layer = 0; layer < config.layerNames.size(); layer++)
		layers[layer].name = config.layerNames[layer];
	maxLayer = config.layerNames.size()-1;

	//Load resources
	bufferSet.emplace_back();
	bufferData.emplace_back();
	shaderSet.emplace_back();
	fontSet.emplace_back();
	for(std::string file : config.textureFiles)
		UpdateList::loadResource(file);

	//Show loading screen
	BeginDrawing();
	backgroundColor = config.backgroundColor;
	ClearBackground(rayColor(backgroundColor));
	EndDrawing();

	#ifdef _DEBUG
		setupDebugTools();
	#endif

	//std::thread testing = std::thread(testThread);
	//testing.join();

	#ifdef PLATFORM_WEB
		std::cout << "SKYRMION: Initializing web\n";

		initialize();
	#else
		std::cout << "SKYRMION: Initializing desktop\n";

		//Start update thread and initialize
		updates = std::thread(initialize);
		while(!UpdateList::running)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		std::cout << "SKYRMION: Starting Rendering\n";
		while(!WindowShouldClose() && UpdateList::running)
			frame();
	#endif

	cleanup();
}

void UpdateList::startEngine() {
	std::cout << "SKYRMION: Update thread starting\n";

	event_queue.emplace_back(EVENT_RESIZE, true, GetRenderWidth()/GetScreenWidth(), GetScreenWidth(), GetScreenHeight());

	//Initial node update
	for(int layer = 0; layer <= maxULayer; layer++) {
		UNode *source = uLayers[layer];

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

	#ifdef PLATFORM_WEB
		emscripten_set_main_loop(UpdateList::frame, 0, 1);
	#else
		double lastTime = GetTime();
		while(UpdateList::running) {

			//Calculate delta times
			double delta = GetTime()-lastTime;
			DebugTimers::updateTimes.addDelta(delta);
			lastTime = GetTime();

			//Update nodes
			UpdateList::update(delta);
			DebugTimers::updateLiteralTimes.addDelta(GetTime()-lastTime);

			std::this_thread::sleep_for(
				std::chrono::milliseconds(10-(int)((GetTime()-lastTime)/1000)));
		}
	#endif

	std::cout << "SKYRMION: Update thread ending\n";
}

void UpdateList::cleanup() {
	std::cout << "SKYRMION: Cleanup Rendering\n";
	running = false;
	updates.join();

	//Unload resources
	for(Texture2D texture : textureSet)
		UnloadTexture(texture);
	for(RenderTexture2D buffer : bufferSet)
		UnloadRenderTexture(buffer);
	for(Font font : fontSet)
		UnloadFont(font);
	for(Shader shader : shaderSet)
		UnloadShader(shader);

	rlImGuiShutdown();
	CloseWindow();
}

void UpdateList::stopEngine() {
	running = false;
}

bool UpdateList::isRunning() {
	return running;
}

int main() {
	UpdateList::init();
	return 0;
}
