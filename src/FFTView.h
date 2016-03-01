#ifndef BACKYARDBRAINS_FFTVIEW_H
#define BACKYARDBRAINS_FFTVIEW_H

#include "widgets/Widget.h"
#include "engine/FFTBackend.h"
#include <stdint.h>
#include <vector>
#include <utility>
#include <complex>
#include <SDL_opengl.h>

namespace BackyardBrains {

class RecordingManager;
class AnalysisManager;
class AudioView;

class FFTView : public Widgets::Widget {
public:
	FFTView(AudioView &av, RecordingManager &manager, AnalysisManager &anaman, Widget *parent = NULL);
	virtual ~FFTView();
	bool active() const;
	void setActive(bool);
	void update();
private:
	
	// Frequency scale properties
	static const int SCALETICKS = 4;

	int32_t _fftviewbuffer[FFTBackend::FFTFRES][FFTBackend::FFTTRES];

	int _active;
	int _startTime;

	RecordingManager &_manager;
	AnalysisManager &_anaman;
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
