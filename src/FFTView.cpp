#include "FFTView.h"
#include "AudioView.h"
#include "engine/RecordingManager.h"
#include "engine/AnalysisManager.h"
#include "widgets/Application.h"
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include "widgets/BitmapFontGL.h"
#include <cmath>
#include "Log.h"
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

FFTView::FFTView(AudioView &av, RecordingManager &manager, AnalysisManager &anaman, Widget *parent) : Widget(parent), _active(0),
	_startTime(-2000), _manager(manager), _anaman(anaman), _av(av),_ffttex(0)
	 {
	setSizeHint(Widgets::Size());
	setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Fixed));

	_ffttex = 0;
	
	// prefill buffer with blue
	for(int f = 0; f < FFTBackend::FFTFRES; f++) {
		for(int t = 0; t < FFTBackend::FFTTRES; t++) {
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
	glDeleteTextures(1, &_ffttex);
}

void FFTView::setActive(bool active) {
	if(_active != active) {
		_startTime = SDL_GetTicks();
		_active = active;
		_anaman.fft.force();
	}
}

bool FFTView::active() const {
	return _active;
}

void FFTView::drawDataRect() const {
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	float texoff = _anaman.fft.fluidoffset()/(float)FFTBackend::FFTTRES;

	int xoff = AudioView::DATA_XOFF;
	int w = _av.screenWidth();

	Widgets::Rect r(xoff,0, 
			w*FFTBackend::FFTTRES/(float)_anaman.fft.viewwidth(),height());

	glTranslatef(texoff,0,0);
	glMatrixMode(GL_MODELVIEW);
	glBindTexture(GL_TEXTURE_2D, _ffttex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FFTBackend::FFTTRES, FFTBackend::FFTFRES, 0,
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
		o << FFTBackend::FFTMAXF-FFTBackend::FFTMAXF*i/(float)(SCALETICKS+1) << " Hz -";
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

	if(_anaman.fft.viewwidth() != 0) {
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
    _anaman.fft.force();
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

	int len = _av.sampleCount(_av.screenWidth(), _av.scaleWidth());
	int opos = _manager.pos()+_av.channelOffset()-len+_manager.fileMode()*len/2;

	_anaman.fft.setDevice(_manager.selectedVDevice());
	_anaman.fft.request(opos,len);
	for(int f = 0; f < FFTBackend::FFTFRES; f++) {
		for(int t = 0; t < FFTBackend::FFTTRES; t++) {
			float val = tanh(2e-5*_anaman.fft.fftcache()[t][f]);
			val2hue((uint8_t *)&_fftviewbuffer[f][t],val);
		}
	}
}

}
