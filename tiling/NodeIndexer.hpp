#include "../core/UpdateList.h"
#include "GridMaker.h"

class NodeIndexer : public Indexer {
private:
	int layer;

	std::function<int(Node*)> func;
	bool hasFunction = false;

public:
	NodeIndexer(Indexer *previous, int _layer, int scaleX = 1, int scaleY = 1)
		: Indexer(previous, 0, Vector2i(scaleX, scaleY)), layer(_layer) {

	}

	NodeIndexer(Indexer *previous, int _layer, Vector2i scale)
		: Indexer(previous, 0, scale), layer(_layer) {

	}

	Node *getNode(int id) {
		return UpdateList::getNode(layer, id);
	}

	Node *getNode(Vector2f position) {
		//std::cout << "Get pos " << position << " of id " << getTile(position) << "\n";
		return getNode(getTile(position));
	}

	void setNode(Vector2f position, Node *node) {
		int id = (node == NULL) ? 0 : node->getId();
		getPrevious()->setTileI(position.x, position.y, id);
		//std::cout << "Set pos " << position << " to id " << id << " " << getTile(position) << "\n";
	}

	//Get value of tile from map
	int mapTile(int c) override {
		Node *n = getNode(c);
		if(hasFunction && n != NULL)
			return func(n);
		return c;
	}
};