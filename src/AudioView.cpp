#include "AudioView.h"
#include "SDL_opengl.h"
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

AudioView::AudioView(Widgets::Widget *parent, RecordingManager *mngr) : Widgets::Widget(parent), clickedSlider(-1), clickedPixelOffset(0), channelOffset(0), manager(mngr), displayScale(0.1f)  {
}

void AudioView::updateView(int n) {
	if(n < (int)views.size()) {
		views.resize(n);
	} else if(n > (int)views.size()) {
		int oldsize = views.size();
		views.resize(n);
		for(int i = oldsize; i < n ; i++) {
			views[i].gain = 1.;
			views[i].pos = 0.4f+0.1f*i;
		}
	}
}

int AudioView::offset() {
	return channelOffset;
}

float AudioView::scaleWidth() {
	return 0.05f*rect().width()/displayScale;
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
	float scalew = scaleWidth();
	float xoff = MOVEPIN_SIZE*1.48f;

	int samples = sampleCount(screenWidth(), scalew);
	int unit = -std::log(displayScale)/std::log(10);
	float shownscalew = scalew/std::pow(10, unit);


	for(unsigned int i = 0; i < views.size(); i++) {
		float yoff = views[i].pos*rect().height();
		Widgets::Painter::setColor(CHANNEL_COLORS[i%(sizeof(CHANNEL_COLORS)/sizeof(CHANNEL_COLORS[0]))]);
		if(manager->channelVirtualDevice(i) != RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX) {
			std::vector<std::pair<int16_t, int16_t> > data =
				manager->channelSamplesEnvelope(i,manager->pos()+channelOffset-samples, samples, samples > screenWidth() ? samples/screenWidth() : 1);

			float dist = screenWidth()/((float)data.size()-1);

			glBegin(GL_LINE_STRIP);
			for(unsigned int j = 0; j < data.size(); j++) {
				glVertex3f((int)(j*dist+xoff), -data[j].first*views[i].gain+yoff, 0);
				glVertex3f((int)(j*dist+xoff), -data[j].second*views[i].gain+yoff, 0);
			}
			glEnd();

			Widgets::Painter::setColor(CHANNEL_COLORS[i%(sizeof(CHANNEL_COLORS)/sizeof(CHANNEL_COLORS[0]))]);
			Widgets::TextureGL::get("data/pin.png")->bind();
			Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, views[i].pos*rect().height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	std::stringstream o;
	o << pow(10,-unit%3) << ' ';
	o << get_unit_str(unit/3);
	Widgets::Painter::setColor(Widgets::Colors::white);
	Widgets::Painter::drawRect(Widgets::Rect(rect().width()-shownscalew-20,rect().height()*0.9f, shownscalew, 1));
	Widgets::Application::getInstance()->font()->draw(o.str().c_str(), rect().width()-shownscalew/2-20, rect().height()*0.9f+15, Widgets::AlignHCenter);

}

int AudioView::determineSliderHover(int x, int y) {
	int xx = MOVEPIN_SIZE-x;
	xx *= xx;

	for(unsigned int i = 0; i < views.size(); i++) {
		int yy = rect().height()*views[i].pos - y;
		yy *= yy;
		if(xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.2) {
			return i;
		}
	}

	return -1;
}

void AudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::LeftButton) {
		if(clickedSlider != -1 || x >= MOVEPIN_SIZE*3/2)
			return;

		clickedSlider = determineSliderHover(x,y);
		if(clickedSlider != -1) {
			clickedPixelOffset = rect().height()*views[clickedSlider].pos-y;
			event->accept();
		}

	} else if(event->button() == Widgets::WheelUpButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2 && (s = determineSliderHover(x,y)) != -1) {
			views[s].gain = std::min(10.f, views[s].gain*1.2f);
		} else {
			displayScale = std::max(1.f/RecordingManager::SAMPLE_RATE, displayScale*0.8f);
			setOffset(channelOffset);
		}
	} else if(event->button() == Widgets::WheelDownButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2 && (s = determineSliderHover(x,y)) != -1) {
			views[s].gain = std::max(0.001f, views[s].gain*0.8f);
		} else {
			displayScale = std::min(2.f, displayScale*1.2f);
			setOffset(channelOffset); // or else the buffer end will become shown
		}
	}
}

void AudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton)
		clickedSlider = -1;
}

void AudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(clickedSlider != -1) {
		views[clickedSlider].pos = std::max(0.05f,std::min(0.95f, (event->pos().y+clickedPixelOffset)/(float)rect().height()));
	}
}

void AudioView::resizeEvent(Widgets::ResizeEvent *e) {

}

}
