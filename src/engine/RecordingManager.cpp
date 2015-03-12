#include "RecordingManager.h"
#include "SampleBuffer.h"
#include "FileRecorder.h"

#include <iostream>
#include <cstdlib>

namespace BackyardBrains {

const int RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX = -2;
const int RecordingManager::DEFAULT_SAMPLE_RATE = 22050;

RecordingManager::RecordingManager() : _pos(0), _paused(false), _threshMode(false), _fileMode(false), _sampleRate(DEFAULT_SAMPLE_RATE), _selectedVDevice(0), _threshAvgCount(1) {
	std::cout << "Initializing libbass...\n";
	if(!BASS_Init(-1, _sampleRate, 0, 0, NULL)) {
		std::cerr << "Bass Error: Initialization failed: " << BASS_ErrorGetCode() << "\n";
		exit(1);
	}

	_player.start(_sampleRate);
	initRecordingDevices();
}

RecordingManager::~RecordingManager() {
	clear();
	_player.stop();
	BASS_Free();
}

void RecordingManager::constructMetadata(MetadataChunk *m) const {
	unsigned int nchan = 0;

	for(unsigned int i = 0; i < _recordingDevices.size(); i++)
		if(_recordingDevices[i].bound)
			nchan++;

	assert(m->channels.size() <= nchan); // don't want to delete stuff here
	m->channels.resize(nchan);

	int chani = 0;
	for(unsigned int i = 0; i < _recordingDevices.size(); i++) {
		if(_recordingDevices[i].bound) {
			m->channels[chani].threshold = _recordingDevices[i].threshold;
			m->channels[chani].name = _recordingDevices[i].name;

			chani++;
		}
	}

	m->markers = _markers;
}

void RecordingManager::applyMetadata(const MetadataChunk &m) {
	for(unsigned int i = 0; i < m.channels.size(); i++) {
		_recordingDevices[i].threshold = m.channels[i].threshold;
		_recordingDevices[i].name = m.channels[i].name;
	}

	std::list<std::string> neuronNames;
	_spikeTrains.clear();
	_markers.clear();
	for(std::list<std::pair<std::string, int64_t> >::const_iterator it = m.markers.begin(); it != m.markers.end(); it++) {
		if(it->first.compare(0,7,"_neuron") == 0) {
			int neunum = -1;
			int i = 0;
			for(std::list<std::string>::iterator it2 = neuronNames.begin(); it2 != neuronNames.end(); it2++, i++) {
				if(*it2 == it->first)
					neunum = i;
			}

			if(neunum == -1) {
				neuronNames.push_back(it->first);
				neunum = neuronNames.size()-1;
				if((int)_spikeTrains.capacity() == neunum)
					_spikeTrains.reserve(_spikeTrains.capacity()*2);
				_spikeTrains.push_back(std::list<int64_t>());
			}

			_spikeTrains[neunum].push_back(it->second);
		} else {
			_markers.push_back(*it);
		}
	}
}

void RecordingManager::clear() {
	if(!_fileMode) {
		for(int i = 0; i < (int)_recordingDevices.size(); i++)
			decRef(i);
	} else {
		for(std::map<int, Device>::iterator it = _devices.begin(); it != _devices.end(); ++it) {
			BASS_StreamFree(it->second.handle);
			it->second.destroy();
		}
	}

	_player.setPos(0);

	_markers.clear();
	_triggers.clear();
	_devices.clear();
	_recordingDevices.clear();
	_pos = 0;
	_selectedVDevice = 0;
}

bool RecordingManager::loadFile(const char *filename) {
	HSTREAM stream = BASS_StreamCreateFile(false, filename, 0, 0, BASS_STREAM_DECODE);
	if(stream == 0) {
		std::cerr << "Bass Error: Failed to load file '" << filename << "': " << BASS_ErrorGetCode() << "\n";
		return false;
	}


	BASS_CHANNELINFO info;
	BASS_ChannelGetInfo(stream, &info);

	int bytespersample = info.origres/8;
	if(bytespersample == 0)
		return false;
	if(bytespersample >= 3)
		bytespersample = 4; // bass converts everything it doesn’t support.

	clear();
	setSampleRate(info.freq);
	_devices[0].create(_pos, info.chans);
	_devices[0].bytespersample = bytespersample;

	_recordingDevices.resize(info.chans);
	for(unsigned int i = 0; i < info.chans; i++) {
		VirtualDevice &virtualDevice = _recordingDevices[i];

		virtualDevice.enabled = true;
		virtualDevice.device = 0;
		virtualDevice.channel = i;
		virtualDevice.name = "Some Channel";
		virtualDevice.threshold = 100;
		virtualDevice.bound = 0;
	}
	_devices[0].handle = stream;
	_fileMode = true;
	_filename = filename;

	deviceReload.emit();
	_player.setVolume(100);

	if(!_paused) {
		pauseChanged.emit();
		setPaused(true);
	}

	return true;
}

void RecordingManager::initRecordingDevices() {
	BASS_DEVICEINFO info;
	VirtualDevice virtualDevice;

	clear();
	_fileMode = false;

	for (int i = 0; BASS_RecordGetDeviceInfo(i, &info); i++)
	{
		virtualDevice.enabled = info.flags & BASS_DEVICE_ENABLED;
		for (int j = 0; j < 2; j++)	{
			virtualDevice.device = i;
			virtualDevice.channel = j;
			virtualDevice.name = std::string(info.name)/*.simplified()*/ + ((j == 0) ? " [Left]" : " [Right]");
			virtualDevice.threshold = 100;
			virtualDevice.bound = 0;
			_recordingDevices.push_back(virtualDevice);
		}
	}


	_player.setVolume(0);
	setSampleRate(DEFAULT_SAMPLE_RATE);
	deviceReload.emit();
}

// TODO: consolidate this function somewhere
static int64_t snapTo(int64_t val, int64_t increments) {
	if (increments > 1) {
		val /= increments;
		val *= increments;
	}
	return val;
}

void RecordingManager::setPaused(bool pausing) {
	if (_paused == pausing)
		return;
	_paused = pausing;

	_player.setPaused(pausing);

	if(_fileMode) { // reset the stream when end of file was reached
		if(!pausing && _pos >= fileLength()-1) {
			_triggers.clear();
			setPos(0);
		}
	} else {
		for(std::map<int, Device>::const_iterator it = _devices.begin(); it != _devices.end(); ++it) {
			if(pausing) {
				if(!BASS_ChannelPause(it->second.handle))
					std::cerr << "Bass Error: pausing channel failed: " << BASS_ErrorGetCode() << "\n";
			} else {
				if(!BASS_ChannelPlay(it->second.handle, FALSE))
					std::cerr << "Bass Error: resuming channel playback failed: " << BASS_ErrorGetCode() << "\n";
			}
		}
	}
}

Player &RecordingManager::player() {
	return _player;
}

int RecordingManager::sampleRate() const {
	return _sampleRate;
}

void RecordingManager::setSampleRate(int sampleRate) {
	_player.setSampleRate(sampleRate);
	_sampleRate = sampleRate;
}

void RecordingManager::setThreshMode(bool threshMode) {
	_threshMode = threshMode;
	if(threshMode)
		_triggers.clear();
}

void RecordingManager::setThreshAvgCount(int threshAvgCount) {
	_threshAvgCount = std::max(0,threshAvgCount);
	_triggers.clear();
}

void RecordingManager::setSelectedVDevice(int virtualDevice) {
	if(_selectedVDevice == virtualDevice)
		return;

	_selectedVDevice = virtualDevice;
	_player.setPos(_pos); // empty player buffer
	_triggers.clear();
}

void RecordingManager::setVDeviceThreshold(int virtualDevice, int threshold) {
	_recordingDevices[virtualDevice].threshold = threshold;
	_triggers.clear();
}

int64_t RecordingManager::fileLength() {
	assert(_fileMode && !_devices.empty());

	int64_t len = BASS_ChannelGetLength(_devices[0].handle, BASS_POS_BYTE)/_devices[0].bytespersample/_devices[0].channels;
	assert(len != -1);

	return len;
}

void RecordingManager::addMarker(const std::string &id, int64_t offset) {
	_markers.push_back(std::make_pair(id, _pos + offset));
}

const char *RecordingManager::fileMetadataString() {
	assert(_fileMode);
	return BASS_ChannelGetTags(_devices[0].handle, BASS_TAG_RIFF_INFO);
}

void RecordingManager::getData(int virtualDevice, int64_t offset, int64_t len, int16_t *dst) {
	sampleBuffer(virtualDevice)->getData(dst, offset, len);
}

std::vector< std::pair<int16_t, int16_t> > RecordingManager::getSamplesEnvelope(int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip) {
	const int64_t pos2 = snapTo(offset+len, sampleSkip);
	const int64_t pos1 = snapTo(offset, sampleSkip);

	std::vector< std::pair<int16_t, int16_t> > result;
	if (_devices.count(_recordingDevices[virtualDeviceIndex].device) > 0)
		result = sampleBuffer(virtualDeviceIndex)->getDataEnvelope(pos1, pos2 - pos1, sampleSkip);
	else
		result.resize((pos2 - pos1)/sampleSkip);

	return result;
}

std::vector<std::pair<int16_t,int16_t> > RecordingManager::getTriggerSamplesEnvelope(int virtualDeviceIndex, int64_t len, int sampleSkip) {
	std::vector<std::pair<int,int> > buf(len/sampleSkip);

	for(std::list<int64_t>::iterator it = _triggers.begin(); it != _triggers.end(); it++) {
		const int64_t pos2 = snapTo(*it+len/2, sampleSkip);
		const int64_t pos1 = snapTo(*it-len/2, sampleSkip);

		std::vector<std::pair<int16_t,int16_t> > tmp = sampleBuffer(virtualDeviceIndex)->getDataEnvelope(pos1, pos2-pos1, sampleSkip);

		for(unsigned int i = 0; i < std::min(tmp.size(),buf.size()); i++) {
			buf[i].first += tmp[i].first;
			buf[i].second += tmp[i].second;
		}
	}

	std::vector<std::pair<int16_t,int16_t> > result(buf.size());
	if(!_triggers.empty()) {
		for(unsigned int i = 0; i < result.size(); i++) {
			result[i].first = buf[i].first/(int)_triggers.size();
			result[i].second = buf[i].second/(int)_triggers.size();
		}
	}

	return result;
}

void RecordingManager::advanceFileMode(uint32_t samples) {
	if(!_paused && _pos >= fileLength()-1) {
		pauseChanged.emit();
		setPaused(true);
		return;
	}

	const unsigned int bufsize = 1*sampleRate();
	for(std::map<int, Device>::iterator it = _devices.begin(); it != _devices.end(); ++it) {
		if(it->second.sampleBuffers[0]->head()%(SampleBuffer::SIZE/2) >= SampleBuffer::SIZE/2-1 || it->second.sampleBuffers[0]->pos() >= fileLength()-1)
			continue;
		const int channum = it->second.channels;
		const int bytespersample = it->second.bytespersample;
		std::vector<int16_t> *channels = new std::vector<int16_t>[channum];
		uint8_t *buffer = new uint8_t[bytespersample*channum*bufsize];

		unsigned int len;
		if(it->second.sampleBuffers[0]->head() < SampleBuffer::SIZE/2)
			len = SampleBuffer::SIZE/2-1 - it->second.sampleBuffers[0]->head();
		else
			len = SampleBuffer::SIZE-1 - it->second.sampleBuffers[0]->head();

		DWORD samplesRead = BASS_ChannelGetData(it->second.handle, buffer, channum*std::min(len,bufsize)*bytespersample);
		if(samplesRead == (DWORD)-1) {
			std::cerr << "Bass Error: getting channel data failed: " << BASS_ErrorGetCode() << "\n";
			delete[] channels;
			delete[] buffer;
			continue;
		}
		samplesRead /= bytespersample;

		for(int chan = 0; chan < channum; chan++)
			channels[chan].resize(samplesRead/channum);

		// de-interleave the channels
		for(DWORD i = 0; i < samplesRead/channum; i++) {
			for(int chan = 0; chan < channum; chan++) {
				if(bytespersample == 1) { // unsigned 8 bit format
					channels[chan][i] = (buffer[i*channum + chan]-128)<<7;
				} else { // else assume signedness and take the 2 most significant bytes
					memcpy(&channels[chan][i], buffer+(i*channum + chan)*bytespersample, 2);
				}

				if(it->second.dcBiasNum < _sampleRate*10) {
					it->second.dcBiasSum[chan] += channels[chan][i];
					if(chan == 0)
						it->second.dcBiasNum++;
				}
			}
		}

		for(int chan = 0; chan < channum; chan++) {
			int dcBias = it->second.dcBiasSum[chan]/it->second.dcBiasNum;

			for(DWORD i = 0; i < samplesRead/channum; i++) {
				channels[chan][i] -= dcBias;
			}

			it->second.sampleBuffers[chan]->addData(channels[chan].data(), samplesRead/channum);
		}

		delete[] channels;
		delete[] buffer;
	}

	if(!_paused) {
		if(_threshMode) {
			SampleBuffer &s = *sampleBuffer(_selectedVDevice);

			for(int64_t i = _pos; i < _pos+samples; i++) {
				const int thresh = _recordingDevices[_selectedVDevice].threshold;

				if(_triggers.empty() || i - _triggers.front() > _sampleRate/10) {
					if((thresh > 0 && s.at(i) > thresh) || (thresh <= 0 && s.at(i) < thresh)) {
						_triggers.push_front(i);
						if(_triggers.size() > (unsigned int)_threshAvgCount)
							_triggers.pop_back();
					}
				}
			}
		}

		SampleBuffer &s = *sampleBuffer(_selectedVDevice);
		if(s.pos() > _player.pos()) {
			const uint32_t bsamples = s.pos()-_player.pos();

			if(_player.volume() > 0) {
				int16_t *buf = new int16_t[bsamples];

				s.getData(buf, _player.pos(), bsamples);
				_player.push(buf, bsamples*sizeof(int16_t));

				delete[] buf;
			} else {
				_player.setPos(_pos);
			}
		}

		setPos(_pos + samples, false);
	}
}

void RecordingManager::advance(uint32_t samples) {
	if(_fileMode) {
		advanceFileMode(samples);
		return;
	}

	if(_devices.empty() || _paused)
		return;

	const int64_t oldPos = _pos;
	int64_t newPos = _pos;
	bool firstTime = true;

	uint32_t len = 5*sampleRate();
	len = std::min(samples, len);

	for (std::map<int, Device>::iterator it = _devices.begin(); it != _devices.end(); ++it) {
		const int channum = it->second.channels;
		std::vector<int16_t> *channels = new std::vector<int16_t>[channum];
		int16_t *buffer = new int16_t[channum*len];

		DWORD samplesRead = BASS_ChannelGetData(it->second.handle, buffer, channum*len*sizeof(int16_t));
		if(samplesRead == (DWORD)-1) {
			std::cerr << "Bass Error: getting channel data failed: " << BASS_ErrorGetCode() << "\n";
			delete[] channels;
			delete[] buffer;
			continue;
		}
		samplesRead /= sizeof(int16_t);

		for(int chan = 0; chan < channum; chan++)
			channels[chan].resize(len);

		// de-interleave the channels
		for (DWORD i = 0; i < samplesRead/channum; i++) {
			for(int chan = 0; chan < channum; chan++) {
				channels[chan][i] = buffer[i*channum + chan];

				if(it->second.dcBiasNum < _sampleRate*10) {
					it->second.dcBiasSum[chan] += channels[chan][i];
					if(chan == 0)
						it->second.dcBiasNum++;
				}
			}
		}

		for(int chan = 0; chan < channum; chan++) {
			int dcBias = it->second.dcBiasSum[chan]/it->second.dcBiasNum;

			for(DWORD i = 0; i < samplesRead/channum; i++) {
				channels[chan][i] -= dcBias;
				if(_threshMode && it->first*channum+chan == _selectedVDevice) {
					const int64_t ntrigger = oldPos + i;
					const int thresh = _recordingDevices[_selectedVDevice].threshold;

					if(_triggers.empty() || ntrigger - _triggers.front() > _sampleRate/10) {
						if((thresh > 0 && channels[chan][i] > thresh) || (thresh <= 0 && channels[chan][i] < thresh)) {
							_triggers.push_front(oldPos + i);
							if(_triggers.size() > (unsigned int)_threshAvgCount)
								_triggers.pop_back();
						}
					}
				}
			}

			if(it->second.sampleBuffers[0]->empty()) {
				it->second.sampleBuffers[chan]->setPos(oldPos);
			}
			it->second.sampleBuffers[chan]->addData(channels[chan].data(), samplesRead/channum);
		}
		const int64_t posA = it->second.sampleBuffers[0]->pos();
		if(!it->second.sampleBuffers[0]->empty() && (firstTime || posA < newPos)) {
			newPos = posA;
			firstTime = false;
		}

		delete[] channels;
		delete[] buffer;
	}

	if(_pos-_sampleRate/2 > _player.pos()) {
		const uint32_t bsamples = _pos-_player.pos();

		if(_player.volume() > 0) {
			int16_t *buf = new int16_t[bsamples];

			SampleBuffer *s = sampleBuffer(_selectedVDevice);
			if(s != NULL) {
				s->getData(buf, _player.pos(), bsamples);
			} else {
				memset(buf, 0, bsamples*sizeof(int16_t));
			}

			_player.push(buf, bsamples*sizeof(int16_t));

			delete[] buf;
		} else {
			_player.setPos(_pos);
		}
	}
	_player.paused();


	if(newPos > oldPos)
		_pos = newPos;

}

void RecordingManager::Device::create(int64_t pos, int nchan) {
	destroy();
	channels = nchan;
	sampleBuffers.resize(nchan);
	dcBiasSum.resize(nchan);
	for (int i = 0; i < channels; i++)
		sampleBuffers[i] = new SampleBuffer(pos);
}

RecordingManager::Device::~Device() {
	destroy();
}

void RecordingManager::Device::destroy() {
	for (int i = 0; i < channels; i++) {
		if (sampleBuffers[i]) {
			delete sampleBuffers[i];
			sampleBuffers[i] = NULL;
		}
	}
}


bool RecordingManager::incRef(int virtualDeviceIndex) {
	assert(virtualDeviceIndex >= 0);
	if(virtualDeviceIndex >= (int)_recordingDevices.size())
		return false;

	const int device = _recordingDevices[virtualDeviceIndex].device;

	if (_devices.count(device) == 0)
		_devices[device].create(_pos, 2);
	
	_devices[device].refCount++;
	
	if (!_fileMode && _devices[device].handle == 0) {
		// make sure the device exists
		BASS_DEVICEINFO info;
		if(!BASS_RecordGetDeviceInfo(device, &info)) {
			std::cerr << "Bass Error: getting record device info failed: " << BASS_ErrorGetCode() << "\n";
			goto error;
		}

		// initialize the recording device if we haven't already
		if(!(info.flags & BASS_DEVICE_INIT)) {
			if(!BASS_RecordInit(device)) {
				std::cerr << "Bass Error: initializing record device failed: " << BASS_ErrorGetCode() << "\n";
				goto error;
			}
		}

		// subsequent API calls will operate on this recording device
		if(!BASS_RecordSetDevice(device)) {
			std::cerr << "Bass Error: setting record device failed: " << BASS_ErrorGetCode() << "\n";
			goto error;
		}

		const HRECORD handle = BASS_RecordStart(_sampleRate, 2, (_paused ? BASS_RECORD_PAUSE : 0), NULL, NULL);
		if (handle == FALSE)
		{
			std::cerr << "Bass Error: starting the recording failed: " << BASS_ErrorGetCode() << "\n";
			goto error;
		}
		_devices[device].handle = handle;
	}
	_recordingDevices[virtualDeviceIndex].bound++;
	return true;
error:
	_devices[device].refCount--;
	if(_devices[device].refCount == 0) {
		_devices[device].destroy();
		_devices.erase(device);
	}
	return false;
}

void RecordingManager::decRef(int virtualDeviceIndex) {
	assert(virtualDeviceIndex >= 0);
	const int device = _recordingDevices[virtualDeviceIndex].device;
	if (_devices.count(device) == 0)
		return;
	_devices[device].refCount--;
	_recordingDevices[virtualDeviceIndex].bound--;

	assert(_recordingDevices[virtualDeviceIndex].bound >= 0);
	assert(_devices[device].refCount >= 0);

	if (_devices[device].refCount == 0)	{
		if(!_fileMode) {
			// make sure the device exists
			BASS_DEVICEINFO info;
			if (!BASS_RecordGetDeviceInfo(device, &info)) {
				std::cerr << "Bass Error: getting record device info failed: " << BASS_ErrorGetCode() << "\n";
				return;
			}

			// subsequent API calls will operate on this recording device
			if (!BASS_RecordSetDevice(device)) {
				std::cerr << "Bass Error: setting record device failed: " << BASS_ErrorGetCode() << "\n";
				return;
			}

			// free the recording device if we haven't already
			if(info.flags & BASS_DEVICE_INIT) {
				if(!BASS_RecordFree()) {
					std::cerr << "Bass Error: freeing record device failed: " << BASS_ErrorGetCode() << "\n";
				}
			}

			_devices[device].destroy();
			_devices.erase(device);
		}
	}
}

void RecordingManager::setPos(int64_t pos, bool artificial) {
	assert(_fileMode);
	pos = std::max((int64_t)0, std::min(fileLength()-1, pos));

	if(pos == _pos)
		return;

	const int halfsize = SampleBuffer::SIZE/2;

	const int seg1 = std::min(fileLength()-1,_pos+halfsize/2)/halfsize;
	const int seg2 = std::min(fileLength()-1,pos+halfsize/2)/halfsize;


	if(seg1 != seg2) {
		for(std::map<int, Device>::const_iterator it = _devices.begin(); it != _devices.end(); ++it) {
			const int nchan = it->second.channels;
			const int npos = seg2*halfsize;

			for(unsigned int i = 0; i < it->second.sampleBuffers.size(); i++) {
				SampleBuffer &s = *it->second.sampleBuffers[i];

				if(abs(seg2-seg1) > 1 || seg2 == 0 || s.head()%halfsize != halfsize-1)
					s.reset();

				BASS_ChannelSetPosition(it->second.handle, it->second.bytespersample*npos*nchan, BASS_POS_BYTE);
				s.setHead(s.head() > halfsize ? 0 : halfsize); // to make it enter the next half segment

				s.setPos(npos);

			}
		}
	}
	if(artificial)
		_player.setPos(pos);

	_pos = pos;
}

SampleBuffer *RecordingManager::sampleBuffer(int virtualDeviceIndex) {
	assert(virtualDeviceIndex >= 0 && virtualDeviceIndex < (int) _recordingDevices.size());
	const int device = _recordingDevices[virtualDeviceIndex].device;
	const int channel = _recordingDevices[virtualDeviceIndex].channel;
	if(_devices.count(device) == 0 || (unsigned int)channel >= _devices[device].sampleBuffers.size())
		return NULL;
	SampleBuffer *result = _devices[device].sampleBuffers[channel];
	return result;
}


} // namespace BackyardBrains
