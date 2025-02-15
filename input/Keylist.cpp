#include "Settings.h"

nlohmann::json Settings::data({});
std::vector<std::pair<std::string, std::string>> Settings::edits;

std::map<std::string, int> Settings::KEYMAP = {
	{"COMBO", 		-3},
	{"MOVING", 		-2},
	{"", 			-1},
	{"A", 			sf::Keyboard::A},
	{"B", 			sf::Keyboard::B},
	{"C", 			sf::Keyboard::C},
	{"D", 			sf::Keyboard::D},
	{"E", 			sf::Keyboard::E},
	{"F", 			sf::Keyboard::F},
	{"G", 			sf::Keyboard::G},
	{"H", 			sf::Keyboard::H},
	{"I", 			sf::Keyboard::I},
	{"J", 			sf::Keyboard::J},
	{"K", 			sf::Keyboard::K},
	{"L", 			sf::Keyboard::L},
	{"M", 			sf::Keyboard::M},
	{"N", 			sf::Keyboard::N},
	{"O", 			sf::Keyboard::O},
	{"P", 			sf::Keyboard::P},
	{"Q", 			sf::Keyboard::Q},
	{"R", 			sf::Keyboard::R},
	{"S", 			sf::Keyboard::S},
	{"T", 			sf::Keyboard::T},
	{"U", 			sf::Keyboard::U},
	{"V", 			sf::Keyboard::V},
	{"W", 			sf::Keyboard::W},
	{"X", 			sf::Keyboard::X},
	{"Y", 			sf::Keyboard::Y},
	{"Z", 			sf::Keyboard::Z},
	{"NUM0", 		sf::Keyboard::Num0},
	{"NUM1", 		sf::Keyboard::Num1},
	{"NUM2", 		sf::Keyboard::Num2},
	{"NUM3", 		sf::Keyboard::Num3},
	{"NUM4", 		sf::Keyboard::Num4},
	{"NUM5", 		sf::Keyboard::Num5},
	{"NUM6", 		sf::Keyboard::Num6},
	{"NUM7", 		sf::Keyboard::Num7},
	{"NUM8", 		sf::Keyboard::Num8},
	{"NUM9", 		sf::Keyboard::Num9},
	{"ESCAPE", 		sf::Keyboard::Escape},
	{"ESC", 		sf::Keyboard::Escape},
	{"LCONTROL",	sf::Keyboard::LControl},
	{"LCTRL",		sf::Keyboard::LControl},
	{"LSHIFT", 		sf::Keyboard::LShift},
	{"LALT", 		sf::Keyboard::LAlt},
	{"LSYSTEM", 	sf::Keyboard::LSystem},
	{"LBRACKET",	sf::Keyboard::LBracket},
	{"RCONTROL",	sf::Keyboard::RControl},
	{"RCTRL",		sf::Keyboard::RControl},
	{"RSHIFT", 		sf::Keyboard::RShift},
	{"RALT", 		sf::Keyboard::RAlt},
	{"RSYSTEM", 	sf::Keyboard::RSystem},
	{"RBRACKET",	sf::Keyboard::RBracket},
	{"SEMICOLON",	sf::Keyboard::Semicolon},
	{"COMMA",		sf::Keyboard::Comma},
	{"PERIOD",		sf::Keyboard::Period},
	{"QUOTE",		sf::Keyboard::Quote},
	{"SLASH",		sf::Keyboard::Slash},
	{"BACKSLASH",	sf::Keyboard::Backslash},
	{"TILDE",		sf::Keyboard::Tilde},
	{"EQUAL",		sf::Keyboard::Equal},
	{"HYPHEN",		sf::Keyboard::Hyphen},
	{"DASH",		sf::Keyboard::Hyphen},
	{"SPACE",		sf::Keyboard::Space},
	{"ENTER",		sf::Keyboard::Enter},
	{"RETURN",		sf::Keyboard::Enter},
	{"BACKSPACE",	sf::Keyboard::Backspace},
	{"TAB",			sf::Keyboard::Tab},
	{"PAGEUP",		sf::Keyboard::PageUp},
	{"PAGEDOWN",	sf::Keyboard::PageDown},
	{"END",			sf::Keyboard::End},
	{"HOME",		sf::Keyboard::Home},
	{"INSERT",		sf::Keyboard::Insert},
	{"DELETE",		sf::Keyboard::Delete},
	{"ADD",			sf::Keyboard::Add},
	{"SUBTRACT",	sf::Keyboard::Subtract},
	{"MULTIPLY",	sf::Keyboard::Multiply},
	{"DIVIDE",		sf::Keyboard::Divide},
	{"LEFT",		sf::Keyboard::Left},
	{"RIGHT",		sf::Keyboard::Right},
	{"UP",			sf::Keyboard::Up},
	{"DOWN",		sf::Keyboard::Down},
	{"NUMPAD0", 	sf::Keyboard::Numpad0},
	{"NUMPAD1", 	sf::Keyboard::Numpad1},
	{"NUMPAD2", 	sf::Keyboard::Numpad2},
	{"NUMPAD3", 	sf::Keyboard::Numpad3},
	{"NUMPAD4", 	sf::Keyboard::Numpad4},
	{"NUMPAD5", 	sf::Keyboard::Numpad5},
	{"NUMPAD6", 	sf::Keyboard::Numpad6},
	{"NUMPAD7", 	sf::Keyboard::Numpad7},
	{"NUMPAD8", 	sf::Keyboard::Numpad8},
	{"NUMPAD9", 	sf::Keyboard::Numpad9},
	{"F1", 			sf::Keyboard::F1},
	{"F2", 			sf::Keyboard::F2},
	{"F3", 			sf::Keyboard::F3},
	{"F4", 			sf::Keyboard::F4},
	{"F5", 			sf::Keyboard::F5},
	{"F6", 			sf::Keyboard::F6},
	{"F7", 			sf::Keyboard::F7},
	{"F8", 			sf::Keyboard::F8},
	{"F9", 			sf::Keyboard::F9},
	{"F10", 		sf::Keyboard::F10},
	{"F11", 		sf::Keyboard::F11},
	{"F12", 		sf::Keyboard::F12},
	{"F13", 		sf::Keyboard::F13},
	{"F14", 		sf::Keyboard::F14},
	{"F15", 		sf::Keyboard::F15},
	{"MOUSELEFT",	MOUSE_OFFSET+0},
	{"MOUSERIGHT",	MOUSE_OFFSET+1},
	{"MOUREMIDDLE",	MOUSE_OFFSET+2},
	{"MOUSE1",		MOUSE_OFFSET+3},
	{"MOUSE2",		MOUSE_OFFSET+4},
	{"WHEELUP",		MOUSE_OFFSET+5},
	{"WHEELDOWN",	MOUSE_OFFSET+6},
	{"JOYSTICKA",	JOYSTICK_OFFSET+0},
	{"JOYSTICKB",	JOYSTICK_OFFSET+1},
	{"JOYSTICKX",	JOYSTICK_OFFSET+2},
	{"JOYSTICKY",	JOYSTICK_OFFSET+3},
	{"JOYSTICKLB",	JOYSTICK_OFFSET+4},
	{"JOYSTICKRB",	JOYSTICK_OFFSET+5},
	{"JOYSTICKBACK",	JOYSTICK_OFFSET+6},
	{"JOYSTICKSTART",	JOYSTICK_OFFSET+7},
	{"JOYSTICK1",	JOYSTICK_OFFSET+8},
	{"JOYSTICKLJ",	JOYSTICK_OFFSET+9},
	{"JOYSTICKRJ",	JOYSTICK_OFFSET+10},
	{"JOYSTICK2",	JOYSTICK_OFFSET+11},
	{"JOYSTICK3",	JOYSTICK_OFFSET+12},
	{"JOYSTICK4",	JOYSTICK_OFFSET+13},
	{"JOYSTICK5",	JOYSTICK_OFFSET+14},
	{"JOYSTICK6",	JOYSTICK_OFFSET+15},
	{"JOYSTICK7",	JOYSTICK_OFFSET+16},
	{"JOYSTICK8",	JOYSTICK_OFFSET+17},
	{"JOYSTICK9",	JOYSTICK_OFFSET+18},
	{"JOYSTICK10",	JOYSTICK_OFFSET+19},
	{"JOYSTICK11",	JOYSTICK_OFFSET+20},
	{"JOYSTICK12",	JOYSTICK_OFFSET+21},
	{"JOYSTICK13",	JOYSTICK_OFFSET+22},
	{"JOYSTICK14",	JOYSTICK_OFFSET+23},
	{"JOYSTICK15",	JOYSTICK_OFFSET+24},
	{"JOYSTICK16",	JOYSTICK_OFFSET+25},
	{"JOYSTICK17",	JOYSTICK_OFFSET+26},
	{"JOYSTICK18",	JOYSTICK_OFFSET+27},
	{"JOYSTICK19",	JOYSTICK_OFFSET+28},
	{"JOYSTICK20",	JOYSTICK_OFFSET+29},
	{"JOYSTICK21",	JOYSTICK_OFFSET+30},
	{"JOYSTICK22",	JOYSTICK_OFFSET+31},
	{"JOYSTICK23",	JOYSTICK_OFFSET+32},
	{"JOYSTICK1LEFT",	JOYSTICK_OFFSET+50},
	{"JOYSTICK1RIGHT",	JOYSTICK_OFFSET+51},
	{"JOYSTICK1UP",		JOYSTICK_OFFSET+52},
	{"JOYSTICK1DOWN",	JOYSTICK_OFFSET+53},
	{"JOYSTICK2LEFT",	JOYSTICK_OFFSET+54},
	{"JOYSTICK2RIGHT",	JOYSTICK_OFFSET+55},
	{"JOYSTICK2UP",		JOYSTICK_OFFSET+56},
	{"JOYSTICK2DOWN",	JOYSTICK_OFFSET+57},
	{"JOYSTICK3LEFT",	JOYSTICK_OFFSET+58},
	{"JOYSTICK3RIGHT",	JOYSTICK_OFFSET+59},
	{"JOYSTICK3UP",		JOYSTICK_OFFSET+60},
	{"JOYSTICK3DOWN",	JOYSTICK_OFFSET+61},
	{"JOYSTICK4LEFT",	JOYSTICK_OFFSET+62},
	{"JOYSTICK4RIGHT",	JOYSTICK_OFFSET+63},
	{"JOYSTICK4UP",		JOYSTICK_OFFSET+64},
	{"JOYSTICK4DOWN",	JOYSTICK_OFFSET+65},
};

std::map<sf::Mouse::Button, int> Settings::MOUSEBUTTON = {
	{sf::Mouse::Left,		MOUSE_OFFSET+0},
	{sf::Mouse::Right,		MOUSE_OFFSET+1},
	{sf::Mouse::Middle,		MOUSE_OFFSET+2},
	{sf::Mouse::XButton1,	MOUSE_OFFSET+3},
	{sf::Mouse::XButton2,	MOUSE_OFFSET+4}
};

std::map<int, sf::Joystick::Axis> Settings::JOYSTICKAXIS = {
	{0, sf::Joystick::X},
	{1, sf::Joystick::Y},
	{2, sf::Joystick::U},
	{3, sf::Joystick::V},
	{4, sf::Joystick::Z},
	{5, sf::Joystick::R},
	{6, sf::Joystick::PovX},
	{7, sf::Joystick::PovY}
};

std::map<sf::Joystick::Axis, int> Settings::JOYSTICKID = {
	{sf::Joystick::X, 		0},
	{sf::Joystick::Y, 		1},
	{sf::Joystick::U, 		2},
	{sf::Joystick::V, 		3},
	{sf::Joystick::Z, 		4},
	{sf::Joystick::R, 		5},
	{sf::Joystick::PovX, 	6},
	{sf::Joystick::PovY, 	7}
};