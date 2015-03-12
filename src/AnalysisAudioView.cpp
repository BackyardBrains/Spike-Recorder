#include "AnalysisAudioView.h"
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include <SDL_opengl.h>
namespace BackyardBrains {

AnalysisAudioView::AnalysisAudioView(RecordingManager &manager, SpikeSorter &spikes, Widgets::Widget *parent) : AudioView(parent, manager), _spikes(spikes), _clickedThresh(-1) {
	_threshPos[0] = 0.3f;
	_threshPos[1] = 0.4f;
}

void AnalysisAudioView::drawTargetMarkers() {
	Widgets::Painter::setColor(MARKER_COLORS[1 % MARKER_COLOR_NUM]);
	Widgets::TextureGL::get("data/threshpin.png")->bind();
	for(int i = 0; i < 2; i++)
		Widgets::Painter::drawTexRect(Widgets::Rect(width()-MOVEPIN_SIZE*1.5f, _threshPos[i]*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
	glBindTexture(GL_TEXTURE_2D, 0);

	for(int i = 0; i < 2; i++)
		Widgets::Painter::drawRect(Widgets::Rect(MOVEPIN_SIZE*1.5f,_threshPos[i]*height(), width()-3*MOVEPIN_SIZE+3, 1));
}

int AnalysisAudioView::screenWidth() const {
	int screenw = width()-MOVEPIN_SIZE*3;
	return std::max(0,screenw);
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

	std::vector<std::pair<int16_t, int16_t> > data;
	int pos = _manager.pos()-samples/2;
	data = _manager.getSamplesEnvelope(_channels[0].virtualDevice, pos, samples, screenWidth() == 0 ? 1 : std::max(samples/screenWidth(),1));

	Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
	drawData(data, 0, samples, MOVEPIN_SIZE*1.5f, height()/2, screenWidth());

	drawMarkers();

	drawTargetMarkers();
	drawScale();

	Widgets::Painter::setColor(Widgets::Colors::white);
	if(!_channels.empty()) {
		for(unsigned int i = 0; i < _spikes.spikes().size(); i++) {
			int samplepos = _manager.pos()-_spikes.spikes()[i].first;
			if(samplepos < -samples/2 || samplepos > samples/2)
				continue;

			const int y = height()*_channels[0].pos - _spikes.spikes()[i].second*height()*ampScale*_channels[0].gain;
			const float x = MOVEPIN_SIZE*1.48f+screenWidth()*(samples/2-samplepos)/(float)samples;

			bool selected = y >= std::min(_threshPos[0], _threshPos[1])*height() && y <= std::max(_threshPos[0], _threshPos[1])*height();
			if(selected)
				Widgets::Painter::setColor(MARKER_COLORS[1 % MARKER_COLOR_NUM]);
			Widgets::Painter::drawRect(Widgets::Rect(x-1,y-1, 3, 3));
			if(selected)
				Widgets::Painter::setColor(Widgets::Colors::white);

		}
	}

	Widgets::TextureGL::get("data/pin.png")->bind();
	Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, _channels[0].pos*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
	glBindTexture(GL_TEXTURE_2D, 0);

	drawGainControls();
}

void AnalysisAudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::RightButton)
		return;
	if(event->button() == Widgets::LeftButton) {
		if(x < MOVEPIN_SIZE*1.5f && determineGainControlHover(x, y) == 0)
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
