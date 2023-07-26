//
//  BYBArduino.h
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 11/26/14.
//  Copyright (c) 2014 BYB. All rights reserved.
//


#ifndef BACKYARDBRAINS_ARDUINOSERIAL_H
#define BACKYARDBRAINS_ARDUINOSERIAL_H


#define SIZE_OF_MESSAGES_BUFFER 64
#define ESCAPE_SEQUENCE_LENGTH 6
#define SIZE_OF_INPUT_HARDWARE_CIRC_BUFFER 193152


#include <iostream>
#include <fcntl.h>
#include <list>
#include <string>
#include "constants.h"

//just MAC classes
#ifdef __APPLE__
    #include <termios.h>
    #include <CoreFoundation/CoreFoundation.h>
    #include <IOKit/IOKitLib.h>
    #include <IOKit/serial/IOSerialKeys.h>
    #include <IOKit/IOBSD.h>
#elif __linux__
    #include <termios.h>
#elif _WIN32
    #include <windows.h>
#endif


#ifdef _WIN32
    #include "mingw.thread.h"
#else
    #include <thread>
#endif

namespace BackyardBrains {

    enum SerialDevice {
        unknown = 0,
        muscle = 1,
        plant = 2,
        heart = 3,
        neuron = 4
    };

    class RecordingManager;





    class ArduinoSerial {
    public:
        enum SerialDevice {
            unknown = 0,
            muscle = 1,
            plant = 2,
            heart = 3,
            heartOneChannel = 4,
            heartPro = 5,
            neuronOneChannel = 6,
            erg=7,
            muscleusb=8,
            humansb=9,
            hhibox=10,
            sbpromusclecdc=11,
            sbproneuroncdc=12,
            sbproneuronmfi=13,
            unibox=14,
            extclaw=15
        };
        struct SerialPort
        {
            SerialPort()
            {
                deviceType = ArduinoSerial::SerialDevice::unknown;
                numOfTrials = 0;
                baudRate = 222222;
            }
            std::string portName;
            SerialDevice deviceType;
            int baudRate;
            int numOfTrials;
        };


        ArduinoSerial();
        void setRecordingManager(RecordingManager *rm);
        int openSerialDevice(const char *portName);
        int readPort(char * buffer);
        int getNewSamples(int16_t * obuffer);
        int processDataIntoSamples(char * buffer, int size, int16_t * obuffer);
        std::list<std::string> list;
        std::list<SerialPort> ports;
        void getAllPortsList();
        void closeSerial(void);
        void setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate);
        int writeToPort(const void *ptr, int len);
        int maxSamplingRate();
        int maxNumberOfChannels();
        int numberOfChannels();
        const char * currentPortName();
        std::string errorString;
        bool portOpened();
        void sendEventMessage(int eventType);
        void sendPotentiometerMessage(uint8_t value);
        void checkAllPortsForArduino(ArduinoSerial * workingArduinoRef);
        void askForBoardType();
        void askForImportantStates();
        void askForP300AudioState();
        void askForExpansionBoardType();
        void sendMessageToPutBoardIntoBootloaderMode();
        void setBaudRate(int baudRate);
        int getBaudRate(){return currentTestingBaudRate;}

        //std::string firmwareVersion;
        //std::string hardwareVersion;
        std::string hardwareType;
        SerialPort currentPort;

        void startScanningForArduinos(ArduinoSerial * refToWorkingArduinoSerial);
        static bool openPortLock;
        void setSampleRateAndNumberOfChannelsBasedOnType();
        int getSampleRate(){return _samplingRate;}
        bool waitingForRestart(){return _shouldRestartDevice;}
        void deviceRestarted(){_shouldRestartDevice = false;}
        int addOnBoardPressent();

        void pressKey(int keyIndex);
        void releaseKey(int keyIndex);

        void closeCurrentMainSerial(void);
        
        void setGain(bool active);
        void setHPF(bool active);
        
        void setP300(bool active);
        void setP300AudioStimulation(bool active);
    private:
        RecordingManager *_manager;
        bool _shouldRestartDevice;
        int openPort(const char *portName);
        std::thread scanningThread;
        std::thread closePortPatchThread;
    #if defined(_WIN32)
        void closePortPatchThreadFunction(ArduinoSerial * selfRef);
    #endif
        void refreshPortList(std::list<SerialPort> newPorts);
        void scanPortsThreadFunction(ArduinoSerial * selfRef, ArduinoSerial * workingArduinoRef);
        int openSerialDeviceWithoutLock(const char *portName);
        void refreshPortsDataList();
        void setDeviceTypeToCurrentPort(SerialDevice deviceType);
        void checkIfWeHavetoAskBoardSomething(void);
        char circularBuffer[SIZE_OF_INPUT_HARDWARE_CIRC_BUFFER];
        int cBufHead;
        int cBufTail;
        int fd;//file descriptor
        int _numberOfChannels;
        int _samplingRate;
        int serialCounter;
        int numberOfTimouts;

        bool returnTailForOneAndCheck();
        bool checkIfNextByteExist();
        bool areWeAtTheEndOfFrame();
        bool checkIfHaveWholeFrame();
        int currentTestingBaudRate;
        bool testingHighBaudRate;

        void testEscapeSequence(unsigned int newByte, int offset);
        void executeContentOfMessageBuffer(int offset);
        void executeOneMessage(std::string typeOfMessage, std::string valueOfMessage, int offsetin);
        //void executeOneMessage(std::string typeOfMessage, std::string valueOfMessage, int offsetin);
        //void executeContentOfMessageBuffer(int offset);
        int escapeSequenceDetectorIndex;
        bool weAreInsideEscapeSequence;
        char messagesBuffer[SIZE_OF_MESSAGES_BUFFER];//contains payload inside escape sequence
        int messageBufferIndex;
        unsigned int escapeSequence[ESCAPE_SEQUENCE_LENGTH];
        unsigned int endOfescapeSequence[ESCAPE_SEQUENCE_LENGTH];
        const std::string currentDateTime();

        #ifdef __APPLE__
            void macos_ports(io_iterator_t  * PortIterator);

        #elif _WIN32
            HANDLE port_handle;
            COMMCONFIG port_cfg_orig;
            COMMCONFIG port_cfg;
            void enumerateSerialPortsFriendlyNames( std::string& portForBootloader);
        #endif
        bool _justScanning;
        int currentAddOnBoard;
        std::string _portName;
        bool _portOpened;
        bool triedToConfigureAgain;
        int batchSizeForSerial;

        //for Joystick

        #if defined(_WIN32)
        KeyForJoystick keysForJoystick[8];
        #endif


        uint8_t previousButtonState;
        uint8_t currentButtonState;

        bool checkIfKeyWasPressed(int keyIndex);
        bool checkIfKeyWasReleased(int keyIndex);
        void turnONJoystickLed(int ledIndex);
        void turnOFFJoystickLed(int ledIndex);



        //buffering from serial device
        char circularInputBuffer[SIZE_OF_INPUT_HARDWARE_CIRC_BUFFER];
        int headHardwareCircular;
        int tailHardwareCircular;
        int getNewDataFromHardwareBuffer(char * buffer, int max_data_length, int* availableData);
        std::thread readingHardwareThread;
        void readHardwareThreadFunction(ArduinoSerial* ref);
        bool prepareForDisconnect;

    };


}

#endif /* defined(__SpikeRecorder__BYBArduino__) */
