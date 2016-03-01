#include "AnalysisManager.h"
#include "RecordingManager.h"

namespace BackyardBrains {

AnalysisManager::AnalysisManager(RecordingManager &recman)
	: ekg(recman), fft(recman), _recman(recman) {
}

float AnalysisManager::calculateRMS(int vdevice, int64_t startsample, int64_t length) {
	float sum = 0.f;
	int16_t *data = new int16_t[length];
	_recman.getData(vdevice, startsample, length, data);

	for(int i = 0; i < length; i++) {
		sum += data[i]*data[i];
	}
	delete []data;

	if(length > 0) {
		sum /= length;
		return std::sqrt(sum);
	}

	return 0.f;
}

}
