#ifndef BACKYARDBRAINS_FFTVIEW_H
#define BACKYARDBRAINS_FFTVIEW_H

#include "widgets/Widget.h"
#include <stdint.h>
#include <vector>
#include <utility>
#include <complex>
#include <SDL_opengl.h>

namespace BackyardBrains {

class RecordingManager;
class AudioView;

class FFTView : public Widgets::Widget {
public:
	FFTView(AudioView &av, RecordingManager &manager, Widget *parent = NULL);
	virtual ~FFTView();
	bool active() const;
	void setActive(bool);
	void update(int force);
private:
	static const int FFTTRES = 256; // time axis resolution
	static const int FFTFRES = 32; // frequency axis resolution
	static const int FFTMAXF = 50;
	
	static const int SWINDOW = 4096; // window length in samples
	
	static const int OFFSCREENWINS = 2; // number of windows to compute in advance
	// Frequency scale properties
	static const int SCALETICKS = 4;

	int16_t *_samplebuf;
	std::vector<std::complex<float> > _fftbuf;

	int32_t _fftviewbuffer[FFTFRES][FFTTRES];
	int _viewwidth;

	// variables to handle forced refreshes
	int _laststate;
	int _lastfirst;
	int _lastlast;
	
	int _offset;
	float _offsetcorrection;

	int _active;
	int _startTime;

	RecordingManager &_manager;
	AudioView &_av;

	GLuint _ffttex;

	void addWindow(uint32_t *result, int pos, int device, int len, int samplerate);
	void drawDataRect() const;
	void drawTooShortMsg() const;
	void drawScale() const;
	void initTextures();	
	void paintEvent();
	void glResetEvent();
	void advance();
};

}

#endif
