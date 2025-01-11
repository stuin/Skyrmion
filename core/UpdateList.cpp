#include "UpdateList.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE
#include "../sokol_gp/thirdparty/sokol_gfx.h"
#include "../sokol_gp/sokol_gp.h"
#include "../sokol_gp/thirdparty/sokol_app.h"
#include "../sokol_gp/thirdparty/sokol_glue.h"
#include "../sokol_gp/thirdparty/sokol_log.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_SIMD
#define STBI_ONLY_PNG
#include "../sokol_gp/thirdparty/stb_image.h"

#define FRAME_DELAY sf::milliseconds(16.6)

/*
 * Manages layers of nodes through update cycle
 */

//Static variables
Node *UpdateList::screen[MAXLAYER];
std::bitset<MAXLAYER> UpdateList::staticLayers;
std::bitset<MAXLAYER> UpdateList::pausedLayers;
std::vector<Node *> UpdateList::deleted;

std::atomic_int UpdateList::event_count = 0;
std::deque<sapp_event*> event_queue;
std::unordered_map<int, std::vector<Node *>> listeners;

Node *UpdateList::camera = NULL;
WindowSize UpdateList::windowSize;
std::bitset<MAXLAYER> UpdateList::hiddenLayers;
std::vector<Node *> UpdateList::reloadBuffer;
std::vector<sg_image> textureSet;

Layer UpdateList::max = 0;
bool UpdateList::running = true;

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
	auto it = listeners.find(type);
	if(it == listeners.end()) {
		std::vector<Node *> vec = {item};
		listeners.emplace(type, vec);
	} else
		it->second.push_back(item);
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

//Check if layer is marked hidden
bool UpdateList::isLayerHidden(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return hiddenLayers[layer];
}

//Get highest occupied layer
int UpdateList::getMaxLayer() {
	return max;
}

static sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if (!data) {
        return img;
    }
    sg_image_desc image_desc = {0};
    image_desc.width = width;
    image_desc.height = height;
    image_desc.data.subimage[0][0].ptr = data;
    image_desc.data.subimage[0][0].size = (size_t)(width * height * 4);
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    return img;
}

//Load texture from file and add to set
int UpdateList::loadTexture(std::string filename) {
	textureSet.push_back(load_image(filename.c_str()));
	return textureSet.size() - 1;
}

//Get texture from set
/*sf::Texture *UpdateList::getTexture(sint index) {
	if(index > textureSet.size())
		throw std::invalid_argument("Texture " + std::to_string(index) + " not found");
	return textureSet[index];
}*/

//Process window events on update thread
void UpdateList::processEvents() {
	/*int count = event_count;
	event_count -= count;
	WindowSize size = windowSize;
	for(int i = 0; i < count; i++) {
		//Send event to marked listeners
		sf::Event event = event_queue.back();
		event_queue.pop_back();
		auto it = listeners.find(event.type);
		if(it != listeners.end())
			for(Node *node : it->second)
				node->recieveEvent(event, &size);
	}*/
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
void UpdateList::draw(sf::Vector2f offset) {
	//Find camera position
	sf::FloatRect cameraRect = sf::FloatRect(-100000,-100000,200000,200000);
	if(camera != NULL)
		cameraRect = camera->getRect();
	cameraRect = sf::FloatRect(cameraRect.left + offset.x, cameraRect.top + offset.y,
		cameraRect.width, cameraRect.height);

	//Render each node in order
	for(int layer = 0; layer <= max; layer++) {
		Node *source = screen[layer];

		if(!hiddenLayers[layer])
			while(source != NULL) {
				if(!source->isHidden() &&
					(staticLayers[layer] || source->getRect().intersects(cameraRect))) {

					sgp_reset_color();
					if(source->getTexture() != -1) {
						sgp_set_image(0, textureSet[source->getTexture()]);
						sf::FloatRect rect = source->getDrawRect();
						sgp_draw_filled_rect(rect.left, rect.top, rect.width, rect.height);
						sgp_reset_image(0);
					}
				}
				source = source->getNext();
			}
	}
}

//Seperate rendering thread
/*void UpdateList::renderingThread(std::string title) {
	//Set frame rate manager
	sf::Clock clock;
	sf::Time nextFrame = clock.getElapsedTime();

	std::cout << "Thread starting\n";
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), title);

    //Run rendering loop
	while(window.isOpen()) {
		//Calculate window size and scaling
		float shiftX = viewPlayer.getSize().x / window.getSize().x;
		float shiftY = viewPlayer.getSize().y / window.getSize().y;
		int cornerX = viewPlayer.getCenter().x - viewPlayer.getSize().x/2;
		int cornerY = viewPlayer.getCenter().y - viewPlayer.getSize().y/2;
		windowSize = {shiftX, shiftY, cornerX, cornerY};

		//Check event updates
		sf::Event event;
		while(window.pollEvent(event)) {
			if(event.type == sf::Event::Closed)
				window.close();
			else if(listeners.find(event.type) != listeners.end()) {
				event_queue.push_front(event);
				event_count++;
			}

			//Adjust window size
			if(event.type == sf::Event::Resized) {
				if(camera == NULL) {
					sf::Vector2f size(event.size.width, event.size.height);
					viewPlayer.setSize(size);
				}
				std::cout << "Window Size: " << cornerX << "x" << cornerY << "\n";
			}
		}
		if(!UpdateList::running)
			window.close();

		sf::Time time = clock.getElapsedTime();
		if(time >= nextFrame) {
			//Next update time
			nextFrame = time + FRAME_DELAY;

			//Loop through list to reload any buffers
			std::vector<Node *>::iterator rit = reloadBuffer.begin();
			while(rit != reloadBuffer.end()) {
				(*rit)->reloadBuffer();
				rit++;
			}
			reloadBuffer.clear();

			//Loop through list to delete nodes
			std::vector<Node *>::iterator dit = deleted.begin();
			while(dit != deleted.end()) {
				Node *node = *dit;
				dit++;
				delete node;
			}
			deleted.clear();

			//Set camera position
			if(camera != NULL) {
				viewPlayer.setCenter(camera->getGPosition());
				window.setView(viewPlayer);
			}

			//Update window
			window.clear();
			UpdateList::draw(window);
			window.display();
		}
		time = clock.getElapsedTime();
		std::this_thread::sleep_for(
			std::chrono::microseconds((nextFrame - time).asMicroseconds()));
	}

	std::cout << "Thread ending\n";

	UpdateList::running = false;
}*/

#if __linux__
	#include <X11/Xlib.h>
	#define initThreads XInitThreads
#else
	#define initThreads void
#endif

/*void UpdateList::startEngine(std::string title) {
	init();

	//Set frame rate manager
	sf::Clock clock;

	std::thread rendering(UpdateList::renderingThread, title);
	sf::Time nextFrame = clock.getElapsedTime() + sf::milliseconds(FRAME_DELAY.asMilliseconds() / 2);

	std::cout << "Starting\n";

	//Initial update
	for(Node *layer : screen) {
		Node *source = layer;

		while(source != NULL) {
			source->update(0);
			source = source->getNext();
		}
	}

    //Run main window
    sf::Time lastTime = clock.getElapsedTime();
	while (UpdateList::running) {
		//Manage frame rate
		sf::Time time = clock.getElapsedTime();
		if(time >= nextFrame) {
			nextFrame = time + FRAME_DELAY;

			UpdateList::processEvents();

			//Calculate update time
			double delta = (time - lastTime).asSeconds();
			lastTime = time;

			//Update nodes and sprites
			UpdateList::update(delta);
		}
		time = clock.getElapsedTime();
		std::this_thread::sleep_for(
			std::chrono::microseconds((nextFrame - time).asMicroseconds()));
	}

	rendering.join();
}*/

static void frame(void) {
    UpdateList::update(sapp_frame_duration());

	// Get current window size.
    int width = sapp_width(), height = sapp_height();

    // Begin recording draw commands for a frame buffer of size (width, height).
    sgp_begin(width, height);
    // Set frame buffer drawing region to (0,0,width,height).
    sgp_viewport(0, 0, width, height);

    // Clear the frame buffer.
    sgp_set_color(0.1f, 0.1f, 0.1f, 1.0f);
    sgp_clear();

    UpdateList::draw(sf::Vector2f(0,0));

    // Begin a render pass.
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);
    // Dispatch all draw commands to Sokol GFX.
    sgp_flush();
    // Finish a draw command queue, clearing it.
    sgp_end();
    // End render pass.
    sg_end_pass();
    // Commit Sokol render.
    sg_commit();
}

static void init(void) {
	// initialize Sokol GFX
    sg_desc sgdesc = {0};
    sgdesc.environment = sglue_environment();
    sgdesc.logger.func = slog_func;
    sg_setup(&sgdesc);
    if(!sg_isvalid()) {
        fprintf(stderr, "Failed to create Sokol GFX context!\n");
        exit(-1);
    }

    // initialize Sokol GP
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);
    if(!sgp_is_valid()) {
        fprintf(stderr, "Failed to create Sokol GP context: %s\n", sgp_get_error_message(sgp_get_last_error()));
        exit(-1);
    }

    initThreads();

    initialize();

    std::cout << "Starting\n";

    //Initial node update
	/*for(int i = 0; i < UpdateList::max; i++) {
		Node *source = UpdateList::getNode(i);

		while(source != NULL) {
			source->update(0);
			source = source->getNext();
		}
	}*/

	/*for(sg_image &image : textureSet) {
    	if(sg_query_image_state(image)) {
    		fprintf(stderr, "failed to load images");
        	exit(-1);
    	}
    }*/
}

static void cleanup(void) {
    sgp_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
   	sapp_desc desc = {0};
   	desc.init_cb = init;
   	desc.frame_cb = frame;
   	desc.cleanup_cb = cleanup;
   	desc.window_title = "Sokol";
   	desc.logger.func = slog_func;
    return desc;
}

void UpdateList::stopEngine() {
	running = false;
}

bool UpdateList::isRunning() {
	return running;
}
