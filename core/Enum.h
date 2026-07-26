#pragma once

//Macro to build enum + array of names
#define GENERATE_TYPE(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,
#define GENERATE_STRING1(STRING) GENERATE_STRING(_ ## STRING)
#define GENERATE_TYPE2(ENUM, STRING) ENUM,
#define GENERATE_STRING2(ENUM, STRING) STRING,

#define NAMED_ENUM(ENUM) enum ENUM##_TYPES { ENUM##_FOREACH(GENERATE_TYPE) }; static std::vector<std::string> ENUM##_NAMES = { ENUM##_FOREACH(GENERATE_STRING) };
#define FILE_ENUM(ENUM) enum ENUM##_TYPES { ENUM##_FOREACH(GENERATE_TYPE, GENERATE_TYPE2) }; static std::vector<std::string> ENUM##_FILES = { ENUM##_FOREACH(GENERATE_STRING1, GENERATE_STRING2) };

static std::vector<std::string> BLANK_NAMES;