#include "FileReadUtil.h"
#include "Log.h"
#include "BASSErrors.h"
#include <bass.h>
#include <cassert>
#include <cstring>

namespace BackyardBrains {

bool OpenWAVFile(const char *file, HSTREAM &handle, int &nchan, int &samplerate, int &bytespersample) {
	handle = BASS_StreamCreateFile(false, file, 0, 0, BASS_STREAM_DECODE);
	if(handle == 0) {
		Log::error("Bass Error: Failed to load file '%s': %s", file, GetBassStrError());
		return false;
	}

	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(handle, &info);

	bytespersample = info.origres/8;
	if(bytespersample == 0) {
		Log::error("Bass Error: Failed to load file '%s': strange bytes per sample", file);
        bytespersample = 2;
		//return false;
	}
	if(bytespersample >= 3)
		bytespersample = 4; // bass converts everything it doesn’t support.

	nchan = info.chans;
	samplerate = info.freq;

	return true;
}

bool ReadWAVFile(std::vector<std::vector<int16_t> > &channels, int len, HSTREAM handle, int nchan, int bytespersample) {
	channels.resize(nchan);
	uint8_t *buffer = new uint8_t[len];
	DWORD samplesRead = BASS_ChannelGetData(handle, buffer, len);
	if(samplesRead == (DWORD)-1) {
		Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
		delete[] buffer;
		return false;
	}

	samplesRead /= bytespersample;

	for(int chan = 0; chan < nchan; chan++)
		channels[chan].resize(samplesRead/nchan);

	// de-interleave the channels
	for(DWORD i = 0; i < samplesRead/nchan; i++) {
		for(int chan = 0; chan < nchan; chan++) {
			if(bytespersample == 1) { // unsigned 8 bit format
				channels[chan][i] = (buffer[i*nchan + chan]-128)<<7;
			} else { // else assume signedness and take the 2 most significant bytes
				memcpy(&channels[chan][i], buffer+(i*nchan + chan)*bytespersample, 2);
			}
		}
	}
	delete[] buffer;
	return true;
}
	
}

