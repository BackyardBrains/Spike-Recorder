#include "EkgBackend.h"
#include "RecordingManager.h"

#include <SDL.h>

namespace BackyardBrains {
EkgBackend::EkgBackend(RecordingManager &manager) {
	reset();
	manager.triggered.connect(this,&EkgBackend::beat);
	manager.thresholdChanged.connect(this, &EkgBackend::reset);
}

void EkgBackend::reset() {
	_frequency = 0;
	_lastTime = 0;
}

float EkgBackend::frequency() const{
	return _frequency;
}

void EkgBackend::beat() {
	// Using SDL_GetTicks is dirty, but I don’t know if RecordingManager positions are reliable for this.
	unsigned int time = SDL_GetTicks();
	float nfreq = 1000.f/(time-_lastTime);
	_frequency = nfreq;//0.3*(nfreq-_frequency); // exponential moving average. this may need tweaking
	_lastTime = time;
}

}
