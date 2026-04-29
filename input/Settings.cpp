#include "Settings.h"
#include "../core/UpdateList.h"

#include "../include/json.hpp"

nlohmann::json data({});
int count = 0;

std::vector<string_pair> Settings::edits;
std::unordered_set<std::string> Settings::markKeycode;
std::string Settings::filename = "";

using json_pointer = nlohmann::json::json_pointer;

/*
 * Read/write from a centralized json settings file
 */

//Load settings from file
void Settings::loadSettings(std::string _filename, bool saveFile) {
	if(saveFile)
		filename = _filename;

	char *text = IO::openFile(_filename);
	if(text == NULL) {
		std::cout << "Missing settings file " << _filename << "\n";
		IO::closeFile(text);
		return;
	}

	nlohmann::json input = nlohmann::json::parse(text);
	for(auto& el : input.items())
		data[el.key()] = el.value();
	IO::closeFile(text);
	//settingsEvent();
}

std::vector<string_pair> Settings::listKeys(std::string field) {
	nlohmann::json root = data;
	if(field != "")
		root = data[json_pointer(field)];

	std::vector<string_pair> out;
	for(auto& [key, val] : root.items())
		out.push_back(std::make_pair(key, val.type_name()));
	return out;
}

//Get key number from key name
int Settings::mapKeycode(const std::string &_keyname) {
	//Convert to uppercase
	std::string keyname = _keyname;
	for(sint i = 0; i < keyname.length(); i++)
		keyname[i] = toupper(keyname[i]);
	auto key = EVENT_KEYMAP.find(keyname);
	if(key == EVENT_KEYMAP.end())
		return -5;
	return key->second;
}

//Get key name from key number
std::string Settings::reverseKeycode(int _code) {
	for(const auto& [key, code] : EVENT_KEYMAP) {
		if(code == _code)
			return key;
	}
	return "";
}

//Get value functions
bool Settings::getBool(const std::string &field) {
	return data.value(json_pointer(field), false);
}

int Settings::getInt(const std::string &field, int def) {
	return data.value(json_pointer(field), def);
}

std::string Settings::getString(const std::string &field) {
	return data.value(json_pointer(field), "");
}

//Get key number from settings field
int Settings::getControl(const std::string &field) {
	std::string keyname = data.value(json_pointer(field), "");
	markKeycode.insert(field);
	return mapKeycode(keyname);
}

bool Settings::isKeycode(const std::string &field) {
	return markKeycode.count(field) > 0;
}

//Set value in memory
void Settings::setBool(std::string field, bool value) {
	json_pointer pointer = json_pointer(field);
	std::string name = '"' + pointer.back() + "\": ";
	std::string old = getBool(field) ? "true" : "false";
	edits.push_back(std::make_pair(name + old, name + (value ? "true" : "false")));
	data[pointer] = value;
	settingsEvent();
}

void Settings::setInt(std::string field, int value) {
	json_pointer pointer = json_pointer(field);
	std::string name = '"' + pointer.back() + "\": ";
	std::string old = std::to_string(getInt(field));
	edits.push_back(std::make_pair(name + old, name + std::to_string(value)));
	data[pointer] = value;
	settingsEvent();
}

void Settings::setString(std::string field, std::string value) {
	json_pointer pointer = json_pointer(field);
	std::string name = '"' + pointer.back() + "\": \"";
	std::string old = getString(field);
	edits.push_back(std::make_pair(name + old, name + value));
	data[pointer] = value;
	settingsEvent();
}

//Save edited values back to file
void Settings::save(std::string _filename) {
	if(_filename == "")
		_filename = filename;

	char *inFile = IO::openFile(filename);
	std::string outFile(inFile);
	size_t i = 0;

	//WIP: save new fields
	for(string_pair edit : edits) {
		i = outFile.find(edit.first);
		if(i != std::string::npos)
			outFile.replace(i, edit.first.length(), edit.second);
	}

	IO::closeFile(inFile);
	IO::writeFile(filename, outFile);
	UpdateList::queueEvent(Event(EVENT_SETTINGS, true, count++));
}

void Settings::settingsEvent() {
	UpdateList::queueEvent(Event(EVENT_SETTINGS, false, count++));
}