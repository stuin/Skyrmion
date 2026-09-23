#pragma once

//template <typename T>
//class Array : public std::initializer_list<T> {
//	T& operator[](int index) {
//		return begin() + index;
//	}
//};

template <typename T>
using Array = std::initializer_list<T>;

//Macro to build enum + array of names
#define GENERATE_TYPE(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_STRING1(STRING) GENERATE_STRING(_ ## STRING)
#define GENERATE_TYPE2(ENUM, STRING) ENUM,
#define GENERATE_STRING2(ENUM, STRING) STRING,

#define NAMED_ENUM(ENUM) enum ENUM##_TYPES { ENUM##_FOREACH(GENERATE_TYPE) }; static Array<std::string> ENUM##_NAMES = { ENUM##_FOREACH(GENERATE_STRING) };
#define FILE_ENUM(ENUM) enum ENUM##_TYPES { ENUM##_FOREACH(GENERATE_TYPE, GENERATE_TYPE2) }; static Array<std::string> ENUM##_FILES = { ENUM##_FOREACH(GENERATE_STRING1, GENERATE_STRING2) };

static Array<std::string> BLANK_NAMES;