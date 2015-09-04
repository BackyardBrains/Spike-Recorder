#include "FFTView.h"
#include "AudioView.h"
#include "Log.h"
#include "engine/RecordingManager.h"
#include "engine/FFTBackend.h"
#include "widgets/Application.h"
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include "widgets/BitmapFontGL.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

#include <SDL.h>
namespace BackyardBrains {

static void val2hue(uint8_t *p, double val) {
	if(val > 1.)
		val = 1.;
	val *= 4;
	double m = val-((int)(val));
	double q = 255*(m);
	double t = 255*(1-m);

	//assert(!isnan(val) && !isinf(val));
	switch(((int)val)%6) {
	case 0:
		p[0] = 0;
		p[1] = q;
		p[2] = 200;
		break;
	case 1:
		p[0] = 0;
		p[1] = 255;
		p[2] = t*200/255;
		break;
	case 2:
		p[0] = q;
		p[1] = 255;
		p[2] = 0;
		break;
	case 3:
		p[0] = 255;
		p[1] = t;
		p[2] = 0;
		break;
	case 4:
		p[0] = 255;
		p[1] = 0;
		p[2] = q;
		break;
	case 5:
		p[0] = 255;
		p[1] = q;
		p[2] = 255;
		break;
	}

	p[3] = 255;
}

FFTView::FFTView(AudioView &av, RecordingManager &manager, Widget *parent) : Widget(parent), _active(0),
	_startTime(-2000), _manager(manager), _av(av),_ffttex(0)
	 {
	setSizeHint(Widgets::Size());
	setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Fixed));

	_samplebuf = new int16_t[SWINDOW];
	_fftbuf.reserve(SWINDOW);
	_ffttex = 0;
	_viewwidth = FFTTRES;

	// prefill buffer with blue
	for(int f = 0; f < FFTFRES; f++) {
		for(int t = 0; t < FFTTRES; t++) {
			uint8_t *p = (uint8_t *)&_fftviewbuffer[f][t];
			p[0] = 0;
			p[1] = 0;
			p[2] = 255;
			p[3] = 255;
		}
	}
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
	if(_active != active) {
		_startTime = SDL_GetTicks();
		_active = active;
		update(true);
	}
}

bool FFTView::active() const {
	return _active;
}

void FFTView::drawDataRect() const {
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	float texoff = (_offset+_offsetcorrection)/(float)FFTTRES;

	int xoff = AudioView::DATA_XOFF;
	int w = _av.screenWidth();

	Widgets::Rect r(xoff,0,
			w*FFTTRES/(float)_viewwidth,height());

	glTranslatef(texoff,0,0);
	glMatrixMode(GL_MODELVIEW);
	glBindTexture(GL_TEXTURE_2D, _ffttex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FFTTRES, FFTFRES, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, _fftviewbuffer);

	Widgets::Painter::drawTexRect(r);
	glBindTexture(GL_TEXTURE_2D, 0);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void FFTView::drawScale() const {

	int x = 3;
	for(int i = 1; i < SCALETICKS+1; i++) {
		std::stringstream o;
		int y = height()*i/(float)(SCALETICKS+1);
		o << FFTMAXF-FFTMAXF*i/(float)(SCALETICKS+1) << " Hz -";
		Widgets::Application::font()->draw(o.str().c_str(), x, y, Widgets::AlignVCenter);
	}
}

void FFTView::drawTooShortMsg() const {
	const char *msg = "-- Time window too small for FFT --";
	Widgets::Application::font()->draw(msg, width()/2, height()/2, Widgets::AlignCenter);
}

void FFTView::paintEvent() {
	if(sizeHint().h == 0)
		return;

	if(_ffttex == 0)
		init_texture(_ffttex);

	Widgets::Painter::setColor(Widgets::Colors::white);

	if(_viewwidth != 0) {
		drawDataRect();
	} else {
		drawTooShortMsg();
	}

	drawScale();
}

void FFTView::glResetEvent() {
	if(_ffttex != 0)
		glDeleteTextures(1, &_ffttex);
	init_texture(_ffttex);
	update(1);
}

void FFTView::addWindow(uint32_t *result, int pos, int device, int len, int samplerate) {
	_manager.getData(device, pos, len, _samplebuf);

	const int ds = 4; // simple downsampling because only low frequencies are required
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

		double max = std::abs(_fftbuf[lower]);
		for(int j = lower; j <= upper; j++) {
			if(std::abs(_fftbuf[j]) > max) {
				max = std::abs(_fftbuf[j]);
			}
		}

		double val = tanh(2e-5*max); // TODO: replace this with something smarter
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
	int opos = _manager.pos()+_av.channelOffset()-len+_manager.fileMode()*len/2;
	uint32_t resultbuf[FFTFRES];
	int windowdist = SWINDOW;

	if(_av.selectedChannel() == -1) {
		memset(_fftviewbuffer,0,FFTTRES*FFTFRES*sizeof(int32_t));
		return;
	}

	if(_laststate != len) {
		force = true;
		_laststate = len;
	}

	_viewwidth = len/SWINDOW;
	if(_viewwidth > FFTTRES) {
		windowdist = len/FFTTRES;
		_viewwidth = FFTTRES;
	}

	int pos = opos/windowdist*windowdist;

	if(force) { // overwrite all ffts
		_lastfirst = -1;
		_lastlast = -1;
		_offset = 0;
	}

	_offset += (pos-_lastfirst)/windowdist;
	_offsetcorrection = (opos-pos)/(float)windowdist;

	for(int i = 0; i < _viewwidth+OFFSCREENWINS; i++) {
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
	_lastlast = pos+(_viewwidth-1)*windowdist;
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

	update(false);

}

}
