#include "FFTView.h"
#include "AudioView.h"
#include "engine/RecordingManager.h"
#include "engine/FFTBackend.h"
#include "widgets/Application.h"
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include <cmath>
#include <iostream>
#include <cassert>

#include <SDL.h>
namespace BackyardBrains {

static void val2hue(uint8_t *p, double val) {
	val *= 6;
	double m = val-((int)(val));
	double q = 255*(m);
	double t = 255*(1-m);
	
	assert(!isnan(val) && !isinf(val));
	if(isnan(val)) {
		printf("nan!!\n");
		p[0] = 0;
		p[1] = 0;
		p[2] = 0;
	}
	switch(((int)val)%6) {
	case 3:
		p[0] = 255;
		p[1] = t;
		p[2] = 0;
		break;
	case 2:
		p[0] = q;
		p[1] = 255;
		p[2] = 0;
		break;
	case 1:
		p[0] = 0;
		p[1] = 255;
		p[2] = t;
		break;
	case 0:
		p[0] = 0;
		p[1] = q;
		p[2] = 255;
		break;
	case 5:
		p[0] = 255;
		p[1] = q;
		p[2] = 255;
		break;
	case 4:
		p[0] = 255;
		p[1] = 0;
		p[2] = q;
		break;
	}

	p[3] = 255;
}

FFTView::FFTView(AudioView &av, RecordingManager &manager, Widget *parent) : Widget(parent), _active(0),
	_startTime(-2000), _manager(manager), _av(av)
	 {
	setSizeHint(Widgets::Size());
	setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Fixed));

	_samplebuf = new int16_t[SWINDOW];
	_fftbuf.reserve(SWINDOW);
	_ffttex = 0;
	_viewwidth = FFTTRES;
	memset(_fftviewbuffer,0, sizeof(_fftviewbuffer));	
}

static void init_texture(GLuint &tex) {
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindTexture(GL_TEXTURE_2D, 0);
}

FFTView::~FFTView() {
	delete[] _samplebuf;
	glDeleteTextures(1, &_ffttex);
}

void FFTView::setActive(bool active) {
	_startTime = SDL_GetTicks();
	_active = active;
	update(1);
}

bool FFTView::active() const {
	return _active;
}

void FFTView::paintEvent() {
	if(sizeHint().h == 0)
		return;
	
	if(_ffttex == 0)
		init_texture(_ffttex);

	Widgets::Painter::setColor(Widgets::Colors::white);
	
	int xoff = AudioView::MOVEPIN_SIZE*1.48f;
	int w = _av.screenWidth();

	Widgets::Rect r1(xoff,0,
			w*FFTTRES/(float)_viewwidth,height());
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	float texoff = _offset/(float)FFTTRES;

	glTranslatef(texoff,0,0);
	glMatrixMode(GL_MODELVIEW);
	glBindTexture(GL_TEXTURE_2D, _ffttex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FFTTRES, FFTFRES, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, _fftviewbuffer);
		
	Widgets::Painter::drawTexRect(r1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void FFTView::addWindow(uint32_t *result, int pos, int device, int len, int samplerate) {
	_manager.getData(device, pos, len, _samplebuf);

	int ds = 4; // simple downsampling because only low frequencies are required
	_fftbuf.resize(len/ds);
	for(int i = 0; i < len/ds; i++)
		_fftbuf[i] = _samplebuf[ds*i];

	float windowt = len/(float)samplerate;
	float binsize = windowt*FFTMAXF/(float)FFTFRES;

	FFTBackend::transform(_fftbuf);
	for(int i = 0; i < FFTFRES; i++) {
		float f = i*FFTMAXF/(float)FFTFRES;

		int lower = f*windowt;
		int upper = std::min((int)(f*windowt+binsize+1), len/ds-1);

		double max = creal(_fftbuf[lower]);
		for(int j = lower; j <= upper; j++) {
			if(cabs(_fftbuf[j]) > max) {
				max = cabs(_fftbuf[j]);
			}
		}
		
		double val = tanh(2e-5*max);
		val2hue((uint8_t *)&result[FFTFRES-1-i],val);
	}	

	for(int i = 1; i < FFTFRES-1; i++) {
		uint8_t *p = (uint8_t *)&result[i];
		uint8_t *pm = (uint8_t *)&result[i-1];
		uint8_t *pp = (uint8_t *)&result[i+1];
		for(int j = 0; j < 3; j++) {
			p[j] = (pm[j]+2*p[j]+pp[j])/4.;
		}
	}

}

void FFTView::update(int force) {
	int len = _av.sampleCount(_av.screenWidth(), _av.scaleWidth());
	int pos = _manager.pos()+_av.channelOffset()-len+_manager.fileMode()*len/2;
	uint32_t resultbuf[FFTFRES];
	int windowdist = SWINDOW;

	if(_laststate != len) {
		force = true;
		_laststate = len;
	}

	_viewwidth = len/SWINDOW;
	if(_viewwidth > FFTTRES) {
		windowdist = SWINDOW*_viewwidth/(float)FFTTRES;
		_viewwidth = FFTTRES;
	}

	pos = pos/windowdist*windowdist;

	if(force) { // overwrite all ffts
		_lastfirst = -1;
		_lastlast = -1;
		_offset = 0;
	}
		
	_offset += (pos-_lastfirst)/(float)windowdist;
	for(int i = 0; i < _viewwidth; i++) {
		int spos = pos+i*windowdist;
		if(spos >= _lastfirst && spos < _lastlast) { // already computed
			continue;
		}
		addWindow(resultbuf, pos+i*windowdist, _av.channelVirtualDevice(_av.selectedChannel()),
				SWINDOW, _manager.sampleRate());
		for(int j = 0; j < FFTFRES; j++) {
			int idx = (((i+(int)_offset)%FFTTRES)+FFTTRES)%FFTTRES;
			_fftviewbuffer[j][idx] = resultbuf[j];
		}
	}

	_lastfirst = pos;
	_lastlast = pos+_viewwidth*windowdist;
}

void FFTView::advance() {
	int t = SDL_GetTicks()-_startTime;
	if((t > 0 && t < 1300) || sizeHint().h > 0) {
		if(_active) {
			setSizeHint(Widgets::Size(sizeHint().w, 200.f*std::tanh(t/500.f)));
			Widgets::Application::getInstance()->updateLayout();
		} else {
			setSizeHint(Widgets::Size(sizeHint().w, 200.f-200.f*std::tanh(t/500.f)));
			Widgets::Application::getInstance()->updateLayout();
		}
	}

	if(!_active)
		return;

	//if(abs(_manager.pos()+_av.channelOffset()-_pos) >= SWINDOW) {
	update(false);
		
}

}
