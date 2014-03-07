#ifndef BACKYARDBRAINS_RECORDINGMANAGER_H
#define BACKYARDBRAINS_RECORDINGMANAGER_H

#include <bass.h>
#include <sigslot.h>

#include <vector>
// #include <pair>
#include <string>
#include <map>
#include <stdint.h>

namespace BackyardBrains {

class SampleBuffer;

class RecordingManager : public sigslot::has_slots<>
{
public:
	static const int SAMPLE_RATE = 44100;

	struct VirtualDevice
	{
		int index;
		std::string name;
		bool enabled;
	};
	static const int INVALID_VIRTUAL_DEVICE_INDEX;
	typedef std::vector<VirtualDevice> VirtualDevices;
public:
	RecordingManager();
	~RecordingManager();
	int64_t pos() const {return _pos;}
	VirtualDevices recordingDevices() const {return _recordingDevices;}
	std::vector< std::pair<int16_t, int16_t> > getSamplesEnvelope(unsigned int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip);
	bool paused() const {return _paused;}

	void incRef(int virtualDeviceIndex);
	void decRef(int virtualDeviceIndex);

// public slots:
	void setPaused(bool pausing);
	void togglePaused();
// signals:
	sigslot::signal2<int64_t /*offset*/, int64_t /*len*/> samplesAdded;

	void advance();
private:
	static const unsigned int BUFFER_SIZE = 5*SAMPLE_RATE;
	VirtualDevices _EnumerateRecordingDevices();


	struct Device
	{
		Device() : handle(0), refCount(0), dcBiasNum(1)
		{
			sampleBuffers[0] = NULL;
			sampleBuffers[1] = NULL;

			dcBiasSum[0] = 0;
			dcBiasSum[1] = 0;
		}
		void create(int64_t pos);
		void destroy();
		bool needed() const {return refCount;}
		HRECORD handle;
		SampleBuffer * sampleBuffers[2];
		int refCount;
		long dcBiasSum[2];
		long dcBiasNum;
	};



	SampleBuffer *sampleBuffer(int virtualDeviceIndex);

	VirtualDevices _recordingDevices;
	std::map<int, Device> _devices;
	int64_t _pos;
	bool _paused;
};

} // namespace BackyardBrains

#endif
