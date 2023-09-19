#pragma once

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

	Vertex<size> **_getSelectPointer() {
		return selected;
	}

	Vertex<size> *getSelected() {
		return *selected;
	}

	bool isSelected() {
		return *selected == this;
	}

	Vertex<size> *getVertex(Edge edge) {
		return edges[edge % size];
	}

	Vertex<size> *select() {
		if(*selected != this) {
			(*selected)->onSelect(false);
			*selected = this;
			(*selected)->onSelect(true);
		}
		//printAddress();
		return *selected;
	}

	Vertex<size> *select(Edge edge) {
		if(edges[edge % size] == NULL)
			return select();
		return edges[edge % size]->select();
	}

	void setVertex(Edge edge, Vertex<size> *vertex) {
		edges[edge % size] = vertex;
	}

	void setVertex(Edge edge, Vertex<size> *vertex, Edge opposite) {
		setVertex(edge, vertex);
		vertex->setVertex(opposite, this);
	}

	void addVertex(Edge edge, Vertex<size> *vertex, Edge opposite) {
		if(vertex == NULL || selected != vertex->selected)
			return;

		edge = edge % size;
		if(edges[edge] == NULL)
			setVertex(edge, vertex, opposite);
		else
			edges[edge]->addVertex(edge, vertex, opposite);
	}

	void addVertex(Edge edge, Edge opposite) {
		addVertex(edge, new Vertex<size>(root), opposite);
	}

	void printAddress() {
		std::cout << displayName() << ": [";
		for(Edge e = 0; e < size; e++) {
			if(edges[e] != NULL)
				std::cout << edges[e]->displayName();
			else
				std::cout << "_";
			std::cout << ",";
		}
		std::cout << "]\n";
	}

	void printEdge(Edge edge, Vertex<size> *vertex) {
		std::cout << "\t" << displayName() << "->";
		std::cout << vertex->displayName(); 
		if(edge % 2 == 1)
			std::cout << " [color = red]";
		std::cout << "\n";
	}

	virtual std::string displayName() {
		return std::to_string(getID());
	}

	virtual void onSelect(bool selected) {}
};