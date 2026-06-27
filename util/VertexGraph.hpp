#pragma once

/*
 * Directed n-edge node graph/tree
 */

template<int size>
class Vertex {
private:
	Vertex<size> *vParent;
	Vertex<size> *edges[size] = {NULL};
	int vID = 0;

public:
	bool printed = false;

	Vertex(Vertex<size> *_vParent) {
		this->vParent = _vParent;
		if(vParent != NULL)
			vID = ++getVRoot()->vID;
	}

	int getSize() {
		return size;
	}

	int getVID() {
		if(vParent == NULL)
			return 0;
		return vID;
	}

	Vertex<size> *getVRoot() {
		if(vParent == NULL)
			return this;
		return vParent;
	}

	Vertex<size> *getVParent() {
		return vParent;
	}

	bool hasParent(Vertex<size> *vertex) {
		if(vParent == NULL)
			return false;
		if(vParent == vertex)
			return true;
		return vParent->hasParent(vertex);
	}

	bool hasEitherParent(Vertex<size> *vertex) {
		return this->hasParent(vertex) || vertex->hasParent(this);
	}

	bool hasEdge(int edge) {
		return edges[edge % size] != NULL;
	}

	Vertex<size> *getVertex(int edge) {
		return edges[edge % size];
	}

	void setVParent(Vertex<size> *_parent) {
		vParent = _parent;
	}

	Vertex<size> *setVertex(int edge, Vertex<size> *vertex) {
		edges[edge % size] = vertex;
		return vertex;
	}

	Vertex<size> *setVertex(int edge, int opposite, Vertex<size> *vertex) {
		setVertex(edge, vertex);
		vertex->setVertex(opposite, this);
		return vertex;
	}

	Vertex<size> *addVertex(int edge, int opposite, Vertex<size> *vertex=NULL) {
		if(vertex == NULL)
			vertex = new Vertex<size>(this);

		edge = edge % size;
		if(edges[edge] == NULL) {
			setVertex(edge, opposite, vertex);
			//vertex->setVParent(this);
		} else
			edges[edge]->addVertex(edge, opposite, vertex);
		return vertex;
	}

	void printAddress() {
		printed = true;
		std::cout << displayName() << ": [";
		for(int e = 0; e < size; e++) {
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

	void printEdge(int edge, Vertex<size> *vertex) {
		std::cout << "\t" << displayName() << "->";
		std::cout << vertex->displayName();
		if(edge % 2 == 1)
			std::cout << " [color = red]";
		std::cout << "\n";
	}

	virtual std::string displayName() {
		return std::to_string(getVID());
	}
};