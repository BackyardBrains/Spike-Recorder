#include "AudioView.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include "engine/SampleBuffer.h"
#include <iostream>
#include <sstream>
#include <cmath>

namespace BackyardBrains {


static const Widgets::Color CHANNEL_COLORS[] = {
	Widgets::Color(225,252,90),
	Widgets::Color(255,138,91),
	Widgets::Color(106,106,233)
};

AudioView::AudioView(Widgets::Widget *parent, RecordingManager *mngr)
	: Widgets::Widget(parent), clickedSlider(-1), clickedPixelOffset(0), channelOffset(0), selectedChannel(0), threshMode(false), manager(mngr), timeScale(0.1f)  {
}

void AudioView::updateView(int n) {
	if(n < (int)views.size()) {
		views.resize(n);
	} else if(n > (int)views.size()) {
		int oldsize = views.size();
		views.resize(n);
		for(int i = oldsize; i < n ; i++) {
			views[i].gain = 1.f;
			views[i].pos = 0.4f+0.1f*i;
			views[i].thresh = 0.1f;
		}
	}
}

int AudioView::offset() {
	return channelOffset;
}

float AudioView::scaleWidth() {
	return 0.05f*rect().width()/timeScale;
}

float AudioView::screenWidth() {
	return rect().width() - MOVEPIN_SIZE*1.48f;
}

float AudioView::sampleCount(float screenw, float scalew) {
	return screenw*RecordingManager::SAMPLE_RATE/scalew;
}

void AudioView::setOffset(int offset) {
	int samples = sampleCount(screenWidth(), scaleWidth());

	channelOffset = std::min(0,offset);
	if(channelOffset < -SampleBuffer::SIZE+samples) // because that's what's visible on the screen
		channelOffset = -SampleBuffer::SIZE+samples;

	relOffsetChanged.emit(1000.f*channelOffset/(SampleBuffer::SIZE-samples)+1000);
}

void AudioView::toggleThreshMode() {
	threshMode = !threshMode;
}

void AudioView::setRelOffset(int reloffset) {
	float f = reloffset*0.001f-1.f;
	int count = SampleBuffer::SIZE-sampleCount(screenWidth(), scaleWidth());
	channelOffset = f*count;
}

static const char *get_unit_str(int unit) {
	switch(unit) {
		case 1:
			return "ms";
		case 2:
			return "\xe6s";
		case 3:
			return "ns";
		default:
			return "s";
	}
}

void AudioView::paintEvent() {
	if(threshMode) {
		paintThreshMode();
	} else {
		paintNormal();
	}
}

void AudioView::drawScale() {
	int unit = -std::log(timeScale)/std::log(10);
	float shownscalew = scaleWidth()/std::pow(10, unit);
	std::stringstream o;
	o << pow(10,-unit%3) << ' ';
	o << get_unit_str(unit/3);
	Widgets::Painter::setColor(Widgets::Colors::white);
	Widgets::Painter::drawRect(Widgets::Rect(rect().width()-shownscalew-20,rect().height()*0.9f, shownscalew, 1));
	Widgets::Application::getInstance()->font()->draw(o.str().c_str(), rect().width()-shownscalew/2-20, rect().height()*0.9f+15, Widgets::AlignHCenter);
}

void AudioView::drawData(int channel, int samples, float x, float y, float width) {
	std::vector<std::pair<int16_t, int16_t> > data =
		manager->channelSamplesEnvelope(channel,manager->pos()+channelOffset-samples, samples, samples > screenWidth() ? samples/screenWidth() : 1);


	float dist = width/((float)data.size()-1);
	float scale = rect().height()*ampScale;
	glBegin(GL_LINE_STRIP);
	for(unsigned int j = 0; j < data.size(); j++) {
		glVertex3f((int)(j*dist+x), -data[j].first*views[channel].gain*scale+y, 0);
		glVertex3f((int)(j*dist+x), -data[j].second*views[channel].gain*scale+y, 0);
	}
	glEnd();
}

void AudioView::paintNormal() {
	float scalew = scaleWidth();
	int samples = sampleCount(screenWidth(), scalew);
	float xoff = MOVEPIN_SIZE*1.48f;

	for(unsigned int i = 0; i < views.size(); i++) {
		float yoff = views[i].pos*rect().height();
		Widgets::Painter::setColor(CHANNEL_COLORS[i%(sizeof(CHANNEL_COLORS)/sizeof(CHANNEL_COLORS[0]))]);
		if(manager->channelVirtualDevice(i) != RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX) {
			drawData(i, samples, xoff, yoff, screenWidth());

			Widgets::TextureGL::get("data/pin.png")->bind();
			Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, views[i].pos*rect().height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	drawScale();

}

void AudioView::paintThreshMode() {
	Widgets::TextureGL::get("data/pin.png")->bind();
	for(unsigned int i = 0; i < views.size(); i++) {
		Widgets::Color tmp = CHANNEL_COLORS[i%(sizeof(CHANNEL_COLORS)/sizeof(CHANNEL_COLORS[0]))];
		if((int)i != selectedChannel)
			tmp.setAlpha(90);
		Widgets::Painter::setColor(tmp);
		Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, rect().height()*(i+0.5f)/views.size()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	int screenw = rect().width() - MOVEPIN_SIZE*3.5f;
	Widgets::Painter::setColor(CHANNEL_COLORS[selectedChannel%(sizeof(CHANNEL_COLORS)/sizeof(CHANNEL_COLORS[0]))]);
	drawData(selectedChannel, sampleCount(screenw, scaleWidth()), MOVEPIN_SIZE*2, rect().height()/2, screenw);

	Widgets::TextureGL::get("data/threshpin.png")->bind();

	float threshh = views[selectedChannel].thresh*views[selectedChannel].gain;
	Widgets::Painter::drawTexRect(Widgets::Rect(rect().width()-MOVEPIN_SIZE*1.5f, rect().height()*(0.5f-threshh)-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
	glBindTexture(GL_TEXTURE_2D, 0);

	int dotw = 20;
	int movement = SDL_GetTicks()/20%dotw;
	int y = rect().height()*(0.5f-threshh);
	for(int i = 0; i <= screenw/dotw+1; i++) {
		float x = MOVEPIN_SIZE*2+dotw*i-movement;

		glBegin(GL_LINES);
		glVertex3f(std::max(MOVEPIN_SIZE*2.f, x), y, 0.f);
		glVertex3f(std::min(MOVEPIN_SIZE*2.f+screenw, x+dotw*0.7f), y, 0.f);
		glEnd();
	}
	drawScale();
}

int AudioView::determineSliderHover(int x, int y) {
	int xx = MOVEPIN_SIZE-x;
	xx *= xx;

	for(unsigned int i = 0; i < views.size(); i++) {
		int yy;
		if(!threshMode)
			yy = rect().height()*views[i].pos - y;
		else
			yy = rect().height()*(i+0.5f)/views.size() - y;

		yy *= yy;
		if(xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.25f) {
			return i;
		}
	}

	return -1;
}

int AudioView::determineThreshHover(int x, int y) {

	int xx = rect().width()-MOVEPIN_SIZE-x;
	int yy = rect().height()*(0.5f-views[selectedChannel].thresh*views[selectedChannel].gain) - y;

	xx *= xx;
	yy *= yy;

	return xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.25f ? 1 : -1;
}

void AudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::LeftButton) {

		if(!threshMode) {
			if(clickedSlider == -1 || x >= MOVEPIN_SIZE*1.5f) {
				clickedSlider = determineSliderHover(x,y);
				if(clickedSlider != -1) {

					clickedPixelOffset = rect().height()*views[clickedSlider].pos-y;
					event->accept();
				}
			}
		} else {
			int i = determineSliderHover(x, y);
			if(i != -1) {
				selectedChannel = i;
				event->accept();
			}

			clickedSlider = determineThreshHover(x, y);
			if(clickedSlider != -1) {
				clickedPixelOffset = rect().height()*(0.5f-views[selectedChannel].thresh*views[selectedChannel].gain) - y;
				event->accept();
			}
		}

	} else if(event->button() == Widgets::WheelUpButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2 && (s = determineSliderHover(x,y)) != -1) {
			views[s].gain = std::min(10.f, views[s].gain*1.2f);
			views[s].thresh = std::max(0.f,std::min(0.5f, views[s].thresh*views[s].gain))/views[s].gain;
		} else {
			timeScale = std::max(1.f/RecordingManager::SAMPLE_RATE, timeScale*0.8f);
			setOffset(channelOffset);
		}
		event->accept();
	} else if(event->button() == Widgets::WheelDownButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2 && (s = determineSliderHover(x,y)) != -1) {
			views[s].gain = std::max(0.001f, views[s].gain*0.8f);
			views[s].thresh = std::max(0.f,std::min(0.5f, views[s].thresh*views[s].gain))/views[s].gain;
		} else {
			timeScale = std::min(2.f, timeScale*1.2f);
			setOffset(channelOffset); // or else the buffer end will become shown
		}
		event->accept();
	}
}

void AudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton)
		clickedSlider = -1;
}

void AudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(clickedSlider != -1) {
		if(!threshMode) {
			views[clickedSlider].pos = std::max(0.05f,std::min(0.95f, (event->pos().y+clickedPixelOffset)/(float)rect().height()));
		} else {
			views[selectedChannel].thresh = std::max(0.f,std::min(0.5f, (rect().height()/2-event->pos().y+clickedPixelOffset)/(float)rect().height()))/views[selectedChannel].gain;
		}
	}
}

void AudioView::resizeEvent(Widgets::ResizeEvent *e) {

}

}
