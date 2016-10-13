#include "FFTBackend.h"
#include "RecordingManager.h"
#include <cassert>
#include <cmath>
#include <complex>
#include <vector>


namespace BackyardBrains {
    
    FFTBackend::FFTBackend(RecordingManager &manager) : _manager(manager) {
        _samplebuf = new int16_t[SWINDOW];
        _fftbuf.reserve(SWINDOW);
        
        _begin = 0;
        _end = 0;
        _viewwidth = 0;
        _offset = 0;
        _device = 0;
        lowPassAlphaWaves = 0;
        
        
    }
    
    FFTBackend::~FFTBackend() {
        delete[] _samplebuf;
    }
    
    
    void FFTBackend::updateAlphawave( int device, int samplerate )
    {
        _manager.getData(device, _manager.pos()-SWINDOW-1, SWINDOW, _samplebuf);
        // simple downsampling because only low frequencies are required
        const int ds = 4;
        _fftbuf.resize(SWINDOW/ds);
        for(int i = 0; i < SWINDOW/ds; i++)
            _fftbuf[i] = _samplebuf[ds*i];
        
        float windowt = SWINDOW/(float)samplerate;
        float binsize = windowt*FFTMAXF/(float)FFTFRES;
        
        transform(_fftbuf);
        double alphapower = 1;
        // resampling the result to have exactly FFTFRES bins
        for(int i = 0; i < FFTFRES; i++) {
            float f = i*FFTMAXF/(float)FFTFRES;
            
            int lower = f*windowt;
            int upper = std::min((int)(f*windowt+binsize+1), SWINDOW/ds-1);
            
            double max = std::abs(_fftbuf[lower]);
            for(int j = lower; j <= upper; j++) {
                if(std::abs(_fftbuf[j]) > max) {
                    max = std::abs(_fftbuf[j]);
                }
            }

            //calculating power of alpha waves
            if(f>9.0 && f<11.0)
            {
                alphapower = max;
            }
        }
         lowPassAlphaWaves = 0.98*lowPassAlphaWaves+0.02*alphapower;
    }
    
    
    void FFTBackend::addWindow(float *result, int pos, int device, int len, int samplerate, bool calculateAlphaPower) {
        _manager.getData(device, pos, len, _samplebuf);
        
        // simple downsampling because only low frequencies are required
        const int ds = 4;
        _fftbuf.resize(len/ds);
        for(int i = 0; i < len/ds; i++)
            _fftbuf[i] = _samplebuf[ds*i];
        
        float windowt = len/(float)samplerate;
        float binsize = windowt*FFTMAXF/(float)FFTFRES;
        
        transform(_fftbuf);
        
      //  double alphapower = 1;
        // resampling the result to have exactly FFTFRES bins
        for(int i = 0; i < FFTFRES; i++) {
            float f = i*FFTMAXF/(float)FFTFRES;
            
            int lower = f*windowt;
            int upper = std::min((int)(f*windowt+binsize+1), len/ds-1);
            
            double max = std::abs(_fftbuf[lower]);
            for(int j = lower; j <= upper; j++) {
                if(std::abs(_fftbuf[j]) > max) {
                    max = std::abs(_fftbuf[j]);
                }
            }
            
            
            result[FFTFRES-1-i] = max/1.0;
            //calculating power of alpha waves
           // if(f>9.0 && f<11.0 && calculateAlphaPower)
           // {
           //     alphapower = max;
           // }
           

        }
        /*if(fortyHzPower<1.0)
        {
            fortyHzPower = 1.0;
        }*/
        
        //lowPassAlphaWaves = 0.98*lowPassAlphaWaves+0.02*alphapower;
        //std::cout<< lowPassAlphaWaves<<"   "<<alphapower<<"\n";
        // smoothing filter on the result in the frequency domain
        for(int i = 1; i < FFTFRES-1; i++) {
            uint8_t *p = (uint8_t *)&result[i];
            uint8_t *pm = (uint8_t *)&result[i-1];
            uint8_t *pp = (uint8_t *)&result[i+1];
            for(int j = 0; j < 3; j++) {
                p[j] = (pm[j]+2*p[j]+pp[j])/1.;
            }
        }
        
    }
    
    void FFTBackend::force() {
        // this will force the view to discard the cache
        
        _begin = -1;
        _end = -1;
        _offset = 0;
        _viewwidth = 0;
    }
    
    void FFTBackend::setDevice(int device) {
        if(_device != device) {
            _device = device;
            force();
        }
    }
    
    void FFTBackend::request(int64_t position, int length) {
        float resultbuf[FFTFRES];
        int windowdist = SWINDOW;
        
        _viewwidth = length/SWINDOW;
        if(_viewwidth > FFTTRES) {
            windowdist = length/FFTTRES;
            _viewwidth = FFTTRES;
        }
        
        int pos = (position/windowdist)*windowdist;//rounding to whole number of windows
        
        if(!_manager.isBufferLoadedAtPosition(pos+length))
        {
            return;
        }
        _offset += (pos-_begin)/windowdist;
        _offsetcorrection = (position-pos)/(float)windowdist;
        
        for(int i = 0; i < _viewwidth+OFFSCREENWINS; i++) {
        //for(int i = 0; i < _viewwidth; i++) {
            int spos = pos+i*windowdist;
            if(spos >= _begin && spos < _end) { // already computed
                continue;
            }
            addWindow(resultbuf, pos+i*windowdist, _device,
                      SWINDOW, _manager.sampleRate(), false);
            for(int j = 0; j < FFTFRES; j++) {
                int idx = (((i+(int)_offset)%FFTTRES)+FFTTRES)%FFTTRES;
                _fftcache[idx][j] = resultbuf[j];
            }
        }
        
        updateAlphawave(_device, _manager.sampleRate() );
        
        _begin = pos;
        _end = pos+(_viewwidth-1)*windowdist;
    }
    
    const float (*FFTBackend::fftcache() const)[FFTFRES] {
        return _fftcache;
    }
    
    const float *FFTBackend::fftcache_last() const {
        return _fftcache[(((_viewwidth+(int)_offset)%FFTTRES)+FFTTRES)%FFTTRES];
    }
    
    int FFTBackend::offset() const {
        return _offset;
    }
    
    float FFTBackend::fluidoffset() const {
        return _offset+_offsetcorrection;
    }
    
    int FFTBackend::viewwidth() const {
        return _viewwidth;
    }
    
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