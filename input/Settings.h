#pragma once

#include "../include/json.hpp"

#include <fstream>
#include <iostream>
#include <string>

#include "../core/Event.h"

using json_pointer = nlohmann::json::json_pointer;

/*
 * Read/write from a centralized json settings file
 */

class Settings {
private:
	static nlohmann::json data;
	static std::vector<std::pair<std::string, std::string>> edits;
	static std::map<std::string, int> EVENT_KEYMAP;

public:
	//Load settings from file
	static void loadSettings(std::string filename) {
		std::ifstream file(filename);
		nlohmann::json input = nlohmann::json::parse(file);
		for(auto& el : input.items())
			data[el.key()] = el.value();
	}

	//Get value functions
	static bool getBool(std::string field) {
		return data.value(json_pointer(field), false);
	}

	static int getInt(std::string field) {
		return data.value(json_pointer(field), 0);
	}

	static std::string getString(std::string field) {
		return data.value(json_pointer(field), "");
	}

	//Get key number from key name
	static int mapKeycode(std::string keyname) {
		//Convert to uppercase
		for(long unsigned int i = 0; i < keyname.length(); i++)
			keyname[i] = toupper(keyname[i]);
		return EVENT_KEYMAP[keyname];
	}

	//Get key number from settings field
	static int getControl(std::string field) {
		std::string keyname = data.value(json_pointer(field), "");
		return mapKeycode(keyname);
	}

	//Set value in memory
	static void setBool(std::string field, bool value) {
		json_pointer pointer = json_pointer(field);
		std::string name = '"' + pointer.back() + "\": ";
		std::string old = getBool(field) ? "true" : "false";
		edits.push_back(std::make_pair(name + old, name + (value ? "true" : "false")));
		data[pointer] = value;
	}

	static void setInt(std::string field, int value) {
		json_pointer pointer = json_pointer(field);
		std::string name = '"' + pointer.back() + "\": ";
		std::string old = std::to_string(getInt(field));
		edits.push_back(std::make_pair(name + old, name + std::to_string(value)));
		data[pointer] = value;
	}

	static void setString(std::string field, std::string value) {
		json_pointer pointer = json_pointer(field);
		std::string name = '"' + pointer.back() + "\": \"";
		std::string old = getString(field);
		edits.push_back(std::make_pair(name + old, name + value));
		data[pointer] = value;
	}

	//Save edited values back to file
	static void save(std::string filename) {
		std::string tempname = filename + ".replace";
		std::ifstream in(filename);
		std::ofstream out(tempname);
		std::string line;
		size_t i = 0;

		while(std::getline(in, line)) {
			for(std::pair<std::string, std::string> edit : edits) {
				i = line.find(edit.first);
			    if(i != std::string::npos)
			    	line.replace(i, edit.first.length(), edit.second);
			}

			//Write edited line to output
			for(char c : line)
				out.put(c);
			out.put('\n');
		}

		//Close files
		in.close();
		out.close();
		edits.clear();

		//Replace original file with new
		std::remove(filename.c_str());
		std::rename(tempname.c_str(), filename.c_str());
	}
};