#include "AnalysisManager.h"
#include "RecordingManager.h"

namespace BackyardBrains {

AnalysisManager::AnalysisManager(RecordingManager &recman)
	: ekg(recman), fft(recman), _recman(recman) {
        
        
        _recman.bufferReset.connect(this, &AnalysisManager::resetBuffers);
}

float AnalysisManager::calculateRMS(int16_t *data, int length) {
	// mean is 0, so we do not need to substract mean^2
	float sum = 0.f;
	for(int i = 0; i < length; i++) {
		sum += data[i]*data[i];
	}

	if(length > 0) {
		sum /= length;
		return std::sqrt(sum);
	}

	return 0.f;
}
    
void AnalysisManager::resetBuffers()
{
    fft.force();
    std::cout<<"--------Buffer reset\n";
}

}
