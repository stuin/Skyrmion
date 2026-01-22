//Add node to update/draw cycle
void UpdateList::addNode(Node *next) {
	int layer = next->getLayer();
	if(layer < 0)
		throw new std::invalid_argument(DRAWLAYERERROR);
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
Node *UpdateList::getNode(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].root;
}

//Remove all nodes in layer
void UpdateList::clearLayer(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);

	UNode *source = layers[layer].root;
	while(source != NULL) {
		source->setDelete();
		source = source->getNext();;
	}
}

//Add UNode to update cycle
void UpdateList::addUNode(UNode *next) {
	int layer = next->getLayer();
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(layer > maxULayer)
		maxULayer = layer;
	if(-layer > maxULayer)
		maxULayer = -layer;
	if(uLayers[layer+MAXLAYER] == NULL)
		uLayers[layer+MAXLAYER] = next;
	else
		uLayers[layer+MAXLAYER]->addNode(next);
}

//Get UNode in specific layer
UNode *UpdateList::getUNode(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return uLayers[layer+MAXLAYER];
}

//Send signal message to all nodes in layer
void UpdateList::sendSignal(int layer, int id, UNode *sender) {
	UNode *source = layers[layer].root;
	while(source != NULL) {
		source->recieveSignal(id, sender);
		source = source->getNext();
	}
}

//Send signal message to all nodes in game
void UpdateList::sendSignal(int id, UNode *sender) {
	for(int layer = 0; layer <= maxLayer; layer++)
		sendSignal(layer, id, sender);
}

//Set camera to follow node
Node *UpdateList::setCamera(Node *follow, Vector2f size, Vector2f position) {
	if(camera != NULL) {
		camera->setSize(size);
		camera->setParent(follow);
	} else
		camera = new Node(0, size, true, follow);
	cameraRect = FloatRect(position, size);
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
	return cameraRect.size() / screenRect.size();
}

//Do not update nodes
void UpdateList::pauseLayer(int layer, bool pause) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	layers[layer].paused = pause;
}

//Do not render nodes
void UpdateList::hideLayer(int layer, bool hidden) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	layers[layer].hidden = hidden;
}

//Update nodes outside of camera bounds
void UpdateList::globalLayer(int layer, bool global) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	layers[layer].global = global;
}

//Check if layer is paused
bool UpdateList::isLayerPaused(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].paused;
}

//Check if layer is marked hidden
bool UpdateList::isLayerHidden(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].hidden;
}

//Get all data for layer
LayerData &UpdateList::getLayerData(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer];
}

//Get number of occupied layers
int UpdateList::getLayerCount() {
	return maxLayer + 1;
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
			for(UNode *node : listeners[event.type % EVENT_MAX])
				node->recieveEvent(event);
			event_previous[event.type % EVENT_MAX] = event;
		}
	}
}

//Update all nodes in list
void UpdateList::update(double time) {
	UpdateList::processEvents();
	UpdateList::processAudio();

	deleted2.insert(deleted2.end(), deleted1.begin(), deleted1.end());
	deleted1.clear();

	//Pre update UNodes
	for(int layer = -maxULayer; layer < 0; layer++) {
		UNode *uSource = uLayers[layer+MAXLAYER];
		if(uSource != NULL && uSource->isDeleted()) {
			deleted1.push_back(uSource);
			uSource = uSource->getNext();
			uLayers[layer+MAXLAYER] = uSource;
		}

		//For each node in layer order
		while(uSource != NULL) {
			//Update each node
			uSource->update(time);

			//Check next node for removing from list
			while(uSource->getNext() != NULL && uSource->getNext()->isDeleted()) {
				deleted1.push_back((Node*)uSource->getNext());
				uSource->deleteNext();
			}

			uSource = (Node*)uSource->getNext();
		}
	}

	//Check collisions and updates
	for(int layer = 0; layer <= maxLayer; layer++) {
		Node *source = layers[layer].root;

		if(!layers[layer].paused) {
			//Check first node for deletion
			if(source != NULL && source->isDeleted()) {
				deleted1.push_back(source);
				source = (Node*)source->getNext();
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
							other = (Node*)other->getNext();
						}
						collisionLayer++;
					}

					//Update each object
					source->update(time);
				}

				//Check next node for removing from list
				while(source->getNext() != NULL && source->getNext()->isDeleted()) {
					deleted1.push_back((Node*)source->getNext());
					source->deleteNext();
					layers[source->getLayer()].count--;
				}

				source = (Node*)source->getNext();
			}
		}
	}

	//Post update UNodes
	for(int layer = 0; layer <= maxULayer; layer++) {
		UNode *uSource = uLayers[layer+MAXLAYER];
		if(uSource != NULL && uSource->isDeleted()) {
			deleted1.push_back(uSource);
			uSource = uSource->getNext();
			uLayers[layer+MAXLAYER] = uSource;
		}

		//For each node in layer order
		while(uSource != NULL) {
			//Update each node
			uSource->update(time);

			//Check next node for removing from list
			while(uSource->getNext() != NULL && uSource->getNext()->isDeleted()) {
				deleted1.push_back((Node*)uSource->getNext());
				uSource->deleteNext();
			}

			uSource = (Node*)uSource->getNext();
		}
	}
}