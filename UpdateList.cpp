#include "UpdateList.h"

#define FRAME_DELAY sf::milliseconds(15)

/*
 * Created by Stuart Irwin on 4/15/2019.
 * Manages layers of nodes through update cycle
 */

//Static variables
Node *(UpdateList::screen)[MAXLAYER];
std::bitset<MAXLAYER> UpdateList::alwaysLoadedLayers;
std::vector<Node *> UpdateList::deleted;
std::unordered_map<sf::Event::EventType, std::vector<Node *>> UpdateList::listeners;

Node *UpdateList::camera = NULL;
sf::View UpdateList::viewPlayer;
std::bitset<MAXLAYER> UpdateList::hiddenLayers;

Layer UpdateList::max = MAXLAYER;
bool UpdateList::running = true;

//Add node to update cycle
void UpdateList::addNode(Node *next) {
	Layer layer = next->getLayer();
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(screen[layer] == NULL)
		screen[layer] = next;
	else
		screen[layer]->addNode(next);
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

//Add node to list connected to cetain event
void UpdateList::addListener(Node *item, sf::Event::EventType type) {
	auto it = listeners.find(type);
	if(it == listeners.end()) {
		std::vector<Node *> vec = {item};
		listeners.emplace(type, vec);
	} else
		it->second.push_back(item);
}

//Set camera to follow node
Node *UpdateList::setCamera(Node *follow, sf::Vector2f size) {
	if(camera != NULL) {
		camera->setSize(sf::Vector2i(size.x,size.y));
		camera->setParent(follow);
	} else
		camera = new Node(0, sf::Vector2i(size.x,size.y), true, follow);
	viewPlayer.setSize(size);
	return camera;
}

void UpdateList::alwaysLoadLayer(Layer layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	alwaysLoadedLayers[layer] = true;
}

void UpdateList::hideLayer(Layer layer, bool hidden) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	hiddenLayers[layer] = hidden;
}

//Update all nodes in list
void UpdateList::update(double time) {
	//Check collisions and updates
	for(int layer = 0; layer <= max; layer++) {
		Node *source = screen[layer];

		//Check first node for deletion
		if(source != NULL && source->isDeleted()) {
			deleted.push_back(source);
			source = source->getNext();
			screen[layer] = source;
		}

		//For each node in layer order
		while(source != NULL) {
			if(alwaysLoadedLayers[layer] || camera == NULL || source->checkCollision(camera)) {
				//Check each selected collision layer
				int collisionLayer = 0;
				for(int i = 0; i < (int)source->getCollisionLayers().count(); i++) {
					while(!source->getCollisionLayer(collisionLayer))
						collisionLayer++;

					//Check collision box of each node
					Node *other = screen[collisionLayer];
					while(other != NULL) {
						if(other != source && source->checkCollision(other))
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

//Thread safe draw nodes in list
void UpdateList::draw(sf::RenderWindow &window) {
	//Loop through list to delete
	std::vector<Node *>::iterator it = deleted.begin();
	while(it != deleted.end()) {
		Node *node = *it;
		it++;
		delete node;
	}
	deleted.clear();

	//Render each node in order
	for(int layer = 0; layer <= max; layer++) {
		Node *source = screen[layer];

		if(!hiddenLayers[layer])
			while(source != NULL) {
				if(!source->isHidden() &&
					(alwaysLoadedLayers[layer] || camera == NULL || source->checkCollision(camera))) {
					//Check for parent node
					if(source->getParent() != NULL) {
						sf::Transform translation;
						translation.translate(source->getParent()->getGPosition());
						window.draw(*source, translation);
					} else
						window.draw(*source);
				}
				source = source->getNext();
			}
	}
}

//Seperate rendering thread
void UpdateList::renderingThread(std::string title) {
	//Set frame rate manager
	sf::Clock clock;
	sf::Time nextFrame = clock.getElapsedTime();

	std::cout << "Thread starting\n";
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), title);
	window.setVerticalSyncEnabled(true);

    //Run rendering loop
	while(window.isOpen()) {
		//Check event updates
		sf::Event event;
		//Calculate window sizing
		int shiftX = viewPlayer.getSize().x / window.getSize().x;
		int shiftY = viewPlayer.getSize().y / window.getSize().y;
		while(window.pollEvent(event)) {
			if(event.type == sf::Event::Closed)
				window.close();
			else {
				//Send event to marked listeners
				auto it = listeners.find(event.type);
				if(it != listeners.end())
					for(Node *node : it->second)
						if(!node->isHidden())
							node->recieveEvent(event, shiftX, shiftY);
			}

			//Adjust window size
			if(event.type == sf::Event::Resized && camera == NULL) {
				sf::Vector2f size(event.size.width, event.size.height);
				viewPlayer.setSize(size);
			}
		}
		if(!UpdateList::running)
			window.close();

		sf::Time time = clock.getElapsedTime();
		if(time >= nextFrame) {
			//Next update time
			nextFrame = time + FRAME_DELAY;

			//Update window
			window.clear();
			UpdateList::draw(window);

			//Set camera position
			if(camera != NULL) {
				viewPlayer.setCenter(camera->getGPosition());
				window.setView(viewPlayer);
			}

			window.display();
		}
		time = clock.getElapsedTime();
		std::this_thread::sleep_for(
			std::chrono::microseconds((nextFrame - time).asMicroseconds()));
	}

	std::cout << "Thread ending\n";

	UpdateList::running = false;
}

#if __linux__
	#include <X11/Xlib.h>
	#define init XInitThreads
#else
	#define init void
#endif

void UpdateList::startEngine(std::string title, Layer max) {
	init();

	//Set frame rate manager
	sf::Clock clock;
	UpdateList::max = max;

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
			//Next update time
			double delta = (time - lastTime).asSeconds();
			lastTime = time;
			nextFrame = time + FRAME_DELAY;

			//Update nodes and sprites
			UpdateList::update(delta);
		}
		time = clock.getElapsedTime();
		std::this_thread::sleep_for(
			std::chrono::microseconds((nextFrame - time).asMicroseconds()));
	}

	rendering.join();
}

void UpdateList::stopEngine() {
	running = false;
}
