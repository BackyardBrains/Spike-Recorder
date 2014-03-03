#include "widgets/Application.h"
// #include "widgets/Widget.h"
#include "Game.h"
#include <SDL.h>

int main(int argc, char *argv[])
{
	BackyardBrains::Game game;
	
	game.run();
	return 0;
}
