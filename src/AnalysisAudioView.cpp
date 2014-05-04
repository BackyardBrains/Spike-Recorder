#include "AnalysisAudioView.h"
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include <SDL_opengl.h>
#include <iostream>
namespace BackyardBrains {

AnalysisAudioView::AnalysisAudioView(RecordingManager &manager, SpikeSorter &spikes, Widgets::Widget *parent) : AudioView(parent, manager), _spikes(spikes), _clickedThresh(-1) {
	_threshPos[0] = 0.3f;
	_threshPos[1] = 0.4f;
}

void AnalysisAudioView::drawTargetMarkers() {
	Widgets::Painter::setColor(Widgets::Color(80,140,180));
	Widgets::TextureGL::get("data/threshpin.png")->bind();
	for(int i = 0; i < 2; i++)
		Widgets::Painter::drawTexRect(Widgets::Rect(width()-MOVEPIN_SIZE*1.5f, _threshPos[i]*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
	glBindTexture(GL_TEXTURE_2D, 0);

	for(int i = 0; i < 2; i++)
		Widgets::Painter::drawRect(Widgets::Rect(MOVEPIN_SIZE*1.5f,_threshPos[i]*height(), width()-3*MOVEPIN_SIZE+3, 1));
}


int AnalysisAudioView::relPosToAmp(float rpos) const {
	return (_channels[0].pos - rpos)/ampScale/_channels[0].gain;
}

int AnalysisAudioView::upperThresh() const {
	return relPosToAmp(std::min(_threshPos[0], _threshPos[1]));
}

int AnalysisAudioView::lowerThresh() const {
	return relPosToAmp(std::max(_threshPos[0], _threshPos[1]));
}

void AnalysisAudioView::paintEvent() {
	const int samples = sampleCount(screenWidth(), scaleWidth());

	drawAudio();
	Widgets::Color bg = Widgets::Colors::background;
	bg.a = 200;
	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect().outset(150));

	drawMarkers();

	drawTargetMarkers();
	drawScale();

	if(!_channels.empty()) {
		Widgets::Painter::setColor(Widgets::Color(100,200,255));
		for(unsigned int i = 0; i < _spikes.spikes().size(); i++) {
			int samplepos = _spikes.spikes()[i].first - _manager.pos();
			if(samplepos < -samples/2 || samplepos > samples/2)
				continue;

			const int y = height()*_channels[0].pos - _spikes.spikes()[i].second*height()*ampScale*_channels[0].gain;
			const float x = width()+screenWidth()*(samplepos-samples/2)/(float)samples;


			Widgets::Painter::drawRect(Widgets::Rect(x-1,y-1, 3, 3));
		}
	}


}

void AnalysisAudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::RightButton)
		return;
	if(event->button() == Widgets::LeftButton) {
		if(x < MOVEPIN_SIZE*1.5f)
			return;

		if(_clickedThresh == -1 && x > width() - MOVEPIN_SIZE*1.5f) {
			for(int i = 0; i < 2; i++) {
				if(determineThreshHover(x, y, _threshPos[i]*height(), &_clickOffset)) {
					_clickedThresh = i;
					event->accept();
					return;
				}
			}
		}
	}

	AudioView::mousePressEvent(event);
}

void AnalysisAudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(_clickedThresh != -1) {
		_threshPos[_clickedThresh] = std::min(1.f, std::max(0.f, (event->pos().y-_clickOffset)/(float)height()));
		event->accept();
	}

	AudioView::mouseMotionEvent(event);
}

void AnalysisAudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		_clickedThresh = -1;
	}

	AudioView::mouseReleaseEvent(event);
}

}
