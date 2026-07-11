#define DTOR 0.0174532925199
#define RTOD 57.2957795131

#define TEXTUREERROR "Texture does not exist"
#define BUFFERERROR "Cannot replace texture with render buffer"
#define FILEERROR "Failed to read file"
#define UNKNOWNRESOURCE "_UNKNOWN_RESOURCE"
#define UNKNOWNSPACE "_EMPTY_SPACE"

//Add node to update/draw cycle
void UpdateList::addNode(Node *next) {
	int layer = next->getLayer();
	if(layer < 0 || next == NULL)
		throw new std::invalid_argument(DRAWLAYERERROR);
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(layer > maxLayer)
		maxLayer = layer;
	if(layers[layer].root == NULL)
		layers[layer].root = next;
	else
		layers[layer].root->addNode(next);

	//Set node ID
	layers[layer].count++;
	next->setId(layers[layer].count);
}

void UpdateList::addNodes(std::vector<Node *> nodes) {
	for(Node *node : nodes)
		addNode(node);
}

//Get node in specific layer
Node *UpdateList::getNode(int layer, sint id) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(id > 0)
		return (Node*)layers[layer].root->getNext(id);
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
	if(layer < 0 || next == NULL)
		throw new std::invalid_argument(DRAWLAYERERROR);
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	if(layer > maxULayer)
		maxULayer = layer;
	if(layers[layer].uRoot == NULL)
		layers[layer].uRoot = next;
	else
		layers[layer].uRoot->addNode(next);

	//Set node ID
	layers[layer].uCount++;
	next->setId(layers[layer].uCount);
}

//Get UNode in specific layer
UNode *UpdateList::getUNode(int layer) {
	if(layer >= MAXLAYER)
		throw new std::invalid_argument(LAYERERROR);
	return layers[layer].uRoot;
}

//Send signal message to all nodes in layer
void UpdateList::sendSignal(int layer, int id, Node *sender) {
	Node *source = layers[layer].root;
	while(source != NULL) {
		source->recieveSignal(id, sender);
		source = (Node*)source->getNext();
	}
}

//Send signal message to all nodes in game
void UpdateList::sendSignal(int id, Node *sender) {
	for(int layer = 0; layer <= maxLayer; layer++)
		sendSignal(layer, id, sender);
}

//Set camera to follow node
Node *UpdateList::setCamera(Node *follow, Vector2f size, Vector2f position) {
	if(camera != NULL) {
		camera->setSize(size);
		camera->setParent(follow);
	} else
		camera = new Node(0, RENDER_NONE, size, follow);
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
Vector2i UpdateList::getScreenSize() {
	return screenRect.size();
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
sint UpdateList::getLayerCount() {
	return maxLayer + 1;
}

//Subscribe node to cetain event type
void UpdateList::addListener(UNode *item, int type) {
	listeners[type].push_back(item);
}

//Send custom event
void UpdateList::queueEvent(Event event) {
	if(event.type >= EVENT_MAX || event != event_previous[event.type % EVENT_MAX])
		event_queue.push_back(event);
}

void UpdateList::queueEvent(int type, bool down, int code, float x, float y) {
	queueEvent(Event(type, down, code, x, y));
}

//Register keycode for polling
void UpdateList::watchKeycode(int keycode) {
	watchedKeycodes.push_back(keycode);
	watchedKeycodesPrevious.push_back(false);
}

void UpdateList::startRemap() {
	remapKeycode = true;
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

		int type = event.type;
		if(type >= EVENT_MAX)
			type = EVENT_CUSTOM;

		//Skip duplicates
		if(event != event_previous[type]) {
			for(UNode *node : listeners[type])
				node->recieveEvent(event);
			event_previous[type] = event;
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
	for(int layer = 0; layer <= maxULayer; layer++) {
		UNode *uSource = layers[layer].uRoot;

		if(uSource != NULL && uSource->isDeleted()) {
			deleted1.push_back(uSource);
			uSource = uSource->getNext();
			layers[layer].uRoot = uSource;
		}

		//For each node in layer order
		while(uSource != NULL) {

			//Update each node
			uSource->update(time);

			//Check next node for removing from list
			while(uSource->getNext() != NULL && uSource->getNext()->isDeleted()) {
				deleted1.push_back(uSource->getNext());
				uSource->deleteNext();
			}

			uSource = uSource->getNext();
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
					//Do not reuse ids
					//layers[source->getLayer()].count--;
				}

				source = (Node*)source->getNext();
			}
		}
	}
}

//Get size of texture
Vector2i UpdateList::getTextureSize(sint texture) {
	if(texture >= resourceData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	return resourceData[texture].size;
}

//Get resource data
ResourceData &UpdateList::getResourceData(sint index) {
	if(index >= resourceData.size())
		throw new std::invalid_argument(TEXTUREERROR);
	return resourceData[index];
}

sint UpdateList::getResourceCount() {
	return resourceData.size();
}

//Create render buffer resource
sint UpdateList::createBuffer(BufferData data) {
	data.texture = createResource(data.texture, data.size, bufferData.size(), SK_INVALID_BUFFER);
	bufferData.push_back(data);
	return data.texture;
}

//Schedule buffer draw before next draw
void UpdateList::scheduleBufferRefresh(sint texture) {
	bufferData[resourceData[texture].index].redraw = true;
}

BufferData &UpdateList::getBufferData(sint texture) {
	return bufferData[resourceData[texture].index];
}

//Create uniform object
sint UpdateList::createUniform(sint rIndex, sint shader, std::string name, std::vector<float> values) {
	ShaderUniform uniform(shader, name, values, SKU_FLOAT3_VECTOR);
	uniform.texture = createResource(rIndex, Vector2i(3, values.size()/3), shaderUniforms.size(), SK_SHADER_UNIFORM);
	//std::cout << "INFO: SHADER UNIFORM: " << uniform.texture << ": " << shaderUniforms.size() << "\n";
	shaderUniforms.push_back(uniform);
	return uniform.texture;
}
sint UpdateList::createUniform(sint rIndex, sint shader, std::string name, std::vector<int> values, int type) {
	ShaderUniform uniform(shader, name, values, type);
	uniform.texture = createResource(rIndex, Vector2i(3, values.size()/3), shaderUniforms.size(), SK_SHADER_UNIFORM);
	shaderUniforms.push_back(uniform);
	return uniform.texture;
}

sint UpdateList::createUniform(sint rIndex, sint shader, std::string name, float value) {
	ShaderUniform uniform(shader, name, std::vector<float>({value}), SKU_FLOAT);
	uniform.texture = createResource(rIndex, Vector2i(1, 1), shaderUniforms.size(), SK_SHADER_UNIFORM);
	shaderUniforms.push_back(uniform);
	return uniform.texture;
}
sint UpdateList::createUniform(sint rIndex, sint shader, std::string name, int value) {
	ShaderUniform uniform(shader, name, std::vector<int>({value}), SKU_INT);
	uniform.texture = createResource(rIndex, Vector2i(1, 1), shaderUniforms.size(), SK_SHADER_UNIFORM);
	shaderUniforms.push_back(uniform);
	return uniform.texture;
}
sint UpdateList::createUniform(sint rIndex, sint shader, std::string name, Vector2f value) {
	ShaderUniform uniform(shader, name, std::vector<float>({value.x, value.y}), SKU_FLOAT2);
	uniform.texture = createResource(rIndex, Vector2i(2, 1), shaderUniforms.size(), SK_SHADER_UNIFORM);
	shaderUniforms.push_back(uniform);
	return uniform.texture;
}

//Store new uniform values
void UpdateList::updateUniform(sint rIndex, std::vector<float> values) {
	sint uniform = resourceData[rIndex].index;
	if(values != shaderUniforms[uniform].fValues)
		shaderUniforms[uniform].fValues = values;
	shaderUniforms[uniform].update = true;

	//Check for buffers to redraw
	for(sint i = 0; i < bufferData.size(); i++)
		if(bufferData[i].shader == shaderUniforms[uniform].shader)
			bufferData[i].redraw = true;
}
void UpdateList::updateUniform(sint rIndex, std::vector<int> values) {
	sint uniform = resourceData[rIndex].index;
	if(values != shaderUniforms[uniform].iValues)
		shaderUniforms[uniform].iValues = values;
	shaderUniforms[uniform].update = true;

	//Check for buffers to redraw
	for(sint i = 0; i < bufferData.size(); i++)
		if(bufferData[i].shader == shaderUniforms[uniform].shader)
			bufferData[i].redraw = true;
}

void UpdateList::updateUniform(sint rIndex, float value) {
	UpdateList::updateUniform(rIndex, std::vector<float>({value}));
}
void UpdateList::updateUniform(sint rIndex, int value) {
	UpdateList::updateUniform(rIndex, std::vector<int>({value}));
}

void UpdateList::updateUniform(sint rIndex, Vector2f value) {
	UpdateList::updateUniform(rIndex, std::vector<float>({value.x, value.y}));
}

ShaderUniform &UpdateList::getUniform(sint rIndex) {
	sint uIndex = resourceData[rIndex].index;
	return shaderUniforms[uIndex];
}

#ifndef NETWORK_STRING
void recieveNetworkString(std::string data, int code) {
	std::cout << "NETWORK_STRING disabled during compilation\n";
	std::cout << "NETWORK: Received string " << data << "\n";
}
#endif