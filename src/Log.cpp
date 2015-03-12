#include "Log.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <string>

namespace BackyardBrains {

Log *Log::_log = 0;
Log::Log() {
	_out = stdout;
}

void Log::init() {
	if(_log == 0)
		_log = new Log();
}

void Log::msg(const char *fmt, ...) {
	init();

	std::string format = "-- ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
	vfprintf(_log->_out, format.c_str(), args);
	va_end(args);
}

void Log::warn(const char *fmt, ...) {
	init();

	std::string format = "ww Warning: ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
	vfprintf(_log->_out, format.c_str(), args);
	va_end(args);
}

void Log::error(const char *fmt, ...) {
	init();

	std::string format = "EE Error: ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
	vfprintf(_log->_out, format.c_str(), args);
	va_end(args);
}

void Log::fatal(const char *fmt, ...) {
	init();

	std::string format = "FF FATAL: ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
	vfprintf(_log->_out, format.c_str(), args);
	va_end(args);

	abort();
}

}
