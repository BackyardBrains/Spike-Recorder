#include "Game.h"
#include "MainView.h"
#include "Log.h"
#include "widgets/TextureGL.h"
#include <SDL.h>

namespace BackyardBrains {

Game::Game() : _fileRec(_manager) {
	Log::msg("Loading Resources...");
	loadResources();
	Log::msg("Creating Window...");

	addWindow(new MainView(_manager, _fileRec));
	createWindow(800,600);
	setWindowTitle("BYB Spike Recorder");

	Log::msg("Starting GUI...");

}

Game::~Game() {
	
}


void Game::loadResources() {
	Widgets::TextureGL::load("data/pause.png");
	Widgets::TextureGL::load("data/pausehigh.png");
	Widgets::TextureGL::load("data/play.png");
	Widgets::TextureGL::load("data/playhigh.png");

	Widgets::TextureGL::load("data/forward.png");
	Widgets::TextureGL::load("data/forwardhigh.png");
	Widgets::TextureGL::load("data/backward.png");
	Widgets::TextureGL::load("data/backwardhigh.png");

	Widgets::TextureGL::load("data/config.png");
	Widgets::TextureGL::load("data/confighigh.png");
	Widgets::TextureGL::load("data/thresh.png");
	Widgets::TextureGL::load("data/threshhigh.png");
	Widgets::TextureGL::load("data/ekg.png");
	Widgets::TextureGL::load("data/ekghigh.png");
	Widgets::TextureGL::load("data/rec.png");
	Widgets::TextureGL::load("data/rechigh.png");
	Widgets::TextureGL::load("data/file.png");
	Widgets::TextureGL::load("data/filehigh.png");
	Widgets::TextureGL::load("data/analysis.png");
	Widgets::TextureGL::load("data/analysishigh.png");
	Widgets::TextureGL::load("data/fft.png");
	Widgets::TextureGL::load("data/ffthigh.png");

	Widgets::TextureGL::load("data/save.png");
	Widgets::TextureGL::load("data/savehigh.png");

	Widgets::TextureGL::load("data/pin.png");
	Widgets::TextureGL::load("data/threshpin.png");
	Widgets::TextureGL::load("data/dropdown.png");
	Widgets::TextureGL::load("data/gainup.png");
	Widgets::TextureGL::load("data/gaindown.png");

	Widgets::TextureGL::load("data/ckboxon.png");
	Widgets::TextureGL::load("data/ckboxoff.png");
	
	Widgets::TextureGL::load("data/plotview.png");
	Widgets::TextureGL::load("data/plotviewhigh.png");
	Widgets::TextureGL::load("data/plotviewdown.png");
	Widgets::TextureGL::load("data/plotviewdownhigh.png");
	Widgets::TextureGL::load("data/disconnected.png");
	Widgets::TextureGL::load("data/connected.png");
    Widgets::TextureGL::load("data/usbcon.png");
    Widgets::TextureGL::load("data/usbconhigh.png");
    Widgets::TextureGL::load("data/usbdiscon.png");
    Widgets::TextureGL::load("data/usbdisconhigh.png");     
	Widgets::TextureGL::load("data/heart.png");
	Widgets::TextureGL::load("data/speaker.png");
	Widgets::TextureGL::load("data/speakeroff.png");
}

void Game::advance() {
	static uint32_t t = 0; // TODO make this cleaner
	uint32_t newt = SDL_GetTicks();
    float timediff = (newt-t);
    float sampleNumber = (float)(_manager.sampleRate())*(timediff/1000.0);

    _manager.advance((int32_t)sampleNumber); // fetch more samples than necessary to prevent lag
	_fileRec.advance();

	t = newt;
}



} // namespace BackyardBrains
