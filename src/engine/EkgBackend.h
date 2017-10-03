#ifndef BACKYARDBRAINS_EKGBACKEND_H
#define BACKYARDBRAINS_EKGBACKEND_H
#define NUMBER_OF_BEATS_TO_REMEMBER 100
#define NUMBER_OF_SEC_TO_AVERAGE 6
#include <sigslot.h>

namespace BackyardBrains {

class RecordingManager;

class EkgBackend : public sigslot::has_slots<> {
public:
	EkgBackend(RecordingManager &manager);

	float frequency() const; // in Hz
	void reset();
private:
    unsigned int beatTimestamps [NUMBER_OF_BEATS_TO_REMEMBER];
    int currentBeatIndex;
    int beatsCollected;
	float _frequency;
	unsigned int _lastTime;
    
	void beat();
};

}

#endif
