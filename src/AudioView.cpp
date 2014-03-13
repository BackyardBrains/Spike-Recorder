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
#include <cassert>
#include <cstdlib>

namespace BackyardBrains {

const Widgets::Color AudioView::COLORS[] = {
	Widgets::Colors::black,
	Widgets::Color(225,252,90),
	Widgets::Color(255,138,91),
	Widgets::Color(106,233,106),
	Widgets::Color(0,190,200)
};

const int AudioView::COLOR_NUM = sizeof(AudioView::COLORS)/sizeof(AudioView::COLORS[0]);

const float AudioView::ampScale = .0005f;

AudioView::AudioView(Widgets::Widget *parent, RecordingManager &mngr)
	: Widgets::Widget(parent), clickedGain(-1), clickedSlider(-1), clickedPixelOffset(0),
	clickedThresh(false), channelOffset(0), manager(mngr), timeScale(0.1f)  {
}

int AudioView::addChannel(int virtualDevice) {
	manager.incRef(virtualDevice);
	channels.push_back(AudioView::Channel());
	channels.back().virtualDevice = virtualDevice;

	if(channels.size() != 1)
		channels.back().pos = rand()/(float)RAND_MAX;
	return channels.size()-1;
}

void AudioView::removeChannel(int virtualDevice) {
	manager.decRef(virtualDevice);

	int removed = 0;
	for(unsigned int i = 0; i < channels.size()-removed; i++) {
		if(channels[i].virtualDevice == virtualDevice) {
			channels[i] = *(channels.end()-1-removed);
			removed++;
		}
	}
	channels.resize(channels.size() - removed);
	assert(removed > 0);
}


void AudioView::setChannelColor(int channel, int coloridx) {
	channels[channel].coloridx = std::max(0,std::min(COLOR_NUM-1, coloridx));
}

int AudioView::channelColor(int channel) const {
	return channels[channel].coloridx;
}

int AudioView::channelVirtualDevice(int channel) const {
	return channels[channel].virtualDevice;
}

int AudioView::virtualDeviceChannel(int virtualDevice) const {
	for(int i = 0; i < (int)channels.size(); i++)
		if(channels[i].virtualDevice == virtualDevice)
			return i;
	return -1;
}

int AudioView::selectedChannel() const {
	return virtualDeviceChannel(manager.threshVDevice());
}

void AudioView::standardSettings() {
	clickedGain = -1;
	clickedSlider = -1;
	clickedThresh = false;

	channelOffset = 0;

	channels.clear();
	if(manager.fileMode()) {
		addChannel(0);
		relOffsetChanged.emit(0);
	} else {
		addChannel(0);
		relOffsetChanged.emit(1000);
	}
}

int AudioView::channelCount() const {
	return channels.size();
}

int AudioView::offset() {
	return channelOffset;
}

float AudioView::scaleWidth() {
	return 0.05f*screenWidth()/timeScale;
}

int AudioView::screenWidth() {
	int screenw = width()-MOVEPIN_SIZE*1.5f;
	if(manager.threshMode())
		screenw -= MOVEPIN_SIZE*1.5f;
	return std::max(0,screenw);
}

int AudioView::sampleCount(int screenw, float scalew) {
	return screenw == 0 ? 0 : RecordingManager::SAMPLE_RATE/scalew*screenw;
}

float AudioView::thresholdPos() {
	return height()*(channels[selectedChannel()].pos-manager.recordingDevices()[manager.threshVDevice()].threshold*ampScale*channels[selectedChannel()].gain);
}

void AudioView::setOffset(int64_t offset) {
	int samples = sampleCount(screenWidth(), scaleWidth());
	int reloffset;

	if(!manager.fileMode()) {
		channelOffset = std::min((int64_t)0,offset);
		if(channelOffset < -SampleBuffer::SIZE+samples) // because that's what's visible on the screen
			channelOffset = -SampleBuffer::SIZE+samples;

		reloffset = 1000.f*channelOffset/(SampleBuffer::SIZE-samples)+1000;
	} else { // when we are reading a file, real seeking is allowed
		offset = std::min(manager.fileLength()-1, offset);
		offset = std::max((int64_t)0, offset);
		manager.setPos(offset);
		reloffset = round(1000.f*manager.pos()/(float)(manager.fileLength()-1));
	}
	relOffsetChanged.emit(reloffset);
}

void AudioView::setRelOffset(int reloffset) {
	if(!manager.fileMode()) {
		float f = reloffset*0.001f-1.f;
		int count = SampleBuffer::SIZE-sampleCount(screenWidth(), scaleWidth());
		channelOffset = f*count;
	} else {
		float f = reloffset*0.001f;
		int64_t count = manager.fileLength()-1;
		manager.setPos(f*count);
	}

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


void AudioView::drawScale() {
	int unit = -std::log(timeScale)/std::log(10);
	float shownscalew = scaleWidth()/std::pow(10, unit);
	std::stringstream o;
	o << pow(10,-unit%3) << ' ';
	o << get_unit_str(unit/3);
	Widgets::Painter::setColor(Widgets::Colors::white);
	Widgets::Painter::drawRect(Widgets::Rect(width()-shownscalew-20,height()*0.9f, shownscalew, 1));
	Widgets::Application::font()->draw(o.str().c_str(), width()-shownscalew/2-20, height()*0.9f+15, Widgets::AlignHCenter);
}

void AudioView::drawData(int channel, int samples, int x, int y, int width) {
	std::vector<std::pair<int16_t, int16_t> > data;
	if(!manager.threshMode())
		data = manager.getSamplesEnvelope(channels[channel].virtualDevice,manager.pos()+channelOffset-samples, samples, samples > width ? samples/width : 1);
	else
		data = manager.getTriggerSamplesEnvelope(channels[channel].virtualDevice, samples, samples > width ? samples/width : 1);

	float scale = height()*ampScale;
	glBegin(GL_LINE_STRIP);
	for(int j = 0; j < std::min(width,(int)data.size()); j++) {
		glVertex3i(j+x, -data[j].first*channels[channel].gain*scale+y, 0);
		glVertex3i(j+x, -data[j].second*channels[channel].gain*scale+y, 0);
	}
	glEnd();
}

void AudioView::paintEvent() {
	float scalew = scaleWidth();
	float xoff = MOVEPIN_SIZE*1.48f;
	int screenw = screenWidth();
	int samples = sampleCount(screenw, scalew);

	for(int i = channels.size() - 1; i >= 0; i--) {
		float yoff = channels[i].pos*height();
		Widgets::Painter::setColor(COLORS[channels[i].coloridx]);
		if(channels[i].virtualDevice != RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX) {
			drawData(i, samples, xoff, yoff, screenw);

			Widgets::TextureGL::get("data/pin.png")->bind();
			Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, channels[i].pos*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	if(manager.threshMode())
		drawThreshold(screenw);
	drawScale();
}

void AudioView::drawThreshold(int screenw) {
	Widgets::Painter::setColor(COLORS[channels[selectedChannel()].coloridx]);

	if(thresholdPos() > MOVEPIN_SIZE/2) {
		Widgets::TextureGL::get("data/threshpin.png")->bind();
		Widgets::Painter::drawTexRect(Widgets::Rect(width()-MOVEPIN_SIZE*1.5f, thresholdPos()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
		glBindTexture(GL_TEXTURE_2D, 0);

		const int dotw = 20;
		int movement = SDL_GetTicks()/20%dotw;
		int y = thresholdPos();
		for(int i = 0; i <= screenw/dotw+1; i++) {
			float x = MOVEPIN_SIZE*1.5f+dotw*i-movement;

			glBegin(GL_LINES);
			glVertex3f(std::max(MOVEPIN_SIZE*1.5f, std::min(MOVEPIN_SIZE*1.5f+screenw, x)), y, 0.f);
			glVertex3f(std::max(MOVEPIN_SIZE*1.5f, std::min(MOVEPIN_SIZE*1.5f+screenw, x+dotw*0.7f)), y, 0.f);
			glEnd();
		}
		drawScale();
	} else {
		Widgets::TextureGL::get("data/threshpin.png")->bind();
		glPushMatrix();
		glTranslatef(width()-MOVEPIN_SIZE, MOVEPIN_SIZE*0.5f, 0);
		glRotatef(90,0,0,1);
		Widgets::Painter::drawTexRect(Widgets::Rect(-MOVEPIN_SIZE/2, -MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void AudioView::advance() {
	if(manager.fileMode() && !manager.paused())
		setOffset(manager.pos());
}

int AudioView::determineSliderHover(int x, int y, int *yoffset) {
	int xx = MOVEPIN_SIZE-x;
	xx *= xx;

	for(unsigned int i = 0; i < channels.size(); i++) {
		int dy = y - height()*channels[i].pos ;

		int yy = dy*dy;
		if(xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.25f) {
			if(yoffset)
				*yoffset = dy;
			return i;
		}
	}

	return -1;
}

int AudioView::determineThreshHover(int x, int y, int *yoffset) {

	int xx = width()-MOVEPIN_SIZE-x;
	int dy = y - std::max(MOVEPIN_SIZE/2.f, thresholdPos());

	int yy = dy*dy;
	xx *= xx;

	if(xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.25f) {
		if(yoffset)
			*yoffset = dy;
		return 1;
	}

	return 0;
}

void AudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::LeftButton) {

		if(clickedSlider == -1 && x <= MOVEPIN_SIZE*1.5f) {
			clickedSlider = determineSliderHover(x,y,&clickedPixelOffset);
			if(clickedSlider != -1) {
				manager.setThreshVDevice(channels[clickedSlider].virtualDevice);
				event->accept();
			}
		} else if(clickedGain == -1 && (!manager.threshMode() || x <= width()-MOVEPIN_SIZE*1.5f)) { // if in thresh mode we don't want it to react on the tresh slider area
			int yy;
			unsigned int i;
			for(i = 0; i < channels.size(); i++) {
				yy = height()*channels[i].pos-event->pos().y;
				if(yy < 0)
					yy *= -1;
				if(yy < 40+20*channels[i].gain)
					break;
			}

			if(i != channels.size()) {
				clickedGain = i;
				clickedPixelOffset = yy;
				prevGain = channels[i].gain;
				event->accept();
			}
		}


		if(manager.threshMode()) {
			clickedThresh = determineThreshHover(x, y, &clickedPixelOffset);
			if(clickedThresh)
				event->accept();
		}

	} else if(event->button() == Widgets::WheelUpButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2) {
			if((s = determineSliderHover(x,y,NULL)) != -1)
				channels[s].gain = std::min(10.f, channels[s].gain*1.2f);
		} else if(!manager.threshMode() || x < width()-MOVEPIN_SIZE*3/2) {
			timeScale = std::max(1.f/RecordingManager::SAMPLE_RATE, timeScale*0.8f);
			if(!manager.fileMode())
				setOffset(channelOffset);
		}
		event->accept();
	} else if(event->button() == Widgets::WheelDownButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2) {
			if((s = determineSliderHover(x,y,NULL)) != -1)
			channels[s].gain = std::max(0.001f, channels[s].gain*0.8f);
		} else if(!manager.threshMode() || x < width()-MOVEPIN_SIZE*3/2) {
			timeScale = std::min(2.f, timeScale*1.2f);
			if(!manager.fileMode())
				setOffset(channelOffset); // or else the buffer end will become shown
		}
		event->accept();
	}
	assert((clickedGain != -1) + (clickedSlider != -1) + clickedThresh <= 1);
}

void AudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		clickedSlider = -1;
		clickedThresh = false;
		clickedGain = -1;
	}
}

void AudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(clickedSlider != -1)
		channels[clickedSlider].pos = std::max(0.05f,std::min(0.95f, (event->pos().y-clickedPixelOffset)/(float)height()));
	if(clickedThresh) {
		int selected = selectedChannel();

		float t = (event->pos().y-clickedPixelOffset)/(float)height();
		t = std::max(MOVEPIN_SIZE/(float)height(), t);
		t = std::min(1.f-MOVEPIN_SIZE/(float)height(), t);
		manager.setVDeviceThreshold(manager.threshVDevice(), (channels[selected].pos - t)/channels[selected].gain/ampScale);
	}

	if(clickedGain != -1) {
		float newGain = prevGain*std::fabs((height()*channels[clickedGain].pos-event->pos().y)/(float)clickedPixelOffset);
		channels[clickedGain].gain = std::max(0.001f, std::min(10.f, newGain));
	}

}

void AudioView::resizeEvent(Widgets::ResizeEvent *e) {

}

}
