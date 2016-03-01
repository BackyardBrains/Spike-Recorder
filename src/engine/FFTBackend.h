#ifndef BACKYARDBRAINS_FFTBACKEND_H
#define BACKYARDBRAINS_FFTBACKEND_H

#include <stdint.h>
#include <vector>
#include <utility>
#include <complex>

namespace BackyardBrains {

class RecordingManager;

class FFTBackend {
public:
	static const int FFTTRES = 256; // time axis resolution
	static const int FFTFRES = 16; // frequency axis resolution
	static const int FFTMAXF = 50;
	
	static const int SWINDOW = 4096; // window length in samples
	
	static const int OFFSCREENWINS = 2; // number of windows to compute in advance

	static void transform(std::vector<std::complex<float> > &data); // inplace fft

	FFTBackend(RecordingManager &manager);
	~FFTBackend();

	void setDevice(int device);

	// Request fftcache to show the ffts centered on position in a timewindow of length.
	// Both parameters are in samples.
	void request(int64_t position, int length);
	// Discard all cached data
	void force();

	// Before accessing fftcache, call request to make sure the data is up to date.
	const float (*fftcache() const)[FFTFRES];
	int offset() const;
	float fluidoffset() const; // sub integer precision
	int viewwidth() const;

	// Just get the most recent window
	const float *fftcache_last() const;

private:
	RecordingManager &_manager;

	void addWindow(float *result, int pos, int device, int len, int samplerate);

	int _device; // recording device to fetch data from

	int _begin; // first sample shown in the buffer
	int _end; // last sample shown in the buffer

	int _viewwidth; // time span covered by the whole buffer in window lengths

	int16_t *_samplebuf; // internal buffer for samples received from RecordingManager
	std::vector<std::complex<float> > _fftbuf; // internal buffer for storing a single fft window

	float _fftcache[FFTTRES][FFTFRES]; // cache for fft windows. circular in time
	int _offset; // circular offset of _fftcache 
	float _offsetcorrection; // exact sub-index fractional part of _offset
};

}

#endif
