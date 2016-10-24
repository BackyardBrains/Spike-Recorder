#include "AudioView.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include "widgets/Painter.h"
#include "widgets/TextureGL.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Application.h"
#include "engine/SampleBuffer.h"
#include "engine/FileRecorder.h"
#include "Log.h"
#include <sstream>
#include <utility>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstdlib>

namespace BackyardBrains {

const Widgets::Color AudioView::COLORS[] = {
	Widgets::Colors::black,
    //Widgets::Color(218,218,218),
    Widgets::Color(0,255,28),
    Widgets::Color(255,0,59),
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

AudioView::AudioView(Widgets::Widget *parent, RecordingManager &mngr, AnalysisManager &anaman)
	: Widgets::Widget(parent), _manager(mngr), _anaman(anaman), _clickedGain(-1), _clickedSlider(-1), _clickedPixelOffset(0),
	_clickedThresh(false), _rulerClicked(false), _rulerStart(-1.f), _rulerEnd(-1.f), _channelOffset(0), _timeScale(0.1f)  {

	_gainCtrlHoldTime = 0;
	updateChannels();
}

AudioView::~AudioView() {
}

void AudioView::updateChannels() {
	auto vdevices = _manager.virtualDevices();
	std::map<int, int> vdev2chan;
	for(int i = 0; i < (int)_channels.size(); i++)
		vdev2chan[_channels[i].virtualDevice] = i;

	for(int i = 0; i < (int) vdevices.size(); i++) {
		if(vdevices[i].bound) {
			// if there is no channel for that vdevice, add it.
			if(vdev2chan.count(i) == 0) {
				_channels.push_back(Channel());
                if(_channels.size()==1)
                {
                    _channels.back().pos = 0.5;
                }
				_channels.back().virtualDevice=i;
                _channels.back().colorIdx=(i+1)%(COLOR_NUM-1);
			}
		} else {
			// if there is a channel, but the vdevice is not bound, remove it.
			if(vdev2chan.count(i) != 0) {
				_channels.erase(_channels.begin()+vdev2chan[i]);
			}
		}
	}

    //get saved gain and timescale
    loadAndApplyAudioInputConfig();

    if(_manager.fileMode()) {

       // _manager.setPos(numberOfSamplesToLoad);
    }
    else
    {
        //position seek bar to end
        relOffsetChanged.emit(1000);
    }
}

//
// Load configuration of gain and timescale for
// currently used input type and apply to all
// active channels
//
void AudioView::loadAndApplyAudioInputConfig()
{
    _timeScale = _manager.loadTimeScaleForAudioInput();

    float newGain = _manager.loadGainForAudioInput();
    for(unsigned int ic = 0; ic < _channels.size(); ic++) {
        _channels[ic].gain = newGain;
    }

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


    //positionseek bar to begining so that we see half of the screen filled with signal
    //when user click play we will reset file to begining

    float portionToPreload = 1000*((float)(sampleCount(screenWidth(), scaleWidth())/2))/((float)_manager.fileLength());
      relOffsetChanged.emit(portionToPreload);

    _manager.setPos(sampleCount(screenWidth(), scaleWidth())/2);

	//assert(m.channels.size() == _channels.size()); // i do not know if this works yet with the new channel stuff
	for(unsigned int i = 0; i < m.channels.size(); i++) {
		_channels[i].gain = m.channels[i].gain;
		_channels[i].colorIdx = m.channels[i].colorIdx;
		_channels[i].pos = m.channels[i].pos;
	}

}



void AudioView::setChannelColor(int channel, int colorIdx) {
	_channels.at(channel).colorIdx = std::max(0,std::min(COLOR_NUM-1, colorIdx));
}

int AudioView::channelColor(int channel) const {
	return _channels.at(channel).colorIdx;
}

int AudioView::channelVirtualDevice(int channel) const {
	return _channels.at(channel).virtualDevice;
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

	if(_manager.fileMode()) {
		relOffsetChanged.emit(0);
	} else {
		relOffsetChanged.emit(1000);
	}
}

int AudioView::channelCount() const {
	return _channels.size();
}

int AudioView::offset() const{
	if(_manager.fileMode())
		return _manager.pos();

	return _channelOffset;
}

int AudioView::channelOffset() const {
	return _channelOffset;
}

float AudioView::scaleWidth() const {
	return 0.05f*screenWidth()/_timeScale;
}

//Setter for timescale
void AudioView::setTimeScale(float newTimesScale)
{
    _timeScale = newTimesScale;
}

int AudioView::screenWidth() const {
	int screenw = width()-DATA_XOFF;
	if(_manager.threshMode())
		screenw -= DATA_XOFF;
	return std::max(0,screenw);
}

//
// Number of samples that are displayed on the screen at once
//
int AudioView::sampleCount(int screenw, float scalew)  const {
	if(screenw == 0)
		return 0;

	int samples = screenw == 0 ? 0 : (_manager.sampleRate()/scalew)*screenw;
	const int snap = std::max(samples/screenw,1);
	samples /= snap;
	samples *= snap;
	return samples;
}

float AudioView::thresholdPos() {
	if(_channels.size() == 0)
		return 0;
	return height()*(_channels[selectedChannel()].pos-_manager.virtualDevices()[_manager.selectedVDevice()].threshold*ampScale*_channels[selectedChannel()].gain);
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
        //std::cout<<f*count<<" ";
		_manager.setPos(f*count);
	}

}

void AudioView::navigateCurrentRecordingPosition(bool navigateForward)
{
    int samplesOnScreen = sampleCount(screenWidth(), scaleWidth());
    int deltaMove = samplesOnScreen/20;
    if(deltaMove<1)
    {
        deltaMove = 1;
    }

    int64_t currentSample = offset();
    if(navigateForward)
    {
        currentSample = currentSample+deltaMove;
    }
    else
    {
        currentSample = currentSample-deltaMove;
    }
    if(currentSample>0)
    {
        currentSample= 0;
    }

    setOffset(currentSample);
    //relOffsetChanged.emit(round(1000.f*_manager.pos()/(float)(_manager.fileLength()-1)));//update slider at the bottom of the scree
}

void AudioView::navigateFilePosition(bool navigateForward)
{
        int samplesOnScreen = sampleCount(screenWidth(), scaleWidth());
        int deltaMove = samplesOnScreen/20;
        if(deltaMove<1)
        {
            deltaMove = 1;
        }
        int64_t currentSample = _manager.pos();
        if(navigateForward)
        {
            currentSample = currentSample+deltaMove;
        }
        else
        {
            currentSample = currentSample-deltaMove;
        }

        _manager.setPos(currentSample);//update waveform

        relOffsetChanged.emit(round(1000.f*_manager.pos()/(float)(_manager.fileLength()-1)));//update slider at the bottom of the screen
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
	float shownscalew = scaleWidth()/std::pow(10.f, (float)unit);
	std::stringstream o;
	o << pow(10,-unit%3) << ' ';
	o << get_unit_str(unit/3);
	Widgets::Painter::setColor(Widgets::Colors::white);
	Widgets::Painter::drawRect(Widgets::Rect(width()-shownscalew-20,height()-50, shownscalew, 1));
	Widgets::Application::font()->draw(o.str().c_str(), width()-shownscalew/2-20, height()-50+15, Widgets::AlignHCenter);
}

void AudioView::drawData(std::vector<std::pair<int16_t, int16_t> > &data, int channel, int samples, int x, int y, int width) {
	float dist = width/(float)(data.size()-1);

	if(fabs(dist-1.f) < 0.003f)
		dist = 1.f; // we don’t want round off artifacts

	float scale = height()*ampScale;
	glBegin(GL_LINE_STRIP);
    //std::cout<<"Wave gain: "<<_channels[channel].gain<<" scale:"<<scale<<"\n";
	for(int j = 0; j < (int)data.size(); j++) {
		int xc = j*dist+x;

		glVertex3i(xc, -data[j].first*_channels[channel].gain*scale+y, 0);
		glVertex3i(xc, -data[j].second*_channels[channel].gain*scale+y, 0);
	}
	glEnd();

}

void AudioView::drawMarkers() {
	int samples = sampleCount(screenWidth(), scaleWidth());
	int lastx = 0;
	int lastw = 0;
	bool lastyoff = 0;

	for(std::list<std::pair<std::string, int64_t> >::const_iterator it = _manager.markers().begin(); it != _manager.markers().end(); it++) {
		float x = width()+screenWidth()*(it->second-_manager.pos()-_channelOffset)/(float)samples;
		if(x < DATA_XOFF || x > width())
			continue;
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

void AudioView::drawGainControls() {
	if(_channels.size() == 0)
		return;
	int y = _channels[selectedChannel()].pos*height();
	Widgets::TextureGL::get("data/gaindown.bmp")->bind();
	Widgets::Painter::drawTexRect(Widgets::Rect(GAINCONTROL_XOFF-GAINCONTROL_RAD,y+GAINCONTROL_YOFF-GAINCONTROL_RAD,2*GAINCONTROL_RAD,2*GAINCONTROL_RAD));
	Widgets::TextureGL::get("data/gainup.bmp")->bind();
	Widgets::Painter::drawTexRect(Widgets::Rect(GAINCONTROL_XOFF-GAINCONTROL_RAD,y-GAINCONTROL_YOFF-GAINCONTROL_RAD,2*GAINCONTROL_RAD,2*GAINCONTROL_RAD));
	glBindTexture(GL_TEXTURE_2D, 0);
}

//
// timePeriod - time period for calibration in seconds
// voltagePtoP - voltage of calibrator signal peak to peak in mV
// returns how many our units is equal to 1mV
//
float AudioView::calculateCalibrationCoeficient(float timePeriod, float voltagePtoP)
{
    int lengthOfData = _manager.sampleRate()*timePeriod;

    int vdevice = _channels[0].virtualDevice;
    int16_t *rmsData;
    float rms;
    rmsData = new int16_t[lengthOfData];
    _manager.getData(vdevice, _manager.pos()-lengthOfData, lengthOfData, rmsData);
    rms = _anaman.calculateRMS(rmsData, lengthOfData);

    delete[] rmsData;

    return (rms/0.7071f)/voltagePtoP;
}


void AudioView::drawAudio() {
	float scalew = scaleWidth();
	float xoff = DATA_XOFF;
	int screenw = screenWidth();
	int samples = sampleCount(screenw, scalew);

	Widgets::Color bg = Widgets::Colors::background;
	bg.a = 200;

	for(int i = _channels.size() - 1; i >= 0; i--) {
		float yoff = _channels[i].pos*height();
		Widgets::Painter::setColor(COLORS[_channels[i].colorIdx]);
		std::vector<std::pair<int16_t, int16_t> > data;

		if(!_manager.threshMode()) {
			if(_manager.serialMode())
			{
				// pos = (number of samples from begining of the time)
				//       + (offset in samples (negative value) used when returning in time to browse past values)
				//       - (number of samples that we need to display)

				int pos = _manager.pos()+_channelOffset-samples;
				int16_t tempData[samples];
				std::vector< std::pair<int16_t, int16_t> > tempVectorData(samples);
				_manager.getData(_channels[i].virtualDevice, pos, samples, tempData);
				for(int ind = 0;ind<samples;ind++)
				{
					tempVectorData[ind].first = tempData[ind];
					tempVectorData[ind].second = tempData[ind];
				}
				data = tempVectorData;
			}
			else
			{
				// pos = (number of samples from begining of the time)
				//       + (offset in samples (negative value) used when returning in time to browse past values)
				//       - (number of samples that we need to display)
				int pos = _manager.pos()+_channelOffset-samples;

				data = _manager.getSamplesEnvelope(_channels[i].virtualDevice, pos, samples, screenw == 0 ? 1 : std::max(samples/screenw,1));
			}
		} else {
			data = _manager.getTriggerSamplesEnvelope(_channels[i].virtualDevice, samples, screenw == 0 ? 1 : std::max(samples/screenw,1));
		}

		drawData(data, i, samples, xoff, yoff, screenw);

		Widgets::TextureGL::get("data/pin.bmp")->bind();
		Widgets::Painter::drawTexRect(Widgets::Rect(MOVEPIN_SIZE/2, _channels[i].pos*height()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
		glBindTexture(GL_TEXTURE_2D, 0);

		if(_rulerEnd != _rulerStart) {
			int startsample = std::min(_rulerStart, _rulerEnd)*samples;
			int endsample = std::max(_rulerStart, _rulerEnd)*samples;
			int vdevice = _channels[i].virtualDevice;


			int16_t *rmsData;
			float rms;

			if(_manager.threshMode()) {
				assert(endsample < samples);
				rmsData = new int16_t[samples];
				_manager.getTriggerData(vdevice, samples, rmsData);
				rms = _anaman.calculateRMS(rmsData+startsample,endsample-startsample);
            }
            else
            {
				long pos = _manager.pos()+_channelOffset-samples;
				rmsData = new int16_t[endsample-startsample];

                //when in file mode playing head is at the middle of the screen
                //while in normal (realtime view) signal playing head is at right hand side of the screen

                _manager.getData(vdevice, pos+startsample, endsample-startsample, rmsData);
				rms = _anaman.calculateRMS(rmsData, endsample-startsample);
			}

			delete[] rmsData;

			std::stringstream s;
			s.precision(3);
			if(_manager.isCalibrated())
            {
                s <<"RMS:"<< std::fixed << rms*_manager.getCalibrationCoeficient()<<" mV";
            }
            else
            {
                s <<"RMS:"<< std::fixed << rms/200.0 ;
            }


			Widgets::Painter::setColor(bg);
			drawtextbgbox(s.str(), width()-20, _channels[i].pos*height()+30, Widgets::AlignRight);
			Widgets::Painter::setColor(Widgets::Colors::white);
			Widgets::Application::font()->draw(s.str().c_str(), width()-20, _channels[i].pos*height()+30, Widgets::AlignRight);
		}

	}
	Widgets::Painter::setColor(Widgets::Colors::white);
	drawGainControls();

}

void AudioView::drawRulerBox() {
	if(_rulerEnd != _rulerStart) {
		int x = std::min(_rulerEnd, _rulerStart)*screenWidth();
		int w = std::max(_rulerEnd, _rulerStart)*screenWidth() - x;

		Widgets::Painter::setColor(Widgets::Color(50,50,50));
		Widgets::Painter::drawRect(Widgets::Rect(x+DATA_XOFF, -100, w, height()+200));
	}
}

void AudioView::drawRulerTime() {
	const int screenw = screenWidth();
	const int samples = sampleCount(screenw, scaleWidth());

	if(_rulerEnd != _rulerStart) {
		float w = fabs(_rulerStart-_rulerEnd);
		float dtime = w*samples/_manager.sampleRate();
		int unit = -std::log(dtime/100)/std::log(1000);
		unit = std::max(0, unit);
		dtime *= std::pow(1000.f, (float)unit);
		std::stringstream s;
		s.precision(3);
		s << std::fixed << dtime << " " << get_unit_str(unit);

		Widgets::Painter::setColor(Widgets::Color(50,50,50,200));
		drawtextbgbox(s.str(), (_rulerStart+_rulerEnd)*screenWidth()/2.f+DATA_XOFF, height()-50, Widgets::AlignCenter);
		Widgets::Painter::setColor(Widgets::Colors::white);
		Widgets::Application::font()->draw(s.str().c_str(), (_rulerStart+_rulerEnd)/2.f*screenWidth()+DATA_XOFF, height()-50, Widgets::AlignCenter);

	}
}


void AudioView::drawSpikeTrainStatistics()
{
    if(_rulerEnd != _rulerStart) {

        //calculate time in seconds
        int samples = sampleCount(screenWidth(), scaleWidth());
		float w = fabs(_rulerStart-_rulerEnd);
		float dtime = w*samples/_manager.sampleRate();

        //calculate sample for beginning and end of interval in absolute sample
        int64_t startsample = std::min(_rulerStart, _rulerEnd)*samples+_manager.pos()+_channelOffset-samples;
        int64_t endsample = std::max(_rulerStart, _rulerEnd)*samples+_manager.pos()+_channelOffset-samples;


        //go through all spike trains
        for(unsigned int i = 0; i < _manager.spikeTrains().size(); i++) {
                if(_manager.spikeTrains()[i].spikes.size()<1)
                {
                    continue;
                }
            int numberOfSpikes = 0;
            int clr = _manager.spikeTrains()[i].color;//get color
            //go through all spikes in current spike train
            for(unsigned int j = 0; j < _manager.spikeTrains()[i].spikes.size(); j++) {
               int64_t t = _manager.spikeTrains()[i].spikes[j];
               if(t>=startsample && t<=endsample)
               {
                    numberOfSpikes++;
               }
            }

            //make big color marker
			float y = height()*(0.1f+0.1f*i);
			Widgets::Painter::setColor(MARKER_COLORS[clr % MARKER_COLOR_NUM]);
			Widgets::Painter::drawRect(Widgets::Rect(20,y-18,10,10));


            //draw text
            Widgets::Color bg = Widgets::Colors::background;
            bg.a = 200;
            std::stringstream s;
            s <<""<< numberOfSpikes<<" ("<<numberOfSpikes/dtime<<"Hz)";
            Widgets::Painter::setColor(bg);
            drawtextbgbox(s.str(), 35, y-20, Widgets::AlignLeft);
            Widgets::Painter::setColor(Widgets::Colors::white);
            Widgets::Application::font()->draw(s.str().c_str(), 35, y-20, Widgets::AlignLeft);
        }
    }

}

//
// Draw spike trains (spike markers) on the top of the waveform
//
void AudioView::drawSpikeTrain() {
	int samples = sampleCount(screenWidth(), scaleWidth());
	//go through all spike trains
	for(unsigned int i = 0; i < _manager.spikeTrains().size(); i++) {
		int clr = _manager.spikeTrains()[i].color;//get color
		//go through all spikes in current spike train
		for(unsigned int j = 0; j < _manager.spikeTrains()[i].spikes.size(); j++) {
			int64_t t = _manager.spikeTrains()[i].spikes[j];
			if(_manager.pos()+_channelOffset-t > samples || _manager.pos()+_channelOffset-t < -samples/2)
				continue;

			float x = width()+screenWidth()*(t-_manager.pos()-_channelOffset)/(float)samples;
			float y = height()*(0.1f+0.1f*i);
			Widgets::Painter::setColor(MARKER_COLORS[clr % MARKER_COLOR_NUM]);
			Widgets::Painter::drawRect(Widgets::Rect(x-1,y-1,3,3));
		}
	}
}

void AudioView::paintEvent() {
	drawRulerBox();

	if(!_manager.threshMode())
    {
		drawSpikeTrain();
    }
	drawAudio();

	if(_manager.threshMode())
    {
        if(_manager.getThresholdSource()==0)
        {
            drawThreshold(screenWidth());
        }
    }
    else
    {
		drawMarkers();
		drawSpikeTrainStatistics();
    }

	drawRulerTime();
	drawScale();

	if(!_manager.fileMode() && _manager.virtualDevices().size() == 0) {
		Widgets::Application::font()->draw("No input device available", width()/2, height()/2, Widgets::AlignCenter);
	}
}

void AudioView::drawThreshold(int screenw) {
	if(_channels.size() == 0)
		return;
	Widgets::Painter::setColor(COLORS[_channels[selectedChannel()].colorIdx]);

	if(thresholdPos() > MOVEPIN_SIZE/2 && thresholdPos() < height() - MOVEPIN_SIZE/2) {
		Widgets::TextureGL::get("data/threshpin.bmp")->bind();
		Widgets::Painter::drawTexRect(Widgets::Rect(width()-DATA_XOFF, thresholdPos()-MOVEPIN_SIZE/2, MOVEPIN_SIZE, MOVEPIN_SIZE));
		glBindTexture(GL_TEXTURE_2D, 0);

		const int dotw = 20;
		int movement = SDL_GetTicks()/20%dotw;
		int y = thresholdPos();
		for(int i = 0; i <= screenw/dotw+1; i++) {
			float x = DATA_XOFF+dotw*i-movement;

			glBegin(GL_LINES);
			glVertex3f(std::max((float)DATA_XOFF, std::min((float)DATA_XOFF+screenw, x)), y, 0.f);
			glVertex3f(std::max((float)DATA_XOFF, std::min((float)DATA_XOFF+screenw, x+dotw*0.7f)), y, 0.f);
			glEnd();
		}
		drawScale();
	} else {
		Widgets::TextureGL::get("data/threshpin.bmp")->bind();
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

	if(_gainCtrlHoldTime) {
		int t = SDL_GetTicks() - _gainCtrlHoldTime;

		if(t > 400) {
			Channel &c = _channels[selectedChannel()];
            float newGain = c.gain*pow(1.01f,_gainCtrlDir);
			c.setGain(newGain);
            _manager.saveGainForAudioInput(newGain);
		}
	}
}

int AudioView::determineGainControlHover(int x, int y) {
	if(_channels.size() == 0)
		return 0;
	int xx = GAINCONTROL_XOFF-x;
	int dy = _channels[selectedChannel()].pos*height()-y;
	xx *= xx;

	dy -= GAINCONTROL_YOFF;
	if(xx + dy*dy < GAINCONTROL_RAD*GAINCONTROL_RAD)
		return 1;
	dy += 2*GAINCONTROL_YOFF;
	if(xx + dy*dy < GAINCONTROL_RAD*GAINCONTROL_RAD)
		return -1;

	return 0;
}

int AudioView::determineSliderHover(int x, int y, int *yoffset) {
	int xx = MOVEPIN_SIZE-x;
	xx *= xx;

	for(unsigned int i = 0; i < _channels.size(); i++) {
		int dy = y - height()*_channels[i].pos;
		int yy = dy*dy;
		if(xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.25f) {
			if(yoffset)
				*yoffset = dy;
			return i;
		}
	}

	return -1;
}

int AudioView::determineThreshHover(int x, int y, int threshPos, int *yoffset) {
	if(_channels.size() == 0)
		return 0;
	int xx = width()-MOVEPIN_SIZE-x;
	int dy = y - std::min(height()-MOVEPIN_SIZE/2, std::max(MOVEPIN_SIZE/2, threshPos));

	int yy = dy*dy;
	xx *= xx;

	if(xx + yy < MOVEPIN_SIZE*MOVEPIN_SIZE*0.25f) {
		if(yoffset)
			*yoffset = dy;
		return 1;
	}

	return 0;
}

void AudioView::Channel::setGain(float ngain) {
	gain = std::min(10.f, std::max(0.001f, ngain));
}


void AudioView::mousePressEventFromAnalysisView(Widgets::MouseEvent *event)
{
    analysisViewIsActive = true;
    AudioView::mousePressEvent(event);
    analysisViewIsActive = false;
}


void AudioView::mousePressEvent(Widgets::MouseEvent *event) {
	int x = event->pos().x;
	int y = event->pos().y;

	if(event->button() == Widgets::LeftButton) {

		if(_clickedSlider == -1 && x <= DATA_XOFF) {
			_gainCtrlDir = determineGainControlHover(x,y);
			if(_gainCtrlDir != 0) {
				Channel &c = _channels[selectedChannel()];
				float gainCoef = pow(1.3f,_gainCtrlDir);//0.769230783//1.29999995
                float newGain = c.gain*pow(1.3f,_gainCtrlDir);
                c.setGain(newGain);
                _manager.saveGainForAudioInput(newGain);

				_gainCtrlHoldTime = SDL_GetTicks();
				event->accept();
			} else {
				_clickedSlider = determineSliderHover(x,y,&_clickedPixelOffset);
				if(_clickedSlider != -1) {
					_manager.setSelectedVDevice(_channels[_clickedSlider].virtualDevice);
					event->accept();
				}
			}
		} else if(_clickedGain == -1 && (!_manager.threshMode() || x <= width()-DATA_XOFF)) { // in thresh mode we don't want it to react on the tresh slider area
			int yy;
			unsigned int i;
			for(i = 0; i < _channels.size(); i++) {
				yy = height()*_channels[i].pos-event->pos().y;
				if(yy < 0)
					yy *= -1;
				if(yy < 80+30*_channels[i].gain)
					break;
			}

			if(i != _channels.size()) {
				_clickedGain = i;
				_clickedPixelOffset = yy;
				_prevGain = _channels[i].gain;
				_prevDragX = x/(float)width();
				_prevDragOffset = offset();
				event->accept();
			}
		} else if(_manager.threshMode()) {
			_clickedThresh = determineThreshHover(x, y, thresholdPos(), &_clickedPixelOffset);
			if(_clickedThresh)
				event->accept();
		}

	} else if(event->button() == Widgets::WheelUpButton) {
		int s = -1;
		if(x < DATA_XOFF) {
			if((s = determineSliderHover(x,y,NULL)) != -1)
            {
                float newGain = _channels[s].gain*1.2f;
                _channels[s].setGain(newGain);
                _manager.saveGainForAudioInput(newGain);

            }
		} else if(!_manager.threshMode() || x < width()-DATA_XOFF) {//do not react on right handle in threshold mode


            //calculate offset of buffer after zoom in normal mode
            double positionOfCursorInPercOfScreen = (((double)(x- DATA_XOFF))/(double)screenWidth());
            int positionOfSampleUnderCursor = _channelOffset  - (sampleCount(screenWidth(), scaleWidth()) - sampleCount(screenWidth(), scaleWidth()) *positionOfCursorInPercOfScreen);
            int numberOfSamplesOnLeftInNewScale = sampleCount(screenWidth(), scaleWidth()/0.8f)*positionOfCursorInPercOfScreen;

            int positionOfRightSideOfNewScreen = positionOfSampleUnderCursor - numberOfSamplesOnLeftInNewScale + sampleCount(screenWidth(), scaleWidth()/0.8f);



            //calculate offset of buffer after zoom in file mode
            //calculate index of sample under cursor
            int sampleIndexInFile = _manager.pos() - sampleCount(screenWidth(), scaleWidth()) + positionOfCursorInPercOfScreen*sampleCount(screenWidth(), scaleWidth());

            //calculate what will be sample index of sample in righthand edge of the screen (playing head)
            //so that our sample under mouse cursor don't change position
            int positionForFile = sampleIndexInFile - numberOfSamplesOnLeftInNewScale + sampleCount(screenWidth(), scaleWidth()/0.8f);


           // std::cout<<"offset: "<<_channelOffset<<" position: "<<_manager.pos()<<"\n";
			_timeScale = std::max(1.f/_manager.sampleRate(), _timeScale*0.8f);
            _manager.saveTimeScaleForAudioInput(_timeScale);//save preference for this input type
			if(!_manager.fileMode()) {
                if(_manager.paused())
                {
                    setOffset(positionOfRightSideOfNewScreen);
                }
                else
                {
                    setOffset(_channelOffset);
                }
			}
            else
            {
                if(_manager.paused())
                {
                    if(!analysisViewIsActive)
                    {
                        setOffset(positionForFile);
                    }

                }

            }
		}
		event->accept();
	} else if(event->button() == Widgets::WheelDownButton) {
		int s = -1;
		if(x < DATA_XOFF) {
			if((s = determineSliderHover(x,y,NULL)) != -1)
            {
                float newGain = _channels[s].gain*0.8f;
                _channels[s].setGain(newGain);
                _manager.saveGainForAudioInput(newGain);
            }
		} else if(!_manager.threshMode() || x < width()-DATA_XOFF) {
			const int centeroff = (-sampleCount(screenWidth(), scaleWidth())+sampleCount(screenWidth(), scaleWidth()/1.2f))/2;
			_timeScale = std::min(2.f, _timeScale*1.2f);
            _manager.saveTimeScaleForAudioInput(_timeScale);
			if(!_manager.fileMode())
            {
				setOffset(_channelOffset + centeroff*_manager.paused()); // or else the buffer end will become shown
            }
            else
            {
                if(_manager.paused())
                {
                    if(!analysisViewIsActive)
                    {
                        setOffset(_manager.pos() + centeroff);
                    }


                }
            }
		}
		event->accept();
	} else if(event->button() == Widgets::RightButton) {
		if(_rulerEnd == _rulerStart && x > DATA_XOFF && (!_manager.threshMode() || x <= width()-DATA_XOFF)) {
			_rulerClicked = true;
			_rulerStart = std::max(0.f,std::min(1.f,(x-DATA_XOFF)/(float)screenWidth()));
			_rulerEnd = std::max(0.f, std::min(1.f,(x-DATA_XOFF)/(float)screenWidth()));
			event->accept();
		} else if(_rulerEnd != _rulerStart) {
			 _rulerEnd = 0;
			 _rulerStart = 0;
		}
	}

	assert((_clickedGain != -1) + (_clickedSlider != -1) + _clickedThresh <= 1);


}

void AudioView::mouseReleaseEvent(Widgets::MouseEvent *event) {
	if(event->button() == Widgets::LeftButton) {
		_clickedSlider = -1;
		_clickedThresh = false;
		_clickedGain = -1;
		_gainCtrlHoldTime = 0;
	}

	if(event->button() == Widgets::RightButton ) {
		_rulerClicked = false;
		if(fabs(_rulerStart-_rulerEnd)*screenWidth() < 1.f) {
			_rulerEnd = 0;
			_rulerStart = 0;
		}
	}
}

void AudioView::mouseMotionEvent(Widgets::MouseEvent *event) {
	if(_clickedSlider != -1) {
		_channels[_clickedSlider].pos = std::max(0.05f,std::min(0.95f, (event->pos().y-_clickedPixelOffset)/(float)height()));
		event->accept();
	}

	if(_gainCtrlHoldTime) {
		int dir = determineGainControlHover(event->pos().x, event->pos().y);

		if(dir != _gainCtrlDir)
			_gainCtrlHoldTime = 0;
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
		int doffset = (width()*_prevDragX-event->pos().x)/(float)scaleWidth()*_manager.sampleRate();
		setOffset(_prevDragOffset+doffset);


        _channels[_clickedGain].setGain(newGain);
        _manager.saveGainForAudioInput(newGain);

		event->accept();
	}

	if(_rulerClicked) {
		_rulerEnd = std::min(1.f,std::max(event->pos().x-DATA_XOFF, 0)/(float)screenWidth());
		event->accept();
	}

}

void AudioView::resizeEvent(Widgets::ResizeEvent *e) {

}

}
