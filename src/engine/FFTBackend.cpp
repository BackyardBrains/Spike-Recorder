#include "FFTBackend.h"
#include <cassert>
#include <cmath>
#include <complex>
#include <vector>

namespace BackyardBrains {


static void permute(std::vector<std::complex<float> > &data) {
	int n = data.size();
	if(n <= 2)
		return;
	int r = 0;
	for(int x = 1; x < n; x++) {
		for(unsigned int m = n >>1; (!((r^=m)&m)); m>>=1);
		if(r > x) {
			std::complex<float> tmp = data[x];
			data[x] = data[r];
			data[r] = tmp;
		}
	}
}

void FFTBackend::transform(std::vector<std::complex<float> > &data) { // Cooley-Turkey FFT
	assert(!(data.size() == 0) && !(data.size() & (data.size()-1))); // must be power of two

	permute(data);

	int r = 1;
	while(data.size()>>r !=1)
		r++;
	for(int e = 0; e < r; e++) {
		for(int n = 0; n < 1<<(r-e-1); n++) {
			for(int i = 0; i < 1<<e; i++) {
				std::complex<float> f = std::exp(std::complex<float>(0.,-M_PI*i/(1<<e)));
				assert(f == f);
				int idx1 = (1<<(e+1))*n+i;
				int idx2 = idx1+(1<<e);

				std::complex<float> c1 = data[idx1];
				std::complex<float> c2 = data[idx2];

				data[idx1] = c1 + f*c2;
				data[idx2] = c1 - f*c2;
			}
		}
	}
}

}
