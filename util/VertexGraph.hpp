#pragma once

using Edge = unsigned short;

/*
 * Directed n-edge node graph/tree
 */

template<Edge size>
class Vertex {
private:
	Vertex<size> *root;
	Vertex<size> **selected;
	Vertex<size> *edges[size] = {NULL};

public:
	int count = 0;
	bool printed = false;

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

	bool hasEdge(Edge edge) {
		return edges[edge % size] != NULL;
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

	Vertex<size> *setVertex(Edge edge, Vertex<size> *vertex) {
		edges[edge % size] = vertex;
		return vertex;
	}

	Vertex<size> *setVertex(Edge edge, Vertex<size> *vertex, Edge opposite) {
		setVertex(edge, vertex);
		vertex->setVertex(opposite, this);
		return vertex;
	}

	Vertex<size> *addVertex(Edge edge, Vertex<size> *vertex, Edge opposite) {
		if(vertex == NULL || selected != vertex->selected)
			return NULL;

		edge = edge % size;
		if(edges[edge] == NULL)
			setVertex(edge, vertex, opposite);
		else
			edges[edge]->addVertex(edge, vertex, opposite);
		return vertex;
	}

	Vertex<size> *addVertex(Edge edge, Edge opposite) {
		return addVertex(edge, new Vertex<size>(root), opposite);
	}

	void printAddress() {
		printed = true;
		std::cout << displayName() << ": [";
		for(Edge e = 0; e < size; e++) {
			if(edges[e] != NULL) {
				if(!edges[e]->printed)
					edges[e]->printAddress();
				else
					std::cout << edges[e]->displayName();
			} else
				std::cout << "_";
			std::cout << ",";
		}
		std::cout << "]";
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