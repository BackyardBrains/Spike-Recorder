#include "SpikeAnalysis.h"
#include "FileReadUtil.h"
#include "Log.h"
#include <cassert>
#include <cmath>

namespace BackyardBrains {

void SpikeAnalysis::crossCorrelation(std::vector<int> &histogram,
		const std::vector<std::pair<int64_t, int16_t> > &train1,
		const std::vector<std::pair<int64_t, int16_t> > &train2,
		int maxsamples,
		int binsize) {
	assert(binsize > 1);
	assert(maxsamples > 0);

	int n = ceil((2*maxsamples+binsize)/(float)binsize);
	histogram.resize(n);
	
	for(int i = 0; i < n; i++)
		histogram[i] = 0;

	int mindiff = -maxsamples-binsize/2;
	int maxdiff = maxsamples+binsize/2;

	for(unsigned int i = 0; i < train1.size(); i++) {
		int64_t t1 = train1[i].first;
		bool ininterval = false;

		for(unsigned int j = 0; j < train2.size(); j++) {
			int64_t t2 = train2[j].first;
			int64_t diff = t1-t2;
			if(diff > mindiff && diff < maxdiff) {
				ininterval = true;
				int idx = (diff-mindiff)/binsize;
				histogram[idx]++;
			} else if(ininterval) {
				break; // break after interval is over
			}
		}
	}
}

void SpikeAnalysis::autoCorrelation(std::vector<int> &histogram,
		const std::vector<std::pair<int64_t, int16_t> > &train,
		int maxsamples,
		int binsize) {
	int cutoff = maxsamples/binsize;
	std::vector<int> tmp;

	crossCorrelation(histogram, train, train, maxsamples, binsize);
	
	tmp.assign(histogram.begin()+cutoff, histogram.end());
	histogram = tmp;
}

void SpikeAnalysis::isiPartition(std::vector<double> &binxs, int nbin) {
	const double MIN = -3*M_LN10;
	const double MAX = 1*M_LN10;

	assert(nbin > 0);

	double delta = (MAX-MIN)/nbin;
	binxs.resize(nbin);
	for(int i = 0; i < nbin; i++)
		binxs[i] = exp(MIN+delta*i);
}

void SpikeAnalysis::isi(std::vector<int> &hist,
		const std::vector<std::pair<int64_t, int16_t> > &train,
		const std::vector<double> &binxs,
		int nbin, int samplerate) {
	assert(nbin > 0);
	assert(samplerate > 0);

	hist.resize(nbin);
	for(int i = 0; i < nbin; i++)
		hist[i] = 0;

	for(unsigned int i = 1; i < train.size(); i++) {
		double dt = (train[i].first-train[i-1].first)/(double)samplerate;
		for(int j = 0; j < nbin; j++) {
			if(dt > binxs[j]) {
				hist[j]++;
				break;
			}
		}
	}
}	

void SpikeAnalysis::averageWaveform(std::vector<double> &average, std::vector<double> &std,
		const std::vector<int64_t> &train, const char *filename, int chan) {
	const int BUFSIZE = 32768;
	HSTREAM handle;
	int chans, samplerate, bytespersample;
	bool rc;

	rc = OpenWAVFile(filename, handle, chans, samplerate, bytespersample); 
	assert(chan < chans);
	if(rc == false) {
		Log::error("Calculating average waveform cancelled.");
		return;
	}
	int size = 2*(int)(SPIKEHALFLEN*samplerate)+1;
	int64_t filelen = BASS_ChannelGetLength(handle, BASS_POS_BYTE)/bytespersample/chans-1;
	int64_t pos = 0;

	average.resize(size);
	std.resize(size);
	for(int i = 0; i < size; i++) {
		average[i] = 0;
		std[i] = 0;
	}

	std::vector<std::vector<int16_t> > channels;
	for(int i = 0; i < chans; i++)
		channels[i].reserve(BUFSIZE);

	int lastend = 0;
	int spiken = 0;

	while(pos < filelen) {
		int len = BUFSIZE;
		if(pos + len >= filelen)
			len = filelen-pos;
		rc = ReadWAVFile(channels, len, handle, chans, bytespersample);
		if(rc == false) {
			Log::error("Calculating average waveform cancelled due to reading error.");
			return;
		}
		
		for(unsigned int i = lastend; i < train.size(); i++) {
			if(train[i]-size/2 < pos)
				continue;
			if(train[i]+size/2 > pos+(int64_t)channels[chan].size()) {
				lastend = i;
				break;
			}
			for(int j = 0; j < size ; j++) {
				int16_t val = channels[chan][train[i]+j-size/2-pos];
				average[j] += val;
				std[j] += val*val;
			}
			spiken++;
		}
		pos += channels[chan].size();
	}

	for(int i = 0; i < size; i++) {
		average[i] /= spiken;
		std[i] /= spiken;
	}

	for(int i = 0; i < size; i++) {
		std[i] = sqrt(std[i]-average[i]*average[i]);
	}
}	
}
