#include "SpikeSorter.h"
#include <cstring>
#include <iostream>
#include <bass.h>

namespace BackyardBrains {

const std::vector<std::pair<int64_t, int16_t> > &SpikeSorter::spikes() const {
	return _spikes;
}

static int16_t convert_bytedepth(int8_t *pos, int bytes) {
	int16_t res;
	if(bytes == 1) { // unsigned 8 bit format
		res = (*pos-128)<<7;
	} else { // else assume signedness and take the 2 most significant bytes
		memcpy(&res, pos, 2);
	}

	return res;
}

void SpikeSorter::searchPart(int8_t *buffer, int size, int chan, int channels, int bytedepth, int threshold, int holdoff, int64_t toffset) {
	for(int i = 0; i < size/channels/bytedepth; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth);

		if((val > threshold || val < -threshold) && (_spikes.empty() || toffset+i-_spikes.back().first > holdoff)) {
			if(_spikes.size() >= _spikes.capacity())
				_spikes.reserve(_spikes.capacity()*2);

			bool positive = val > threshold;

			int16_t peakval = val;
			int64_t peakpos = i;

			for(int j = 0; j < holdoff; j++) {
				if(i+j >= size/channels/bytedepth)
					break;

				const int16_t nval = convert_bytedepth(&buffer[((i+j)*channels+chan)*bytedepth], bytedepth);
				if(positive ? nval > peakval : nval < peakval) {
					peakval = nval;
					peakpos = i+j;
				}
			}

			i = peakpos+holdoff;

			_spikes.push_back(std::make_pair(toffset+peakpos, peakval));
		}
	}
}

void SpikeSorter::findSpikes(const std::string &filename, int channel, int threshold, int holdoff) {
	std::cout << "Looking for spikes...\n";

	HSTREAM handle = BASS_StreamCreateFile(false, filename.c_str(), 0, 0, BASS_STREAM_DECODE);
	if(handle == 0) {
		std::cerr << "Bass Error: Failed to load file '" << filename << "': " << BASS_ErrorGetCode() << "\n";
		return;
	}

	int8_t buffer[BUFSIZE];

	BASS_CHANNELINFO info;

	BASS_ChannelGetInfo(handle, &info);
	int bytespersample = info.origres/8;


	_spikes.reserve(256);

	int64_t left = BASS_ChannelGetLength(handle, BASS_POS_BYTE);
	int64_t pos = 0;
	while(left > 0) {
		DWORD bytesread = BASS_ChannelGetData(handle, buffer, std::min(left,(int64_t)BUFSIZE));
		if(bytesread == (DWORD)-1) {
			std::cerr << "Bass Error: getting channel data failed: " << BASS_ErrorGetCode() << "\n";
			break;
		}

		searchPart(buffer, bytesread, channel, info.chans, bytespersample, threshold, holdoff, pos);
		pos += bytesread/info.chans/bytespersample;
		left -= bytesread;
	}

	for(unsigned int i = 0; i < _spikes.size(); i++) {
		std::cout << _spikes[i].first << " " <<_spikes[i].second << "\n";
	}

	BASS_StreamFree(handle);
}

void SpikeSorter::freeSpikes() {
	_spikes.clear();
}

}
