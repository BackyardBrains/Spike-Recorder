#ifndef BACKYARDBRAINS_RECORDINGMANAGER_H
#define BACKYARDBRAINS_RECORDINGMANAGER_H

#include <bass.h>
#include <sigslot.h>

#include <vector>
// #include <pair>
#include <string>
#include <map>
#include <stdint.h>
#include "Player.h"

namespace BackyardBrains {

class SampleBuffer;

class RecordingManager : public sigslot::has_slots<>
{
public:
	static const int SAMPLE_RATE = 44100;

	struct VirtualDevice
	{
		int device;
		int channel;
		std::string name;
		bool enabled;
		int threshold;
		int bound;
	};
	static const int INVALID_VIRTUAL_DEVICE_INDEX;
	typedef std::vector<VirtualDevice> VirtualDevices;
public:
	RecordingManager();
	~RecordingManager();

	bool loadFile(const char *filename);
	void initRecordingDevices();

	Player &player();

	int64_t pos() const {return _pos;}
	void setPos(int64_t pos, bool artificial = true); // file mode only
	VirtualDevices &recordingDevices() {return _recordingDevices;}
	void getData(int virtualDevice, int64_t offset, int64_t len, int16_t *device);
	std::vector< std::pair<int16_t, int16_t> > getSamplesEnvelope(int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip);
	std::vector< std::pair<int16_t, int16_t> > getTriggerSamplesEnvelope(int virtualDeviceIndex, int64_t len, int sampleSkip);
	bool paused() const {return _paused;}
	bool threshMode() const {return _threshMode;}
	bool fileMode() const {return _fileMode;}
	int64_t fileLength(); // file mode only
	int threshAvgCount() const {return _threshAvgCount;}
	int selectedVDevice() const {return _selectedVDevice;}
	void incRef(int virtualDeviceIndex);
	void decRef(int virtualDeviceIndex);

	void setPaused(bool pausing);
	void setThreshMode(bool threshMode);
	void setThreshAvgCount(int threshAvgCount);
	void setSelectedVDevice(int virtualDevice);
	void setVDeviceThreshold(int virtualDevice, int threshold);

	sigslot::signal0<> deviceReload;
	sigslot::signal0<> pauseChanged;

	void advance(uint32_t milliseconds);
private:
	static const unsigned int BUFFER_SIZE = 5*SAMPLE_RATE;

	struct Device
	{
		Device() : handle(0), refCount(0), dcBiasNum(1), channels(0)
		{
		}
		~Device();
		void create(int64_t pos, int nchan);
		void destroy();
		bool needed() const {return refCount;}
		HRECORD handle;
		std::vector<SampleBuffer *> sampleBuffers;
		int refCount;
		std::vector<int64_t> dcBiasSum;
		int64_t dcBiasNum;
		int channels;
	};

	void clear();
	void advanceFileMode(uint32_t samples);
	SampleBuffer *sampleBuffer(int virtualDeviceIndex);

	VirtualDevices _recordingDevices;
	std::map<int, Device> _devices;
	int64_t _pos;
	bool _paused;
	bool _threshMode;

	bool _fileMode;

	int _selectedVDevice;
	int _threshAvgCount;
	std::list<int64_t> _triggers;

	Player _player;
};

} // namespace BackyardBrains

#endif
