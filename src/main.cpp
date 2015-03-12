#include "widgets/Application.h"
#include "Game.h"
#include "Log.h"
#include <SDL.h>
#include <cstring>
#include <string>
#include <cerrno>

#ifdef __WIN32__
#include <direct.h>
#else
#include <unistd.h>
#endif

int main(int argc, char *argv[]) {
	char *lastslash = strrchr(argv[0], '/');
	if(lastslash == NULL)
		lastslash = argv[0]+strlen(argv[0]);

	std::string path(argv[0], lastslash);
	int ret;
	ret = chdir(path.c_str());

#ifdef __APPLE__
	ret += chdir("../Resources");
#endif
	if(ret != 0)
		BackyardBrains::Log::fatal("Fatal: could not change directory: %s", strerror(errno));

	BackyardBrains::Game game;
	
	game.run();
	return 0;
}
