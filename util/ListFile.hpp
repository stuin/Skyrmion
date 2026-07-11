#include "../core/Event.h"

class ListFile {
private:
	std::string filename;
	std::string fallback;
	std::vector<std::string> contents;

public:
	ListFile(std::string _filename, std::string _fallback="") : filename(_filename), fallback(_fallback) {

		if(IO::hasFile(_filename))
			load(_filename);
	}

	std::string getLine(sint l) {
		if(l < contents.size())
			return contents[l];
		return fallback;
	}

	int getInt(sint l) {
		if(l < contents.size())
			return stoi(contents[l]);
		return 0;
	}

	void setLine(sint l, std::string s, bool _save=true) {
		while(l >= contents.size())
			contents.push_back(fallback);
		contents[l] = s;
		if(_save)
			save();
	}

	void setInt(sint l, int value, bool _save=true) {
		while(l >= contents.size())
			contents.push_back(fallback);
		contents[l] = std::to_string(value);
		if(_save)
			save();
	}

	void increment(sint l, bool _save=true) {
		setInt(l, getInt(l)+1, _save);
	}

	void linkNode(sint l, Node *node) {
		while(l >= contents.size())
			contents.push_back(fallback);
		node->setString(contents[l].c_str());
	}

	void load(std::string file="") {
		if(file == "")
			file = filename;

		char *mapFile = IO::openFile(file);
		char *line = mapFile;

		//Read file by line
		sint i = 0;
		while(line[0] != '\0') {
			if(i >= contents.size())
				contents.push_back("");

			//Copy string
			int j = 0;
			while(line[j] != '\0' && line[j] != '\n' && line[j] != '\r') {
				contents[i].push_back(line[j]);
				++j;
			}
			i++;

			if(line[j] != '\0') {
				while(line[j] == '\n' || line[j] == '\r')
					j++;
			}
			line += j;
		}
		IO::closeFile(mapFile);
	}

	void save(std::string file="") {
		if(file == "")
			file = filename;

		int size = 1;
		for(std::string s : contents)
			size += s.size() + 1;

		char text[size];

		//Loop through tiles
		int i = 0;
		for(sint y = 0; y < contents.size(); y++) {
			for(sint x = 0; x < contents[y].size(); x++) {
				text[i+x] = contents[y][x];
			}
			i += contents[y].size();
			text[i] = '\n';
			i++;
		}
		text[size-1] = '\0';

		IO::writeFile(file, text);
	}
};