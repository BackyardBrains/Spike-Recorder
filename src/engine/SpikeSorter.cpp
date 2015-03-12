#include "SpikeSorter.h"
#include "Log.h"
#include "BASSErrors.h"
#include <cstring>
#include <bass.h>
#include <cmath>
#include <algorithm>

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

double SpikeSorter::calcRMS(int8_t *buffer, int size, int chan, int channels, int bytedepth) {
	double sum = 0;
	double mean = 0;
	const int nsamples = size/channels/bytedepth;

	for(int i = 0; i < nsamples; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth);
		mean += val;
	}

	mean /= nsamples;

	for(int i = 0; i < nsamples; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth)-mean;
		sum += val*val;
	}

	return sqrt(sum/nsamples);
}

void SpikeSorter::searchPart(int8_t *buffer, int size, int chan, int channels, int bytedepth, int threshold, int holdoff, int64_t toffset) {
	int trigger = 0;
	int16_t peakval = 0;
	int64_t peakpos = 0;
	const int nsamples = size/channels/bytedepth;
	std::vector<std::pair<int64_t, int16_t> > posspikes, negspikes;

	// looking for positive peaks
	for(int i = 0; i < nsamples; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth);

		if(trigger) {
			if(val < 0) {
				trigger = 0;
				posspikes.push_back(std::make_pair(toffset+peakpos, peakval));

			} else if(val > peakval) {
				peakval = val;
				peakpos = i;
			}
		} else if(val > threshold) {
			trigger = 1;
			peakval = val;
			peakpos = i;
		}
	}

	// looking for negative peaks
	for(int i = 0; i < nsamples; i++) {
		const int16_t val = convert_bytedepth(&buffer[(i*channels+chan)*bytedepth], bytedepth);
		if(trigger) {
			if(val > 0) {
				trigger = 0;
				negspikes.push_back(std::make_pair(toffset+peakpos, peakval));

			} else if(val < peakval) {
				peakval = val;
				peakpos = i;
			}
		} else if(val < -threshold) {
			trigger = 1;
			peakval = val;
			peakpos = i;
		}
	}

	std::vector<std::pair<int64_t, int16_t> >::iterator itp = posspikes.begin(), itn = negspikes.begin(), litp, litn;
	litn = itn; // last added positive
	litp = itp; // last added negative
	while(itp != posspikes.end() || itn != negspikes.end()) {
		int next = 1;
		if(itp == posspikes.end())
			next = 0;
		else if(itn != negspikes.end())
			next = itp->first < itn->first;


		if(next) {
			if((itp->second >= litp->second && (itp == posspikes.end()-1 || itp->second >= (itp+1)->second)) || itp->first-litp->first > holdoff) {
				_spikes.push_back(*itp);
				litp = itp;
			}

			itp++;
		} else {
			if((itn->second <= litn->second && (itn == negspikes.end()-1 || itn->second <= (itn+1)->second)) || itn->first-litn->first > holdoff) {
				_spikes.push_back(*itn);
				litn = itn;
			}
			
			itn++;
		}
	}
}

int SpikeSorter::findThreshold(int handle, int channel, int channels, int bytedepth) {
	int64_t left = BASS_ChannelGetLength(handle, BASS_POS_BYTE);
	int8_t buffer[BUFSIZE];

	std::vector<double> rms(left/channels/bytedepth/BUFSIZE);
	unsigned int i = 0;
	while(left > 0) {
		DWORD bytesread = BASS_ChannelGetData(handle, buffer, std::min(left,(int64_t)BUFSIZE));
		if(bytesread == (DWORD)-1) {
			Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
			break;
		}

		double r = calcRMS(buffer, bytesread, channel, channels, bytedepth);
		left -= bytesread;

		if(i == rms.size())
			rms.resize(2*rms.size());
		rms[i++] = r;
	}

	rms.resize(i+1);
	std::sort(rms.begin(),rms.end());
	BASS_ChannelSetPosition(handle, 0, BASS_POS_BYTE);
	return rms[rms.size()*4/10];
}

void SpikeSorter::findSpikes(const std::string &filename, int channel, int holdoff) {
	HSTREAM handle = BASS_StreamCreateFile(false, filename.c_str(), 0, 0, BASS_STREAM_DECODE);
	if(handle == 0) {
		Log::error("Bass Error: Failed to load file '%s': %s", filename.c_str(), GetBassStrError());
		return;
	}

	int8_t buffer[BUFSIZE];

	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(handle, &info);
	int bytespersample = info.origres/8;

	_spikes.reserve(256);

	int threshold = findThreshold(handle, channel, info.chans, bytespersample);

	int64_t left = BASS_ChannelGetLength(handle, BASS_POS_BYTE);
	int64_t pos = 0;
	while(left > 0) {
		DWORD bytesread = BASS_ChannelGetData(handle, buffer, std::min(left,(int64_t)BUFSIZE));
		if(bytesread == (DWORD)-1) {
			Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
			break;
		}

		searchPart(buffer, bytesread, channel, info.chans, bytespersample, threshold, holdoff, pos);
		pos += bytesread/info.chans/bytespersample;
		left -= bytesread;
	}

	BASS_StreamFree(handle);
}


void SpikeSorter::freeSpikes() {
	_spikes.clear();
}

}
