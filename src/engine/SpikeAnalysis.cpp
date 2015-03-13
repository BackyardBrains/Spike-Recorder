#include "SpikeAnalysis.h"
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

void averageWaveform(std::vector<double> &average, std::vector<double> &std,
	const std::vector<int64_t> &train, RecordingManager &manager) {

}	
}
