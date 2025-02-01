#pragma once

enum LOGLEVELS {
	SKLOG_ALL = 0,        // Display all logs
    SKLOG_TRACE,          // Trace logging, intended for internal use only
    SKLOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    SKLOG_INFO,           // Info logging, used for program execution info
    SKLOG_WARNING,        // Warning logging, used on recoverable failures
    SKLOG_ERROR,          // Error logging, used on unrecoverable failures
    SKLOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    SKLOG_NONE
};

class IO {
public:
	//Engine compatible file read/write
	static char *openFile(std::string filename);
	static void closeFile(char *file);

	//Logging passthrough
	template<typename... Args>
	static void log(int logLevel, const char *text, Args... args);
};