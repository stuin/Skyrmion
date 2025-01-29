#include <array>

#include "UpdateList.h"
#include "Event.h"
#include "../debug/TimingStats.hpp"

#include "../include/imgui/imgui.h"//
#include "../include/rlImGui/rlImGui.h"//

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define DTOR 0.0174532925199
#define RTOD 57.2957795131

#define TEXTUREERROR "Texture does not exist"
#define BUFFERERROR "Cannot replace texture with render buffer"

/*
 * Manages layers of nodes through update cycle
 */

//Static variables
LayerData UpdateList::layers[MAXLAYER];
std::vector<Node *> UpdateList::deleted;
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

//Textures stored in this file
std::vector<TextureData> textureData;
std::vector<Texture2D> textureSet;
std::vector<RenderTexture2D> bufferSet;

//Buffers
struct BufferQueue {
	sint buffer;
	Node *source;
	skColor clear;
};
std::vector<BufferQueue> reloadBuffer;

std::thread updates;

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
	if(texture < textureData.size() && textureData[texture].valid)
		throw new std::invalid_argument(BUFFERERROR);

	//Create and add buffer
	sint i = bufferSet.size();
	textureData[texture].size = size;
	textureData[texture].buffer = i;
	return i;
}

//Schedule reload call before next draw
void UpdateList::scheduleReload(sint buffer, Node *source, skColor clear) {
	reloadBuffer.push_back(BufferQueue{buffer, source, clear});
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

//Engine compatible file read/write
char *UpdateList::openFile(std::string filename) {
	return LoadFileText(filename.c_str());
}
void UpdateList::closeFile(char *file) {
	UnloadFileText(file);
}

//Process window events on update thread
void UpdateList::processEvents() {
	int count = event_queue.size();
	for(int i = 0; i < count; i++) {
		//Send event to marked listeners
		Event event = event_queue.back();
		event_queue.pop_back();
		for(Node *node : listeners[event.type])
			node->recieveEvent(event);
	}
}

//Update all nodes in list
void UpdateList::update(double time) {
	//Check collisions and updates
	for(Layer layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(!layers[layer].paused) {
			//Check first node for deletion
			if(source != NULL && source->isDeleted()) {
				deleted.push_back(source);
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
					deleted.push_back(source->getNext());
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
	std::vector<TextureRect> *textureRects = source->getTextureRects();

	if(textureRects->size() == 0) {
		//Default square texture
		if(textureData[texture].valid) {
			Rectangle src = {0, 0, (float)rect.width, (float)rect.height};
			Vector2 position = {rect.left, rect.top};
			DrawTextureRec(textureSet[source->getTexture()], src, position, WHITE);
		} else {
			Rectangle dst = {rect.left, rect.top, (float)rect.width, (float)rect.height};
			DrawRectangleRec(dst, PURPLE);
		}
	} else {
		//Tilemapped or partial texture
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			if(tex.pwidth != 0 && tex.pheight != 0) {
				Rectangle dst = {tex.px*scale.x + rect.left, tex.py*scale.y + rect.top, tex.pwidth*scale.x, tex.pheight*scale.y};
				Rectangle src = {(float)tex.tx, (float)tex.ty, (float)tex.twidth, (float)tex.theight};
				Vector2 origin = Vector2{tex.pwidth*scale.x/2, tex.pheight*scale.y/2};
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
	ClearBackground(Color{(Layer)(color.red*255), (Layer)(color.green*255), (Layer)(color.blue*255)});

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

void UpdateList::drawBuffer(sint buffer, Node *source, skColor clear) {
	BeginTextureMode(bufferSet[textureData[buffer].buffer]);
	ClearBackground(Color{(Layer)(clear.red*255), (Layer)(clear.green*255), (Layer)(clear.blue*255)});
	drawNode(source);
	EndTextureMode();
	source->setDirty(false);
}

void UpdateList::startEngine() {
	std::cout << "Update thread starting\n";

    #ifdef _DEBUG
	setupDebugTools();
	#endif

	#if defined(PLATFORM_WEB)
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

	#if defined(PLATFORM_WEB)
	    emscripten_set_main_loop(UpdateList::frame, 0, 1);
	#else

		double lastTime = GetTime();
		while(UpdateList::running) {
			UpdateList::processEvents();

			//Calculate delta times
			double delta = GetTime()-lastTime;
			DebugTimers::updateTimes.addDelta(delta);
			lastTime = GetTime();

			//Update nodes and sprites
			UpdateList::update(delta);
			DebugTimers::updateLiteralTimes.addDelta(GetTime()-lastTime);

			std::this_thread::sleep_for(
				std::chrono::milliseconds(10-(int)((GetTime()-lastTime)/1000)));
		}
	#endif

	std::cout << "Update thread ending\n";
}

void queueEvents() {
	//Keyboard
	for(int code : watchedKeycodes) {
		if(code < MOUSE_OFFSET)
			event_queue.emplace_back(EVENT_KEYPRESS, IsKeyDown(code), code);
		else if(code < MOUSE_OFFSET+7)
			event_queue.emplace_back(EVENT_KEYPRESS, IsMouseButtonDown(code-MOUSE_OFFSET), code);
		else if(code == MOUSE_OFFSET+7)
			event_queue.emplace_back(EVENT_KEYPRESS, GetMouseWheelMoveV().y>0, code);
		else if(code == MOUSE_OFFSET+8)
			event_queue.emplace_back(EVENT_KEYPRESS, GetMouseWheelMoveV().y<0, code);
		else if(code == MOUSE_OFFSET+9)
			event_queue.emplace_back(EVENT_KEYPRESS, GetTouchPointCount()>0, code);
	}

	//Mouse
	bool pressed = false;
	for(int button = 0; button < 7; button++) {
		if(IsMouseButtonDown(button)) {
			event_queue.emplace_back(EVENT_MOUSE, IsMouseButtonDown(button), MOUSE_OFFSET+button, GetMouseX(), GetMouseY());
			pressed = true;
		}
	}
	if(!pressed)
		event_queue.emplace_back(EVENT_MOUSE, false, MOUSE_OFFSET+0, GetMouseX(), GetMouseY());
	if(GetMouseWheelMoveV().y != 0 || GetMouseWheelMoveV().x != 0)
		event_queue.emplace_back(EVENT_SCROLL, GetMouseWheelMoveV().y<0, 0, GetMouseWheelMoveV().x, GetMouseWheelMoveV().y);

	//Touch
	for(int touch = 0; touch < GetTouchPointCount(); touch++)
		event_queue.emplace_back(EVENT_TOUCH, true, touch, GetTouchPosition(touch).x, GetTouchPosition(touch).y);
	if(GetTouchPointCount() == 0)
		event_queue.emplace_back(EVENT_TOUCH, false, 0, GetTouchX(), GetTouchY());

	//Window
	if(IsWindowResized())
		event_queue.emplace_back(EVENT_RESIZE, IsWindowResized(), GetRenderWidth()/GetScreenWidth(), GetScreenWidth(), GetScreenHeight());
	event_queue.emplace_back(EVENT_FOCUS, !IsWindowFocused(), 0);
	event_queue.emplace_back(EVENT_SUSPEND, IsWindowHidden() || IsWindowMinimized(), 0);
}

void UpdateList::frame(void) {
	double delta = GetFrameTime();
    DebugTimers::frameTimes.addDelta(delta);

    #if defined(PLATFORM_WEB)
    	UpdateList::processEvents();
    	UpdateList::update(delta);
    #endif

    double lastTime = GetTime();

	// Get current window size.
    int width = GetRenderWidth();
    int height = GetRenderHeight();
    BeginDrawing();

    queueEvents();

    //Start imgui frame
    rlImGuiBegin();

    //Reload buffer textures
	for(BufferQueue queue : reloadBuffer)
		drawBuffer(queue.buffer, queue.source, queue.clear);
	reloadBuffer.clear();

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
    if(ImGui::BeginMainMenuBar()) {
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

    rlImGuiEnd();

    //Loop through list to delete nodes from memory
	std::vector<Node *>::iterator dit = deleted.begin();
	while(dit != deleted.end()) {
		Node *node = *dit;
		dit++;
		delete node;
	}
	deleted.clear();

	DebugTimers::frameLiteralTimes.addDelta(GetTime()-lastTime);

    EndDrawing();
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

	#if defined(PLATFORM_WEB)
		std::cout << "Initializing web\n";
	    initialize();
	#else
	    std::cout << "Initializing desktop\n";

	    //Start update thread and initialize
		updates = std::thread(initialize);

		while(!UpdateList::running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		//Prepare buffer textures
		for(sint texture = 0; texture < textureData.size(); texture++) {
			TextureData &data = textureData[texture];
			if(data.buffer != 0) {
				bufferSet.push_back(LoadRenderTexture(data.size.x, data.size.y));
				textureSet[texture] = bufferSet[data.buffer].texture;
				data.valid = true;
			}
		}

	    std::cout << "Starting Rendering\n";
	    while(!WindowShouldClose() && UpdateList::running) {
			frame();
		}
	#endif

	cleanup();
}

void UpdateList::cleanup() {
	std::cout << "Cleanup Rendering\n";
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
