#ifndef BACKYARDBRAINS_FFTBACKEND_H
#define BACKYARDBRAINS_FFTBACKEND_H

#include <stdint.h>
#include <vector>
#include <utility>
#include <complex.h>

namespace BackyardBrains {

class FFTBackend {
public:

	//FFTBackend();
	//~FFTBackend();
	static void transform(std::vector<complex float> &data); // inplace fft

};

}

#endif
