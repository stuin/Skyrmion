#include <filesystem>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>

namespace fs = std::filesystem;

//Engine compatible file read/write
char *IO::openFile(std::string filename) {
	char *source = NULL;
	FILE *fp = fopen(filename.c_str(), "r");
	if(fp != NULL) {
	    //Go to the end of the file
	    if(fseek(fp, 0L, SEEK_END) == 0) {
	        //Get the size of the file and go back to the start
	        long bufsize = ftell(fp);
	        if(bufsize == -1 || fseek(fp, 0L, SEEK_SET) != 0)
	        	throw new std::invalid_argument(FILEERROR);

	        //Allocate our buffer to that size
	        source = (char*)malloc(sizeof(char) * (bufsize + 1));

	        //Read the entire file into memory
	        size_t newLen = fread(source, sizeof(char), bufsize, fp);
	        if(ferror(fp) != 0)
	            throw new std::invalid_argument(FILEERROR);
	        source[newLen++] = '\0';
	    }
	    fclose(fp);
	}
	return source;
}
void IO::closeFile(char *file) {
	free(file);
}

void IO::writeFile(std::string filename, char *text) {
	std::ofstream file;
	file.open(filename);
	file << text;
	file.close();
}
void IO::writeFile(std::string filename, std::string text) {
	std::ofstream file;
	file.open(filename);
	file << text;
	file.close();
}

void IO::deleteFile(std::string filename) {
	if(fs::exists(filename))
		fs::remove(filename);
}
bool IO::hasFile(std::string filename) {
	return fs::exists(filename);
}
int IO::fileSize(std::string filename) {
	return fs::file_size(filename);
}
void IO::createFolder(std::string filename) {
	if(!fs::exists(filename))
		fs::create_directory(filename);
}