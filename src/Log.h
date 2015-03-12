#ifndef BACKYARDBRAINS_LOG_H
#define BACKYARDBRAINS_LOG_H

#include <cstdio>

namespace BackyardBrains {

class Log {
public:
	Log();
	static void msg(const char *fmt, ...);
	static void warn(const char *fmt, ...);
	static void error(const char *fmt, ...);
	static void fatal(const char *fmt, ...); // aborts program
private:
	static Log *_log;
	FILE *_out;

	static void init();
};

}

#endif
