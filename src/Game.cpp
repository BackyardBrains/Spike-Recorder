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
    roundingDifference = 0;
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
	Widgets::TextureGL::load("data/connected-high.bmp");
    Widgets::TextureGL::load("data/disconnected-high.bmp");
    Widgets::TextureGL::load("data/connected.bmp");

    Widgets::TextureGL::load("data/bdisconnected.bmp");
    Widgets::TextureGL::load("data/bconnected-high.bmp");
    Widgets::TextureGL::load("data/bdisconnected-high.bmp");
    Widgets::TextureGL::load("data/bconnected.bmp");


    Widgets::TextureGL::load("data/update.bmp");
    Widgets::TextureGL::load("data/update-high.bmp");
    Widgets::TextureGL::load("data/bupdate.bmp");
    Widgets::TextureGL::load("data/bupdate-high.bmp");

    Widgets::TextureGL::load("data/usbcon.bmp");
    Widgets::TextureGL::load("data/usbconhigh.bmp");
    Widgets::TextureGL::load("data/usbdiscon.bmp");
    
    Widgets::TextureGL::load("data/neuronprocon.bmp");
    Widgets::TextureGL::load("data/neuronproconhigh.bmp");
    Widgets::TextureGL::load("data/neuronprodiscon.bmp");
    
    Widgets::TextureGL::load("data/emgprocon.bmp");
    Widgets::TextureGL::load("data/emgproconhigh.bmp");
    Widgets::TextureGL::load("data/emgprodiscon.bmp");

    Widgets::TextureGL::load("data/plantcon.bmp");
    Widgets::TextureGL::load("data/plantconhigh.bmp");
    Widgets::TextureGL::load("data/plantdiscon.bmp");

    Widgets::TextureGL::load("data/musclecon.bmp");
    Widgets::TextureGL::load("data/muscleconhigh.bmp");
    Widgets::TextureGL::load("data/musclediscon.bmp");

    
    Widgets::TextureGL::load("data/neuroncon.bmp");
    Widgets::TextureGL::load("data/neurondiscon.bmp");
    
    Widgets::TextureGL::load("data/heartcon.bmp");
    Widgets::TextureGL::load("data/heartconhigh.bmp");
    Widgets::TextureGL::load("data/heartdiscon.bmp");

    Widgets::TextureGL::load("data/conne1.bmp");
    Widgets::TextureGL::load("data/conne2.bmp");
    Widgets::TextureGL::load("data/conne3.bmp");
    Widgets::TextureGL::load("data/conne4.bmp");
    Widgets::TextureGL::load("data/conne5.bmp");
    Widgets::TextureGL::load("data/conne6.bmp");

    Widgets::TextureGL::load("data/dconne1.bmp");
    Widgets::TextureGL::load("data/dconne2.bmp");
    Widgets::TextureGL::load("data/dconne3.bmp");
    Widgets::TextureGL::load("data/dconne4.bmp");
    Widgets::TextureGL::load("data/dconne5.bmp");
    Widgets::TextureGL::load("data/dconne6.bmp");

    Widgets::TextureGL::load("data/connn1.bmp");
    Widgets::TextureGL::load("data/connn2.bmp");
    Widgets::TextureGL::load("data/connn3.bmp");
    Widgets::TextureGL::load("data/connn4.bmp");
    Widgets::TextureGL::load("data/connn5.bmp");
    Widgets::TextureGL::load("data/connn6.bmp");
    
    Widgets::TextureGL::load("data/dconnn1.bmp");
    Widgets::TextureGL::load("data/dconnn2.bmp");
    Widgets::TextureGL::load("data/dconnn3.bmp");
    Widgets::TextureGL::load("data/dconnn4.bmp");
    Widgets::TextureGL::load("data/dconnn5.bmp");
    Widgets::TextureGL::load("data/dconnn6.bmp");

    Widgets::TextureGL::load("data/connm1.bmp");
    Widgets::TextureGL::load("data/connm2.bmp");
    Widgets::TextureGL::load("data/connm3.bmp");
    Widgets::TextureGL::load("data/connm4.bmp");
    Widgets::TextureGL::load("data/connm5.bmp");
    Widgets::TextureGL::load("data/connm6.bmp");

    Widgets::TextureGL::load("data/dconnm1.bmp");
    Widgets::TextureGL::load("data/dconnm2.bmp");
    Widgets::TextureGL::load("data/dconnm3.bmp");
    Widgets::TextureGL::load("data/dconnm4.bmp");
    Widgets::TextureGL::load("data/dconnm5.bmp");
    Widgets::TextureGL::load("data/dconnm6.bmp");

    Widgets::TextureGL::load("data/connp1.bmp");
    Widgets::TextureGL::load("data/connp2.bmp");
    Widgets::TextureGL::load("data/connp3.bmp");
    Widgets::TextureGL::load("data/connp4.bmp");
    Widgets::TextureGL::load("data/connp5.bmp");
    Widgets::TextureGL::load("data/connp6.bmp");

    Widgets::TextureGL::load("data/dconnp1.bmp");
    Widgets::TextureGL::load("data/dconnp2.bmp");
    Widgets::TextureGL::load("data/dconnp3.bmp");
    Widgets::TextureGL::load("data/dconnp4.bmp");
    Widgets::TextureGL::load("data/dconnp5.bmp");
    Widgets::TextureGL::load("data/dconnp6.bmp");

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
    Widgets::TextureGL::load("data/e10.bmp");

    Widgets::TextureGL::load("data/analysiscrossed.bmp");
    Widgets::TextureGL::load("data/fftcrossed.bmp");
    Widgets::TextureGL::load("data/threshcrossed.bmp");
    Widgets::TextureGL::load("data/configcrossed.bmp");
    Widgets::TextureGL::load("data/calibratebtn-high.bmp");
    Widgets::TextureGL::load("data/cancel-high.bmp");
    Widgets::TextureGL::load("data/calibratebtn-normal.bmp");
    Widgets::TextureGL::load("data/cancel-normal.bmp");
    Widgets::TextureGL::load("data/okbtn-normal.bmp");
    Widgets::TextureGL::load("data/okbtn-high.bmp");
    
    Widgets::TextureGL::load("data/p300normal.bmp");
    Widgets::TextureGL::load("data/p300high.bmp");
    Widgets::TextureGL::load("data/p300selected.bmp");
    Widgets::TextureGL::load("data/p300audio-normal.bmp");
    Widgets::TextureGL::load("data/p300audio-high.bmp");
    Widgets::TextureGL::load("data/p300audio-selected.bmp");
    
    Widgets::TextureGL::load("data/preset-neuron.bmp");
    Widgets::TextureGL::load("data/preset-plant.bmp");
    Widgets::TextureGL::load("data/preset-eeg.bmp");
    Widgets::TextureGL::load("data/preset-ecg.bmp");
    Widgets::TextureGL::load("data/preset-emg.bmp");
    Widgets::TextureGL::load("data/preset-neuron-high.bmp");
    Widgets::TextureGL::load("data/preset-plant-high.bmp");
    Widgets::TextureGL::load("data/preset-eeg-high.bmp");
    Widgets::TextureGL::load("data/preset-ecg-high.bmp");
    Widgets::TextureGL::load("data/preset-emg-high.bmp");

}

void Game::advance() {
	static uint64_t t = 0; // TODO make this cleaner
	uint64_t newt = SDL_GetTicks();
    double timediff = (newt-t);
   // std::cout<<"Time: "<<timediff<<"\n";
    double sampleNumber = (double)(_manager.sampleRate())*(timediff/1000.0) + roundingDifference;
    double trimmedNumber = (double)(int64_t)sampleNumber;
    roundingDifference = sampleNumber-trimmedNumber;
    _manager.advance((int32_t)sampleNumber); // fetch more samples than necessary to prevent lag
	_fileRec.advance();

	t = newt;
}



} // namespace BackyardBrains
