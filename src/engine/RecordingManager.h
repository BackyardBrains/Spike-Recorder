#ifndef BACKYARDBRAINS_RECORDINGMANAGER_H
#define BACKYARDBRAINS_RECORDINGMANAGER_H

#include <bass.h>
#include <sigslot.h>

#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include "Player.h"
#include "ArduinoSerial.h"
#include "HIDUsbManager.h"
#include "NotchFilter.h"
#include "LowPassFilter.h"
#include "HighPassFilter.h"
#include "AudioInputConfig.h"
#include "defaults/DefaultConfig.h"

#if defined(_WIN32)
   // #include "FirmwareUpdater.h"
    #include "BYBFirmwareVO.h"
   // #include "BSLFirmwareUpdater.h"
    #define LOCATION_OF_FIRMWARES "./firmwares"
#endif

#define HOW_LONG_FLUSH_IS_ACTIVE 1
#define AM_CARRIER_FREQUENCY 5000.0
#define AM_DEMODULATION_CUTOFF 500.0

#define THRESHOLD_SOURCE_SIGNAL 0
#define THRESHOLD_SOURCE_ALL_EVENTS 10

#define NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK 3
#define MAX_TIMER_FOR_KEY_RELEASE 700
namespace BackyardBrains {

class SampleBuffer;
struct MetadataChunk;

struct SpikeTrain {
	SpikeTrain() : upperThresh(0), lowerThresh(0), channelIndex(0), color(0) {}
	std::vector<int64_t> spikes;
	int upperThresh;
	int lowerThresh;
    int16_t channelIndex;

	int color;
};

class RecordingManager : public sigslot::has_slots<>
{
public:
	struct VirtualDevice
	{
		int device;
		int channel;
		std::string name;

		int threshold;
		bool bound;

	};
	static const int INVALID_VIRTUAL_DEVICE_INDEX;
	static const int DEFAULT_SAMPLE_RATE;

	RecordingManager();
	~RecordingManager();

	bool loadFile(const char *filename);
	void initRecordingDevices();

	Player &player();

    //alpha wave audio feedback

    float alphaWavePower;
    void turnAlphaFeedbackON();
    void turnAlphaFeedbackOFF();

    //position of the current file/recfording in samples
    //(this sample is at the middle of the screen in the audio view)
	int64_t pos() const {return _pos;}
	void setPos(int64_t pos, bool artificial = true); // file mode only

	void constructMetadata(MetadataChunk *m) const;
	void applyMetadata(const MetadataChunk &m);

	int sampleRate() const;
	void setSampleRate(int sampleRate);
	std::vector<VirtualDevice> &virtualDevices() {return _virtualDevices;}

	// Bound vdevices will have their sources initialized and be displayed in AudioView
	//
	// Parameters:
	// vdevice - index in _virtualDevices
	//
	bool bindVirtualDevice(int vdevice);
	bool unbindVirtualDevice(int vdevice);

	void getData(int virtualDevice, int64_t offset, int64_t len, int16_t *dest);
	void getTriggerData(int virtualDevice, int64_t len, int16_t *dest);
	std::vector< std::pair<int16_t, int16_t> > getSamplesEnvelope(int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip);
	std::vector< std::pair<int16_t, int16_t> > getTriggerSamplesEnvelope(int virtualDeviceIndex, int64_t len, int sampleSkip);
	bool paused() const {return _paused;}
	bool threshMode() const {return _threshMode;}
    void addTrigger(int64_t position);

	bool fileMode() const {return _fileMode;}
    std::list<std::string> serailPortsList() const {return _arduinoSerial.list;}
    std::list<ArduinoSerial::SerialPort> serailPorts() const {return _portScanningArduinoSerial.ports;}
    ArduinoSerial::SerialPort getCurrentPort() const {return _arduinoSerial.currentPort;}
	const std::string &fileName() const { return _filename; }
	int64_t fileLength(); // file mode only
	const char *fileMetadataString(); // file mode only
	int threshAvgCount() const {return _threshAvgCount;}
	int selectedVDevice() const {return _selectedVDevice;}
    int getThresholdSource();
    void setThresholdSource(int newThresholdSource);
    void clearTriggers();
    int getLastEventThresholded(void){return _lastThresholdedEvent;}
    void clearLastEventThresholded(void){_lastThresholdedEvent = -1;}


	std::vector<SpikeTrain> &spikeTrains() { return _spikeTrains; }
	const std::list<std::pair<std::string, int64_t> > &markers() const {return _markers;}
	void addMarker(const std::string &id, int64_t offset);

	void setPaused(bool pausing);
	void setThreshMode(bool threshMode);
	void setThreshAvgCount(int threshAvgCount);
	void setSelectedVDevice(int virtualDevice);
	void setVDeviceThreshold(int virtualDevice, int threshold);

	sigslot::signal0<> devicesChanged;
	sigslot::signal0<> pauseChanged;
	sigslot::signal0<> thresholdChanged;

	sigslot::signal0<> triggered;
    sigslot::signal0<> bufferReset;

	void advance(uint32_t milliseconds);

    //Serial port functions
    bool serialMode() const {return _serialMode;}
    void changeSerialPort(int portIndex);
    bool initSerial(const char *portNamem, int baudRate);
    void disconnectFromSerial();
    int serialPortIndex();
    void setSerialNumberOfChannels(int numberOfChannels);
    int numberOfSerialChannels();
    std::string serialError;
    void refreshSerialPorts();
    void resetCurrentSerial();

    //HID functions
    bool hidMode() const {return _hidMode;}
    bool initHIDUSB(HIDBoardType deviceType);
    void disconnectFromHID();
    int numberOfHIDChannels();
    void scanForHIDDevices();
    void scanUSBDevices();
    void sendEKGImpuls();
    void sendAlphaWavePowerToSerial();
    int currentAddOnBoard();
    bool isRTRepeating();
    void swapRTRepeating();
    void reloadHID();
    bool _HIDShouldBeReloaded;
    int currentlyConnectedHIDBoardType();
    bool isHIDBoardTypeAvailable(HIDBoardType hd);
    bool ignoreHIDReconnect();


    int numberOfChannels();


    void enable50HzFilter(){ _60HzFilterEnabled = false; _50HzFilterEnabled = true;}
    void disable50HzFilter(){_50HzFilterEnabled = false;}
    void enable60HzFilter(){_50HzFilterEnabled = false;_60HzFilterEnabled = true;}
    void disable60HzFilter(){_60HzFilterEnabled = false;}
    bool fiftyHzFilterEnabled() {return _50HzFilterEnabled;}
    bool sixtyHzFilterEnabled() {return _60HzFilterEnabled;}

    bool lowPassFilterEnabled() {return _lowPassFilterEnabled;}
    void enableLowPassFilterWithCornerFreq(float cornerFreq);
    void disableLowPassFilter(){_lowPassFilterEnabled = false;}

    bool highPassFilterEnabled() {return _highPassFilterEnabled;}
    void enableHighPassFilterWithCornerFreq(float cornerFreq);
    void disableHighPassFilter(){_highPassFilterEnabled = false;}
    int highCornerFrequency();
    int lowCornerFrequency();

    bool weShouldDisplayWaveform();

    void setKeyForJoystick(int channelIndex, int keyIndex);
    int getKeyIndexForJoystick(int channelIndex);
    void initDefaultJoystickKeys();

    void setCalibrationCoeficient(float newCalibrationCoeficient);
    void resetCalibrationCoeficient();
    bool isCalibrated(){return systemIsCalibrated;}
    float getCalibrationCoeficient(){return calibrationCoeficient;}

    std::string currentHIDFirmwareVersion() {return _hidUsbManager.firmwareVersion;}
    std::string currentHIDFirmwareType() {return _hidUsbManager.hardwareType;}
    std::string currentHIDHardwareVersion() {return _hidUsbManager.hardwareVersion;}


    void saveGainForAudioInput(float newGain);
    void saveTimeScaleForAudioInput(float newTimeScale);
    float loadGainForAudioInput();
    float loadTimeScaleForAudioInput();
    bool firstTimeInitializationOfSettingsForAudioInput = true;

    //used for fake load of half of the screen with waveform from file
    bool fileIsLoadedAndFirstActionDidNotYetOccurred= false;

    //check if buffer has loaded data at "pos" position
    bool isBufferLoadedAtPosition(long pos);

    #if defined(_WIN32)
        int prepareForHIDFirmwareUpdate(BYBFirmwareVO * firmwareToUpdate);
        int getUSBFirmwareUpdateStage();
        bool shouldStartFirmwareUpdatePresentation;
        bool firmwareAvailable();
        int finishAndCleanFirmwareUpdate();
        std::list<BYBFirmwareVO> firmwareList();
        int powerStateOnHID();
        void askForPowerStateHIDDevice();
        void startActualFirmwareUpdateOnDevice();

    #endif


    std::string hidError;

    int getCurrentInputType();
    int deviceUsedForRecordingFile();

private:
	struct Device
	{
		// samplerate is a reference because we can only have one sample rate application wide
		// i.e. the one in _sampleRate. When it changes, this one needs to change, too.
		Device(int index, int nchan, int &samplerate);
		~Device();

		// Initialize device so it is ready for recording.
		// Returns true on success.
		bool enable(int64_t pos);

		// Deinitialize device after it is no longer used.
		// Returns true on success.
		bool disable();

		// Type of the device. This will decide what enable()/disable() will do.
		// We could also use subclassing or something to do this.
		enum {
			Audio,
			File,
			HID,
			Serial
		} type;

		int index;
		HRECORD handle;

		bool enabled;

        std::vector<NotchFilter> _50HzNotchFilters;
        std::vector<NotchFilter> _60HzNotchFilters;
        std::vector<LowPassFilter> _lowPassFilters;
        std::vector<HighPassFilter> _highPassFilters;
		std::vector<SampleBuffer> sampleBuffers;
		std::vector<int64_t> dcBiasSum;

		int64_t dcBiasNum;
		int channels;
		int &samplerate;
		int bytespersample;
	};

	void clear();
	void advanceSerialMode(uint32_t samples);
	void advanceFileMode(uint32_t samples);
	void advanceHidMode(uint32_t samples);
	void sincDisconnectFromSerial();
	bool waitToDisconnectFromSerial;
	void closeSerial();
	void closeHid();
	SampleBuffer *sampleBuffer(int virtualDeviceIndex);

    uint32_t numTicksBefore = 0;
    bool systemIsCalibrated;
    float calibrationCoeficient;

	std::vector<VirtualDevice> _virtualDevices;
	std::vector<Device> _devices;

	int64_t _pos; //position of the current file/recording in samples
	bool _paused;
	bool _threshMode;

    void startRemovingMeanValue();

    bool _50HzFilterEnabled = false;
    bool _60HzFilterEnabled= false;
    bool _lowPassFilterEnabled = false;
    bool _highPassFilterEnabled = false;
    float _highCornerFreq;
    float _lowCornerFreq;

	int64_t currentPositionOfWaveform;

	bool _fileMode;
	bool _serialMode;
	bool _hidMode;
	std::string _filename;
    int debugNumberOfSamplesThatWeNeed;
    int debugNumberOfSamplesThatWeGet;

    //it keeps last zoom, gain and filter configs for each type of inputs
    AudioInputConfig audioInputConfigArray[11];//standard audio, file, HID, 3  shields, and unknown shield
    std::list<AudioInputConfig> arduinoShieldsConfigs;
    void initInputConfigPersistance();
    void saveInputConfigSettings();
    AudioInputConfig * getInputConfigForType(int inputType);

    void loadFilterSettings();
    void makeNewSerialAudioConfig(std::string nameOfThePort);
    std::list<AudioInputConfig>::iterator iteratorPointerToCurrentSerialAudioConfig;

	int _sampleRate;

	int _selectedVDevice; // triggers threshold/is played on the speakers
	int _threshAvgCount;
	std::list<int64_t> _triggers;
	std::list<std::pair<std::string,int64_t> > _markers;
	std::vector<SpikeTrain> _spikeTrains;

	Player _player;
    LowPassFilter filterSoundForPlayer;
	int _thresholdSource = 0;//signal is the default one (1,2,3,4 ... are events)
    ///used to keep last sample for threshold so that we can detect crossing of
    //threshold
    int lastSampleForThreshold = 0;
    int _lastThresholdedEvent = -1;

	int _serialPortIndex;
	ArduinoSerial _arduinoSerial;
    ArduinoSerial _portScanningArduinoSerial;
	int _numOfSerialChannels;

	HIDUsbManager _hidUsbManager;
	int _numOfHidChannels;
	clock_t timerUSB = 0;
	clock_t timerEKG = 0;

    int _keyIndexSetForJoystick[NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK];
    int64_t _timeOfLastTriggerJoystick[NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK];
    int _lastValueOfSignalJoystick[NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK];
    std::vector<int> keyReleaseList;
    int64_t _timersForKeyRelease[NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK];

	int _firmwareUpdateStage;//this needs to be outside exclusive win block

    #if defined(_WIN32)
       // FirmwareUpdater _xmlFirmwareUpdater;
      //  BSLFirmwareUpdater _bslFirmwareUpdater;
        std::list<BYBFirmwareVO> _currentFirmwares;
        std::vector<BYBFirmwareVO> getAllFirmwaresFromLocal();
    #endif
    #if defined(_WIN32)
    DefaultConfig configValues;
    #endif
    double _alphaAudioTime = 0;
    bool alphaFeedbackActive;

    bool loadSecondSegmentOfBuffer = false;//used to force loading ofwhole buffer after reseting buffer


    NotchFilter amDetectionNotchFilter;
    LowPassFilter amDemodulationLowPassFilter[6];

    int16_t * amBuffer;
    float rmsOfOriginalSignal = 0;
    float rmsOfNotchedAMSignal = 0;
    bool weAreReceivingAMSignal = false;


    int deviceTypeUsedDuringRecordingOfCurrentFile = 0;

};

} // namespace BackyardBrains

#endif
