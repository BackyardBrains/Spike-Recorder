#include "AudioView.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include "engine/SampleBuffer.h"
#include "engine/FileRecorder.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
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

const Widgets::Color AudioView::MARKER_COLORS[] = {
	Widgets::Color(255, 236, 148),
	Widgets::Color(255, 174, 174),
	Widgets::Color(176, 229, 124),
	Widgets::Color(180, 216, 231),
	Widgets::Color(147, 226, 213),
	Widgets::Color(193, 218, 214),
	Widgets::Color(172, 209, 233),
	Widgets::Color(216, 180, 231),
	Widgets::Color(174, 255, 174),
	Widgets::Color(255, 236, 255),
};

const int AudioView::MARKER_COLOR_NUM = sizeof(AudioView::MARKER_COLORS)/sizeof(AudioView::MARKER_COLORS[0]);

const float AudioView::ampScale = .0005f;

AudioView::AudioView(Widgets::Widget *parent, RecordingManager &mngr)
	: Widgets::Widget(parent), _clickedGain(-1), _clickedSlider(-1), _clickedPixelOffset(0),
	_clickedThresh(false), _rulerStart(-1), _rulerEnd(-1), _channelOffset(0), _manager(mngr), _timeScale(0.1f)  {
}

int AudioView::addChannel(int virtualDevice) {
	_manager.incRef(virtualDevice);
	_channels.push_back(AudioView::Channel());
	_channels.back().virtualDevice = virtualDevice;

	if(_channels.size() != 1)
		_channels.back().pos = rand()/(float)RAND_MAX;
	return _channels.size()-1;
}

void AudioView::removeChannel(int virtualDevice) {
	_manager.decRef(virtualDevice);

	int removed = 0;
	for(unsigned int i = 0; i < _channels.size()-removed; i++) {
		if(_channels[i].virtualDevice == virtualDevice) {
			_channels[i] = *(_channels.end()-1-removed);
			removed++;
		}
	}
	_channels.resize(_channels.size() - removed);
	assert(removed > 0);
}

void AudioView::clearChannels() {
	for(unsigned int i = 0; i < _channels.size(); i++)
		_manager.decRef(_channels[i].virtualDevice);

	_channels.clear();
}

static bool compare_second(const std::pair<int, int> &a, const std::pair<int, int> &b) {
	return a.second <= b.second;
}

void AudioView::constructMetadata(MetadataChunk *m) const {
	// as the channels in the file will be ordered by their virtualDevice
	// index, not by the channel index here, we have to predict those new
	// indices by sorting.

	std::vector<std::pair<int, int> > tmp(_channels.size());
	for(unsigned int i = 0; i < tmp.size(); i++)
		tmp[i] = std::make_pair(i, _channels[i].virtualDevice);
	std::sort(tmp.begin(),tmp.end(), compare_second);

	std::vector<int> idx(_channels.size());
	for(unsigned int i = 0; i < tmp.size(); i++)
		idx[tmp[i].first] = i;

	m->timeScale = _timeScale;
	m->channels.resize(_channels.size());
	for(unsigned int ic = 0; ic < _channels.size(); ic++) {
		int i = idx[ic];
		m->channels[i].gain = _channels[ic].gain;
		m->channels[i].colorIdx = _channels[ic].colorIdx;
		m->channels[i].pos = _channels[ic].pos;
	}
}

void AudioView::applyMetadata(const MetadataChunk &m) {
	_timeScale = m.timeScale;

	clearChannels();

	for(unsigned int i = 0; i < m.channels.size(); i++) {
		addChannel(i);
		_channels[i].gain = m.channels[i].gain;
		_channels[i].colorIdx = m.channels[i].colorIdx;
		_channels[i].pos = m.channels[i].pos;
	}

	if(m.channels.size() == 0 && _manager.recordingDevices().size() != 0)
		addChannel(0);
}

void AudioView::setChannelColor(int channel, int colorIdx) {
	_channels[channel].colorIdx = std::max(0,std::min(COLOR_NUM-1, colorIdx));
}

int AudioView::channelColor(int channel) const {
	return _channels[channel].colorIdx;
}

int AudioView::channelVirtualDevice(int channel) const {
	return _channels[channel].virtualDevice;
}

int AudioView::virtualDeviceChannel(int virtualDevice) const {
	for(int i = 0; i < (int)_channels.size(); i++)
		if(_channels[i].virtualDevice == virtualDevice)
			return i;
	return -1;
}

int AudioView::selectedChannel() const {
	return virtualDeviceChannel(_manager.selectedVDevice());
}

void AudioView::standardSettings() {
	_clickedGain = -1;
	_clickedSlider = -1;
	_clickedThresh = false;

	_channelOffset = 0;

	clearChannels();
	if(_manager.fileMode()) {
		relOffsetChanged.emit(0);
	} else {
		relOffsetChanged.emit(1000);
	}

	addChannel(0);
}

int AudioView::channelCount() const {
	return _channels.size();
}

int AudioView::offset() const{
	if(_manager.fileMode())
		return _manager.pos();

	return _channelOffset;
}

float AudioView::scaleWidth() {
	return 0.05f*screenWidth()/_timeScale;
}

int AudioView::screenWidth() {
	int screenw = width()-MOVEPIN_SIZE*1.5f;
	if(_manager.threshMode())
		screenw -= MOVEPIN_SIZE*1.5f;
	return std::max(0,screenw);
}

int AudioView::sampleCount(int screenw, float scalew) {
	int samples = screenw == 0 ? 0 : _manager.sampleRate()/scalew*screenw;
	const int snap = std::max(samples/screenw,1);
	samples /= snap;
	samples *= snap;
	return samples;
}

float AudioView::thresholdPos() {
	return height()*(_channels[selectedChannel()].pos-_manager.recordingDevices()[_manager.selectedVDevice()].threshold*ampScale*_channels[selectedChannel()].gain);
}

void AudioView::setOffset(int64_t offset) {
	int samples = sampleCount(screenWidth(), scaleWidth());
	int reloffset;

	if(!_manager.fileMode()) {
		_channelOffset = std::min((int64_t)0,offset);
		if(_channelOffset < -SampleBuffer::SIZE+samples) // because that's what's visible on the screen
			_channelOffset = -SampleBuffer::SIZE+samples;

		reloffset = 1000.f*_channelOffset/(SampleBuffer::SIZE-samples)+1000;
	} else { // when we are reading a file, real seeking is allowed
		offset = std::min(_manager.fileLength()-1, offset);
		offset = std::max((int64_t)0, offset);
		_manager.setPos(offset);
		reloffset = round(1000.f*_manager.pos()/(float)(_manager.fileLength()-1));
	}
	relOffsetChanged.emit(reloffset);
}

void AudioView::setRelOffset(int reloffset) {
	if(!_manager.fileMode()) {
		float f = reloffset*0.001f-1.f;
		int count = SampleBuffer::SIZE-sampleCount(screenWidth(), scaleWidth());
		_channelOffset = f*count;
	} else {
		float f = reloffset*0.001f;
		int64_t count = _manager.fileLength()-1;
		_manager.setPos(f*count);
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
	int unit = -std::log(_timeScale)/std::log(10);
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

	if(!_manager.threshMode()) {
		int pos = _manager.pos()+_channelOffset-samples;
		if(_manager.fileMode())
			pos += samples/2;
		data = _manager.getSamplesEnvelope(_channels[channel].virtualDevice, pos, samples, std::max(samples/width,1));
	} else {
		data = _manager.getTriggerSamplesEnvelope(_channels[channel].virtualDevice, samples, std::max(samples/width,1));
	}

	float dist = width/(float)(data.size()-1);

	if(fabs(dist-1.f) < 0.003f)
		dist = 1.f; // we don’t want round off artifacts

	float scale = height()*ampScale;
	glBegin(GL_LINE_STRIP);
	for(int j = 0; j < (int)data.size(); j++) {
		int xc = j*dist+x;

		glVertex3i(xc, -data[j].first*_channels[channel].gain*scale+y, 0);
		glVertex3i(xc, -data[j].second*_channels[channel].gain*scale+y, 0);
	}
	glEnd();


	if(_rulerStart != -1) {
		// Calculate the RMS. This is a horrible place to do this, I know, but it’s the most efficient place. Will change this soon.
		float sum = 0.f;
		int startsample = (std::min(_rulerStart, _rulerEnd)-MOVEPIN_SIZE*1.5f)*data.size()/width;
		int endsample = (std::max(_rulerStart, _rulerEnd)-MOVEPIN_SIZE*1.5f)*data.size()/width;

		assert(startsample < (int)data.size() && endsample < (int)data.size());

		for(int i = startsample; i < endsample; i++) {
			sum += data[i].first*data[i].first;
			sum += data[i].second*data[i].second;
		}

		if(endsample-startsample > 0) {
			sum /= (endsample-startsample)*2;
			_channels[channel].rms = std::sqrt(sum);
		} else {
			_channels[channel].rms = 0.f;
		}

	}
}

void AudioView::drawMarkers() {
	int samples = sampleCount(screenWidth(), scaleWidth());
	int lastx = 0;
	int lastw = 0;
	bool lastyoff = 0;

	for(std::list<std::pair<std::string, int64_t> >::const_iterator it = _manager.markers().begin(); it != _manager.markers().end(); it++) {
		if(_manager.pos()+_channelOffset-it->second > samples || _manager.pos()+_channelOffset-it->second < 0)
			continue;

		float x = width()+screenWidth()*(it->second-_manager.pos()-_channelOffset)/(float)samples;
		assert(it->first.size() > 0);
		Widgets::Painter::setColor(MARKER_COLORS[(it->first[0]-'0') % MARKER_COLOR_NUM]);

		glBegin(GL_LINES);
		glVertex3f(x, -100, 0);
		glVertex3f(x, height()+100, 0);
		glEnd();

		int w = Widgets::Application::font()->characterWidth()*it->first.size()+5;
		int h = Widgets::Application::font()->characterHeight();

		bool yoff = 0;
		if(it != _manager.markers().begin() && x-lastx < (w+lastw)/2)
			yoff = !lastyoff;

		Widgets::Painter::drawRect(Widgets::Rect(x-w/2,20+yoff*(h+4),w,h));
		Widgets::Painter::setColor(Widgets::Color(30,30,30));
		Widgets::Application::font()->draw(it->first.c_str(), x+1, 21+yoff*(h+4), Widgets::AlignHCenter);

		lastx = x;
		lastw = w;
		lastyoff = yoff;
	}
}

static void drawtextbgbox(const std::string &s, int x, int y, Widgets::Alignment a) {
	const int pad = 3;
	const int w = s.size()*Widgets::Application::font()->characterWidth();
	const int h = Widgets::Application::font()->characterHeight();

	int rx = x-pad;
	int ry = y-pad;

	if(a & Widgets::AlignRight)
		rx -= w;
	if(a & Widgets::AlignBottom)
		ry -= h;
	if(a & Widgets::AlignHCenter)
		rx -= w/2;
	if(a & Widgets::AlignVCenter)
		ry -= h/2;

	Widgets::Painter::drawRect(Widgets::Rect(rx, ry, w+2*pad, h+2*pad-1));
}

void AudioView::paintEvent() {
	float scalew = scaleWidth();
	float xoff = MOVEPIN_SIZE*1.48f;
	int screenw = screenWidth();
	int samples = sampleCount(screenw, scalew);

	if(_rulerStart != -1) {
		int x = std::min(_rulerEnd, _rulerStart);
		int w = std::max(_rulerEnd, _rulerStart) - x;

		Widgets::Painter::setColor(Widgets::Color(50,50,50));
		Widgets::Painter::drawRect(Widgets::Rect(x, -100, w, height()+200));
	}


	for(int i = _channels.size() - 1; i >= 0; i--) {
		float yoff = _channels[i].pos*height();
		Widgets::Painter::setColor(COLORS[_channels[i].colorIdx]);
		if(_channels[i].virtualDevice != RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX) {
			drawData(i, samples, xoff, yoff, screenw);

			Widgets::TextureGL::get("data/pin.png")->bind();
			Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, _channels[i].pos*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	if(_manager.threshMode())
		drawThreshold(screenw);
	else
		drawMarkers();

	if(_rulerStart != -1) {
		Widgets::Color bg = Widgets::Colors::background;
		bg.a = 200;

		int w = abs(_rulerStart-_rulerEnd);
		float dtime = w/(float)screenw*samples/_manager.sampleRate();
		int unit = -std::log(dtime/100)/std::log(1000);
		unit = std::max(0, unit);
		dtime *= std::pow(1000, unit);
		std::stringstream s;
		s.precision(3);
		s << std::fixed << dtime << " " << get_unit_str(unit);

		Widgets::Painter::setColor(Widgets::Color(50,50,50,200));
		drawtextbgbox(s.str(), (_rulerStart+_rulerEnd)/2, height()-50, Widgets::AlignCenter);
		Widgets::Painter::setColor(Widgets::Colors::white);
		Widgets::Application::font()->draw(s.str().c_str(), (_rulerStart+_rulerEnd)/2, height()-50, Widgets::AlignCenter);

		for(unsigned int i = 0; i < _channels.size(); i++) {
			s.clear();
			s.str("");
			s << "RMS:" << _channels[i].rms;

			Widgets::Painter::setColor(bg);
			drawtextbgbox(s.str(), width()-20, _channels[i].pos*height()+30, Widgets::AlignRight);
			Widgets::Painter::setColor(Widgets::Colors::white);
			Widgets::Application::font()->draw(s.str().c_str(), width()-20, _channels[i].pos*height()+30, Widgets::AlignRight);
		}
	}


	drawScale();
}

void AudioView::drawThreshold(int screenw) {
	Widgets::Painter::setColor(COLORS[_channels[selectedChannel()].colorIdx]);

	if(thresholdPos() > MOVEPIN_SIZE/2 && thresholdPos() < height() - MOVEPIN_SIZE/2) {
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

		bool bottom = thresholdPos() > MOVEPIN_SIZE/2;

		int y = MOVEPIN_SIZE*0.5f;
		if(bottom)
			y = height()-MOVEPIN_SIZE*0.5f;

		glTranslatef(width()-MOVEPIN_SIZE, y, 0);
		glRotatef(90-180*bottom,0,0,1);
		Widgets::Painter::drawTexRect(Widgets::Rect(-MOVEPIN_SIZE/2, -MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
		glPopMatrix();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void AudioView::advance() {
	if(_manager.fileMode() && !_manager.paused())
		setOffset(_manager.pos());
}

int AudioView::determineSliderHover(int x, int y, int *yoffset) {
	int xx = MOVEPIN_SIZE-x;
	xx *= xx;

	for(unsigned int i = 0; i < _channels.size(); i++) {
		int dy = y - height()*_channels[i].pos ;

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
	int dy = y - std::min(height()-MOVEPIN_SIZE/2.f, std::max(MOVEPIN_SIZE/2.f, thresholdPos()));

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

		if(_clickedSlider == -1 && x <= MOVEPIN_SIZE*1.5f) {
			_clickedSlider = determineSliderHover(x,y,&_clickedPixelOffset);
			if(_clickedSlider != -1) {
				_manager.setSelectedVDevice(_channels[_clickedSlider].virtualDevice);
				event->accept();
			}
		} else if(_clickedGain == -1 && (!_manager.threshMode() || x <= width()-MOVEPIN_SIZE*1.5f)) { // if in thresh mode we don't want it to react on the tresh slider area
			int yy;
			unsigned int i;
			for(i = 0; i < _channels.size(); i++) {
				yy = height()*_channels[i].pos-event->pos().y;
				if(yy < 0)
					yy *= -1;
				if(yy < 40+20*_channels[i].gain)
					break;
			}

			if(i != _channels.size()) {
				_clickedGain = i;
				_clickedPixelOffset = yy;
				_prevGain = _channels[i].gain;
				event->accept();
			}
		} else if(_manager.threshMode()) {
			_clickedThresh = determineThreshHover(x, y, &_clickedPixelOffset);
			if(_clickedThresh)
				event->accept();
		}

	} else if(event->button() == Widgets::WheelUpButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2) {
			if((s = determineSliderHover(x,y,NULL)) != -1)
				_channels[s].gain = std::min(10.f, _channels[s].gain*1.2f);
		} else if(!_manager.threshMode() || x < width()-MOVEPIN_SIZE*3/2) {
			_timeScale = std::max(1.f/_manager.sampleRate(), _timeScale*0.8f);
			if(!_manager.fileMode())
				setOffset(_channelOffset);
		}
		event->accept();
	} else if(event->button() == Widgets::WheelDownButton) {
		int s = -1;
		if(x < MOVEPIN_SIZE*3/2) {
			if((s = determineSliderHover(x,y,NULL)) != -1)
			_channels[s].gain = std::max(0.001f, _channels[s].gain*0.8f);
		} else if(!_manager.threshMode() || x < width()-MOVEPIN_SIZE*3/2) {
			_timeScale = std::min(2.f, _timeScale*1.2f);
			if(!_manager.fileMode())
				setOffset(_channelOffset); // or else the buffer end will become shown
		}
		event->accept();
	} else if(event->button() == Widgets::RightButton) {
		 if(_rulerStart == -1 && x > MOVEPIN_SIZE*1.5f && (!_manager.threshMode() || x <= width()-MOVEPIN_SIZE*1.5f)) {
			_rulerStart = x;
			_rulerEnd = x;
			event->accept();
		 }
	}

	assert((_clickedGain != -1) + (_clickedSlider != -1) + _clickedThresh <= 1);


}

void AudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		_clickedSlider = -1;
		_clickedThresh = false;
		_clickedGain = -1;
	}

	if(event->button() == Widgets::RightButton) {
		_rulerStart = -1;
	}
}

void AudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(_clickedSlider != -1) {
		_channels[_clickedSlider].pos = std::max(0.05f,std::min(0.95f, (event->pos().y-_clickedPixelOffset)/(float)height()));
		event->accept();
	}

	if(_clickedThresh) {
		int selected = selectedChannel();

		float t = (event->pos().y-_clickedPixelOffset)/(float)height();
		t = std::max(MOVEPIN_SIZE/(float)height(), t);
		t = std::min(1.f-MOVEPIN_SIZE/(float)height(), t);
		_manager.setVDeviceThreshold(_manager.selectedVDevice(), (_channels[selected].pos - t)/_channels[selected].gain/ampScale);
		event->accept();
	}

	if(_clickedGain != -1) {
		float newGain = _prevGain*std::fabs((height()*_channels[_clickedGain].pos-event->pos().y)/(float)_clickedPixelOffset);
		_channels[_clickedGain].gain = std::max(0.001f, std::min(10.f, newGain));
		event->accept();
	}

	if(_rulerStart != -1) {
		_rulerEnd = event->pos().x;
		event->accept();
	}

}

void AudioView::resizeEvent(Widgets::ResizeEvent *e) {

}

}
