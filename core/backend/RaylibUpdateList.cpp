#include <array>
#include <map>
#include <thread>

#include "../UpdateList.h"
#include "../../debug/TimingStats.hpp"

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

//Textures stored in this file
std::vector<TextureData> textureData;
std::vector<Texture2D> textureSet;
std::vector<RenderTexture2D> bufferSet;
std::vector<Node *> reloadBuffer;
sint requestedBuffers = 0;

std::thread updates;

//Engine compatible file read/write
char *UpdateList::openFile(std::string filename) {
	return LoadFileText(filename.c_str());
}
void UpdateList::closeFile(char *file) {
	UnloadFileText(file);
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
	watchedKeycodes.push_back(keycode);
	watchedKeycodesPrevious.push_back(false);
}

//Send custom event
void UpdateList::queueEvent(Event event) {
	if(event != event_previous[event.type % EVENT_MAX])
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

//Load texture from file and add to set
int UpdateList::loadTexture(std::string filename) {
	if(filename.length() > 0 && filename[0] != '#') {
		Texture2D texture = LoadTexture(filename.c_str());
		textureSet.push_back(texture);
		textureData.emplace_back(filename, Vector2i(texture.width, texture.height));
	} else {
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
	textureData[texture].size = size;
	textureData[texture].buffer = requestedBuffers;
	return requestedBuffers++;
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
	rlImGuiImageSize(&(textureSet[texture]), size.x, size.y);
}

//Pick color from texture
skColor UpdateList::pickColor(sint texture, Vector2i position) {
	if(texture >= textureData.size() || !textureData[texture].valid)
		return skColor(0,0,0,0);

	Color color = GetImageColor(LoadImageFromTexture(textureSet[texture]), position.x, position.y);
	return skColor(color.r, color.g, color.b, color.a);
}

//Update all nodes in list
void UpdateList::update(double time) {
	UpdateList::processEvents();
	UpdateList::processNetworking();
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
	{SK_BLEND_ALPHA, BLEND_ALPHA},
	{SK_BLEND_ALPHA_MULT, BLEND_ALPHA_PREMULTIPLY},
	{SK_BLEND_ADD, BLEND_ADDITIVE},
	{SK_BLEND_MULT, BLEND_MULTIPLIED},
};

void UpdateList::drawNode(Node *source) {
	BeginBlendMode(blendModeMap.at(source->getBlendMode()));

	sint texture = source->getTexture();
	FloatRect rect = source->getDrawRect();
	Vector2f scale = source->getGScale();
	Color color = Color{source->getColor().r(), source->getColor().g(), source->getColor().b(), source->getColor().a()};
	std::vector<TextureRect> *textureRects = source->getTextureRects();

	if(textureRects->size() == 0) {
		//Default square texture
		if(texture < textureData.size() && textureData[texture].valid) {
			Rectangle src = {0, 0, (float)rect.width, (float)rect.height};
			Vector2 position = {rect.left, rect.top};
			DrawTextureRec(textureSet[source->getTexture()], src, position, color);
		} else {
			Rectangle dst = {rect.left, rect.top, (float)rect.width, (float)rect.height};
			DrawRectangleRec(dst, color);
		}
	} else {
		//Tilemapped or partial texture
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			if(tex.pwidth != 0 && tex.pheight != 0) {
				Vector2 origin = Vector2{tex.pwidth*std::abs(scale.x/2), tex.pheight*std::abs(scale.y/2)};
				Rectangle dst = {tex.px*scale.x + rect.left+origin.x, tex.py*scale.y + rect.top+origin.y, tex.pwidth*scale.x, tex.pheight*scale.y};
				Rectangle src = {(float)tex.tx, (float)tex.ty, (float)tex.twidth, (float)tex.theight};
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
	raycamera.zoom = screenRect.getSize().x / cameraRect.getSize().x;

    BeginMode2D(raycamera);

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
	EndMode2D();
}

void UpdateList::drawBuffer(Node *source) {
	BeginTextureMode(bufferSet[textureData[source->getBuffer()].buffer]);
	if(source->getColor() != COLOR_NONE)
		ClearBackground(Color{source->getColor().r(), source->getColor().g(),
			source->getColor().b(), source->getColor().a()});
	drawNode(source);
	EndTextureMode();
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

void UpdateList::frame(void) {
	double delta = GetFrameTime();
    DebugTimers::frameTimes.addDelta(delta);

    #ifdef PLATFORM_WEB
    	UpdateList::update(delta);
    #endif

    double lastTime = GetTime();

	// Get current window size.
    int width = GetRenderWidth();
    int height = GetRenderHeight();
    BeginDrawing();

    UpdateList::queueEvents();

    //Start imgui frame
    rlImGuiBegin();

    //Reload buffer textures
	for(Node *node : reloadBuffer)
		drawBuffer(node);
	reloadBuffer.clear();

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

	DebugTimers::frameLiteralTimes.addDelta(GetTime()-lastTime);

    EndDrawing();
}

void testThread() {
	std::cout << "SKYRMION: Test Thread\n";
}

void UpdateList::init() {
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	InitWindow(1920, 1080, windowTitle()->c_str());

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
	for(std::string file : textureFiles())
		UpdateList::loadTexture(file);

	//Show loading screen
    BeginDrawing();
    skColor color = backgroundColor();
    ClearBackground(Color{color.r(), color.g(), color.b()});
    EndDrawing();

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
