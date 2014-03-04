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

	struct Color {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
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
	int channelCount() const {return _channels.size();}
	std::vector< std::pair<int16_t, int16_t> > channelSamplesEnvelope(unsigned int channelIndex, int64_t offset, int64_t len, int sampleSkip) const;
	bool paused() const {return _paused;}
	int channelVirtualDevice(unsigned int channelIndex) const;
	const Color &channelColor(int channelIndex) const;
// public slots:
	void setChannelColor(int channel, uint8_t r, uint8_t g, uint8_t b);
	void setChannelCount(int numChannels);
	void setChannelVirtualDevice(unsigned int channelIndex, int virtualDeviceIndex);
	void setPaused(bool pausing);
	void togglePaused();
// signals:
	sigslot::signal2<int /*channel idx*/, Color> colorChanged;
	sigslot::signal2<int64_t /*offset*/, int64_t /*len*/> samplesAdded;
	sigslot::signal1<int /*numChannels*/> channelCountChanged;
	sigslot::signal2<int /*channelIndex*/, int /*virtualDeviceIndex*/> channelVirtualDeviceChanged;

	void advance();
private:
	static const unsigned int BUFFER_SIZE = 5*SAMPLE_RATE;
	VirtualDevices _EnumerateRecordingDevices();

	struct Channel
	{
		Channel() : virtualDeviceIndex(INVALID_VIRTUAL_DEVICE_INDEX) {}
		int virtualDeviceIndex;
		Color color;
	};
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
	class Devices : public std::map<int, Device>
	{
	public:
		void incRef(int virtualDeviceIndex, int64_t pos, bool paused);
		void decRef(int virtualDeviceIndex);
		bool contains(int virtualDeviceIndex) const {return virtualDeviceIndex >= 0 && std::map<int, Device>::find(virtualDeviceIndex/2) != end();}
		bool empty() const {return std::map<int, Device>::empty();}
		void print() const;
		SampleBuffer * sampleBuffer(int virtualDeviceIndex);
		const SampleBuffer * sampleBuffer(int virtualDeviceIndex) const;
	};

	VirtualDevices _recordingDevices;
	std::vector<Channel> _channels;
	Devices _devices;
	int64_t _pos;
	bool _paused;
};

} // namespace BackyardBrains

#endif
