#include "UpdateList.h"
#include "Event.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE
#define SOKOL_TRACE_HOOKS
#include "../include/sokol/sokol_gfx.h"//
#include "../include/sokol_gp/sokol_gp.h"//
#include "../include/sokol/sokol_app.h"//
#include "../include/sokol/sokol_glue.h"//
#include "../include/sokol/sokol_time.h"//
#include "../include/sokol/sokol_log.h"//

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "../include/sokol_gp/thirdparty/stb_image.h"//

#define SOKOL_IMGUI_IMPL
#include "../include/imgui/imgui.h"//
#include "../include/sokol/util/sokol_imgui.h"//
#include "../include/sokol/util/sokol_gfx_imgui.h"//

/*
 * Manages layers of nodes through update cycle
 */

//Static variables
Node *UpdateList::screen[MAXLAYER];
std::bitset<MAXLAYER> UpdateList::staticLayers;
std::bitset<MAXLAYER> UpdateList::pausedLayers;
std::bitset<MAXLAYER> UpdateList::screenLayers;
std::vector<Node *> UpdateList::deleted;

Node *UpdateList::camera = NULL;
WindowSize UpdateList::windowSize;
std::bitset<MAXLAYER> UpdateList::hiddenLayers;
std::vector<Node *> UpdateList::reloadBuffer;

Layer UpdateList::max = 0;
bool UpdateList::running = false;

//Textures
std::vector<sg_image> textureSet;
std::vector<sf::Vector2i> textureSizes;

//Event handling
std::atomic_int event_count = 0;
std::deque<Event> event_queue;
std::array<std::vector<Node *>, EVENT_MAX> listeners;

std::thread updates;
sgimgui_t sgimgui;

//Add node to update cycle
void UpdateList::addNode(Node *next) {
	Layer layer = next->getLayer();
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(layer > max)
		max = layer;
	if(screen[layer] == NULL)
		screen[layer] = next;
	else
		screen[layer]->addNode(next);
}

void UpdateList::addNodes(std::vector<Node *> nodes) {
	for(Node *node : nodes)
		addNode(node);
}

//Get node in specific layer
Node *UpdateList::getNode(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return screen[layer];
}

//Remove all nodes in layer
void UpdateList::clearLayer(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);

	Node *source = screen[layer];
	while(source != NULL) {
		source->setDelete();
		source = source->getNext();;
	}
}

//Subscribe node to cetain event type
void UpdateList::addListener(Node *item, int type) {
	listeners[type].push_back(item);
}

//Set camera to follow node
Node *UpdateList::setCamera(Node *follow, sf::Vector2f size, sf::Vector2f position) {
	if(camera != NULL) {
		camera->setSize(sf::Vector2i(size.x,size.y));
		camera->setParent(follow);
	} else
		camera = new Node(0, sf::Vector2i(size.x,size.y), true, follow);
	//viewPlayer.setSize(size);
	camera->setPosition(position);
	return camera;
}

//Send signal message to all nodes in layer
void UpdateList::sendSignal(Layer layer, int id, Node *sender) {
	Node *source = screen[layer];
	while(source != NULL) {
		source->recieveSignal(id, sender);
		source = source->getNext();
	}
}

//Send signal message to all nodes in game
void UpdateList::sendSignal(int id, Node *sender) {
	for(int layer = 0; layer <= max; layer++)
		sendSignal(layer, id, sender);
}

//Schedule reload call before next draw
void UpdateList::scheduleReload(Node *buffer) {
	if(buffer != NULL)
		reloadBuffer.push_back(buffer);
}

//Update nodes event when not in camera
void UpdateList::staticLayer(Layer layer, bool _static) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	staticLayers[layer] = _static;
}

//Do not update nodes
void UpdateList::pauseLayer(Layer layer, bool pause) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	pausedLayers[layer] = pause;
}

//Nodes placed relative to camera
void UpdateList::screenSpaceLayer(Layer layer, bool screen) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	screenLayers[layer] = screen;
}

//Do not render nodes
void UpdateList::hideLayer(Layer layer, bool hidden) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	hiddenLayers[layer] = hidden;
}

//Check if layer is marked static
bool UpdateList::isLayerStatic(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return staticLayers[layer];
}

//Check if layer is paused
bool UpdateList::isLayerPaused(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return pausedLayers[layer];
}

//Check if layer is marked screen space
bool UpdateList::isLayerScreenSpace(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return screenLayers[layer];
}

//Check if layer is marked hidden
bool UpdateList::isLayerHidden(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return hiddenLayers[layer];
}

//Get number of occupied layers
int UpdateList::getLayerCount() {
	return max + 1;
}

static sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if (!data) {
    	textureSet.push_back(img);
    	textureSizes.push_back(sf::Vector2i(0,0));
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
    textureSizes.push_back(sf::Vector2i(width, height));
    return img;
}

//Load texture from file and add to set
int UpdateList::loadTexture(std::string filename) {
	if(filename.length() > 0 && filename[0] != '#')
		load_image(filename.c_str());
	else {
		textureSet.emplace_back();
		textureSizes.push_back(sf::Vector2i(0,0));
	}
	return textureSet.size() - 1;
}

//Get size of texture
sf::Vector2i UpdateList::getTextureSize(int index) {
	return textureSizes[index];
}

unsigned long long UpdateList::getImguiTexture(int index) {
	return simgui_imtextureid(textureSet[index]);
}

//Pick color from texture
Color UpdateList::pickColor(int texture, sf::Vector2i position) {
	sint index = (position.x+position.y*textureSizes[texture].x)*4;
	int width, height, channels;
	uint8_t* data = stbi_load(textureFiles()[texture].c_str(), &width, &height, &channels, 4);
	Color color(data + index);
	stbi_image_free(data);
	return color;
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
	for(int layer = 0; layer <= max; layer++) {
		Node *source = screen[layer];

		if(!pausedLayers[layer]) {
			//Check first node for deletion
			if(source != NULL && source->isDeleted()) {
				deleted.push_back(source);
				source = source->getNext();
				screen[layer] = source;
			}

			//For each node in layer order
			while(source != NULL) {
				if(staticLayers[layer] || camera == NULL || source->getRect().intersects(camera->getRect())) {
					//Check each selected collision layer
					int collisionLayer = 0;
					for(int i = 0; i < (int)source->getCollisionLayers().count(); i++) {
						while(!source->getCollisionLayer(collisionLayer))
							collisionLayer++;

						//Check collision box of each node
						Node *other = screen[collisionLayer];
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

				//Check next node for deletion
				while(source->getNext() != NULL && source->getNext()->isDeleted()) {
					deleted.push_back(source->getNext());
					source->deleteNext();
				}

				source = source->getNext();
			}
		}
	}
}

//Thread safe draw nodes in list
void UpdateList::draw(sf::Vector2f offset, sf::Vector2i size) {
	//Find camera position
	sf::FloatRect cameraRect = sf::FloatRect(0,0,size.x,size.y);
	if(camera != NULL)
		cameraRect = camera->getRect();
	cameraRect = sf::FloatRect(cameraRect.left + offset.x, cameraRect.top + offset.y,
		cameraRect.width, cameraRect.height);

	// Begin recording draw commands for a frame buffer of size (width, height).
    sgp_begin(size.x, size.y);
    // Set frame buffer drawing region to (0,0,width,height).
    //sgp_viewport(cameraRect.left, cameraRect.top, cameraRect.width, cameraRect.height);
    sgp_viewport(0,0,size.x,size.y);

    // Clear the frame buffer.
    Color background = backgroundColor();
    sgp_set_color(background.red, background.green, background.blue, background.alpha);
    sgp_clear();

	//Render each node in order
	for(int layer = 0; layer <= max; layer++) {
		Node *source = screen[layer];

		if(screenLayers[layer])
			sgp_project(0, 0+size.x, 0, 0+size.y);
		else
			sgp_project(cameraRect.left, cameraRect.left+cameraRect.width, cameraRect.top, cameraRect.top+cameraRect.height);

		if(!hiddenLayers[layer])
			while(source != NULL) {
				if(!source->isHidden() &&
					(staticLayers[layer] || source->getRect().intersects(cameraRect))) {

					drawNode(source);
				}
				source = source->getNext();
			}
	}
}

void UpdateList::drawNode(Node *source) {
	sgp_reset_color();
	sgp_set_blend_mode((sgp_blend_mode)source->getBlendMode());

	sf::FloatRect rect = source->getDrawRect();
	std::vector<TextureRect> *textureRects = source->getTextureRects();

	if(source->getTexture() != -1 && textureRects->size() == 0) {
		//Default square texture
		sgp_set_image(0, textureSet[source->getTexture()]);
		sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
		sgp_reset_image(0);
	} else if(source->getTexture() != -1) {
		//Tilemapped or partial texture
		sgp_set_image(0, textureSet[source->getTexture()]);
		//sgp_textured_rect outRects[1];
		//int renderCount = 0;
		for(int i = 0; i < textureRects->size(); i++) {
			TextureRect tex = (*textureRects)[i];
			sf::Vector2f scale = source->getScale();
			if(tex.width != 0 && tex.height != 0) {
				sgp_rect dst = {tex.px*scale.x + rect.left, tex.py*scale.y + rect.top, abs(tex.width)*scale.x, abs(tex.height)*scale.y};
				sgp_rect src = {(float)tex.tx, (float)tex.ty, (float)tex.width, (float)tex.height};
				//renderCount++;
				if(tex.rotation != 0) {
					sgp_push_transform();
					sgp_rotate_at(tex.rotation, dst.x + dst.w/2.0, dst.y + dst.h/2.0);
					sgp_draw_textured_rect(0, dst, src);
					sgp_pop_transform();
				} else {
					sgp_draw_textured_rect(0, dst, src);
				}
			}
		}
		//sgp_draw_textured_rects(0, outRects, renderCount);
		sgp_reset_image(0);
	} else {
		sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
	}
}

void UpdateList::updateThread() {
	std::cout << "Initializing game\n";
    initialize();
}

void UpdateList::startEngine() {
	std::cout << "Update thread starting\n";

    #ifdef _DEBUG
	setupDebugTools();
	#endif

	//Initial node update
	for(int layer = 0; layer <= max; layer++) {
		Node *source = screen[layer];

		while(source != NULL) {
			source->update(-1);
			source = source->getNext();
		}
	}
	UpdateList::running = true;

	stm_setup();
	uint64_t lastTime = stm_now();
	while(UpdateList::running) {
		UpdateList::processEvents();

		//Update nodes and sprites
		double delta = stm_sec(stm_laptime(&lastTime));
		UpdateList::update(delta);

		std::this_thread::sleep_for(
			std::chrono::milliseconds(10-(int)stm_ms(stm_since(lastTime))));
	}

	std::cout << "Update thread ending\n";
}

#if __linux__
	#include <X11/Xlib.h>
	#define initThreads XInitThreads
#else
	#define initThreads void
#endif

void event(const sapp_event* event) {
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
}

void UpdateList::frame(void) {
    //update(sapp_frame_duration());

	// Get current window size.
    int width = sapp_width(), height = sapp_height();

    //Start imgui frame
    simgui_frame_desc_t simguidesc = { };
    simguidesc.width = width;
    simguidesc.height = height;
    simguidesc.delta_time = sapp_frame_duration();
    simguidesc.dpi_scale = sapp_dpi_scale();
    simgui_new_frame(&simguidesc);

    //Main draw functions
    draw(sf::Vector2f(0,0), sf::Vector2i(width, height));

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
        gameImguiMenu();
        ImGui::EndMainMenuBar();
    }
    skyrmionImgui();
    gameImgui();
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

    //Loop through list to delete nodes
	std::vector<Node *>::iterator dit = deleted.begin();
	while(dit != deleted.end()) {
		Node *node = *dit;
		dit++;
		delete node;
	}
	deleted.clear();
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

    #ifdef _DEBUG
    addDebugTextures();
    #endif

    //Load textures
	for(std::string file : textureFiles())
		UpdateList::loadTexture(file);

    //Start update thread and initialize
    initThreads();
	updates = std::thread(UpdateList::updateThread);

	while(!UpdateList::running) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

    std::string title = windowTitle();

   	sapp_desc desc = {0};
   	desc.init_cb = UpdateList::init;
   	desc.frame_cb = UpdateList::frame;
   	desc.event_cb = event;
   	desc.cleanup_cb = UpdateList::cleanup;
   	desc.window_title = title.c_str();
   	desc.logger.func = slog_func;
    return desc;
}
