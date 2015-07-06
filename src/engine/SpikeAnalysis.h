#ifndef BACKYARDBRAINS_SPIKEANALYSIS_H
#define BACKYARDBRAINS_SPIKEANALYSIS_H

#include <vector>
#include <stdint.h>

namespace BackyardBrains {

class RecordingManager;

class SpikeAnalysis {
public:
	// computes histogram for values in [-maxtime-binsize/2, maxtime+binsize/2]
	//
	// maxsamples: maximal signal timeshift in samples
	// binsize: binsize of histogram
	static void crossCorrelation(std::vector<int> &histogram,
			const std::vector<int64_t> &train1,
			const std::vector<int64_t> &train2,
			int maxsamples,
			int binsize);

	// like crosscorrelation except only [-binsize/2, maxtime+binsize/2]
	static void autoCorrelation(std::vector<int> &histogram,
			const std::vector<int64_t> &train,
			int maxsamples,
			int binsize);

	// generates logarithmic bin partition for Interspike Interval Analysis.
	// always between 10^-3 and 10^1 s
	static void isiPartition(std::vector<double> &binxs, int nbin);

	// Inter Spike Interval Analysis
	//
	// histogram: output
	// train: train to analyse
	// binxs: bin partition to use (use isipartition() to generate)
	// nbin: number of bins

	static void isi(std::vector<int> &histogram,
			const std::vector<int64_t> &train,
			const std::vector<double> &binxs,
			int samplerate);

	// Average Waveform of spikes
	//
	// average: average of spikes, output
	// train: spike times
	// filename, chan: file and channel to use

	constexpr static double SPIKEHALFLEN = 0.002; // s
	static void averageWaveform(std::vector<float> &average, std::vector<float> &std,
			const std::vector<int64_t> &train, const char *filename, int chan);
};

}

#endif
