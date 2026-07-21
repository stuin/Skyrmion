#pragma once

#include <string>
#include <map>
#include <unordered_set>

#include "../core/Color.h"
#include "../core/Event.h"

using string_pair = std::pair<std::string, std::string>;

/*
 * Read/write from a centralized json settings file
 */

class Settings {
private:
	std::vector<string_pair> edits;
	bool loaded = false;

public:
	static const std::map<std::string, int> EVENT_KEYMAP;
	static const std::map<int, int> FONT_SPRITEMAP;

	std::unordered_set<std::string> markKeycode;
	std::string filename = "";

	//Load settings from file
	void loadSettings(std::string _filename, bool saveFile=true);
	std::vector<string_pair> listKeys(std::string field="");
	bool hasLoaded();

	//Get value functions
	bool getBool(const std::string &field);
	int getInt(const std::string &field, int def=0);
	std::string getString(const std::string &field, std::string def="");
	skColor getColor(const std::string &field, skColor def=COLOR_BLACK);
	Vector2i getVector(const std::string &field, Vector2i def=Vector2i());

	//Control mapping functions
	int getControl(const std::string &field);
	bool isKeycode(const std::string &field);
	int mapKeycode(const std::string &_keyname);
	std::string reverseKeycode(int code);

	//Set value in memory
	void setBool(std::string field, bool value);
	void setInt(std::string field, int value);
	void setString(std::string field, std::string value);
	void setColor(std::string field, skColor value);
	void setVector(std::string field, Vector2i value);

	//Save edited values back to file
	void save(std::string _filename);
	void settingsEvent();
};

static Settings SETTINGS;