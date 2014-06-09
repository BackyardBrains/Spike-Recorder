#ifndef BACKYARDBRAINS_SPIKESORTER_H
#define BACKYARDBRAINS_SPIKESORTER_H

#include <string>
#include <vector>
#include <stdint.h>

namespace BackyardBrains {

class RecordingManager;

class SpikeSorter {
public:
	const std::vector<std::pair<int64_t, int16_t> > &spikes() const;
	void findSpikes(const std::string &filename, int channel, int holdoff);
	void freeSpikes();
private:
	static const int BUFSIZE = 1024;

	void searchPart(int8_t *buffer, int size, int chan, int channels, int bytedepth, int threshold, int holdoff, int64_t toffset);
	int findThreshold(int handle, int channel, int channels, int bytespersample);
	double calcRMS(int8_t *buffer, int size, int chan, int channels, int bytedepth);
	std::vector<std::pair<int64_t, int16_t> > _spikes;
};

}

#endif
