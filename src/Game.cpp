#include "Game.h"
#include "MainView.h"
#include "Log.h"
#include "widgets/TextureGL.h"
#include <SDL.h>

namespace BackyardBrains {

Game::Game() : _anaman(_manager), _fileRec(_manager) {
	Log::msg("Loading Resources...");
	loadResources();
	Log::msg("Creating Window...");

	addWindow(new MainView(_manager, _anaman, _fileRec));
	createWindow(800,600);
	setWindowTitle("BYB Spike Recorder");

	Log::msg("Starting GUI...");

}

Game::~Game() {

}


void Game::loadResources() {
	Widgets::TextureGL::load("data/pause.bmp");
	Widgets::TextureGL::load("data/pausehigh.bmp");
	Widgets::TextureGL::load("data/play.bmp");
	Widgets::TextureGL::load("data/playhigh.bmp");

	Widgets::TextureGL::load("data/forward.bmp");
	Widgets::TextureGL::load("data/forwardhigh.bmp");
	Widgets::TextureGL::load("data/backward.bmp");
	Widgets::TextureGL::load("data/backwardhigh.bmp");

	Widgets::TextureGL::load("data/config.bmp");
	Widgets::TextureGL::load("data/confighigh.bmp");
	Widgets::TextureGL::load("data/thresh.bmp");
	Widgets::TextureGL::load("data/threshhigh.bmp");
	Widgets::TextureGL::load("data/ekg.bmp");
	Widgets::TextureGL::load("data/ekghigh.bmp");
	Widgets::TextureGL::load("data/rec.bmp");
	Widgets::TextureGL::load("data/rechigh.bmp");
	Widgets::TextureGL::load("data/file.bmp");
	Widgets::TextureGL::load("data/filehigh.bmp");
	Widgets::TextureGL::load("data/analysis.bmp");
	Widgets::TextureGL::load("data/analysishigh.bmp");
	Widgets::TextureGL::load("data/fft.bmp");
	Widgets::TextureGL::load("data/ffthigh.bmp");

	Widgets::TextureGL::load("data/save.bmp");
	Widgets::TextureGL::load("data/savehigh.bmp");

	Widgets::TextureGL::load("data/pin.bmp");
	Widgets::TextureGL::load("data/threshpin.bmp");
	Widgets::TextureGL::load("data/dropdown.bmp");
	Widgets::TextureGL::load("data/gainup.bmp");
	Widgets::TextureGL::load("data/gaindown.bmp");

	Widgets::TextureGL::load("data/ckboxon.bmp");
	Widgets::TextureGL::load("data/ckboxoff.bmp");

	Widgets::TextureGL::load("data/plotview.bmp");
	Widgets::TextureGL::load("data/plotviewhigh.bmp");
	Widgets::TextureGL::load("data/plotviewdown.bmp");
	Widgets::TextureGL::load("data/plotviewdownhigh.bmp");
	Widgets::TextureGL::load("data/disconnected.bmp");
	Widgets::TextureGL::load("data/connected.bmp");
    Widgets::TextureGL::load("data/usbcon.bmp");
    Widgets::TextureGL::load("data/usbconhigh.bmp");
    Widgets::TextureGL::load("data/usbdiscon.bmp");
    Widgets::TextureGL::load("data/usbdisconhigh.bmp");
	Widgets::TextureGL::load("data/heart.bmp");
	Widgets::TextureGL::load("data/speaker.bmp");
	Widgets::TextureGL::load("data/speakeroff.bmp");
	Widgets::TextureGL::load("data/bncconn.bmp");
	Widgets::TextureGL::load("data/bncconnhigh.bmp");
	Widgets::TextureGL::load("data/rtimer.bmp");
	Widgets::TextureGL::load("data/rtimerhigh.bmp");
    Widgets::TextureGL::load("data/devbrd.bmp");
    Widgets::TextureGL::load("data/devbrdhigh.bmp");
    Widgets::TextureGL::load("data/okbtn.bmp");
    Widgets::TextureGL::load("data/okbtnhigh.bmp");
    Widgets::TextureGL::load("data/trigger.bmp");
    Widgets::TextureGL::load("data/triggerhigh.bmp");
    Widgets::TextureGL::load("data/e1.bmp");
    Widgets::TextureGL::load("data/e2.bmp");
    Widgets::TextureGL::load("data/e3.bmp");
    Widgets::TextureGL::load("data/e4.bmp");
    Widgets::TextureGL::load("data/e5.bmp");
    Widgets::TextureGL::load("data/e6.bmp");
    Widgets::TextureGL::load("data/e7.bmp");
    Widgets::TextureGL::load("data/e8.bmp");
    Widgets::TextureGL::load("data/e9.bmp");
    
    Widgets::TextureGL::load("data/analysiscrossed.bmp");
    Widgets::TextureGL::load("data/fftcrossed.bmp");
    Widgets::TextureGL::load("data/threshcrossed.bmp");
    Widgets::TextureGL::load("data/configcrossed.bmp");
    
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
