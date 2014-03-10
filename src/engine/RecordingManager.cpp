#include "RecordingManager.h"
#include "SampleBuffer.h"

#include <iostream>
#include <cstdlib>

namespace BackyardBrains {

const int RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX = -2;

RecordingManager::RecordingManager() : _pos(0), _paused(false)
{
	std::cout << "Initializing libbass...\n";
	if(!BASS_Init(-1, RecordingManager::SAMPLE_RATE, 0, 0, NULL)) {
		std::cerr << "Bass Error: Initialization failed: " << BASS_ErrorGetCode() << "\n";
		exit(1);
	}

	_recordingDevices = _EnumerateRecordingDevices();
}

RecordingManager::~RecordingManager() {
	for(int i = 0; i < (int)_recordingDevices.size(); i++) {
		decRef(i);
	}

	BASS_Free();
}

RecordingManager::VirtualDevices RecordingManager::_EnumerateRecordingDevices()
{
	VirtualDevices result;
	BASS_DEVICEINFO info;
	VirtualDevice virtualDevice;

	for (int i = 0; BASS_RecordGetDeviceInfo(i, &info); i++)
	{
		virtualDevice.enabled = info.flags & BASS_DEVICE_ENABLED;
		for (int j = 0; j < 2; j++)
		{
			virtualDevice.index = i*2 + j;
			virtualDevice.name = std::string(info.name)/*.simplified()*/ + ((j == 0) ? " [Left]" : " [Right]");
			result.push_back(virtualDevice);
		}
	}

	return result;
}

// TODO: consolidate this function somewhere
static int64_t snapTo(int64_t val, int64_t increments)
{
	if (increments > 1)
	{
		val /= increments;
		val *= increments;
	}
	return val;
}

void RecordingManager::setPaused(bool pausing)
{
	if (_paused == pausing)
		return;
	_paused = pausing;
	for (std::map<int, Device>::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		if (pausing) {
			if(!BASS_ChannelPause(it->second.handle))
				std::cerr << "Bass Error: pausing channel failed: " << BASS_ErrorGetCode() << "\n";
		} else {
			if(!BASS_ChannelPlay(it->second.handle, FALSE))
				std::cerr << "Bass Error: resuming channel playback failed: " << BASS_ErrorGetCode() << "\n";
		}
	}
}

void RecordingManager::togglePaused()
{

	_paused = !_paused;
	for (std::map<int, Device>::const_iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		if (_paused) {
			if(!BASS_ChannelPause(it->second.handle))
				std::cerr << "Bass Error: pausing channel failed: " << BASS_ErrorGetCode() << "\n";
		} else {
			if(!BASS_ChannelPlay(it->second.handle, FALSE))
				std::cerr << "Bass Error: resuming channel playback failed: " << BASS_ErrorGetCode() << "\n";
		}
	}
}

std::vector< std::pair<int16_t, int16_t> > RecordingManager::getSamplesEnvelope(unsigned int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip)
{
	const int64_t pos2 = snapTo(offset+len, sampleSkip);
	const int64_t pos1 = snapTo(offset, sampleSkip);

	std::vector< std::pair<int16_t, int16_t> > result;
	if (_devices.count(virtualDeviceIndex/2) > 0)
		result = sampleBuffer(virtualDeviceIndex)->getDataEnvelope(pos1, pos2 - pos1, sampleSkip);
	else
		result.resize((pos2 - pos1)/sampleSkip);
	return result;
}

void RecordingManager::advance()
{
	if (_devices.empty() || _paused)
		return;

	std::vector<int16_t> buffer(2*BUFFER_SIZE);
	std::vector<int16_t> left(BUFFER_SIZE);
	std::vector<int16_t> right(BUFFER_SIZE);
	const int64_t oldPos = _pos;
	int64_t newPos = _pos;
	bool firstTime = true;

	int dcBias[2] = {0, 0};
	// qDebug() << "TIMER EVENT";
	// std::cerr << "--------------------" << std::endl;
	for (std::map<int, Device>::iterator it = _devices.begin(); it != _devices.end(); ++it)
	{
		const DWORD samplesRead = BASS_ChannelGetData(it->second.handle, buffer.data(), 2*BUFFER_SIZE*sizeof(int16_t))/sizeof(int16_t);
		if(samplesRead == (DWORD)-1) {
			std::cerr << "Bass Error: getting channel data failed: " << BASS_ErrorGetCode() << "\n";
			continue;
		}

		if (samplesRead > 2*BUFFER_SIZE) // TODO handle this a little better
			continue;

		// de-interleave the left and right channels
		for (DWORD i = 0; i < samplesRead/2; i++)
		{
			left[i] = buffer[i*2];
			right[i] = buffer[i*2 + 1];

			if(it->second.dcBiasNum < SAMPLE_RATE*10) {
				it->second.dcBiasSum[0] += buffer[i*2];
				it->second.dcBiasSum[1] += buffer[i*2+1];
				it->second.dcBiasNum++;
			}
		}

		dcBias[0] = it->second.dcBiasSum[0]/it->second.dcBiasNum;
		dcBias[1] = it->second.dcBiasSum[1]/it->second.dcBiasNum;

		for (DWORD i = 0; i < samplesRead/2; i++) {
			left[i] -= dcBias[0];
			right[i] -= dcBias[1];
		}

		if (it->second.sampleBuffers[0]->empty())
		{
			it->second.sampleBuffers[0]->setPos(oldPos);
			it->second.sampleBuffers[1]->setPos(oldPos);
			// std::cerr << "Set position to " << oldPos << " from " << oldPos << " and now it's " << it->second.sampleBuffers[0]->pos() << std::endl;
		}
		it->second.sampleBuffers[0]->addData(left.data(), samplesRead/2);
		it->second.sampleBuffers[1]->addData(right.data(), samplesRead/2);
		const int64_t posA = it->second.sampleBuffers[0]->pos();
		// std::cerr << it->first << " -- Before: " << before << " After: " << posA << " Empty?: " << wasEmpty << std::endl;
		// qDebug() << "POSA" << posA << "POSB" << posB;
		// qDebug() << "POSA" << posA/44100.0 << "POSB" << posB/44100.0;
		if (!it->second.sampleBuffers[0]->empty() && (firstTime || posA < newPos))
		{
			newPos = posA;
			firstTime = false;
		}
	}
	// std::cerr << "===================" << std::endl;
	if (newPos > oldPos)
	{
		_pos = newPos;
		// qDebug() << _devices.size() << (newPos - oldPos);
		samplesAdded.emit(oldPos, newPos - oldPos);
	}
}

void RecordingManager::Device::create(int64_t pos)
{
	destroy();
	for (int i = 0; i < 2; i++)
		sampleBuffers[i] = new SampleBuffer(pos);
}
void RecordingManager::Device::destroy()
{
	for (int i = 0; i < 2; i++) {
		if (sampleBuffers[i]) {
			delete sampleBuffers[i];
			sampleBuffers[i] = NULL;
		}
	}
}


void RecordingManager::incRef(int virtualDeviceIndex)
{
	if (virtualDeviceIndex < 0)
		return;
	const int device = virtualDeviceIndex/2;

	if (_devices.count(device) == 0)
		_devices[device].create(_pos);

	_devices[device].refCount++;

	if (_devices[device].handle == 0) {
		// make sure the device exists
		BASS_DEVICEINFO info;
		if(!BASS_RecordGetDeviceInfo(device, &info)) {
			std::cerr << "Bass Error: getting record device info failed: " << BASS_ErrorGetCode() << "\n";
			return;
		}

		// initialize the recording device if we haven't already
		if(!(info.flags & BASS_DEVICE_INIT)) {
			if(!BASS_RecordInit(device)) {
				std::cerr << "Bass Error: initializing record device failed: " << BASS_ErrorGetCode() << "\n";
				return;
			}
		}

		// subsequent API calls will operate on this recording device
		if(!BASS_RecordSetDevice(device)) {
			std::cerr << "Bass Error: setting record device failed: " << BASS_ErrorGetCode() << "\n";
			return;
		}

		const HRECORD handle = BASS_RecordStart(SAMPLE_RATE, 2, (_paused ? BASS_RECORD_PAUSE : 0), NULL, NULL);
		if (handle == FALSE)
		{
			std::cerr << "Bass Error: starting the recording failed: " << BASS_ErrorGetCode() << "\n";
			return;
		}
		_devices[device].handle = handle;
	}
}

void RecordingManager::decRef(int virtualDeviceIndex)
{
	if (virtualDeviceIndex < 0)
		return;
	const int device = virtualDeviceIndex/2;
	if (_devices.count(device) == 0)
		return;
	_devices[device].refCount--;
	if (_devices[device].refCount == 0)
	{
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

SampleBuffer * RecordingManager::sampleBuffer(int virtualDeviceIndex)
{
	const int device = virtualDeviceIndex/2;
	const int leftOrRight = virtualDeviceIndex%2;
	SampleBuffer * result = NULL;
	if (_devices.count(device) > 0)
		result = _devices[device].sampleBuffers[leftOrRight];
	return result;
}


} // namespace BackyardBrains
