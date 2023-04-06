using Edge = unsigned short;

template<Edge size> 
class Vertex {
private:
	Vertex<size> *root;
	Vertex<size> **selected;
	Vertex<size> *edges[size] = {NULL};

public:
	int count = 0;

	Vertex(Vertex<size> *_root) {
		if(_root == NULL) {
			this->root = this;
			selected = &root;
		} else {
			this->root = _root->getRoot();
			selected = root->_getSelectPointer();
			count = root->count++;
		}
	}

	Edge getSize() {
		return size;
	}

	int getID() {
		if(selected == &root)
			return -1;
		return count;
	}

	Vertex<size> *getRoot() {
		if(selected == &root)
			return this;
		return root;
	}

	Vertex<size> *getSelected() {
		return *selected;
	}

	Vertex<size> **_getSelectPointer() {
		return selected;
	}

	Edge getOpposite(Edge edge) {
		return (edge + size / 2) % size;
	}

	Vertex<size> *select(Vertex<size> *vertex) {
		if(vertex != NULL && selected == vertex->selected && *selected != vertex) {
			(*selected)->select(false);
			*selected = vertex;
			(*selected)->select(true);
		}
		return *selected;
	}

	Vertex<size> *select(Edge edge) {
		if(edges[edge % size] == NULL)
			return select(this);
		return select(edges[edge % size]);
	}

	void setVertex(Edge edge, Vertex<size> *vertex) {
		edges[edge % size] = vertex;
		vertex->edges[getOpposite(edge)] = this;
	}

	void setOneWay(Edge edge, Vertex<size> *vertex) {
		edges[edge % size] = vertex;
	}

	void addVertex(Edge edge, Vertex<size> *vertex) {
		if(vertex == NULL || selected != vertex->selected)
			return;

		edge = edge % size;
		if(edges[edge] == NULL) {
			edges[edge] = vertex;
			vertex->edges[getOpposite(edge)] = this;
		} else {
			edges[edge]->addVertex(edge, vertex);
		}
	}

	void addVertex(Edge edge) {
		addVertex(edge, new Vertex<size>(root));
	}

	void printAddress() {
		std::cout << getID() << ": [";
		for(Edge e = 0; e < size; e++) {
			if(edges[e] != NULL)
				std::cout << edges[e]->getID();
			else
				std::cout << "_";
			std::cout << ",";
		}
		std::cout << "]\n";
	}

	virtual void onSelect(bool selected) {}
};