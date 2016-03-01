#ifndef BACKYARDBRAINS_EKGBACKEND_H
#define BACKYARDBRAINS_EKGBACKEND_H

#include <sigslot.h>

namespace BackyardBrains {

class RecordingManager;

class EkgBackend : public sigslot::has_slots<> {
public:
	EkgBackend(RecordingManager &manager);

	float frequency() const; // in Hz
	void reset();
private:
	float _frequency;
	unsigned int _lastTime;

	void beat();
};

}

#endif
