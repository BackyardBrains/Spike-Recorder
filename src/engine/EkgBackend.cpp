#include "EkgBackend.h"
#include "RecordingManager.h"

#include <SDL.h>

namespace BackyardBrains {
EkgBackend::EkgBackend(RecordingManager &manager) {
	reset();
	manager.triggered.connect(this,&EkgBackend::beat);
	manager.thresholdChanged.connect(this, &EkgBackend::reset);
    currentBeatIndex = 0;
    beatsCollected = 0;
}

void EkgBackend::reset() {
	_frequency = 0;
	_lastTime = 0;
    beatsCollected = 0;
    currentBeatIndex = 0;
}

float EkgBackend::frequency() const{
	return _frequency;
}

void EkgBackend::beat() {
	// Using SDL_GetTicks is dirty, but I don’t know if RecordingManager positions are reliable for this.
	unsigned int time = SDL_GetTicks();
    beatTimestamps[currentBeatIndex] = time;
    currentBeatIndex++;
    if(currentBeatIndex>NUMBER_OF_BEATS_TO_REMEMBER)
    {
        currentBeatIndex = 0;
    }
    beatsCollected++;
    if(beatsCollected>NUMBER_OF_BEATS_TO_REMEMBER)
    {
        beatsCollected = NUMBER_OF_BEATS_TO_REMEMBER;
    }
    
    int numberOfBeatsSummed = 0;
    float howMuchHistoryWeIncluded = 0;
    int indexOfLastSummed = currentBeatIndex-1;
    
    if(indexOfLastSummed <0)
    {
        indexOfLastSummed = NUMBER_OF_BEATS_TO_REMEMBER;
    }
    int indexOfPrevious;
    //calculate the average beat rate in last NUMBER_OF_SEC_TO_AVERAGE sec
    while((howMuchHistoryWeIncluded<NUMBER_OF_SEC_TO_AVERAGE) && (numberOfBeatsSummed<beatsCollected-1))
    {
        indexOfPrevious = indexOfLastSummed-1;
        if(indexOfPrevious<0)
        {
            indexOfPrevious = NUMBER_OF_BEATS_TO_REMEMBER;
        }
        float timeDiff = ((float)(beatTimestamps[indexOfLastSummed] - beatTimestamps[indexOfPrevious]))/1000.0f;
        if(timeDiff>2.5)
        {
            break;
        }
        howMuchHistoryWeIncluded += timeDiff;
        indexOfLastSummed = indexOfPrevious;
        numberOfBeatsSummed++;
    }
    
    float averageTime = 1.0;
    if(numberOfBeatsSummed>0)
    {
        averageTime = howMuchHistoryWeIncluded/(float)numberOfBeatsSummed;
        _frequency = 1.0f/averageTime;
    }
    else
    {
        _frequency = 0;
    }
    
    

	
	_lastTime = time;
}

}
