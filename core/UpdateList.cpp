#include "UpdateList.h"
#include "Event.h"
#include "../debug/TimingStats.hpp"

#include "../include/imgui/imgui.h"//
#include "../include/rlImGui/rlImGui.h"//

#define DTOR 0.0174532925199
#define RTOD 57.2957795131
#define TEXTUREERROR "Texture does not exist"

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
std::vector<Node *> UpdateList::reloadBuffer;

//Event handling
std::atomic_int event_count = 0;
std::deque<Event> event_queue;
std::array<std::vector<Node *>, EVENT_MAX> listeners;

//Textures stored in this file
std::vector<TextureData> textureData;
std::vector<Texture2D> textureSet;
std::vector<Image> textureSetImage;

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

//Schedule reload call before next draw
void UpdateList::scheduleReload(Node *buffer) {
	if(buffer != NULL)
		reloadBuffer.push_back(buffer);
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
		textureSetImage.push_back(LoadImage(filename.c_str()));
		textureData.emplace_back(filename, Vector2i(texture.width, texture.height));
	} else {
		textureSet.emplace_back();
		textureSetImage.emplace_back();
		textureData.emplace_back(filename);
	}
	return textureSet.size() - 1;
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
	rlImGuiImage(&(textureSet[texture]));
}

//Pick color from texture
skColor UpdateList::pickColor(sint texture, Vector2i position) {
	if(texture >= textureData.size() || !textureData[texture].valid)
		return skColor(0,0,0,0);

	Color color = GetImageColor(textureSetImage[texture], position.x, position.y);
	return skColor(color.r, color.g, color.b, color.a);
}

//Process window events on update thread
void UpdateList::processEvents() {
	int count = event_count;
	event_count -= count;
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

//Thread safe draw nodes in list
void UpdateList::draw(FloatRect cameraRect) {
	skColor color = backgroundColor();
	ClearBackground(Color{(Layer)(color.red*255), (Layer)(color.green*255), (Layer)(color.blue*255)});
    //BeginScissorMode(cameraRect.left, cameraRect.top, cameraRect.width, cameraRect.height);

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
	//EndScissorMode();
	EndMode2D();
}

static const std::map<int, int> blendModeMap = {
	{SK_BLEND_ALPHA, BLEND_ALPHA},
	{SK_BLEND_ALPHA_MULT, BLEND_ALPHA_PREMULTIPLY},
	{SK_BLEND_ADD, BLEND_ADDITIVE},
	{SK_BLEND_MULT, BLEND_MULTIPLIED},
};

void UpdateList::drawNode(Node *source) {
	if(source->getBlendMode() != SK_BLEND_NONE)
		BeginBlendMode(blendModeMap[source->getBlendMode()]);

	sint texture = source->getTexture();
	FloatRect rect = source->getDrawRect();
	std::vector<TextureRect> *textureRects = source->getTextureRects();

	if(texture < textureData.size() && textureData[texture].valid && textureRects->size() == 0) {
		//Default square texture
		DrawTexture(textureSet[source->getTexture()], rect.left, rect.top, WHITE);
	} else if(source->getTexture() != -1) {
		//Tilemapped or partial texture
		for(sint i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			Vector2f scale = source->getGScale();
			if(tex.width != 0 && tex.height != 0) {
				Rectangle dst = {tex.px*scale.x + rect.left, tex.py*scale.y + rect.top, abs(tex.width)*scale.x, abs(tex.height)*scale.y};
				Rectangle src = {(float)tex.tx, (float)tex.ty, (float)tex.width, (float)tex.height};
				Vector2 origin = Vector2{source->getOrigin().x, source->getOrigin().y};
				DrawTexturePro(textureSet[source->getTexture()], src, dst, origin, (float)tex.rotation, WHITE);
			}
		}
	} else {
		DrawRectangle(rect.left, rect.top, rect.width, rect.height, WHITE);
	}
	EndBlendMode();
}

void UpdateList::startEngine() {
	std::cout << "Update thread starting\n";

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

	std::cout << "Update thread ending\n";
}

/*void event(const sapp_event* event) {
	switch(event->type) {
	case SAPP_EVENTTYPE_KEY_DOWN: case SAPP_EVENTTYPE_KEY_UP:
		//Keyboard
		event_queue.emplace_front(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_KEY_DOWN,
			event->key_code, event->frame_count, 0);
		event_count++;
		break;
	case SAPP_EVENTTYPE_MOUSE_DOWN: case SAPP_EVENTTYPE_MOUSE_UP:
		//Mouse button
		event_queue.emplace_front(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_MOUSE_DOWN,
			event->mouse_button + MOUSE_OFFSET);
		event_queue.emplace_front(EVENT_MOUSE, event->type == SAPP_EVENTTYPE_MOUSE_DOWN,
			event->mouse_button + MOUSE_OFFSET, event->mouse_x, event->mouse_y);
		event_count += 2;
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		//Mouse movement
		event_queue.emplace_front(EVENT_MOUSE, event->modifiers & 0x100,
			-1, event->mouse_x, event->mouse_y);
		event_count++;
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		//Mouse Scrolling
		event_queue.emplace_front(EVENT_KEYPRESS, event->scroll_y > 0,
			MOUSE_OFFSET+5);
		event_queue.emplace_front(EVENT_KEYPRESS, event->scroll_y < 0,
			MOUSE_OFFSET+6);
		event_queue.emplace_front(EVENT_SCROLL, event->scroll_y < 0,
			-1, event->scroll_x, event->scroll_y);
		event_count += 3;
		break;
	case SAPP_EVENTTYPE_TOUCHES_BEGAN: case SAPP_EVENTTYPE_TOUCHES_ENDED:
		//Touch
		event_queue.emplace_front(EVENT_KEYPRESS, event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN,
			MOUSE_OFFSET+7);
		event_count++;
		for(int i = 0; i < event->num_touches; i++) {
			event_queue.emplace_front(EVENT_TOUCH, event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN,
				MOUSE_OFFSET+7, event->touches[i].pos_x, event->touches[i].pos_y);
			event_count++;
		}
		break;
	case SAPP_EVENTTYPE_TOUCHES_MOVED:
		//Touch movement
		for(int i = 0; i < event->num_touches; i++) {
			event_queue.emplace_front(EVENT_TOUCH, true,
				-1, event->touches[i].pos_x, event->touches[i].pos_y);
			event_count++;
		}
		break;
	case SAPP_EVENTTYPE_RESIZED:
		event_queue.emplace_front(EVENT_RESIZE, false,
			event->framebuffer_width/event->window_width, event->window_width, event->window_height);
		event_count++;
		break;
	case SAPP_EVENTTYPE_UNFOCUSED: case SAPP_EVENTTYPE_FOCUSED:
		event_queue.emplace_front(EVENT_FOCUS, event->type == SAPP_EVENTTYPE_UNFOCUSED, 0);
		event_count++;
		break;
	case SAPP_EVENTTYPE_ICONIFIED: case SAPP_EVENTTYPE_SUSPENDED: case SAPP_EVENTTYPE_QUIT_REQUESTED:
		event_queue.emplace_front(EVENT_SUSPEND, true, 0);
		event_count++;
		break;
	case SAPP_EVENTTYPE_RESTORED: case SAPP_EVENTTYPE_RESUMED:
		event_queue.emplace_front(EVENT_SUSPEND, false, 0);
		event_count++;
		break;
	default:
		break;
	}

	simgui_handle_event(event);
	sapp_consume_event();
}*/

void UpdateList::frame(void) {
    DebugTimers::frameTimes.addDelta(GetFrameTime());
    uint64_t lastTime = GetTime();

	// Get current window size.
    int width = GetRenderWidth();
    int height = GetRenderHeight();
    BeginDrawing();

    //Start imgui frame
    rlImGuiBegin();

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
        gameImguiMenu();
        ImGui::EndMainMenuBar();
    }
    skyrmionImgui();
    gameImgui();
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
	for(std::string file : textureFiles())
		UpdateList::loadTexture(file);

    //Start update thread and initialize
	updates = std::thread(initialize);

	while(!UpdateList::running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

    std::cout << "Starting Rendering\n";
    while(!WindowShouldClose() && UpdateList::running) {
		frame();
	}

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
