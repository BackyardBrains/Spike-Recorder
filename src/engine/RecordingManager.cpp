#include "RecordingManager.h"
#include "BASSErrors.h"
#include "SampleBuffer.h"
#include "FileRecorder.h"
#include "FileReadUtil.h"
#include "Log.h"
#include <sstream>
#include <cstdlib>
#include <SDL.h>



#if defined(_WIN32)
    #include <unistd.h>
    #include <cmath>
    #include <windows.h>
#endif
#define BOARD_WITH_JOYSTICK 5
#define FIRMWARE_PATH_FOR_STM32 "/firmwareUpdate.hex"
#define HPF_HUMAN_SP_THRESHOLD 20
#define LPF_HUMAN_SP_THRESHOLD 70
namespace BackyardBrains {

const int RecordingManager::INVALID_VIRTUAL_DEVICE_INDEX = -2;
const int RecordingManager::DEFAULT_SAMPLE_RATE = 44100;

RecordingManager::RecordingManager() : _pos(0), _paused(false), _threshMode(false), _fileMode(false), _sampleRate(DEFAULT_SAMPLE_RATE), _selectedVDevice(0), _threshAvgCount(1) {
	Log::msg("Initializing libbass...");
	if(!BASS_Init(-1, _sampleRate, 0, 0, NULL)) {
		Log::msg("Bass 1 initialization failed: %s", GetBassStrError());
		if(!BASS_Init(0, _sampleRate, 0, 0, NULL))
        {
            Log::fatal("Bass 2 initialization failed: %s", GetBassStrError());
        }
	}
	#if defined(_WIN32)
        //load acc plugin for .m4a files ALAC (Apple Lossless Audio Codec)

        if(!BASS_PluginLoad("bass_aac.dll", NULL))
        {
            Log::fatal("Bass plugin failed: %s", GetBassStrError());
        }

        if(!BASS_PluginLoad("bassalac.dll", NULL))
        {
            Log::fatal("Bass plugin failed: %s", GetBassStrError());
        }
	#endif
    _numOfSerialChannels = 1;
    _numOfHidChannels = 2;
    _firmwareUpdateStage = 0;

    initInputConfigPersistance();

    #if defined(_WIN32)
    shouldStartFirmwareUpdatePresentation = false;
    #endif
	_player.start(_sampleRate);
    _HIDShouldBeReloaded = false;
    waitToDisconnectFromSerial = false;
    systemIsCalibrated = false;
    calibrationCoeficient = 1.0f;
    
    _firmwareForBootloaderAvailable = false;
    _bootloaderController.firmwarePath = getRecordingPath()+FIRMWARE_PATH_FOR_STM32;
    _portScanningArduinoSerial.setRecordingManager(this);
    _arduinoSerial.getAllPortsList();

    std::list<std::string>::iterator list_it;
    for(list_it = _arduinoSerial.list.begin(); list_it!= _arduinoSerial.list.end(); list_it++)
    {
            std::cout<<list_it->c_str()<<"\n";
    }

    alphaFeedbackActive = false;
    alphaWavePower = 0;

	initRecordingDevices();

    _arduinoSerial.setRecordingManager(this);


    _portScanningArduinoSerial.startScanningForArduinos(&_arduinoSerial);

    #if defined(_WIN32)
    configValues.loadDefaults();
    #endif
    initDefaultJoystickKeys();
    _p300Active = false;
    _p300AudioStimulationActive = false;

}

RecordingManager::~RecordingManager() {
	clear();
	_player.stop();
	BASS_Free();
	Log::msg("libbass deinitialized.");
}

void RecordingManager::setCalibrationCoeficient(float newCalibrationCoeficient)
{
    systemIsCalibrated = true;
    calibrationCoeficient = newCalibrationCoeficient;
}

void RecordingManager::resetCalibrationCoeficient()
{
    systemIsCalibrated = false;
    calibrationCoeficient = 0.005f;
}




#pragma mark - HID device
//--------------- HID USB functions -------------------------

void RecordingManager::reloadHID()
{
    Log::msg("Reload HID");
    _HIDShouldBeReloaded = true;
}

bool RecordingManager::initHIDUSB(HIDBoardType deviceType)
{

    Log::msg("Init HID");
    saveInputConfigSettings();
    if(!_hidUsbManager.deviceOpened())
    {
        if(_hidUsbManager.openDevice(this, deviceType) == -1)
        {
            _hidMode = false;
            hidError = _hidUsbManager.errorString;
            return false;
        }
    }
    clear();
    DWORD frequency = _hidUsbManager.maxSamplingRate();
    _numOfHidChannels = _hidUsbManager.numberOfChannels();
    //std::cout<<"HID Frequency: "<<frequency<<" Chan: "<<_hidUsbManager.numberOfChannels()<<" Samp: "<<_hidUsbManager.maxSamplingRate()<<"\n";
    Log::msg("HID Frequency: %d Chan: %d Samp: %d", frequency, _hidUsbManager.numberOfChannels(), _hidUsbManager.maxSamplingRate());
    HSTREAM stream = BASS_StreamCreate(frequency, _hidUsbManager.numberOfChannels(), BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
    if(stream == 0) {
        std::cerr << "Bass Error: Failed to open hid stream. \n";
        Log::msg("Bass Error: Failed to open hid stream.");
        hidError = "Bass Error: Failed to open hid stream. \n";
        return false;
    }


    BASS_CHANNELINFO info;
    BASS_ChannelGetInfo(stream, &info);


	_lowPassFilterEnabled = false;
	_highPassFilterEnabled = false;
	_lowCornerFreq = frequency/2;


    _highCornerFreq = 1;

    int bytespersample = info.origres/8;

    bytespersample = 4; // bass converts everything it doesn't support.
    setSampleRate(info.freq);
    filterSoundForPlayer.initWithSamplingRate(info.freq);
    filterSoundForPlayer.setCornerFrequency(1000);
    filterSoundForPlayer.setQ(1.0);
  //_player.setSampleRate(info.freq * 1.05);

    _devices.push_back(Device(0, _numOfHidChannels, _sampleRate));
    _devices[0].bytespersample = bytespersample;
    _devices[0].type = Device::HID;

    for(unsigned int i = 0; i < (unsigned int)_numOfHidChannels; i++) {
         VirtualDevice virtualDevice;

        virtualDevice.device = 0;
        virtualDevice.channel = i;

         std::stringstream sstm; //variable for log
         sstm << "Hid channel "<<(i+1);


        virtualDevice.name = sstm.str();
        virtualDevice.threshold = 100;
        virtualDevice.bound = false;

        _virtualDevices.push_back(virtualDevice);
    }


    _devices[0].handle = stream;
    _fileMode = false;
    _serialMode = false;
    _hidMode = true;

    if(_numOfHidChannels ==4)
    {
        bindVirtualDevice(0);
        bindVirtualDevice(2);
    }
     if(_numOfHidChannels ==3)//this is hack for presentation with hammer
    {
        bindVirtualDevice(0);
        bindVirtualDevice(2);
    }
    else
    {
        if(_numOfHidChannels>0)
        {
            bindVirtualDevice(0);
           // bindVirtualDevice(1);
        }

    }

    /* for(unsigned int i = 0; i < (unsigned int)_numOfHidChannels;i++)
     {
            bindVirtualDevice(i);
     }
*/
    //setCalibrationCoeficient(0.005f);

    //_player.stop();
    //_player.start(_hidUsbManager.maxSamplingRate());
    _player.setVolume(0);

    loadFilterSettings();
    return true;
}

void RecordingManager::disconnectFromHID()
{
     Log::msg("Disconnect from HID ----------------------------------!!!!!!");
    initDefaultJoystickKeys();
    initRecordingDevices();
    closeHid();
}

void RecordingManager::closeHid()
{
    _numOfHidChannels = 2;
    _hidUsbManager.closeDevice();
    _hidMode = false;
}

int RecordingManager::numberOfHIDChannels()
{
    return _numOfHidChannels;
}



void RecordingManager::scanForHIDDevices()
{
    try{
        if(_firmwareUpdateStage<1)
        {
            _hidUsbManager.getAllDevicesList();
        }
    }catch(int e)
    {
        Log::msg("Error while scanning HID devices.");
    }


}

bool RecordingManager::isHIDBoardTypeAvailable(HIDBoardType hd)
{
    return _hidUsbManager.isBoardTypeAvailable(hd);
}

int RecordingManager::currentlyConnectedHIDBoardType()
{
    return _hidUsbManager.currentlyConnectedHIDBoardType();
}

bool RecordingManager::ignoreHIDReconnect()
{
        return _hidUsbManager.ignoreReconnect();
}


void RecordingManager::scanUSBDevices()
{

    clock_t end = clock();
    double elapsed_secs = double(end - timerUSB) / CLOCKS_PER_SEC;
    if(elapsed_secs>1)
    {
        timerUSB = end;

        //Log::msg("Scanning for HID");

        scanForHIDDevices();
    }
}


#if defined(_WIN32)

         std::list<BYBFirmwareVO> RecordingManager::firmwareList()
         {
             _currentFirmwares.clear();

            //first check if there are firmwares that have same hardware version and same hardware type
            for( std::list<BYBFirmwareVO>::iterator ti = _xmlFirmwareUpdater.firmwares.begin();
                    ti != _xmlFirmwareUpdater.firmwares.end();
                    ti ++)
                {
                    if(((BYBFirmwareVO)(*ti)).hardware.compare(_hidUsbManager.hardwareVersion)==0)
                    {
                        //found hardware version
                        if(((BYBFirmwareVO)(*ti)).type.compare(_hidUsbManager.hardwareType)==0)
                        {

                             BYBFirmwareVO tempBYBFirmware;
                            tempBYBFirmware.description = ((BYBFirmwareVO)(*ti)).description;
                            tempBYBFirmware.URL =((BYBFirmwareVO)(*ti)).URL;
                            tempBYBFirmware.version = ((BYBFirmwareVO)(*ti)).version;
                            tempBYBFirmware.type = ((BYBFirmwareVO)(*ti)).type;
                            tempBYBFirmware.id = ((BYBFirmwareVO)(*ti)).id;
                            tempBYBFirmware.hardware = ((BYBFirmwareVO)(*ti)).hardware;
                            tempBYBFirmware.location = REMOTE_FIRMWARE;
                            _currentFirmwares.push_back(tempBYBFirmware);
                        }
                    }

                }


            std::vector<BYBFirmwareVO> tempLocalFirmwares =  getAllFirmwaresFromLocal();
            for (int i=0;i<tempLocalFirmwares.size();i++)
            {
                _currentFirmwares.push_back(tempLocalFirmwares[i]);
            }

            return _currentFirmwares;
         }

        //
        // serach for .txt files in SpikeRecorder/Firmwares directory
        // and make array of firmware objects BYBFirmwareVO
        //
        std::vector<BYBFirmwareVO> RecordingManager::getAllFirmwaresFromLocal()
        {
            std::string folder = LOCATION_OF_FIRMWARES;
            std::vector<BYBFirmwareVO> localFirmwares;
            std::string search_path = folder + "/*.txt";

            WIN32_FIND_DATA fd;
            HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
            if(hFind != INVALID_HANDLE_VALUE) {
                     std::cout<<"\nFound firmware:\n";
                do {
                    // read all (real) files in current folder
                    // , delete '!' read other 2 default folder . and ..
                    if(! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                        std::stringstream descriptionOfFirmware;
                        descriptionOfFirmware <<"Local firmware ("<< fd.cFileName<<")";
                        BYBFirmwareVO tempBYBFirmware;

                        tempBYBFirmware.description = descriptionOfFirmware.str();
                        tempBYBFirmware.URL =fd.cFileName;
                        std::string tempFirmwareFilepath = folder + "/"+fd.cFileName;
                        tempBYBFirmware.filepath =tempFirmwareFilepath;
                        //tempBYBFirmware.version = ((BYBFirmwareVO)(*ti)).version;
                        //tempBYBFirmware.type = ((BYBFirmwareVO)(*ti)).type;
                        //tempBYBFirmware.id = ((BYBFirmwareVO)(*ti)).id;
                        //tempBYBFirmware.hardware = ((BYBFirmwareVO)(*ti)).hardware;
                        tempBYBFirmware.location = LOCAL_FIRMWARE;

                        localFirmwares.push_back(tempBYBFirmware);

                    }
                    std::cout<<fd.cFileName<<"\n";
                }while(::FindNextFile(hFind, &fd));
                ::FindClose(hFind);
            }
            return localFirmwares;
        }


        //
        // Check if we downloaded list of firmwares from server
        // And if there are some firmwares that work with current version of software
        //
        bool  RecordingManager::firmwareAvailable()
        {

                std::list<BYBFirmwareVO> tempCurrentFirmwares;

                //first check if there are firmwares that have same hardware version and hardware type
                for( std::list<BYBFirmwareVO>::iterator ti = _xmlFirmwareUpdater.firmwares.begin();
                    ti != _xmlFirmwareUpdater.firmwares.end();
                    ti ++)
                {
                    //check if hardware version is the same with connected HID device
                    //we don't want to overwrite hardware version that is in firmware with firmware that has
                    //different hardware version

                    //std::cout<<"HDW XML: "<<((BYBFirmwareVO)(*ti)).hardware<<" - HID: "<<_hidUsbManager.hardwareVersion<<" TYPE XML: "<<((BYBFirmwareVO)(*ti)).type<<" - HID: "<<_hidUsbManager.hardwareType<<" \n";
                    if(((BYBFirmwareVO)(*ti)).hardware.compare(_hidUsbManager.hardwareVersion)==0)
                    {
                        //found hardware version

                        //check if we have hardware type that is equal to hardware type that is connected
                        if(((BYBFirmwareVO)(*ti)).type.compare(_hidUsbManager.hardwareType)==0)
                        {

                             BYBFirmwareVO tempBYBFirmware;
                            tempBYBFirmware.description = ((BYBFirmwareVO)(*ti)).description;
                            tempBYBFirmware.URL =((BYBFirmwareVO)(*ti)).URL;
                            tempBYBFirmware.version = ((BYBFirmwareVO)(*ti)).version;
                            tempBYBFirmware.type = ((BYBFirmwareVO)(*ti)).type;
                            tempBYBFirmware.id = ((BYBFirmwareVO)(*ti)).id;
                            tempBYBFirmware.hardware = ((BYBFirmwareVO)(*ti)).hardware;
                            tempBYBFirmware.location = REMOTE_FIRMWARE;
                            tempCurrentFirmwares.push_back(tempBYBFirmware);
                        }
                    }

                }

                std::vector<BYBFirmwareVO> tempLocalFirmwares =  getAllFirmwaresFromLocal();
                for (int i=0;i<tempLocalFirmwares.size();i++)
                {
                    _currentFirmwares.push_back(tempLocalFirmwares[i]);
                }

                return tempCurrentFirmwares.size()>0;
        }

        //
        // Update stage of firmware. Integer composed based on update stage of BSLFirmwareUpdater
        // and state of the RecordingManager and Config view
        // -1 - error
        // 0 - update not active
        // getUSBFirmwareUpdateStage >0 update in progress
        //
        int RecordingManager::getUSBFirmwareUpdateStage()
        {

            if(_bslFirmwareUpdater.currentStage>=0)
            {
                return _firmwareUpdateStage+_bslFirmwareUpdater.currentStage;
            }
            else
            {
              _firmwareUpdateStage = 0;
              return -1;
            }

        }

        //
        // Called after firmware update to enable periodic scan for USB device etc.
        // resets update stage flages
        //
        int RecordingManager::finishAndCleanFirmwareUpdate()
        {
            _firmwareUpdateStage = 0;
            _bslFirmwareUpdater.currentStage = 0;
        }


        //
        // Initialize update of firmware
        //
        int RecordingManager::prepareForHIDFirmwareUpdate(BYBFirmwareVO * firmwareToUpdate)
        {
                 //download selected firmware from BYB server
                 if(firmwareToUpdate->location == REMOTE_FIRMWARE)
                 {
                    if(_xmlFirmwareUpdater.downloadFirmware(firmwareToUpdate))
                    {
                        //if we have error while downloading the selected firmware
                        return 1;
                    }
                 }
                 if(firmwareToUpdate->location == LOCAL_FIRMWARE)
                 {
                    CopyFile(firmwareToUpdate->filepath.c_str(), "newfirmware.txt", FALSE);
                 }
                _firmwareUpdateStage = 1;
                _hidUsbManager.askForStateOfPowerRail();
                shouldStartFirmwareUpdatePresentation = true;//this will open the firmware update view
                
                return 0;
        }

        void RecordingManager::startActualFirmwareUpdateOnDevice()
        {
             //send command to HID device to prepare for update
             // this will disconnect HID device and it will enumerate with different VID/PID (TI's update VID/PID)
             _hidUsbManager.putInFirmwareUpdateMode();
             //Start procedure of programming firmware
             _bslFirmwareUpdater.customSelectedFirmware("newfirmware.txt");
             
        }

        int RecordingManager::powerStateOnHID()
        {
            return _hidUsbManager.powerRailIsState();
        }

        void RecordingManager::askForPowerStateHIDDevice()
        {
            _hidUsbManager.askForStateOfPowerRail();
        }
#endif


int RecordingManager::currentAddOnBoard()
{

    if(_serialMode)
	{

	    return _arduinoSerial.addOnBoardPressent();
	}
	else
    {
        return _hidUsbManager.addOnBoardPressent();
    }

}

bool RecordingManager::isRTRepeating()
{
    return _hidUsbManager.isRTRepeating();
}

void RecordingManager::swapRTRepeating()
{
    _hidUsbManager.swapRTRepeat();
}

#pragma mark - Serial device
//--------------- Serial port functions ---------------------

bool RecordingManager::weShouldDisplayWaveform()
{
    if(serialMode())
    {
        return !(_devices.begin()->dcBiasNum < _sampleRate*HOW_LONG_FLUSH_IS_ACTIVE);
    }
        return true;

}


bool RecordingManager::initSerial(const char *portName, int baudRate)
{

    resetCalibrationCoeficient();
    saveInputConfigSettings();

    if(!_arduinoSerial.portOpened())
    {
        _arduinoSerial.setBaudRate(baudRate);
        Log::msg("initSerial - Open serial device %s |||||||||||||||||||||||||||||||||||||||||", portName);
        if(_arduinoSerial.openSerialDevice(portName) == -1)
        {
            _serialMode = false;
            serialError = _arduinoSerial.errorString;
            return false;
        }
    }

    //make audio congfig
    std::string nameOfThePort = portName;
    makeNewSerialAudioConfig(nameOfThePort);
    _arduinoSerial.setSampleRateAndNumberOfChannelsBasedOnType();

    DWORD frequency = _arduinoSerial.getSampleRate();//_arduinoSerial.maxSamplingRate()/_numOfSerialChannels;
    _sampleRate = _arduinoSerial.getSampleRate();
    _numOfSerialChannels = _arduinoSerial.numberOfChannels();
    std::cout<<"Frequency: "<<frequency<<" Chan: "<<_numOfSerialChannels<<" Samp: "<<_arduinoSerial.maxSamplingRate()<<"\n";
    HSTREAM stream = BASS_StreamCreate(frequency, _numOfSerialChannels, BASS_STREAM_DECODE, STREAMPROC_PUSH, NULL);
    if(stream == 0) {
        std::cerr << "Bass Error: Failed to open serial stream. \n";
        serialError = "Bass Error: Failed to open serial stream. \n";
        return false;
    }

    clear();
    BASS_CHANNELINFO info;
    BASS_ChannelGetInfo(stream, &info);


	_lowPassFilterEnabled = false;
	_highPassFilterEnabled = false;
	_lowCornerFreq = frequency/2;
	_highCornerFreq = 0;

    int bytespersample = info.origres/8;
   /* if(bytespersample == 0)
    {
        std::cerr << "Bass Error: Error init serial stream. \n";
        return false;
    }*/

    bytespersample = 4; // bass converts everything it doesnt support.
    setSampleRate(info.freq);
    _devices.push_back(Device(0, _numOfSerialChannels,_sampleRate));
    _devices.back().type = Device::Serial;
    _devices[0].bytespersample = bytespersample;


    for(unsigned int i = 0; i < (unsigned int)_numOfSerialChannels; i++) {
         VirtualDevice virtualDevice;

        virtualDevice.device = 0;
        virtualDevice.channel = i;
        virtualDevice.name = "Serial channel";
        virtualDevice.threshold = 100;
        virtualDevice.bound = false;
        _virtualDevices.push_back(virtualDevice);

    }
    
    

    _devices[0].handle = stream;
    _fileMode = false;
    _hidMode = false;
    _serialMode = true;
    
    _p300Active = false;
    _p300AudioStimulationActive = false;
   // devicesChanged.emit();

    if(_arduinoSerial.currentPort.deviceType==ArduinoSerial::humansb)
    {
        bindVirtualDevice(0);
        if(_numOfSerialChannels>2)
        {
            bindVirtualDevice(2);
        }
    }
    else
    {
        for(unsigned int i = 0; i < (unsigned int)_numOfSerialChannels;i++)
        {
            bindVirtualDevice(i);
        }
    }


  //  _player.stop();
  //  _player.start(_arduinoSerial.maxSamplingRate()/_numOfSerialChannels);
    _player.setVolume(0);
    loadFilterSettings();
    _arduinoSerial.askForImportantStates();
	return true;
}

void RecordingManager::refreshSerialPorts()
{
	_arduinoSerial.getAllPortsList();
}

void RecordingManager::changeSerialPort(int portIndex)
{
	//std::cout<<"Change serial port to: "<<portIndex<<"\n";
	_serialPortIndex = portIndex;
}


void RecordingManager::resetCurrentSerial()
{
    setSerialNumberOfChannels(1);
}

//
// Change number of channels we are sampling through serial port
// set sampling rate to 10000Hz/(number of channels)
//
void RecordingManager::setSerialNumberOfChannels(int numberOfChannels)
{
	std::cout<<"Number of channels on serial: "<<numberOfChannels<<"\n";
	_numOfSerialChannels = numberOfChannels;
	_arduinoSerial.setNumberOfChannelsAndSamplingRate(numberOfChannels, _arduinoSerial.maxSamplingRate()/numberOfChannels);
	initSerial(_arduinoSerial.currentPortName(), _arduinoSerial.getBaudRate());
}

int RecordingManager::numberOfSerialChannels()
{
	return _numOfSerialChannels;
}


void RecordingManager::sendEKGImpuls()
{

    clock_t end = clock();
#if defined(__APPLE__) || defined(__linux__)
    end = end*1000;
#endif
    double elapsed_secs = double(end - timerEKG) / CLOCKS_PER_SEC;
    if(elapsed_secs>0.5)
    {

        timerEKG = end;
        //_arduinoSerial.sendEventMessage(0);
    }

}


//
// Used so that config knows which one is selected in dropdown
//
int RecordingManager::serialPortIndex()
{
   return std::max(0, std::min(_serialPortIndex,(int)(_arduinoSerial.list.size()-1)));
}

void RecordingManager::disconnectFromSerial()
{
    waitToDisconnectFromSerial = true;
}

void RecordingManager::sincDisconnectFromSerial()
{
    waitToDisconnectFromSerial = false;
    closeSerial();
	initRecordingDevices();
}

void RecordingManager::closeSerial()
{
	_numOfSerialChannels = 1;
   // _arduinoSerial.setNumberOfChannelsAndSamplingRate(1, _arduinoSerial.maxSamplingRate());
    _arduinoSerial.closeCurrentMainSerial();
	//_arduinoSerial.closeSerial();
	_serialMode = false;
}
    
void  RecordingManager::setSerialHardwareGain(bool active)
{
    if(_serialMode)
    {
        _arduinoSerial.setGain(active);
    }
}
    
    
void RecordingManager::setSerialHardwareHPF(bool active)
{
    if(_serialMode)
    {
        _arduinoSerial.setHPF(active);
    }
}

void RecordingManager::setP300OnHardware(bool active)
{
    if(_serialMode)
    {
        _arduinoSerial.setP300(active);
        _p300Active = active;
    }
}

void RecordingManager::setP300ActiveStateLocaly(bool active)
{
    _p300Active = active;
}

void RecordingManager::setP300AudioActiveStateLocaly(bool active)
{
    _p300AudioStimulationActive = active;
}

void RecordingManager::setP300SoundStimmulationOnHardware(bool active)
{
    if(_serialMode)
    {
        _arduinoSerial.setP300AudioStimulation(active);
        _p300AudioStimulationActive = active;
    }
}

#pragma mark - File device

bool RecordingManager::loadFile(const char *filename) {

    resetCalibrationCoeficient();
    saveInputConfigSettings();
    resetCalibrationCoeficient();
	_spikeTrains.clear();
	closeSerial();
	closeHid();
    _lowPassFilterEnabled = false;
	_highPassFilterEnabled = false;

    int nchan;
    int samplerate;
    int bytespersample;
    HSTREAM stream;
    
    bool success =  openAnyFile(filename, stream, nchan, samplerate, bytespersample);
    
    if(!success)
    {
        return false;
    }
	
	currentPositionOfWaveform = 0;//set position of waveform to begining

	clear();
	
	setSampleRate(samplerate);
	_devices.push_back(Device(0,nchan,_sampleRate));
	_devices[0].bytespersample = bytespersample;
	_devices[0].type = Device::File;

	_virtualDevices.resize(nchan);
	for(unsigned int i = 0; i < nchan; i++) {
		VirtualDevice &virtualDevice = _virtualDevices[i];

		virtualDevice.device = 0;
		virtualDevice.channel = i;
		std::stringstream sstm;
		sstm << "File channel "<<(i+1);
		virtualDevice.name = sstm.str();
		virtualDevice.threshold = 100;
		virtualDevice.bound = false;
	}

	for(unsigned int i = 0; i < nchan; i++) {
		bindVirtualDevice(i);
	}

	_devices[0].handle = stream;
	_fileMode = true;
	_filename = filename;


	_player.stop();

	_player.start(_sampleRate);
	devicesChanged.emit();

	_player.setVolume(100);

	if(!_paused) {
		pauseChanged.emit();
		setPaused(true);
	}

	Log::msg("loaded file '%s'.", filename);
    fileIsLoadedAndFirstActionDidNotYetOccurred = true;
	return true;
}

void RecordingManager::initRecordingDevices() {
	BASS_DEVICEINFO info;
	VirtualDevice virtualDevice;
    if(getCurrentInputType()!=INPUT_TYPE_STANDARD_AUDIO)
    {
        saveInputConfigSettings();
    }

    resetCalibrationCoeficient();

	clear();
	_fileMode = false;
	_spikeTrains.clear();
	_serialMode = false;
	_hidMode = false;

	_lowPassFilterEnabled = false;
	_highPassFilterEnabled = false;
	setSampleRate(DEFAULT_SAMPLE_RATE);
	int i;
	for(i = 0; BASS_RecordGetDeviceInfo(i, &info); i++) {
		_devices.push_back(Device(i,2,_sampleRate));
		_devices.back().type = Device::Audio;

		// do not add virtualDevices for devices that are not enabled.
		if(!(info.flags & BASS_DEVICE_ENABLED))
			continue;
		for (int j = 0; j < 2; j++)	{
			virtualDevice.device = i;
			virtualDevice.channel = j;
			virtualDevice.name = std::string(info.name) + ((j == 0) ? " [Left]" : " [Right]");
			virtualDevice.threshold = 100;
			virtualDevice.bound = false;

			_virtualDevices.push_back(virtualDevice);
		}
	}
	if(i > 0) {
		bindVirtualDevice(0);
	}

	_player.setVolume(0);
	_player.stop();
	_player.start(_sampleRate);
	setSampleRate(DEFAULT_SAMPLE_RATE);

	_lowCornerFreq = _sampleRate/2;
	_highCornerFreq = 0;
//	devicesChanged.emit();
	Log::msg("Found %d recording devices.", _virtualDevices.size());
    loadFilterSettings();

    //AM detection and demodulation variables
    amDetectionNotchFilter.initWithSamplingRate(_sampleRate);
    amDetectionNotchFilter.setCenterFrequency(AM_CARRIER_FREQUENCY);
    amDetectionNotchFilter.setQ(1.0);

    for (int k = 0;k<6;k++)
    {
        amDemodulationLowPassFilter[k].initWithSamplingRate(_sampleRate);
        amDemodulationLowPassFilter[k].setCornerFrequency(AM_DEMODULATION_CUTOFF);
        amDemodulationLowPassFilter[k].setQ(1.0f);

    }

    rmsOfOriginalSignal = 0;
    rmsOfNotchedAMSignal = 0;
    weAreReceivingAMSignal = false;
    initDefaultJoystickKeys();

}




    void RecordingManager::constructMetadata(MetadataChunk *m) const {
        unsigned int nchan = 0;

        for(unsigned int i = 0; i < _virtualDevices.size(); i++)
            if(_virtualDevices[i].bound)
                nchan++;

        assert(m->channels.size() <= nchan); // don't want to delete stuff here
        m->channels.resize(nchan);

        int chani = 0;
        for(unsigned int i = 0; i < _virtualDevices.size(); i++) {
            if(_virtualDevices[i].bound) {
                m->channels[chani].threshold = _virtualDevices[i].threshold;
                m->channels[chani].name = _virtualDevices[i].name;

                chani++;
            }
        }

        m->markers = _markers;
    }



void RecordingManager::applyMetadata(const MetadataChunk &m) {
    //assert(_virtualDevices.size() == m.channels.size());
    for(unsigned int i = 0; i < m.channels.size(); i++) {
        _virtualDevices[i].threshold = m.channels[i].threshold;
        _virtualDevices[i].name = m.channels[i].name;
    }

    deviceTypeUsedDuringRecordingOfCurrentFile = m.deviceType;

    std::vector<int> neuronIds;
    _spikeTrains.clear();
    _markers.clear();
    for(std::list<std::pair<std::string, int64_t> >::const_iterator it = m.markers.begin(); it != m.markers.end(); it++) {
        const char *name = it->first.c_str();
        int chid = 0;
        if(strncmp(name, "_ch", 3) == 0) {
            char *endptr;
            chid = strtol(name+3, &endptr, 10);
        }
        if(strstr(name,"_neuron"))
        {
            name = strstr(name,"_neuron");
        }

        if(strncmp(name, "_neuron", 7) > -1) {
            char *endptr;
            int neuid = strtol(name+7, &endptr, 10);
            if(name == endptr)//if there was no number
                continue;

            int neuronidx = -1;
            for(unsigned int i = 0; i < neuronIds.size(); i++) {
                if(neuronIds[i] == neuid) {//if we already created spike train for this neuron find neuron index
                    neuronidx = i;
                    break;
                }
            }

            if(neuronidx == -1) {//add new neuron
                neuronIds.push_back(neuid);
                neuronidx = neuronIds.size()-1;
                _spikeTrains.push_back(SpikeTrain());
                int clr = neuid;
                if(clr < 0)
                    clr = 0;
                _spikeTrains.back().color = clr;
                _spikeTrains.back().channelIndex = chid;
            }
            if(strlen(endptr) > 9) {
                int num = atoi(endptr+9);
                if(strncmp(endptr,"threshhig",9) == 0) {
                    _spikeTrains[neuronidx].upperThresh = num;
                } else if(strncmp(endptr,"threshlow",9) == 0) {
                    _spikeTrains[neuronidx].lowerThresh = num;
                }
            } else {
                _spikeTrains[neuronidx].spikes.push_back(it->second);
            }
        } else {
            _markers.push_back(*it);
        }
    }
}


int RecordingManager::deviceUsedForRecordingFile()
{
    return deviceTypeUsedDuringRecordingOfCurrentFile;
}

void RecordingManager::clear() {


    for(int i = (int)_virtualDevices.size()-1; i >=0; i--) {
        unbindVirtualDevice(i);
    }

    for(int i = 0; i < (int)_devices.size(); i++)
        _devices[i].disable();

    _virtualDevices.clear();
    _devices.clear();

    _markers.clear();
    _triggers.clear();
    _lastThresholdedEvent = -1;
    _pos = 0;
    _selectedVDevice = 0;
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
        if(fileIsLoadedAndFirstActionDidNotYetOccurred)
        {
            setPos(0);

        }
		if(!pausing && _pos >= fileLength()-1) {
			_triggers.clear();
            _lastThresholdedEvent = -1;
			setPos(0);
		}
	} else {
		for(int i = 0; i < (int)_devices.size(); i++) {
			if(pausing) {
				if(!BASS_ChannelPause(_devices[i].handle))
					Log::error("Bass Error: pausing channel failed: %s", GetBassStrError());
			} else {
				if(!BASS_ChannelPlay(_devices[i].handle, FALSE))
					Log::error("Bass Error: resuming channel playback failed: %s", GetBassStrError());
			}
		}
	}
}

Player &RecordingManager::player() {
	return _player;
}


int RecordingManager::numberOfChannels()
{

    if(hidMode())
    {
        return _numOfHidChannels;
    }
    else if(serialMode())
    {
        return _arduinoSerial.numberOfChannels();
    }
    else if(fileMode())
    {
        return _devices[0].channels;
    }
    else
    {
        if(_devices.size()>0)
        {
            return _devices[0].channels;
        }
        else
        {
            return 0;
        }
    }

}


int RecordingManager::sampleRate() const {
	return _sampleRate;
}

void RecordingManager::setSampleRate(int sampleRate) {
	_player.setSampleRate(sampleRate);
	_sampleRate = sampleRate;
}

void RecordingManager::addTrigger(int64_t position)
{
    _triggers.push_front(position);
    if(_triggers.size() > (unsigned int)_threshAvgCount)
        _triggers.pop_back();
}

void RecordingManager::setThreshMode(bool threshMode) {
	_threshMode = threshMode;
    _triggers.clear();
    _lastThresholdedEvent = -1;
}

int RecordingManager::getThresholdSource()
{
    return _thresholdSource;
}

void RecordingManager::clearTriggers()
{
    _triggers.clear();
    _lastThresholdedEvent = -1;
}

void RecordingManager::setThresholdSource(int newThresholdSource)
{
    _thresholdSource = newThresholdSource;
    _lastThresholdedEvent = -1;

}

void RecordingManager::setThreshAvgCount(int threshAvgCount) {
	_threshAvgCount = std::max(0,threshAvgCount);
	_triggers.clear();
    _lastThresholdedEvent = -1;
}

void RecordingManager::setSelectedVDevice(int virtualDevice) {
	if(_selectedVDevice == virtualDevice)
		return;

	_selectedVDevice = virtualDevice;
	_triggers.clear();
    _lastThresholdedEvent = -1;
}

void RecordingManager::setVDeviceThreshold(int virtualDevice, int threshold) {
	_virtualDevices[virtualDevice].threshold = threshold;
	_triggers.clear();
    _lastThresholdedEvent = -1;
	thresholdChanged.emit();
}

int64_t RecordingManager::fileLength() {
	assert(_fileMode && !_devices.empty());

	//int64_t len = BASS_ChannelGetLength(_devices[0].handle, BASS_POS_BYTE)/_devices[0].bytespersample/_devices[0].channels;
    int64_t len =  anyFilesLength(_devices[0].handle, _devices[0].bytespersample, _devices[0].channels);
	assert(len != -1);

	return len;
}

void RecordingManager::addMarker(const std::string &id, int64_t offset) {
    std::cout<<"\n\n Add marker _pos: "<<_pos<<" offset: "<<offset<< "  ";
	_markers.push_back(std::make_pair(id, _pos + offset));
	char tempChar = id.at(0);
    int i_dec = tempChar -48;//std::stoi (id);

    if((getThresholdSource() == i_dec || getThresholdSource()==THRESHOLD_SOURCE_ALL_EVENTS) &&  threshMode())
    {
        if(getThresholdSource()==THRESHOLD_SOURCE_ALL_EVENTS)
        {
            _lastThresholdedEvent = i_dec;
        }
         addTrigger(_pos + offset);

    }
}

const char *RecordingManager::fileMetadataString() {
	assert(_fileMode);
    return readEventsAndSpikesForAnyFile(_devices[0].handle);//
	//return BASS_ChannelGetTags(_devices[0].handle, BASS_TAG_RIFF_INFO);
}

void RecordingManager::getData(int virtualDevice, int64_t offset, int64_t len, int16_t *dst) {
	sampleBuffer(virtualDevice)->getData(dst, offset, len);
}

void RecordingManager::getTriggerData(int virtualDevice, int64_t len, int16_t *dst) {
	int16_t *buf = new int16_t[len];
	memset(dst,0,sizeof(int16_t)*len);

	for(std::list<int64_t>::iterator it = _triggers.begin(); it != _triggers.end(); it++) {
		const int64_t pos2 = *it+len/2;
		const int64_t pos1 = *it-len/2;

		sampleBuffer(virtualDevice)->getData(buf, pos1, pos2-pos1);

		for(unsigned int i = 0; i < len; i++) {
			dst[i] += buf[i];
		}
	}


	if(!_triggers.empty()) {
		for(unsigned int i = 0; i < len; i++) {
			dst[i] /= (int)_triggers.size();
		}
	}

	delete[] buf;
}

//
// Parameters:
//    virtualDeviceIndex - device from which we want to read
//    offset - offset in samples from begining of the time
//    len - number of samples to get
//    sampleSkip - ??used when we cant show "len" samples on the screen (not enough pixels)
//                  (for example when we want to display 20 sec of data @ 44kHz on 860px screen)
//                  get every "sampleSkip" sample (skip "sampleSkip"-1 sample after each sample)
//
//     returns roughly (roughly because we use snapTo) len/sampleSkip data samples
//
std::vector< std::pair<int16_t, int16_t> > RecordingManager::getSamplesEnvelope(int virtualDeviceIndex, int64_t offset, int64_t len, int sampleSkip) {

	const int64_t pos2 = snapTo(offset+len, sampleSkip);//end of data
	const int64_t pos1 = snapTo(offset, sampleSkip);//begining of the data

	int64_t newLengthOfData = pos2 - pos1;

	std::vector< std::pair<int16_t, int16_t> > result;

	if (_devices[_virtualDevices[virtualDeviceIndex].device].enabled) {
		result = sampleBuffer(virtualDeviceIndex)->getDataEnvelope(pos1, newLengthOfData, sampleSkip);
	} else {
		result.resize(newLengthOfData/sampleSkip);
	}

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


#pragma mark - Advance one step

void RecordingManager::advanceFileMode(uint32_t samples) {
	if(!_paused && _pos >= fileLength()-1) {
		pauseChanged.emit();
		setPaused(true);
		return;
	}

	int numOfBytesInAudioBuffer = _player.stateOfBuffer();
	//if we don't have enough data preloaded
	//add some more (we have to have enough data for audio until next
    //frame/call of this function)
    if(numOfBytesInAudioBuffer< _sampleRate/15)
    {
        samples +=samples;//double the sample count
    }


    const unsigned int bufsize = SEGMENT_SIZE;//1*sampleRate();
	for(int idx = 0; idx < (int)_devices.size(); idx++) {

        const int channum = _devices[idx].channels;
        //if we are at the end of the file
        if(_devices[idx].sampleBuffers[0].pos() >= fileLength()-1)
        {
            setPos(_pos, true);
            continue;
        }

        int beginingSegmentOfCurrentContinualDataInBuffer = _devices[idx].sampleBuffers[0].head()/SEGMENT_SIZE;
        if(_devices[idx].sampleBuffers[0].segmentsState[beginingSegmentOfCurrentContinualDataInBuffer%NUMBER_OF_SEGMENTS] == 0)
        {
            for(int chan = 0; chan < channum; chan++)
            {
                _devices[idx].sampleBuffers[0].segmentsState[beginingSegmentOfCurrentContinualDataInBuffer] = 1;
            }
        }
        else
        {
            continue;
        }

		const int bytespersample = _devices[idx].bytespersample;
		std::vector<std::vector<int16_t> > channels;


        //-------- Read data ------------
		//bool rc = ReadWAVFile(channels, channum*bufsize*bytespersample, _devices[idx].handle, channum, bytespersample);
        bool rc = readAnyFile(channels, channum*bufsize*bytespersample, _devices[idx].handle, channum, bytespersample);
        

		if(!rc)
			continue;


		// TODO make this more sane
		/*for(int chan = 0; chan < channum; chan++) {
			if(_devices[idx].dcBiasNum < _sampleRate*10) {
				for(unsigned int i = 0; i < channels[chan].size(); i++) {
					_devices[idx].dcBiasSum[chan] += channels[chan][i];
					if(chan == 0)
						_devices[idx].dcBiasNum++;
				}
			}
			int dcBias = _devices[idx].dcBiasSum[chan]/_devices[idx].dcBiasNum;

			for(unsigned int i = 0; i < channels[chan].size(); i++) {
				//channels[chan][i] -= dcBias;
			}
		}*/

		for(int chan = 0; chan < channum; chan++) {
			_devices[idx].sampleBuffers[chan].addData(channels[chan].data(), channels[chan].size());
		}

	}

	if(!_paused) {
		if(_threshMode) {

            if(_thresholdSource==0)//we are triggering on signal
            {
                bool triggerd = false;
                SampleBuffer &s = *sampleBuffer(_selectedVDevice);

                if(_thresholdSource == 0)//if we trigger on signal
                {
                    for(int64_t i = _pos; i < _pos+samples; i++) {
                        const int thresh = _virtualDevices[_selectedVDevice].threshold;

                        if(_triggers.empty() || i - _triggers.front() > _sampleRate/10) {

                            if((thresh > 0 && s.at(i) >= thresh && lastSampleForThreshold<thresh) || (thresh <= 0 && s.at(i) < thresh && lastSampleForThreshold> thresh)) {
                                _triggers.push_front(i);
                                triggerd = true;
                                if(_triggers.size() > (unsigned int)_threshAvgCount)
                                    _triggers.pop_back();
                            }
                        }
                        lastSampleForThreshold = s.at(i);//keep last sample
                    }
                }
                if(triggerd)
                    triggered.emit();
            }
            else
            {//if we are triggering on events

                    for(std::list<std::pair<std::string, int64_t> >::const_iterator it = markers().begin(); it != markers().end(); it++) {
                        try
                        {
                            if(it->second >_pos && (it->second<(_pos+samples)))
                            {

                                //int i_dec = std::stoi (it->first);
                                char tempChar = it->first.at(0);
                                int i_dec = tempChar -48;
                                if(getThresholdSource() == i_dec || getThresholdSource()==THRESHOLD_SOURCE_ALL_EVENTS)
                                {
                                    if(getThresholdSource()==THRESHOLD_SOURCE_ALL_EVENTS)
                                    {
                                        _lastThresholdedEvent = i_dec;
                                    }
                                    addTrigger(it->second);
                                }
                            }

                        }
                        catch (std::invalid_argument&)
                        {
                            continue;
                        }
                        catch (std::out_of_range&)
                        {
                            continue;
                        }
                    }
            }
		}

		SampleBuffer &s = *sampleBuffer(_selectedVDevice);
		const uint32_t bsamples = samples;

		if(_player.volume() > 0) {
			int16_t *buf = new int16_t[bsamples];
			s.getData(buf, _pos, bsamples);
			_player.push(buf, bsamples*sizeof(int16_t));
			delete[] buf;
		}

		setPos(_pos + samples, false);
	}
}



void RecordingManager::advanceSerialMode(uint32_t samples)
{

    uint32_t len = SIZE_OF_INPUT_HARDWARE_CIRC_BUFFER;
    //len = std::min(samples, len);
   // std::cout<<len<<"\n";
	const int channum = _arduinoSerial.numberOfChannels();
	std::vector<int16_t> *channels = new std::vector<int16_t>[channum];//non-interleaved
	int16_t *buffer = new int16_t[channum*len];


	//get interleaved data for all channels

    int samplerateDiv10 = _sampleRate/10;
    //std::cout<<"start"<<"\n";
	int samplesRead = _arduinoSerial.getNewSamples(buffer);
    //	uint32_t numTicksAfter = SDL_GetTicks();
//	std::cout<<"Read: "<<samplesRead<<"\n";
//	Log::msg("Read %d\n", samplesRead);
	//numTicksBefore = SDL_GetTicks();

    if(samplesRead == -1)
    {

        //check if port is still active if not disconnect
        std::list<ArduinoSerial::SerialPort> sps =  serailPorts();
        std::list<ArduinoSerial::SerialPort>::iterator it;
        std::size_t found;
        bool foundPort = false;
        for(it = sps.begin();it!=sps.end();it++)
        {
            found  = getCurrentPort().portName.find(it->portName);
            if (found!=std::string::npos)
            {
                foundPort = true;
            }
        }

        if(!foundPort)
        {
            setSerialNumberOfChannels(1);
            disconnectFromSerial();
            return;
        }
    }

    if(_paused)
    {
        delete[] channels;
	    delete[] buffer;
        return;
    }
	if(samplesRead != -1) {

	    //make separate buffer for every channel
	    for(int chan = 0; chan < channum; chan++)
	        channels[chan].resize(len);

	    // de-interleave the channels
	    for (int i = 0; i < samplesRead; i++) {
	        for(int chan = 0; chan < channum; chan++) {
	            channels[chan][i] = buffer[i*channum + chan];//sort data to channels
	        }
	    }

	    //filter data
        if(fiftyHzFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_50HzNotchFilters[chan].filterIntData(channels[chan].data(), samplesRead);
	        }
        }
        else if(sixtyHzFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_60HzNotchFilters[chan].filterIntData(channels[chan].data(), samplesRead);
	        }
        }

        bool flushData = false;
        /*if(_devices.begin()->dcBiasNum < _sampleRate*HOW_LONG_FLUSH_IS_ACTIVE) {
            flushData = true;
        }*/

         if(lowPassFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_lowPassFilters[chan].filterIntData(channels[chan].data(), samplesRead, flushData);
	        }
        }

         if(highPassFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_highPassFilters[chan].filterIntData(channels[chan].data(), samplesRead, flushData);
	        }
        }

        if(_devices.begin()->dcBiasNum < _sampleRate*10) {
                //DC offset elimination
                 for (int i = 0; i < samplesRead; i++) {
                    for(int chan = 0; chan < channum; chan++) {
                        //if we are in first 10 seconds interval
                        //add current sample to summ used to remove DC component

                            _devices.begin()->dcBiasSum[chan] += channels[chan][i];
                            if(chan == 0)
                            {
                                _devices.begin()->dcBiasNum++;
                            }

                    }
                }
        }



	    bool triggerd = false;
	    for(int chan = 0; chan < channum; chan++) {
	        //calculate DC offset in fist 10 sec for channel
	        int dcBias = _devices.begin()->dcBiasSum[chan]/_devices.begin()->dcBiasNum;
            if(_thresholdSource == 0)//if we trigger on signal
            {
                //Log::msg("HSR: %d", samplesRead);
                for(DWORD i = 0; i < (unsigned int)samplesRead; i++) {

                    channels[chan][i] -= dcBias;//substract DC offset from channels data
                    const int thresh = _virtualDevices[_selectedVDevice].threshold;
                    const int64_t ntrigger = _pos + i;
                    //--------------- joystick related code --------------------------

                    const int currentthresh = _virtualDevices[chan].threshold;
                    if(_timersForKeyRelease[chan]>0)
                    {
                        _timersForKeyRelease[chan] --;
                        if(_timersForKeyRelease[chan]==0)
                        {
                            Log::msg("Release %d", _keyIndexSetForJoystick[chan]-1);
                            _arduinoSerial.releaseKey( _keyIndexSetForJoystick[chan]-1);
                        }
                    }
                    if(((ntrigger - _timeOfLastTriggerJoystick[chan])> samplerateDiv10) && (currentAddOnBoard() == BOARD_WITH_JOYSTICK))
                    {
                        if((currentthresh > 0 && channels[chan][i] > currentthresh && _lastValueOfSignalJoystick[chan] < currentthresh) || (currentthresh <= 0 && channels[chan][i] < currentthresh && _lastValueOfSignalJoystick[chan]>currentthresh))
                        {
                            //we thresholded on signal for one channel
                            _timeOfLastTriggerJoystick[chan] = ntrigger;
                            if(_keyIndexSetForJoystick[chan]>0)//zero is "none of the keys selected"
                            {
                                if(_timersForKeyRelease[chan]==0)
                                {
                                    Log::msg("Press %d",_keyIndexSetForJoystick[chan]-1);
                                    _arduinoSerial.pressKey(_keyIndexSetForJoystick[chan]-1);
                                }
                                _timersForKeyRelease[chan] = MAX_TIMER_FOR_KEY_RELEASE;
                                //keyReleaseList.push_back(_keyIndexSetForJoystick[chan]-1);

                            }
                        }
                    }
                    _lastValueOfSignalJoystick[chan] = channels[chan][i];
                    //-------------------- end of joystick related code --------------

                    //add position of data samples that are greater than threshold to FIFO list _triggers
                    if(_threshMode && _devices.begin()->index*channum+chan == _selectedVDevice) {



                        if(_triggers.empty() || ntrigger - _triggers.front() >samplerateDiv10) {
                            if((thresh > 0 && channels[chan][i] > thresh && lastSampleForThreshold < thresh) || (thresh <= 0 && channels[chan][i] < thresh && lastSampleForThreshold>thresh)) {
                                _triggers.push_front(_pos + i);
                                triggerd = true;
                                if(_triggers.size() > (unsigned int)_threshAvgCount)//_threshAvgCount == 1
                                    _triggers.pop_back();
                            }
                        }
                         lastSampleForThreshold = channels[chan][i];//keep last sample
                    }
                }
            }



	        if(_devices.begin()->sampleBuffers[0].empty()) {
                   // std::cout<<"set pos??"<<"\n";
	            _devices.begin()->sampleBuffers[chan].setPos(_pos);
	        }
	        //Here we add data to Sample buffer !!!!!
	        //copy data from temporary de-inrleaved data buffer to permanent buffer
	        _devices.begin()->sampleBuffers[chan].addData(channels[chan].data(), samplesRead);
	    }
        if(triggerd)
            triggered.emit();

	    delete[] channels;
	    delete[] buffer;




        //Push data to speaker

        //const int nchan = _devices.begin()->channels;
        int bytesPerSample = _devices.begin()->bytespersample;


        if(alphaFeedbackActive)
        {
            int16_t *buf = new int16_t[samples];

            //648044
            //200

            double progress = 0.1+0.4*alphaWavePower/50000.0;
            for( int index= 0;index<samples;index++)
            {
                _alphaAudioTime+=progress;
                buf[index]=2500*sin(_alphaAudioTime);
            }
            _player.push(buf, samples*sizeof(int16_t));
            delete[] buf;
        }
        else
        {
            int16_t *buf = new int16_t[samplesRead];
            if(_player.volume() > 0) {


                SampleBuffer *s = sampleBuffer(_selectedVDevice);
                if(s != NULL) {
                    s->getData(buf, _pos, samplesRead);
                } else {
                    memset(buf, 0, samplesRead*sizeof(int16_t));
                }

                _player.push(buf, samplesRead*sizeof(int16_t));


            } else {
                _player.setPos(_pos, bytesPerSample, 1);
                //std::cout<<"\nSet position: "<<_pos;
            }
             delete[] buf;
        }


        //std::cout<<" Advance _pos: "<<_pos<<"\n";
         _pos+=samplesRead;
        //std::cout<<" After Advance _pos: "<<_pos<<"\n";
         //std::cout<<"End"<<"\n";
	}
	else
	{
	    //No new samples
	    delete[] channels;
	    delete[] buffer;
	}
}


void RecordingManager::advanceHidMode(uint32_t samples)
{

    if(!_hidUsbManager.deviceOpened())
    {
        disconnectFromHID();
        scanForHIDDevices();
    }

    uint32_t len = 30000;
    //len = std::min(samples, len);
   // std::cout<<len<<"\n";
    const int channum = _numOfHidChannels;
    std::vector<int16_t> *channels = new std::vector<int16_t>[channum];//non-interleaved
    int32_t *buffer = new int32_t[channum*len];




    //get interleaved data for all channels
    int samplesRead = _hidUsbManager.readDevice(buffer);

//    printf("Read: %d, Needed: %d\n", samplesRead, samples);
    if(_paused || samplesRead==0)
    {
        delete[] channels;
        delete[] buffer;
        return;
    }
    if(samplesRead != -1) {

        //make separate buffer for every channel
        for(int chan = 0; chan < channum; chan++)
            channels[chan].resize(len);

         // de-interleave the channels
	    for (int i = 0; i < samplesRead; i++) {
	        for(int chan = 0; chan < channum; chan++) {
	            channels[chan][i] = buffer[i*channum + chan];//sort data to channels

	        }
	    }



        int maxChannelToFilter = channum;
        if(maxChannelToFilter>2)
        {
            maxChannelToFilter = 2;
        }

        if(highPassFilterEnabled())
        {
            for(int chan = 0; chan < maxChannelToFilter; chan++) {
                _devices.begin()->_highPassFilters[chan].filterIntData(channels[chan].data(), samplesRead);
            }
        }


        //filter data
        if(fiftyHzFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
                _devices.begin()->_50HzNotchFilters[chan].filterIntData(channels[chan].data(), samplesRead);
            }
        }
        else if(sixtyHzFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
                _devices.begin()->_60HzNotchFilters[chan].filterIntData(channels[chan].data(), samplesRead);
            }
        }

        if(lowPassFilterEnabled())
        {
            for(int chan = 0; chan < maxChannelToFilter; chan++) {
	            _devices.begin()->_lowPassFilters[chan].filterIntData(channels[chan].data(), samplesRead);
	        }
        }



	    //DC offset elimination
	     for (int i = 0; i < samplesRead; i++) {
	        for(int chan = 0; chan < channum; chan++) {
	            //if we are in first 10 seconds interval
	            //add current sample to summ used to remove DC component
	            if(_devices.begin()->dcBiasNum < _sampleRate*10) {
	                _devices.begin()->dcBiasSum[chan] += 0;///// We are not doing that right now on HID
	                if(chan == 0)
	                {
	                    _devices.begin()->dcBiasNum++;
	                }
	            }
	        }
	    }

     
        bool triggerd = false;
        int samplerateDiv10 = _sampleRate/10;
        for(int chan = 0; chan < channum; chan++) {
            //calculate DC offset in fist 10 sec for channel
            int dcBias = _devices.begin()->dcBiasSum[chan]/_devices.begin()->dcBiasNum;

            if(_thresholdSource == 0)//if we trigger on signal
            {
                //Log::msg("HSR: %d", samplesRead);
                for(DWORD i = 0; i < (unsigned int)samplesRead; i++) {

                    channels[chan][i] -= dcBias;//substract DC offset from channels data
                    const int thresh = _virtualDevices[_selectedVDevice].threshold;
                    const int64_t ntrigger = _pos + i;
                    //--------------- joystick related code --------------------------

                    const int currentthresh = _virtualDevices[chan].threshold;
                    if(_timersForKeyRelease[chan]>0)
                    {
                        _timersForKeyRelease[chan] --;
                        if(_timersForKeyRelease[chan]==0)
                        {
                            Log::msg("Release %d", _keyIndexSetForJoystick[chan]-1);
                            _hidUsbManager.releaseKey( _keyIndexSetForJoystick[chan]-1);
                        }
                    }
                    if(((ntrigger - _timeOfLastTriggerJoystick[chan])> samplerateDiv10) && (currentAddOnBoard() == BOARD_WITH_JOYSTICK))
                    {
                        if((currentthresh > 0 && channels[chan][i] > currentthresh && _lastValueOfSignalJoystick[chan] < currentthresh) || (currentthresh <= 0 && channels[chan][i] < currentthresh && _lastValueOfSignalJoystick[chan]>currentthresh))
                        {
                            //we thresholded on signal for one channel
                            _timeOfLastTriggerJoystick[chan] = ntrigger;
                            if(_keyIndexSetForJoystick[chan]>0)//zero is "none of the keys selected"
                            {
                                if(_timersForKeyRelease[chan]==0)
                                {
                                    Log::msg("Press %d",_keyIndexSetForJoystick[chan]-1);
                                    _hidUsbManager.pressKey(_keyIndexSetForJoystick[chan]-1);
                                }
                                _timersForKeyRelease[chan] = MAX_TIMER_FOR_KEY_RELEASE;
                                //keyReleaseList.push_back(_keyIndexSetForJoystick[chan]-1);

                            }
                        }
                    }
                    _lastValueOfSignalJoystick[chan] = channels[chan][i];
                    //-------------------- end of joystick related code --------------

                    //add position of data samples that are greater than threshold to FIFO list _triggers
                    if(_threshMode && _devices.begin()->index*channum+chan == _selectedVDevice) {



                        if(_triggers.empty() || ntrigger - _triggers.front() >samplerateDiv10) {
                            if((thresh > 0 && channels[chan][i] > thresh && lastSampleForThreshold < thresh) || (thresh <= 0 && channels[chan][i] < thresh && lastSampleForThreshold>thresh)) {
                                _triggers.push_front(_pos + i);
                                triggerd = true;
                                if(_triggers.size() > (unsigned int)_threshAvgCount)//_threshAvgCount == 1
                                    _triggers.pop_back();
                            }
                        }
                         lastSampleForThreshold = channels[chan][i];//keep last sample
                    }
                }
            }

            if(_devices.begin()->sampleBuffers[0].empty()) {
                _devices.begin()->sampleBuffers[chan].setPos(_pos);
            }
            //Here we add data to Sample buffer !!!!!
            //copy data from temporary de-inrleaved data buffer to permanent buffer
            _devices.begin()->sampleBuffers[chan].addData(channels[chan].data(), samplesRead);
        }
	if(triggerd)
		triggered.emit();

        delete[] channels;
        delete[] buffer;





        //const int nchan = _devices.begin()->channels;
        int bytesPerSample = _devices.begin()->bytespersample;

       // int16_t *buf = new int16_t[samplesRead];

        // this biggerBufferSize is patch for case when HID device
        //does not send enough samples per second. FOr example when it
        //sends 9999 instead of 10000. Than we can hear clicking sound
        //since audio will need all 10000 samples and we will provide less
        int biggerBufferSize = samples;

        debugNumberOfSamplesThatWeNeed += samples;
        debugNumberOfSamplesThatWeGet += samplesRead;
        if(debugNumberOfSamplesThatWeNeed>100000)
        {
            Log::msg("\n\nNeed: %d, Read:%d -----------------------\n\n",debugNumberOfSamplesThatWeNeed, debugNumberOfSamplesThatWeGet);
            debugNumberOfSamplesThatWeNeed = 0;
            debugNumberOfSamplesThatWeGet = 0;
        }
       /* if(samplesRead>biggerBufferSize)
        {
            biggerBufferSize = samplesRead;
        }*/
        int16_t *buf = new int16_t[biggerBufferSize];


        if(_player.volume() > 0) {


            SampleBuffer *s = sampleBuffer(_selectedVDevice);
            if(s != NULL) {
                s->getData(buf, _pos, biggerBufferSize);
            } else {
                memset(buf, 0, biggerBufferSize*sizeof(int16_t));
            }

            filterSoundForPlayer.filterIntData(buf, biggerBufferSize);
            _player.push(buf, biggerBufferSize*sizeof(int16_t));

        } else {
            _player.setPos(_pos, bytesPerSample, 1);
            //std::cout<<"\nSet position: "<<_pos;
        }
        delete[] buf;




        _pos+=samplesRead;



    }
    else
    {
        //No new samples
        delete[] channels;
        delete[] buffer;
    }
}






void RecordingManager::advance(uint32_t samples) {
	// std::cout<<"Advance ==========================\n";
    try{

		scanUSBDevices();
	}
	catch (int e)
	{
		std::cout<<"Error HID scan\n";
	}
    if(_arduinoSerial.waitingForRestart())
    {
        resetCurrentSerial();
        _arduinoSerial.deviceRestarted();
    }
    if(waitToDisconnectFromSerial)
    {
        sincDisconnectFromSerial();
    }
	if(_serialMode)
	{
		advanceSerialMode(samples);
		return;
	}

	if(_fileMode) {
		advanceFileMode(samples);
		return;
	}

	if(_hidMode)
	{
		if(_HIDShouldBeReloaded)
		{
		    initDefaultJoystickKeys();
			_HIDShouldBeReloaded = false;
			initHIDUSB((HIDBoardType)_hidUsbManager.currentlyConnectedHIDBoardType());
		}
		else
		{

			advanceHidMode(samples);

		}
		return;
	}

	if(_devices.empty() || _paused)
		return;

	const int64_t oldPos = _pos;
	int64_t newPos = _pos;
	bool firstTime = true;

	uint32_t len = 5*sampleRate();
	len = std::min(samples, len);

	for (int idx = 0; idx < (int)_devices.size(); idx++) {

		if(!_devices[idx].enabled)
			continue;
		const int channum = _devices[idx].channels;
		std::vector<int16_t> *channels = new std::vector<int16_t>[channum];
		int16_t *buffer = new int16_t[channum*len];

		DWORD samplesRead = BASS_ChannelGetData(_devices[idx].handle, buffer, channum*len*sizeof(int16_t));
		if(samplesRead == (DWORD)-1) {
			Log::error("Bass Error: getting channel data failed: %s", GetBassStrError());
			delete[] channels;
			delete[] buffer;
			continue;
		}
		samplesRead /= sizeof(int16_t);

		for(int chan = 0; chan < channum; chan++)
			channels[chan].resize(len);

    // de-interleave the channels
	    for (int i = 0; i < samplesRead/channum; i++) {
	        for(int chan = 0; chan < channum; chan++) {
	            channels[chan][i] = buffer[i*channum + chan];//sort data to channels

               if(chan ==0)
               {
                   rmsOfOriginalSignal = 0.0001*((float)(channels[chan][i]*channels[chan][i]))+0.9999*rmsOfOriginalSignal;

               }
	        }
	    }

        int32_t numberOfFramesReceived = samplesRead/channum;
        int16_t * receivedData = channels[0].data();
        amBuffer = (int16_t*) std::malloc( _sampleRate * sizeof(int16_t));
        if(numberOfFramesReceived>_sampleRate)
        {
            numberOfFramesReceived = _sampleRate;
        }
        memcpy(amBuffer, receivedData, numberOfFramesReceived * sizeof(int16_t));

        amDetectionNotchFilter.filterIntData(amBuffer, numberOfFramesReceived);
        for(int32_t i=0;i<numberOfFramesReceived;i++)
        {
            rmsOfNotchedAMSignal = 0.0001*((float)(amBuffer[i]*amBuffer[i]))+0.9999*rmsOfNotchedAMSignal;
        }

        //std::cout<<"Notch: "<<sqrtf(rmsOfNotchedAMSignal)<<" - Normal: "<<sqrtf(rmsOfOriginalSignal)<<" - a/b: "<<sqrtf(rmsOfOriginalSignal)/sqrtf(rmsOfNotchedAMSignal)<<"\n";
        if(sqrtf(rmsOfOriginalSignal)/sqrtf(rmsOfNotchedAMSignal)>5)
        {
            weAreReceivingAMSignal = true;
        }
        else
        {
            weAreReceivingAMSignal = false;
        }

        if(weAreReceivingAMSignal)
        {

            for (int i = 0; i < samplesRead/channum; i++) {
                for(int chan = 0; chan < 1; chan++) {
                    channels[chan][i] = -1*abs(channels[chan][i]);
                }
            }

            int filterIndex = 0;
            for(int i=0;i<2;i++)
            {
                for (int k = 0;k<3;k++)
                {
                    amDemodulationLowPassFilter[filterIndex].filterIntData(channels[i].data(), samplesRead/channum);
                    filterIndex++;
                }
            }
        }
        free(amBuffer);


	    //filter data
        if(fiftyHzFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_50HzNotchFilters[chan].filterIntData(channels[chan].data(), samplesRead/channum);
	        }
        }
        else if(sixtyHzFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_60HzNotchFilters[chan].filterIntData(channels[chan].data(), samplesRead/channum);
	        }
        }

        if(lowPassFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_lowPassFilters[chan].filterIntData(channels[chan].data(), samplesRead/channum);
	        }
        }

         if(highPassFilterEnabled())
        {
            for(int chan = 0; chan < channum; chan++) {
	            _devices.begin()->_highPassFilters[chan].filterIntData(channels[chan].data(), samplesRead/channum);
	        }
        }

	    //DC offset elimination
	     for (int i = 0; i < samplesRead/channum; i++) {
	        for(int chan = 0; chan < channum; chan++) {
	            //if we are in first 10 seconds interval
	            //add current sample to summ used to remove DC component
	            if(_devices.begin()->dcBiasNum < _sampleRate*10) {
	                _devices.begin()->dcBiasSum[chan] += 0;
	                if(chan == 0)
	                {
	                    _devices.begin()->dcBiasNum++;
	                }
	            }
	        }
	    }


		bool triggerd = false;
      //  std::cout<<"Start of add data --------------------------\n";
       // std::cout<<"Device "<<idx<<"\n";
		for(int chan = 0; chan < channum; chan++) {
			int dcBias = _devices[idx].dcBiasSum[chan]/_devices[idx].dcBiasNum;

			if(_thresholdSource == 0)//if we trigger on signal
			{
				for(DWORD i = 0; i < samplesRead/channum; i++) {
					channels[chan][i] -= dcBias;
					if(_threshMode && idx*channum+chan == _selectedVDevice) {
						const int64_t ntrigger = oldPos + i;
						const int thresh = _virtualDevices[_selectedVDevice].threshold;


						if(_triggers.empty() || ntrigger - _triggers.front() > _sampleRate/10) {
							if((thresh > 0 && channels[chan][i] > thresh && lastSampleForThreshold<thresh) || (thresh <= 0 && channels[chan][i] < thresh && lastSampleForThreshold>thresh)) {

								_triggers.push_front(oldPos + i);
								triggerd = true;
								if(_triggers.size() > (unsigned int)_threshAvgCount)
									_triggers.pop_back();
							}
						}
                        lastSampleForThreshold = channels[chan][i];
					}
				}
			}
			if(_devices[idx].sampleBuffers[chan].empty()) {
				_devices[idx].sampleBuffers[chan].setPos(oldPos);
                //std::cout<<"Set old position "<<oldPos<<"\n";
			}
           // std::cout<<"Channel "<<chan<<"\n";
			_devices[idx].sampleBuffers[chan].addData(channels[chan].data(), samplesRead/channum);
		}

		if(triggerd)
			triggered.emit();

		const int64_t posA = _devices[idx].sampleBuffers[0].pos();
		if(!_devices[idx].sampleBuffers[0].empty() && (firstTime || posA < newPos)) {
			newPos = posA;
			firstTime = false;
		}

		delete[] channels;
		delete[] buffer;
	}

	const uint32_t bsamples = newPos-_pos;

	if(_player.volume() > 0) {
		int16_t *buf = new int16_t[bsamples];

		SampleBuffer *s = sampleBuffer(_selectedVDevice);
		if(s != NULL) {
			s->getData(buf, _pos, bsamples);
		} else {
			memset(buf, 0, bsamples*sizeof(int16_t));
		}

		_player.push(buf, bsamples*sizeof(int16_t));

		delete[] buf;
	}

	if(newPos > oldPos)
    {
		_pos = newPos;
    }

}

#pragma mark - Filters


//
// Trigger calculation of mean value to substract from signal
// Trigger elimination of offset
//
void RecordingManager::startRemovingMeanValue()
{
        //Reset mean sum counter
        if(_devices.size()>0)
        {
            _devices.begin()->dcBiasNum = 1;
            for(int chan = 0; chan < numberOfChannels(); chan++) {
                //reset sum for all channels
                _devices.begin()->dcBiasSum[chan]  = 0;
            }
        }
}

int RecordingManager::highCornerFrequency()
{
    int cornerFrequency = (int)_highCornerFreq;
    if(cornerFrequency>_sampleRate/2.0)
    {
        cornerFrequency = _sampleRate/2.0;
    }
    return cornerFrequency;

}
int RecordingManager::lowCornerFrequency()
{
    int cornerFrequency = (int)_lowCornerFreq;
    if(cornerFrequency>_sampleRate/2.0)
    {
        cornerFrequency = _sampleRate/2.0;
    }
    return cornerFrequency;
}

void RecordingManager::enableLowPassFilterWithCornerFreq(float cornerFreq)
{

    float oldLPFCornerFreq =_lowCornerFreq;
    
    if(cornerFreq<0)
    {
        cornerFreq = 0.0f;
    }

    _lowCornerFreq = cornerFreq;

    for(int chan = 0; chan < numberOfChannels(); chan++)
    {
        //we don't limit _lowCornerFreq internaly because we don't want situation
        //when user change from 1 ch to 6ch and return back to 1ch to get samplerate/6
        //max frequency
        if(cornerFreq>_sampleRate/2.0)
        {
            cornerFreq = _sampleRate/2.0;
        }
        _devices.begin()->_lowPassFilters[chan].setCornerFrequency(cornerFreq);
    }

    if(cornerFreq>((_sampleRate/2)-2))
    {
        _lowPassFilterEnabled = false;
    }
    else
    {
        _lowPassFilterEnabled = true;
    }
    

    if(oldLPFCornerFreq>=LPF_HUMAN_SP_THRESHOLD)
    {
        if(_lowCornerFreq<LPF_HUMAN_SP_THRESHOLD)
        {
            //turn ON gain
            setSerialHardwareGain(true);
        }
        
    }
    else
    {
        if(_lowCornerFreq>=LPF_HUMAN_SP_THRESHOLD)
        {
            //turn OFF gain
            setSerialHardwareGain(false);
            
        }
    }
    
}

void RecordingManager::enableHighPassFilterWithCornerFreq(float cornerFreq)
{

    float oldHPFCornerFreq =_highCornerFreq;
    _highCornerFreq = cornerFreq;
    if(cornerFreq<0)
    {
        cornerFreq = 0.0f;
    }

    _highCornerFreq = cornerFreq;

     for(int chan = 0; chan < numberOfChannels(); chan++)
    {
        //we don't limit _lowCornerFreq internaly because we don't want situation
        //when user change from 1 ch to 6ch and return back to 1ch to get samplerate/6
        //max frequency
        if(cornerFreq>_sampleRate/2.0)
        {
            cornerFreq = _sampleRate/2.0;
        }
        _devices.begin()->_highPassFilters[chan].setCornerFrequency(cornerFreq);
    }
    if(cornerFreq<1.0f)
    {
        _highPassFilterEnabled = false;
    }
    else
    {
        _highPassFilterEnabled = true;
       // startRemovingMeanValue();
    }
    

    if(oldHPFCornerFreq>=HPF_HUMAN_SP_THRESHOLD)
    {
        if(_highCornerFreq<HPF_HUMAN_SP_THRESHOLD)
        {
            //turn OFF filter
            setSerialHardwareHPF(false);
        }
        
    }
    else
    {
        if(_highCornerFreq>=20)
        {
            //turn ON filter
             setSerialHardwareHPF(true);
        }
    }
}

void RecordingManager::turnAlphaFeedbackON()
{
    alphaFeedbackActive = true;
    _player.setVolume(100);
}

void RecordingManager::turnAlphaFeedbackOFF()
{
    alphaFeedbackActive = false;
  //  _player.setVolume(0);
}

void RecordingManager::sendAlphaWavePowerToSerial()
{
        std::cout<<(int)(uint8_t)((alphaWavePower/1000000)*255)<<"\n";
        _arduinoSerial.sendPotentiometerMessage((uint8_t)((alphaWavePower/1000000)*255));
}

#pragma mark - Bind/Unbind device

bool RecordingManager::bindVirtualDevice(int vdevice) {
	assert(vdevice >= 0 && vdevice < (int)_virtualDevices.size());
	_virtualDevices[vdevice].bound = true;

	int device = _virtualDevices[vdevice].device;
	bool success = _devices[device].enable(_pos);

	if(success && _selectedVDevice == INVALID_VIRTUAL_DEVICE_INDEX)
		_selectedVDevice = vdevice;

	devicesChanged.emit();
	return success;
}

bool RecordingManager::unbindVirtualDevice(int vdevice) {
	assert(vdevice >= 0 && vdevice < (int)_virtualDevices.size());
	_virtualDevices[vdevice].bound = false;

	bool success = true;

	// if there is no other vdevice bound needing my device, disable it
	bool otherUser = false;
	for(int i = 0; i < (int)_virtualDevices.size(); i++) {
		if(i != vdevice && _virtualDevices[i].bound && _virtualDevices[i].device == _virtualDevices[vdevice].device) {
			otherUser = true;
			break;
		}
	}
	if(!otherUser) {
		success = _devices[_virtualDevices[vdevice].device].disable();
	}

	// if this device is selected, select the first bound vdevice instead or make selection invalid
	if(vdevice == _selectedVDevice) {
		for(int i = 0; i < (int)_virtualDevices.size(); i++) {
			if(i != vdevice && _virtualDevices[i].bound) {
				_selectedVDevice = i;
				break;
			}
		}
		if(vdevice == _selectedVDevice)
			_selectedVDevice = INVALID_VIRTUAL_DEVICE_INDEX;
	}

	devicesChanged.emit();
	return success;
}

RecordingManager::Device::Device(int index, int nchan, int &sampleRate)
	: type(Device::Audio),index(index), handle(0), enabled(false), dcBiasNum(1), channels(nchan), samplerate(sampleRate), bytespersample(2) {




	sampleBuffers.resize(nchan);
	_50HzNotchFilters.resize(nchan);
	for(int i=0;i<nchan;i++)
    {
            _50HzNotchFilters[i].initWithSamplingRate(sampleRate);
            _50HzNotchFilters[i].setCenterFrequency(50.0f);
            _50HzNotchFilters[i].setQ(1.0);
    }

    _60HzNotchFilters.resize(nchan);
    for(int i=0;i<nchan;i++)
    {
            _60HzNotchFilters[i].initWithSamplingRate(sampleRate);
            _60HzNotchFilters[i].setCenterFrequency(60.0f);
            _60HzNotchFilters[i].setQ(1.0);
    }


    _lowPassFilters.resize(nchan);
    for(int i=0;i<nchan;i++)
    {
            _lowPassFilters[i].initWithSamplingRate(sampleRate);
            _lowPassFilters[i].setCornerFrequency(sampleRate/2.0);
            _lowPassFilters[i].setQ(0.5f);
    }

    _highPassFilters.resize(nchan);
    for(int i=0;i<nchan;i++)
    {
            _highPassFilters[i].initWithSamplingRate(sampleRate);
            _highPassFilters[i].setCornerFrequency(0);
            _highPassFilters[i].setQ(0.5f);
    }

	dcBiasSum.resize(nchan);
}

RecordingManager::Device::~Device() {
}

bool RecordingManager::Device::enable(int64_t pos) {
	if(enabled)
		return true;

	Log::msg("RecordingManager::Device - BASS enable %d device", index);

	// reset dc bias calculation
	dcBiasNum = 1;
	for(int i = 0; i < (int)dcBiasSum.size(); i++)
		dcBiasSum[i] = 0;

    int64_t  tempPos = pos;
	for (int i = 0; i < channels; i++) {
		sampleBuffers[i].reset();
		sampleBuffers[i].setPos(tempPos);
	}

	if(type == Device::Audio) {
		// make sure the device exists
		BASS_DEVICEINFO info;
		if(!BASS_RecordGetDeviceInfo(index, &info)) {
			Log::error("Bass Error: getting record device info failed: %s", GetBassStrError());
			return false;
		}

		// initialize the recording device if we haven't already
		if(!(info.flags & BASS_DEVICE_INIT)) {
			if(!BASS_RecordInit(index)) {
				Log::error("Bass Error: initializing record device failed: %s", GetBassStrError());
				return false;
			}
		}

		// subsequent API calls will operate on this recording device
		if(!BASS_RecordSetDevice(index)) {
			Log::error("Bass Error: setting record device failed: %s", GetBassStrError());
			return false;
		}

		handle = BASS_RecordStart(samplerate, 2, 0, NULL, NULL);
		//handle = FALSE;
		if (handle == FALSE) {
			Log::error("Bass Error: starting the recording failed: %s", GetBassStrError());
			printf("\nBass Error: starting the recording failed: %s\n", GetBassStrError());
			return false;
		}
	}

	// if other types of devices need initialization, it should happen here.

	enabled = true;
	return true;
}

bool RecordingManager::Device::disable() {
	if(!enabled)
		return true;

	Log::msg("RecordingManager::Device - BASS disable %d device", index);

	if(type == Device::Audio) {
		// make sure the device exists
		BASS_DEVICEINFO info;
		if (!BASS_RecordGetDeviceInfo(index, &info)) {
			Log::error("Bass Error: getting record device info failed: %s", GetBassStrError());
			return false;
		}

		if (!BASS_ChannelStop(handle)) {
			Log::error("Bass Error: stopping recording failed: %s", GetBassStrError());
			return false;
		}

		// subsequent API calls will operate on this recording device
		if (!BASS_RecordSetDevice(index)) {
			Log::error("Bass Error: setting record device failed: %s", GetBassStrError());
			return false;
		}

		// free the recording device if we haven't already
		if(info.flags & BASS_DEVICE_INIT) {
			if(!BASS_RecordFree()) {
				Log::error("Bass Error: freeing record device failed: %s", GetBassStrError());
				return false;
			}
		}
	} else if(type == Device::File) {
		BASS_StreamFree(handle);
	}

	// if other types of devices need to do deinitialization, it should happen here.

	enabled = false;
	return true;
}

void RecordingManager::setPos(int64_t pos, bool artificial) {
	assert(_fileMode);
	pos = std::max((int64_t)0, std::min(fileLength()-1, pos));
  //  std::cout<<"\nSeek: "<<pos<<" \n";
	//if(pos == _pos)
	//	return;

   // std::cout<<"Aiming at: "<<pos<<"--------------------------------\n";

     fileIsLoadedAndFirstActionDidNotYetOccurred = false;//used for fake preload of file to the half of the screen
    currentPositionOfWaveform = pos;//set position of waveform to new value

	int endSegment = _devices[0].sampleBuffers[0].head()/SEGMENT_SIZE+1;

    int howMuchIsSetInAdvance = 0;
    for(int i=endSegment;i<endSegment+NUMBER_OF_SEGMENTS;i++)
    {
        if(_devices[0].sampleBuffers[0].segmentsState[i%NUMBER_OF_SEGMENTS]==0)
        {
            howMuchIsSetInAdvance+=SEGMENT_SIZE;
        }
    }
    int endOfDataThatNeedsToBeLoaded = _devices[0].sampleBuffers[0].pos() + howMuchIsSetInAdvance;

    //if we are seeking and we jump outside loaded buffer RESET buffers
    int leftBoandaryOfLoading = pos - SampleBuffer::SIZE/8;
    if(leftBoandaryOfLoading<0)
    {
        leftBoandaryOfLoading = 0;
    }

    if(((endOfDataThatNeedsToBeLoaded - leftBoandaryOfLoading)>SampleBuffer::SIZE) || //if we are ouside on left
       (endOfDataThatNeedsToBeLoaded < pos)) //if we are outside on right
    {
        //set playback emediately to correct position
        for(int idx = 0; idx < (int)_devices.size(); idx++)
        {
            for(unsigned int i = 0; i < _devices[idx].sampleBuffers.size(); i++) {
                SampleBuffer &s = _devices[idx].sampleBuffers[i];
                s.reset();
                //round position to segment end
                long safePositionToStartLoading = pos-SampleBuffer::SIZE/2;
                if(safePositionToStartLoading<0)
                {
                    safePositionToStartLoading = 0;
                }
                long positionToStartLoading = safePositionToStartLoading/SEGMENT_SIZE  * SEGMENT_SIZE;
                positionToStartLoading = std::max((int64_t)0, std::min(fileLength()-1, (int64_t)positionToStartLoading));
                const int nchan = _devices[idx].channels;
                BASS_ChannelSetPosition(_devices[idx].handle, _devices[idx].bytespersample*positionToStartLoading*nchan, BASS_POS_BYTE);
                //set cumulative number of samples LOADED from file
                s.setPos(positionToStartLoading);
            }
        }

    }

    //if we are approaching end of the loaded buffer initiate loading of more segments on the right

    if((endOfDataThatNeedsToBeLoaded - pos)< SampleBuffer::SIZE/8  && (endOfDataThatNeedsToBeLoaded<fileLength()))
    {
            int endSegment = _devices[0].sampleBuffers[0].head()/SEGMENT_SIZE;

            int numOfSegmentsToLoad = 6;
            if((numOfSegmentsToLoad-1)*SEGMENT_SIZE>(fileLength()-endOfDataThatNeedsToBeLoaded))
            {
                numOfSegmentsToLoad =(fileLength()-endOfDataThatNeedsToBeLoaded)/SEGMENT_SIZE +1;
            }
           // int newEndOfLoadedData = _devices[0].sampleBuffers[0].pos() + numOfSegmentsToLoad * SEGMENT_SIZE;

           for(int idx = 0; idx < (int)_devices.size(); idx++)
           {
               for(unsigned int i = 0; i < _devices[idx].sampleBuffers.size(); i++) {
                   SampleBuffer &s = _devices[idx].sampleBuffers[i];
                   for(int i=endSegment;i<=endSegment+numOfSegmentsToLoad;i++)
                   {
                       s.segmentsState[i%NUMBER_OF_SEGMENTS] = 0;
                   }
                  // s.setPos(newEndOfLoadedData);
                  // s.setHead(newEndOfLoadedData);
               }
           }

    }


	_pos = pos;//set current position in file (LOADED can be greater than this, loaded is stored in buffer: s._pos)

}

//check if buffer has loaded data at "pos" position
bool RecordingManager::isBufferLoadedAtPosition(long pos)
{
    if(!fileMode())
    {
        return true;
    }
    else
    {
        if(pos<_devices[0].sampleBuffers[0].pos())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

SampleBuffer *RecordingManager::sampleBuffer(int virtualDeviceIndex) {
	assert(virtualDeviceIndex >= 0 && virtualDeviceIndex < (int) _virtualDevices.size());
	const int device = _virtualDevices[virtualDeviceIndex].device;
	const int channel = _virtualDevices[virtualDeviceIndex].channel;

	assert((unsigned int)device < _devices.size() && (unsigned int)channel < _devices[device].sampleBuffers.size());
	SampleBuffer *result = &_devices[device].sampleBuffers[channel];
	return result;
}

#pragma mark - Bootloader

void RecordingManager::checkIfFirmwareIsAvailableForBootloader()
{
    _firmwareForBootloaderAvailable = _bootloaderController.isFirmwareAvailable();
}

void RecordingManager::putBoardInBootloaderMode()
{
    if(serialMode())
    {
        _bootloaderController.stage  = BOOTLOADER_STAGE_INITIALIZED;
        shouldStartFirmwareUpdatePresentation = true;
        _arduinoSerial.sendMessageToPutBoardIntoBootloaderMode();
    }
}


 #ifdef _WIN32
    void RecordingManager::startBootloaderProcess(std::string nameOfThePort, void * portHandle)
    {
        _bootloaderController.portName = nameOfThePort;
        _bootloaderController.portHandle = portHandle;
        _bootloaderController.startUpdateProcess();
    }
#else
    void RecordingManager::startBootloaderProcess(std::string nameOfThePort, int portHandle)
    {
        _bootloaderController.portName = nameOfThePort;
        _bootloaderController.portHandle = portHandle;
        _bootloaderController.startUpdateProcess();
    }
#endif


bool RecordingManager::firmwareUpdateShouldBeActive()
{
    return serialMode() && _firmwareForBootloaderAvailable && _arduinoSerial.currentPort.deviceType==ArduinoSerial::humansb;
}

int RecordingManager::bootloaderState()
{
    
    return _bootloaderController.stage;
}

#pragma mark - Joystick related

void RecordingManager::setKeyForJoystick(int channelIndex, int keyIndex)
{
    if(channelIndex<NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK && channelIndex>=0)
    {
        _keyIndexSetForJoystick[channelIndex] = keyIndex;
    }
    #if defined(_WIN32)
    if(channelIndex == 0)
    {
        configValues.firstChannelButton = keyIndex;
    }
    if(channelIndex == 1)
    {
        configValues.secondChannelButton = keyIndex;
    }
    if(channelIndex == 2)
    {
        configValues.thirdChannelButton = keyIndex;
    }
    configValues.saveDefaults();
    #endif
}

int RecordingManager::getKeyIndexForJoystick(int channelIndex)
{
    if(channelIndex<NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK && channelIndex>=0)
    {
        return _keyIndexSetForJoystick[channelIndex];
    }
    else
    {
        return 0;
    }
}

void RecordingManager::initDefaultJoystickKeys()
{
    for(int i=0;i<NUMBER_OF_AVAILABLE_CHANNELS_FOR_JOYSTICK;i++)
    {
        _keyIndexSetForJoystick[i] =0;
        _timeOfLastTriggerJoystick[i] = 0;
        _lastValueOfSignalJoystick[i] = 0;
    }
    #if defined(_WIN32)
    _keyIndexSetForJoystick[0] =  configValues.firstChannelButton;
    _keyIndexSetForJoystick[1] =  configValues.secondChannelButton;
    _keyIndexSetForJoystick[2] =  configValues.thirdChannelButton;
    #endif
}
#pragma mark - Input Config related

//
// Initial state of zoom and filters for each type of audio input
//
void RecordingManager::initInputConfigPersistance()
{
    //standard line in/microphone input
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].inputType = INPUT_TYPE_STANDARD_AUDIO;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].filter60Hz = false;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].filterLowPass = DEFAULT_SAMPLE_RATE/2;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].filterHighPass = 0.0f;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO].initialized = true;

    //am modulated audio
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].inputType = INPUT_TYPE_AM_AUDIO;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].filter60Hz = false;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].filterLowPass = DEFAULT_SAMPLE_RATE/2;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].filterHighPass = 0.0f;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_AM_AUDIO].initialized = true;

    //arduino unknown
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].inputType = INPUT_TYPE_ARDUINO_UNKOWN;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].filter60Hz = false;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].filterLowPass = 5000;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].filterHighPass = 0.0f;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_ARDUINO_UNKOWN].initialized = true;


    //arduino Plant
    audioInputConfigArray[INPUT_TYPE_PLANTSS].inputType = INPUT_TYPE_PLANTSS;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].filter60Hz = true;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].filterLowPass = 5.0f;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].filterHighPass = 0.0f;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_PLANTSS].initialized = true;


    //arduino EMG
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].inputType = INPUT_TYPE_MUSCLESS;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].filter60Hz = true;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].filterLowPass = 2500.0f;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].filterHighPass = 70.0f;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].timeScale = 1.0f;
    audioInputConfigArray[INPUT_TYPE_MUSCLESS].initialized = true;

    //arduino Heart and Brain
    audioInputConfigArray[INPUT_TYPE_HEARTSS].inputType = INPUT_TYPE_HEARTSS;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].filter60Hz = true;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].filterLowPass = 50.0f;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].filterHighPass = 1.0f;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].timeScale = 1.0f;
    audioInputConfigArray[INPUT_TYPE_HEARTSS].initialized = true;

    //arduino Neuron SpikerBox
    audioInputConfigArray[INPUT_TYPE_NEURONSS].inputType = INPUT_TYPE_NEURONSS;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].filter60Hz = true;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].filterLowPass = 5000.0f;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].filterHighPass = 1.0f;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_NEURONSS].initialized = true;

    //HID - SpikerBox Pro
    audioInputConfigArray[INPUT_TYPE_SB_PRO].inputType = INPUT_TYPE_SB_PRO;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].filter60Hz = false;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].filterLowPass = 5000;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].filterHighPass = 1.0f;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].gain = 0.1f * 0.20717623328f;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_SB_PRO].initialized = true;


    //HID - File (this is not used anywhere for now)
    audioInputConfigArray[INPUT_TYPE_FILE].inputType = INPUT_TYPE_FILE;
    audioInputConfigArray[INPUT_TYPE_FILE].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_FILE].filter60Hz = false;
    audioInputConfigArray[INPUT_TYPE_FILE].filterLowPass = 5000;
    audioInputConfigArray[INPUT_TYPE_FILE].filterHighPass = 0.0f;
    audioInputConfigArray[INPUT_TYPE_FILE].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_FILE].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_FILE].initialized = true;

    audioInputConfigArray[INPUT_TYPE_HHIBOX].inputType = INPUT_TYPE_HHIBOX;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filter60Hz = true;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filterLowPass = 2500;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filterHighPass = 70.0f;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].initialized = true;

    audioInputConfigArray[INPUT_TYPE_HHIBOX].inputType = INPUT_TYPE_HUMANSB;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filter50Hz = false;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filter60Hz = true;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filterLowPass = 2500;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].filterHighPass = 1.0f;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].gain = 0.5f;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].timeScale = 0.1f;
    audioInputConfigArray[INPUT_TYPE_HHIBOX].initialized = true;
}

//
// Returns input config object for particular input type
//
AudioInputConfig * RecordingManager::getInputConfigForType(int inputType)
{
  if(inputType!=INPUT_TYPE_STANDARD_AUDIO && inputType!=INPUT_TYPE_FILE)
  {
      return &(audioInputConfigArray[inputType]);
  }
  else
  {
      return &(audioInputConfigArray[INPUT_TYPE_STANDARD_AUDIO]);
  }

}

int RecordingManager::getCurrentInputType()
{
    if(hidMode())
    {
        return INPUT_TYPE_SB_PRO;
    }
    else if(serialMode())
    {

        switch(_arduinoSerial.currentPort.deviceType) {
            case ArduinoSerial::unknown:
                return INPUT_TYPE_ARDUINO_UNKOWN;
                break;
            case ArduinoSerial::plant:
                return INPUT_TYPE_PLANTSS;
                break;
            case ArduinoSerial::heart:
                return INPUT_TYPE_HEARTSS;
                break;
            case ArduinoSerial::neuronOneChannel:
                return INPUT_TYPE_NEURONSS;
                break;
            case ArduinoSerial::heartOneChannel:
                return INPUT_TYPE_HEARTSS;
                break;
            case ArduinoSerial::heartPro:
                return INPUT_TYPE_HEARTSS;
                break;
            case ArduinoSerial::muscle:
                return INPUT_TYPE_MUSCLESS;
                break;
            case ArduinoSerial::muscleusb:
                return INPUT_TYPE_MUSCLESS;
                break;
            case ArduinoSerial::humansb:
                return INPUT_TYPE_HUMANSB;
                break;
            case ArduinoSerial::hhibox:
                return INPUT_TYPE_HHIBOX;
                break;
            case ArduinoSerial::sbpromusclecdc:
                return INPUT_TYPE_SB_PRO;
                break;
            case ArduinoSerial::sbproneuroncdc:
                return INPUT_TYPE_SB_PRO;
                break;
            default:
                return INPUT_TYPE_ARDUINO_UNKOWN;
                break;
        }
    }
    else if(fileMode())
    {
        return INPUT_TYPE_FILE;
    }
    else
    {//normal audio input
        if(weAreReceivingAMSignal)
        {
            return INPUT_TYPE_AM_AUDIO;
        }
        else
        {
            return INPUT_TYPE_STANDARD_AUDIO;
        }
    }
}

void RecordingManager::saveInputConfigSettings()
{
        //since init for every input type first saves config for previous type and
        //than it loads config for new type we have to avoid saving the config
        //first time, before we initialize it first time
       if(!firstTimeInitializationOfSettingsForAudioInput)
       {
            int inputType = getCurrentInputType();

            if(serialMode())
            {



                std::list<AudioInputConfig>::iterator it;
                std::size_t found;
                bool foundPort = false;
                for(it = arduinoShieldsConfigs.begin();it!=arduinoShieldsConfigs.end();it++)
                {
                    found  = getCurrentPort().portName.find(it->uniqueName);
                    if (found!=std::string::npos)
                    {
                        foundPort = true;
                        it->filter50Hz = fiftyHzFilterEnabled();
                        it->filter60Hz = sixtyHzFilterEnabled();
                        it->filterLowPass = _lowCornerFreq;
                        it->filterHighPass = _highCornerFreq;
                        //gain and timescale should be constanly updated by AudioView
                        it->initialized = true;
                        break;
                    }
                }

            }
            else
            {
                audioInputConfigArray[inputType].filter50Hz = fiftyHzFilterEnabled();
                audioInputConfigArray[inputType].filter60Hz = sixtyHzFilterEnabled();
                audioInputConfigArray[inputType].filterLowPass = _lowCornerFreq;
                audioInputConfigArray[inputType].filterHighPass = _highCornerFreq;
                //gain and timescale should be constanly updated by AudioView
                audioInputConfigArray[inputType].initialized = true;
            }

       }
        firstTimeInitializationOfSettingsForAudioInput = false;
}

void RecordingManager::makeNewSerialAudioConfig(std::string nameOfThePort)
{

    std::size_t found;
    bool foundPort = false;
    for(iteratorPointerToCurrentSerialAudioConfig = arduinoShieldsConfigs.begin();iteratorPointerToCurrentSerialAudioConfig!=arduinoShieldsConfigs.end();iteratorPointerToCurrentSerialAudioConfig++)
    {
        found  = getCurrentPort().portName.find(iteratorPointerToCurrentSerialAudioConfig->uniqueName);
        if (found!=std::string::npos)
        {
            foundPort = true;
            break;
        }
    }
    if(!foundPort)
    {
        int audioInputType = INPUT_TYPE_ARDUINO_UNKOWN;
        switch(_arduinoSerial.currentPort.deviceType) {
            case ArduinoSerial::unknown:
                audioInputType =  INPUT_TYPE_ARDUINO_UNKOWN;
                break;
            case ArduinoSerial::plant:
                audioInputType =  INPUT_TYPE_PLANTSS;
                break;
            case ArduinoSerial::heart:
                audioInputType = INPUT_TYPE_HEARTSS;
                break;
            case ArduinoSerial::heartOneChannel:
                audioInputType = INPUT_TYPE_HEARTSS;
                break;
            case ArduinoSerial::heartPro:
                audioInputType = INPUT_TYPE_HEARTSS;
                break;
            case ArduinoSerial::neuronOneChannel:
                audioInputType = INPUT_TYPE_NEURONSS;
                break;
            case ArduinoSerial::muscle:
                audioInputType =  INPUT_TYPE_MUSCLESS;
                break;
            case ArduinoSerial::muscleusb:
                audioInputType =  INPUT_TYPE_MUSCLESS;
                break;
            case ArduinoSerial::humansb:
                audioInputType =  INPUT_TYPE_HUMANSB;
                break;
            case ArduinoSerial::hhibox:
                audioInputType =  INPUT_TYPE_HHIBOX;
                break;
            default:
                audioInputType = INPUT_TYPE_ARDUINO_UNKOWN;
                break;
        }

        AudioInputConfig newArduinoAudioConfig;
        newArduinoAudioConfig.uniqueName = nameOfThePort;
        newArduinoAudioConfig.inputType = audioInputConfigArray[audioInputType].inputType;
        newArduinoAudioConfig.filter50Hz = audioInputConfigArray[audioInputType].filter50Hz;
        newArduinoAudioConfig.filter60Hz = audioInputConfigArray[audioInputType].filter60Hz;
        newArduinoAudioConfig.filterLowPass = audioInputConfigArray[audioInputType].filterLowPass;
        newArduinoAudioConfig.filterHighPass = audioInputConfigArray[audioInputType].filterHighPass;
        newArduinoAudioConfig.gain = audioInputConfigArray[audioInputType].gain;
        newArduinoAudioConfig.timeScale = audioInputConfigArray[audioInputType].timeScale;
        newArduinoAudioConfig.initialized = audioInputConfigArray[audioInputType].initialized;

        arduinoShieldsConfigs.push_back(newArduinoAudioConfig);

        //just to get iteratorPointerToCurrentSerialAudioConfig to point to right one
        for(iteratorPointerToCurrentSerialAudioConfig = arduinoShieldsConfigs.begin();iteratorPointerToCurrentSerialAudioConfig!=arduinoShieldsConfigs.end();iteratorPointerToCurrentSerialAudioConfig++)
        {
            found  = getCurrentPort().portName.find(iteratorPointerToCurrentSerialAudioConfig->uniqueName);
            if (found!=std::string::npos)
            {

                break;
            }
        }
    }

}

void RecordingManager::saveGainForAudioInput(float newGain)
{
    int inputType = getCurrentInputType();
    if(serialMode())
    {
        iteratorPointerToCurrentSerialAudioConfig->gain = newGain;
    }
    else
    {
        audioInputConfigArray[inputType].gain = newGain;
    }
}

void RecordingManager::saveTimeScaleForAudioInput(float newTimeScale)
{
    int inputType = getCurrentInputType();
    if(serialMode())
    {
        iteratorPointerToCurrentSerialAudioConfig->timeScale = newTimeScale;
    }
    else
    {
        audioInputConfigArray[inputType].timeScale = newTimeScale;
    }
}

float RecordingManager::loadGainForAudioInput()
{
    int inputType = getCurrentInputType();
    if(serialMode())
    {
       return iteratorPointerToCurrentSerialAudioConfig->gain;
    }

    return audioInputConfigArray[inputType].gain;

}

float RecordingManager::loadTimeScaleForAudioInput()
{
    int inputType = getCurrentInputType();
    if(serialMode())
    {
        return iteratorPointerToCurrentSerialAudioConfig->timeScale;
    }
    return audioInputConfigArray[inputType].timeScale;
}

void RecordingManager::loadFilterSettings()
{
    int inputType = getCurrentInputType();
    if(serialMode())
    {
        enableHighPassFilterWithCornerFreq(iteratorPointerToCurrentSerialAudioConfig->filterHighPass);
        enableLowPassFilterWithCornerFreq(iteratorPointerToCurrentSerialAudioConfig->filterLowPass);

        if(_lowCornerFreq<LPF_HUMAN_SP_THRESHOLD)
        {
            //turn ON gain
            setSerialHardwareGain(true);
        }
        else
        {
            setSerialHardwareGain(false);
        }
        
        if(_highCornerFreq<HPF_HUMAN_SP_THRESHOLD)
        {
            //turn OFF filter
            setSerialHardwareHPF(false);
        }
        else
        {
            setSerialHardwareHPF(true);
        }
        
        if(iteratorPointerToCurrentSerialAudioConfig->filter60Hz)
        {
            enable60HzFilter();
        }
        else
        {
            disable60HzFilter();
        }

        if(iteratorPointerToCurrentSerialAudioConfig->filter50Hz)
        {
            enable50HzFilter();
        }
        else
        {
            disable50HzFilter();
        }

    }
    else
    {
        enableHighPassFilterWithCornerFreq(audioInputConfigArray[inputType].filterHighPass);
        enableLowPassFilterWithCornerFreq(audioInputConfigArray[inputType].filterLowPass);

        if(audioInputConfigArray[inputType].filter60Hz)
        {
            enable60HzFilter();
        }
        else
        {
            disable60HzFilter();
        }

        if(audioInputConfigArray[inputType].filter50Hz)
        {
            enable50HzFilter();
        }
        else
        {
            disable50HzFilter();
        }
    }
}



} // namespace BackyardBrains
