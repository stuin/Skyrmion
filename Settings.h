#pragma once

#include <SFML/Window.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <string>
#include <iostream>

#define MOUSE_OFFSET 150
#define JOYSTICK_OFFSET 160

using json_pointer = nlohmann::json::json_pointer;

class Settings {
private:
	static nlohmann::json data;

public:
	static std::map<std::string, int> keymap;
	static std::map<sf::Mouse::Button, int> mousebuttons;

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
		for(long unsigned int i = 0; i < keyname.length(); i++)
			keyname[i] = toupper(keyname[i]);
		return keymap[keyname];
	}

	static int *getControls(std::vector<std::string> keys) {
		int *controls = (int *)malloc(keys.size() * sizeof(int));
		for(long unsigned int i = 0; i < keys.size(); i++)
			controls[i] = getControl(keys[i]);
		return controls;
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