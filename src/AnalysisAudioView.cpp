#include "AnalysisAudioView.h"
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include <SDL_opengl.h>
namespace BackyardBrains {

AnalysisAudioView::AnalysisAudioView(RecordingManager &manager, AnalysisManager &anaman, SpikeSorter &spikes, Widgets::Widget *parent) : AudioView(parent, manager, anaman), _spikes(spikes), _manager(manager), _colorIdx(0), _clickedThresh(-1) {
	_threshPos[0] = -0;
	_threshPos[1] = 0;
	_channels[0].pos = 0.5;
}

void AnalysisAudioView::drawTargetMarkers() {
	Widgets::Painter::setColor(MARKER_COLORS[_colorIdx % MARKER_COLOR_NUM]);
	Widgets::TextureGL::get("data/threshpin.bmp")->bind();

	for(int i = 0; i < 2; i++) {
		float y = ampToRelPos(_threshPos[i])*height();
		glPushMatrix();
		if(y < MOVEPIN_SIZE*0.5f || y > height()-MOVEPIN_SIZE*0.5f) {
			bool bottom = y > MOVEPIN_SIZE/2;

			y = MOVEPIN_SIZE*0.5f;
			if(bottom)
				y = height()-MOVEPIN_SIZE*0.5f;

			glTranslatef(width()-MOVEPIN_SIZE, y, 0);
			glRotatef(90-180*bottom,0,0,1);
		} else {
			glTranslatef(width()-MOVEPIN_SIZE*1.5f, y, 0);
		}
		Widgets::Painter::drawTexRect(Widgets::Rect(0, -MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
		glPopMatrix();
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	for(int i = 0; i < 2; i++) {
		float y = ampToRelPos(_threshPos[i])*height();
		if(y >= MOVEPIN_SIZE*0.5f && y <= height()-MOVEPIN_SIZE*0.5f) {
			Widgets::Painter::drawRect(Widgets::Rect(MOVEPIN_SIZE*1.5f,y, width()-3*MOVEPIN_SIZE+3, 1));
		}
	}
}

int AnalysisAudioView::screenWidth() const {
	int screenw = width()-MOVEPIN_SIZE*3;
	return std::max(0,screenw);
}

int AnalysisAudioView::relPosToAmp(float rpos) const {
	return (_channels[0].pos - rpos)/ampScale/_channels[selectedChannel()].gain;
}

float AnalysisAudioView::ampToRelPos(int amp) const {
	return _channels[0].pos - amp*_channels[selectedChannel()].gain*ampScale;
}

int AnalysisAudioView::upperThresh() const {
	return std::max(_threshPos[0], _threshPos[1]);
}

int AnalysisAudioView::lowerThresh() const {
	return std::min(_threshPos[0], _threshPos[1]);
}

void AnalysisAudioView::setColorIdx(int idx) {
	_colorIdx = idx;
}
void AnalysisAudioView::setThresh(int upper, int lower) {
	_threshPos[0] = upper;
	_threshPos[1] = lower;
}
void AnalysisAudioView::paintEvent() {
	const int samples = sampleCount(screenWidth(), scaleWidth());

	std::vector<std::pair<int16_t, int16_t> > data;
	int pos = _manager.pos()-samples/2;
	//data = _manager.getSamplesEnvelope(_channels[0].virtualDevice, pos, samples, screenWidth() == 0 ? 1 : std::max(samples/screenWidth(),1));
    data = _manager.getSamplesEnvelope(_manager.selectedVDevice(), pos, samples, screenWidth() == 0 ? 1 : std::max(samples/screenWidth(),1));
    
	Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
	drawData(data, selectedChannel(), samples, MOVEPIN_SIZE*1.5f, height()/2.0, screenWidth(), 0);

	drawMarkers(true);

	drawTargetMarkers();
	drawTimeScale();
 
   // std::cout<<"Spike gain: "<<_channels[0].gain<<" scale:"<<height()*ampScale<<"\n";
	Widgets::Painter::setColor(Widgets::Colors::white);
	if(!_channels.empty()) {
		for(unsigned int i = 0; i < _spikes.spikes(_manager.selectedVDevice()).size(); i++) {
			int samplepos = _manager.pos()-_spikes.spikes(_manager.selectedVDevice())[i].first;
			if(samplepos < -samples/2 || samplepos > samples/2)
				continue;
            
            const int y = height()*_channels[0].pos - _spikes.spikes(_manager.selectedVDevice())[i].second*height()*ampScale*_channels[selectedChannel()].gain;
			const float x = MOVEPIN_SIZE*1.48f+screenWidth()*(samples/2-samplepos)/(float)samples;

			bool selected = _spikes.spikes(_manager.selectedVDevice())[i].second >= std::min(_threshPos[0], _threshPos[1]) && _spikes.spikes(_manager.selectedVDevice())[i].second <= std::max(_threshPos[0], _threshPos[1]);
			if(selected)
				Widgets::Painter::setColor(MARKER_COLORS[_colorIdx % MARKER_COLOR_NUM]);
			Widgets::Painter::drawRect(Widgets::Rect(x-1,y-1, 3, 3));
			if(selected)
				Widgets::Painter::setColor(Widgets::Colors::white);

		}
	}

	Widgets::TextureGL::get("data/pin.bmp")->bind();
	Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, _channels[0].pos*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
	glBindTexture(GL_TEXTURE_2D, 0);

    int y = _channels[0].pos*height();
    Widgets::TextureGL::get("data/gaindown.bmp")->bind();
    Widgets::Painter::drawTexRect(Widgets::Rect(GAINCONTROL_XOFF-GAINCONTROL_RAD,y+GAINCONTROL_YOFF-GAINCONTROL_RAD,2*GAINCONTROL_RAD,2*GAINCONTROL_RAD));
    Widgets::TextureGL::get("data/gainup.bmp")->bind();
    Widgets::Painter::drawTexRect(Widgets::Rect(GAINCONTROL_XOFF-GAINCONTROL_RAD,y-GAINCONTROL_YOFF-GAINCONTROL_RAD,2*GAINCONTROL_RAD,2*GAINCONTROL_RAD));
    glBindTexture(GL_TEXTURE_2D, 0);
}

void AnalysisAudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::RightButton)
		return;
	if(event->button() == Widgets::LeftButton) {
		if(x < MOVEPIN_SIZE*1.5f && determineGainControlHoverFromAnalysisView(x, y) == 0)
			return;

		if(_clickedThresh == -1 && x > width() - MOVEPIN_SIZE*1.5f) {
			for(int i = 0; i < 2; i++) {
				if(determineThreshHover(x, y, ampToRelPos(_threshPos[i])*height(), &_clickOffset)) {
					_clickedThresh = i;
					event->accept();
					return;
				}
			}
		}
	}

	AudioView::mousePressEventFromAnalysisView(event);
}
    
    

void AnalysisAudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(_clickedThresh != -1) {
		int y = std::min(height()-MOVEPIN_SIZE/2, std::max(MOVEPIN_SIZE/2,event->pos().y-_clickOffset));
		_threshPos[_clickedThresh] = relPosToAmp(y/(float)(height()));
		event->accept();
	}

	AudioView::mouseMotionEvent(event);
}

void AnalysisAudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		if(_clickedThresh != -1) {
			threshChanged.emit();
		}
		_clickedThresh = -1;
	}

	AudioView::mouseReleaseEvent(event);
}

}
