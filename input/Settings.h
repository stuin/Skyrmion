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
	static std::vector<string_pair> edits;

public:
	static const std::map<std::string, int> EVENT_KEYMAP;
	static const std::map<int, int> FONT_SPRITEMAP;

	static std::unordered_set<std::string> markKeycode;
	static std::string filename;

	//Load settings from file
	static void loadSettings(std::string _filename, bool saveFile=true);
	static std::vector<string_pair> listKeys(std::string field="");

	//Get value functions
	static bool getBool(const std::string &field);
	static int getInt(const std::string &field, int def=0);
	static std::string getString(const std::string &field);
	static skColor getColor(const std::string &field);

	//Control mapping functions
	static int getControl(const std::string &field);
	static bool isKeycode(const std::string &field);
	static int mapKeycode(const std::string &_keyname);
	static std::string reverseKeycode(int code);

	//Set value in memory
	static void setBool(std::string field, bool value);
	static void setInt(std::string field, int value);
	static void setString(std::string field, std::string value);
	static void setColor(std::string field, skColor value);

	//Save edited values back to file
	static void save(std::string _filename);
	static void settingsEvent();
};