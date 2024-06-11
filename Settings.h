#pragma once

#include <nlohmann/json.hpp>
#include <SFML/Window.hpp>

#include <fstream>
#include <iostream>
#include <string>

#define MOUSE_OFFSET 150
#define JOYSTICK_OFFSET 160

using json_pointer = nlohmann::json::json_pointer;

class Settings {
private:
	static nlohmann::json data;

public:
	static std::map<std::string, int> KEYMAP;
	static std::map<sf::Mouse::Button, int> MOUSEBUTTON;
	static std::map<int, sf::Joystick::Axis> JOYSTICKAXIS;
	static std::map<sf::Joystick::Axis, int> JOYSTICKID;

	static void loadSettings(std::string filename) {
		std::ifstream file(filename);
		data = nlohmann::json::parse(file);
	}

	static bool getBool(std::string field) {
		return data.value(json_pointer(field), false);
	}

	static int getInt(std::string field) {
		return data.value(json_pointer(field), 0);
	}

	static std::string getString(std::string field) {
		return data.value(json_pointer(field), "");
	}

	static int getControl(std::string field) {
		std::string keyname = data.value(json_pointer(field), "");
		//Convert to uppercase
		for(long unsigned int i = 0; i < keyname.length(); i++)
			keyname[i] = toupper(keyname[i]);
		return KEYMAP[keyname];
	}

	static void setBool(std::string field, bool value) {
		data[json_pointer(field)] = value;
	}

	static void setInt(std::string field, int value) {
		data[json_pointer(field)] = value;
	}

	static void setString(std::string field, std::string value) {
		data[json_pointer(field)] = value;
	}
};