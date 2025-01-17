#include "../include/backward-cpp/backward.hpp"

#include "../core/UpdateList.h"

/*
 * Debug tool to pause everything at a keypress or breakpoint
 */

std::vector<std::string> debugBreakpointKeys = {
	"/debug/breakpoints/pause",
	"/debug/breakpoints/resume",
	"/debug/breakpoints/nextframe"
};

class DebugBreakpoints : public Node {
private:
	InputHandler input;

public:
	bool pauseFrame = false;
	bool stayPaused = false;

	DebugBreakpoints(int layer) : Node(layer, sf::Vector2i(16, 16), true),
	input(debugBreakpointKeys, layer, this) {

		DebugBreakpoints *_this = this;
		input.pressedFunc = [_this](int i) {
			switch(i) {
			case 0:
				_this->pauseFrame = true;
				break;
			case 1:
				_this->stayPaused = false;
				_this->pauseFrame = false;
				break;
			case 2:
				_this->stayPaused = false;
				_this->pauseFrame = true;
				break;
			}
		};
	}

	void enterBreakpoint() {
		std::cout << "Breakpoint hit\n";
		stayPaused = true;

		backward::StackTrace st;
		st.load_here(32);
		backward::Printer p;
		p.print(st);

		while(stayPaused && UpdateList::isRunning()) {
			//Process input events
			UpdateList::processEvents();

			//Simulate updates for debug layer
			Node *source = UpdateList::getNode(getLayer());
			while(source != NULL) {
				if(source != this && !source->isDeleted())
					source->update(0);
				source = source->getNext();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void update(double time) {
		if(pauseFrame)
			enterBreakpoint();
	}
};

DebugBreakpoints *debugBreakpoints = NULL;

void enterBreakpoint() {
	if(debugBreakpoints != NULL)
		debugBreakpoints->enterBreakpoint();
}

#if _DEBUG
	#define BREAKPOINT enterBreakpoint()
#else
	#define BREAKPOINT void
#endif