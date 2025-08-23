#include <array>
#include <map>
#include <thread>

#include "../UpdateList.h"
#include "../../util/TimingStats.hpp"
#include "SharedUpdateList.hpp"

#include "../../include/imgui/imgui.h"//
#include "../../include/rlImGui/rlImGui.h"//

#ifdef PLATFORM_WEB
	#include <emscripten/emscripten.h>
#endif

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
Camera2D raycamera;

//Event handling
std::deque<Event> event_queue;
std::array<std::vector<Node *>, EVENT_MAX> listeners;
std::vector<int> watchedKeycodes;
std::array<Event, EVENT_MAX> event_previous;
std::vector<bool> watchedKeycodesPrevious;

//System timers
TimingStats DebugTimers::frameTimes;
TimingStats DebugTimers::frameLiteralTimes;
TimingStats DebugTimers::updateTimes;
TimingStats DebugTimers::updateLiteralTimes;

//Textures stored in this file
std::vector<TextureData> textureData;
std::vector<Texture2D> textureSet;
std::vector<BufferData> bufferData;
std::vector<RenderTexture2D> bufferSet;
std::vector<Font> fontSet;

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

//Subscribe node to cetain event type
void UpdateList::addListener(Node *item, int type) {
	listeners[type].push_back(item);
}

void UpdateList::watchKeycode(int keycode) {
	watchedKeycodes.push_back(keycode);
	watchedKeycodesPrevious.push_back(false);
}

//Send custom event
void UpdateList::queueEvent(Event event) {
	if(event != event_previous[event.type % EVENT_MAX])
		event_queue.push_back(event);
}

//Load texture from file and add to set
int UpdateList::loadTexture(std::string filename) {
	if(filename.length() > 0 && filename[0] != '#') {
		Texture2D texture = LoadTexture(filename.c_str());
		textureSet.push_back(texture);
		textureData.emplace_back(filename, Vector2i(texture.width, texture.height));
		if(texture.id == 0 && texture.width == 0) {
			int i = textureSet.size() - 1;
			textureData[i].valid = false;

			//Load font instead
			if(filename.substr(filename.length()-4) == ".ttf") {
				fontSet.push_back(LoadFontEx(filename.c_str(), 200, NULL, 0));
				textureData[i].buffer = -fontSet.size();
			}
		}
	} else {
		textureSet.emplace_back();
		textureData.emplace_back(filename);
	}
	return textureSet.size() - 1;
}

//Replace blank texture with render buffer
int UpdateList::createBuffer(BufferData data) {
	sint texture = data.texture;
	if(texture >= textureData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	if(texture < textureData.size() && textureData[texture].valid && textureData[texture].buffer != 0)
		return textureData[texture].buffer;
	if(texture < textureData.size() && textureData[texture].valid)
		throw new std::invalid_argument(BUFFERERROR);

	//Create and add buffer
	textureData[texture].size = data.size;
	textureData[texture].buffer = bufferData.size();
	bufferData.push_back(data);
	return textureData[texture].buffer;
}

int UpdateList::createBuffer(sint _texture, Vector2i _size, Layer _layer, skColor _color) {
	return createBuffer(BufferData(_texture, _size, _layer, _color));
}

//Schedule reload call before next draw
void UpdateList::scheduleReload(sint buffer) {
	bufferData[buffer].redraw = true;
}

//Get size of texture
Vector2i UpdateList::getTextureSize(sint texture) {
	if(texture >= textureData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	return textureData[texture].size;
}
Vector2i IO::getTextureSize(sint texture) {
	return UpdateList::getTextureSize(texture);
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
	rlImGuiImageSize(&(textureSet[texture]), size.x, size.y);
}

//Pick color from texture
skColor UpdateList::pickColor(sint texture, Vector2i position) {
	if(texture >= textureData.size() || !textureData[texture].valid)
		return skColor(0,0,0,0);

	Color color = GetImageColor(LoadImageFromTexture(textureSet[texture]), position.x, position.y);
	return skColor(color.r, color.g, color.b, color.a);
}

static const std::map<int, int> blendModeMap = {
	{SK_BLEND_ALPHA, BLEND_ALPHA},
	{SK_BLEND_ALPHA_MULT, BLEND_ALPHA_PREMULTIPLY},
	{SK_BLEND_ADD, BLEND_ADDITIVE},
	{SK_BLEND_MULT, BLEND_MULTIPLIED},
};

void UpdateList::drawNode(Node *source) {
	BeginBlendMode(blendModeMap.at(source->getBlendMode()));
	Color color = Color{source->getColor().r(), source->getColor().g(), source->getColor().b(), source->getColor().a()};

	sint texture = source->getTexture();
	FloatRect rect = source->getRect();
	Vector2f scale = source->getScale();
	std::vector<TextureRect> *textureRects = source->getTextureRects();

	if(source->getString() != NULL && texture < 0) {
		//std::cout << rect.pos() << "\n";
		DrawTextEx(fontSet[-texture-1], source->getString(), Vector2{rect.left, rect.top}, source->getSize().y, 1, color);
	} else if(source->getString() != NULL) {
		DrawTextEx(GetFontDefault(), source->getString(), Vector2{rect.left, rect.top}, source->getSize().y, 1, color);
	} else if(textureRects->size() == 0) {
		//Default square texture
		if(texture < textureData.size() && textureData[texture].valid) {
			//Rectangle src = {0, 0, (float)rect.width/scale.x, (float)rect.height/scale.y};
			Vector2 position = {rect.left, rect.top};
			DrawTextureEx(textureSet[source->getTexture()], position, 0, scale.x, color);
		} else {
			Rectangle dst = {rect.left, rect.top, (float)rect.width, (float)rect.height};
			DrawRectangleRec(dst, color);
		}
	} else {
		//Tilemapped or partial texture
		Vector2f flip = Vector2f(scale.x < 0 ? -1 : 1, scale.y < 0 ? -1 : 1);
		Vector2f scaleA = scale.abs();
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			if(tex.pwidth != 0 && tex.pheight != 0) {
				Vector2 origin = Vector2{tex.pwidth*scaleA.x/2, tex.pheight*scaleA.y/2};
				Rectangle dst = {tex.px*scaleA.x+rect.left+origin.x, tex.py*scaleA.y+rect.top+origin.y, tex.pwidth*scale.x, tex.pheight*scale.y};
				Rectangle src = {(float)tex.tx, (float)tex.ty, flip.x*tex.twidth, flip.y*tex.theight};
				if(textureData[texture].valid)
					DrawTexturePro(textureSet[source->getTexture()], src, dst, origin, (float)tex.rotation, WHITE);
				else
					DrawRectanglePro(dst, origin, (float)tex.rotation, PURPLE);
			}
		}
	}

	EndBlendMode();
}

//Thread safe draw nodes in list
void UpdateList::draw(FloatRect cameraRect) {
	skColor color = backgroundColor();
	ClearBackground(Color{color.r(), color.g(), color.b()});

	raycamera.target = Vector2{cameraRect.left, cameraRect.top};
	raycamera.zoom = screenRect.width / cameraRect.width;

	BeginMode2D(raycamera);

	double lastTime = GetTime();

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

	DebugTimers::frameLiteralTimes.addDelta(GetTime()-lastTime);
	EndMode2D();
}

void UpdateList::drawBuffer(BufferData data) {
	BeginTextureMode(bufferSet[textureData[data.texture].buffer]);

	//Clear buffer
	if(data.color != COLOR_NONE)
		ClearBackground(Color{data.color.r(), data.color.g(),
			data.color.b(), data.color.a()});

	//Render nodes in included layers
	for(Layer layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(data.layers[layer]) {
			while(source != NULL) {
				if(!source->isHidden()) {
					drawNode(source);
				}
				source = source->getNext();
			}
		}
	}

	EndTextureMode();
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

//Add events to queue on draw thread
void UpdateList::queueEvents() {
	//Keyboard
	for(sint i = 0; i < watchedKeycodes.size(); i++) {
		bool down = watchedKeycodesPrevious[i];
		int code = watchedKeycodes[i];

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

		//Only send changed keys
		if(down != watchedKeycodesPrevious[i]) {
			event_queue.emplace_back(EVENT_KEYPRESS, down, code);
			watchedKeycodesPrevious[i] = down;
		}
	}

	//Mouse
	bool pressed = false;
	for(int button = 0; button < 7; button++) {
		if(IsMouseButtonDown(button)) {
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
		event_queue.emplace_back(EVENT_RESIZE, IsWindowResized(), GetRenderWidth()/GetScreenWidth(), GetScreenWidth(), GetScreenHeight());
	event_queue.emplace_back(EVENT_FOCUS, !IsWindowFocused(), 0);
	event_queue.emplace_back(EVENT_SUSPEND, IsWindowHidden() || IsWindowMinimized(), 0);
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

		//Skip duplicates
		if(event != event_previous[event.type % EVENT_MAX]) {
			for(Node *node : listeners[event.type % EVENT_MAX])
				node->recieveEvent(event);
			event_previous[event.type % EVENT_MAX] = event;
		}
	}
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

	//Reload buffer textures
	for(BufferData data : bufferData) {
		if(data.redraw) {
			drawBuffer(data);
			data.redraw = false;
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
	#if _DEBUG
	if(ImGui::BeginMainMenuBar()) {
		//Render menu bar
		if(listeners[EVENT_IMGUI].size() > 0) {
			for(Node *node : listeners[EVENT_IMGUI])
				node->recieveEvent(Event(EVENT_IMGUI, true, 0));
		}
		ImGui::EndMainMenuBar();
	}
	//Render individual windows
	for(Node *node : listeners[EVENT_IMGUI])
		node->recieveEvent(Event(EVENT_IMGUI, false, 0));
	#endif

	rlImGuiEnd();

	//Loop through list to delete nodes from memory
	std::vector<Node *>::iterator dit = deleted2.begin();
	while(dit != deleted2.end()) {
		Node *node = *dit;
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
	InitWindow(1920, 1080, windowTitle()->c_str());
	SetExitKey(0);

	//initialize imgui
	rlImGuiSetup(true);

	//Set layer names
	for(Layer layer = 0; layer < layerNames().size(); layer++)
		layers[layer].name = layerNames()[layer];

	#ifdef _DEBUG
		addDebugTextures();
	#endif

	//Load textures
	bufferSet.emplace_back();
	bufferData.emplace_back();
	for(std::string file : textureFiles())
		UpdateList::loadTexture(file);

	//Show loading screen
	BeginDrawing();
	skColor color = backgroundColor();
	ClearBackground(Color{color.r(), color.g(), color.b()});
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

		//Prepare buffer textures
		for(sint texture = 0; texture < textureData.size(); texture++) {
			TextureData &data = textureData[texture];
			if(data.buffer != 0) {
				bufferSet.push_back(LoadRenderTexture(data.size.x, data.size.y));
				textureSet[texture] = bufferSet[data.buffer].texture;
				data.valid = true;
			}
		}

		std::cout << "SKYRMION: Starting Rendering\n";
		while(!WindowShouldClose() && UpdateList::running) {
			frame();
		}
	#endif

	cleanup();
}

void UpdateList::startEngine() {
	std::cout << "SKYRMION: Update thread starting\n";

	#ifdef PLATFORM_WEB
		//Prepare buffer textures
		for(sint texture = 0; texture < textureData.size(); texture++) {
			TextureData &data = textureData[texture];
			if(data.buffer != 0) {
				bufferSet.push_back(LoadRenderTexture(data.size.x, data.size.y));
				textureSet[texture] = bufferSet[data.buffer].texture;
				data.valid = true;
			}
		}
	#endif

	event_queue.emplace_back(EVENT_RESIZE, IsWindowResized(), GetRenderWidth()/GetScreenWidth(), GetScreenWidth(), GetScreenHeight());

	//Initial node update
	for(Layer layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

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

	for(Texture2D texture : textureSet)
		UnloadTexture(texture);

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
