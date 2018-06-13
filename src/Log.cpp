#include "Log.h"
#include "Paths.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cerrno>

#ifdef __APPLE__
#include <syslog.h>
#include <stdarg.h>
#endif


namespace BackyardBrains {

Log *Log::_log = 0;
Log::Log() {
	if(getLoggingPath() == "") {
		_out = stdout;
	} else {
	    fprintf(stderr,getLoggingPath().c_str());
		_out = fopen(getLoggingPath().c_str(), "w");
		if(_out == 0) {
			fprintf(stderr, "Error opening logging destination:%s\nRedirecting log to stdout.\n", strerror(errno));
			_out = stdout;
		}
	}
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
	va_start(args,format);
#ifdef __APPLE__
    char buffer[512];
    vsprintf (buffer,fmt, args);
    //perror (buffer);
    syslog(LOG_ERR, "%s", buffer);
#else
	vfprintf(_log->_out, format.c_str(), args);
#endif
    
   
    
	va_end(args);
}

void Log::warn(const char *fmt, ...) {
	init();

	std::string format = "Warning: ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
	
    #ifdef __APPLE__
        char buffer[512];
        vsprintf (buffer,fmt, args);
        syslog(LOG_WARNING, "%s", buffer);
    #else
        vfprintf(_log->_out, format.c_str(), args);
    #endif
	va_end(args);
}

void Log::error(const char *fmt, ...) {
	init();

	std::string format = "Error: ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
#ifdef __APPLE__
    char buffer[512];
    vsprintf (buffer,fmt, args);
    syslog(LOG_ERR, "%s", buffer);
#else
    vfprintf(_log->_out, format.c_str(), args);
#endif
	va_end(args);
}

void Log::fatal(const char *fmt, ...) {
	init();

	std::string format = "FATAL: ";
	format += fmt;
	format += "\n";
	va_list args;
	va_start(args,fmt);
#ifdef __APPLE__
    char buffer[512];
    vsprintf (buffer,fmt, args);
    syslog(LOG_CRIT, "%s", buffer);
#else
    vfprintf(_log->_out, format.c_str(), args);
#endif
	va_end(args);

	abort();
}

}
