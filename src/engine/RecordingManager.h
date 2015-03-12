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
#include "ArduinoSerial.h"

namespace BackyardBrains {

class SampleBuffer;
struct MetadataChunk;

class RecordingManager : public sigslot::has_slots<>
{
public:

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
	static const int DEFAULT_SAMPLE_RATE;
	typedef std::vector<VirtualDevice> VirtualDevices;

	RecordingManager();
	~RecordingManager();

	bool loadFile(const char *filename);
	void initRecordingDevices();

	Player &player();

	int64_t pos() const {return _pos;}
	void setPos(int64_t pos, bool artificial = true); // file mode only

	void constructMetadata(MetadataChunk *m) const;
	void applyMetadata(const MetadataChunk &m);

	int sampleRate() const;
	void setSampleRate(int sampleRate);
	VirtualDevices &recordingDevices() {return _recordingDevices;}
	void getData(int virtualDevice, int64_t offset, int64_t len, int16_t *device);
	std::vector< std::pair<int16_t, int16_t> > getSamplesEnvelope(int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip);
	std::vector< std::pair<int16_t, int16_t> > getTriggerSamplesEnvelope(int virtualDeviceIndex, int64_t len, int sampleSkip);
	bool paused() const {return _paused;}
	bool threshMode() const {return _threshMode;}
	bool fileMode() const {return _fileMode;}
    std::list<std::string> serailPortsList() const {return _arduinoSerial.list;}	
	const std::string &fileName() const { return _filename; }
	int64_t fileLength(); // file mode only
	const char *fileMetadataString(); // file mode only
	int threshAvgCount() const {return _threshAvgCount;}
	int selectedVDevice() const {return _selectedVDevice;}

	const std::vector<std::list<int64_t> > &spikeTrains() const { return _spikeTrains; }
	const std::list<std::pair<std::string, int64_t> > &markers() const {return _markers;}
	void addMarker(const std::string &id, int64_t offset);

	bool incRef(int virtualDeviceIndex);
	void decRef(int virtualDeviceIndex);

	void setPaused(bool pausing);
	void setThreshMode(bool threshMode);
	void setThreshAvgCount(int threshAvgCount);
	void setSelectedVDevice(int virtualDevice);
	void setVDeviceThreshold(int virtualDevice, int threshold);

	sigslot::signal0<> deviceReload;
	sigslot::signal0<> pauseChanged;

	void advance(uint32_t milliseconds);
	
    //Serial port functions
    bool serialMode() const {return _serialMode;}
    void changeSerialPort(int portIndex);
    bool initSerial(const char *portName);
    void disconnectFromSerial();
    int serialPortIndex();
    void setSerialNumberOfChannels(int numberOfChannels);
    int numberOfSerialChannels();
    std::string serialError;
    void refreshSerialPorts();	
private:
	struct Device
	{
		Device() : handle(0), refCount(0), dcBiasNum(1), channels(0), bytespersample(2)
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
		int bytespersample;
	};

	void clear();
    void advanceSerialMode(uint32_t samples);	
	void advanceFileMode(uint32_t samples);
    void closeSerial();	
	SampleBuffer *sampleBuffer(int virtualDeviceIndex);

	VirtualDevices _recordingDevices;
	std::map<int, Device> _devices;
	int64_t _pos;
	bool _paused;
	bool _threshMode;

	bool _fileMode;
    bool _serialMode;	
	std::string _filename;

	int _sampleRate;

	int _selectedVDevice; // triggers threshold/is played on the speakers
	int _threshAvgCount;
	std::list<int64_t> _triggers;
	std::list<std::pair<std::string,int64_t> > _markers;
	std::vector<std::list<int64_t> > _spikeTrains;
	Player _player;
	
	int _serialPortIndex;
    ArduinoSerial _arduinoSerial;
    int _numOfSerialChannels;
};

} // namespace BackyardBrains

#endif
