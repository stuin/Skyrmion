#include "../core/UpdateList.h"

//Renders a node to a buffer and uses that as a texture
class BufferNode : public Node {
    Node *source = NULL;
    skColor color;

public:
	BufferNode(sint buffer, Node *_source, skColor _color=skColor(0,0,0)) :
        Node(_source->getLayer(), _source->getSize(), false, NULL), source(_source), color(_color) {

        UpdateList::createBuffer(buffer, source->getSize());
		setTexture(buffer);
        setOrigin(source->getOrigin());
        setScale(1, -1);
	}

    void update(double time) {
        source->update(time);
        if(source->isDirty())
            UpdateList::scheduleReload(getTexture(), source, color);
    }
};