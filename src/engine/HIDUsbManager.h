#ifndef HIDUSBMANAGER_H
#define HIDUSBMANAGER_H

#define SIZE_OF_CIRC_BUFFER 4024
#define SIZE_OF_MESSAGES_BUFFER 64
#define ESCAPE_SEQUENCE_LENGTH 6

#define HID_POWER_ON 1
#define HID_POWER_OFF 0
#define HID_POWER_UNKNOWN -1

typedef enum
{
    HID_BOARD_TYPE_NONE,
    HID_BOARD_TYPE_MUSCLE,
    HID_BOARD_TYPE_NEURON
} HIDBoardType;

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"
#include <list>
#include <string>
#include <iostream>
#include <sys/time.h>
//

#ifdef _WIN32
	#include <windows.h>
	#include "mingw.thread.h"
#else
	#include <unistd.h>
	#include <thread>
    #include <functional>
#endif

namespace BackyardBrains {
class RecordingManager;

    typedef struct HIDManagerDevice {

        HIDBoardType deviceType;
        std::string devicePath;
        std::string serialNumber;

    }HIDManagerDevice;

    #ifdef _WIN32
        typedef struct KeyForJoystick {
            BYTE      bVk;
            BYTE      bScan;
            DWORD     dwFlags;
        }KeyForJoystick;
    #endif

class HIDUsbManager
{
    public:
        HIDUsbManager();

        // scan
        void getAllDevicesList();
        std::list<HIDManagerDevice> list;
        int isBoardTypeAvailable(HIDBoardType bt);

        // open / close
        int openDevice(RecordingManager * managerin, HIDBoardType hidBoardType);
        void closeDevice();

        //read device
        int readDevice(int32_t * obuffer);
        int readOneBatch(int32_t * obuffer);
        void askForCapabilities();
        void askForMaximumRatings();
        void askForStateOfPowerRail();
        void askForBoard();
        void askForRTRepeat();
        void pressKey(int keyIndex);
        void releaseKey(int keyIndex);

        //write to device
        int writeToDevice(const unsigned char *ptr, size_t len);
        void setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate);
        void stopDevice();
        void putInFirmwareUpdateMode();

        //properties and state
        int currentlyConnectedHIDBoardType();
        int maxSamplingRate();
        int maxNumberOfChannels();
        int numberOfChannels();
        std::string errorString;
        bool deviceOpened();
        int32_t *mainCircularBuffer;
        int addOnBoardPressent();
        bool isRTRepeating();
        void swapRTRepeat();
        int powerRailIsState();//HID_POWER_ON, HID_POWER_OFF, HID_POWER_UNKNOWN
        std::string firmwareVersion;
        std::string hardwareVersion;
        std::string hardwareType;
    protected:

        void startDevice();
        RecordingManager *_manager;
        char circularBuffer[SIZE_OF_CIRC_BUFFER];
        std::thread t1;
        int mainHead;
        int mainTail;
        int cBufHead;
        int cBufTail;
        hid_device *handle;
        int _numberOfChannels;
        int _samplingRate;
        int _powerRailState=-1;//Power rail is ON 1, OFF 0, unknown -1
        bool returnTailForOneAndCheck();
        bool checkIfNextByteExist();
        bool areWeAtTheEndOfFrame();
        bool checkIfHaveWholeFrame();
        void readThread(HIDUsbManager * ref);
        bool _deviceConnected;
        bool prepareForDisconnect;
        bool hidAccessBlock;

        bool writeWantsToAccessHID;
        bool readGrantsAccessToWriteToHID;
        bool writeWantsToReleaseAccessHID;
        bool readGrantsReleaseAccessToWriteToHID;

        int currentAddOnBoard;
        bool _rtReapeating;
        bool restartDevice;
        int maxSamples;
        int currentConnectedDevicePID;
        void testEscapeSequence(unsigned int newByte, int offset);
        void executeOneMessage(std::string typeOfMessage, std::string valueOfMessage, int offsetin);
        void executeContentOfMessageBuffer(int offset);
        int escapeSequenceDetectorIndex;
        bool weAreInsideEscapeSequence;
        char messagesBuffer[SIZE_OF_MESSAGES_BUFFER];//contains payload inside escape sequence
        int messageBufferIndex;
        unsigned int escapeSequence[ESCAPE_SEQUENCE_LENGTH];
        unsigned int endOfescapeSequence[ESCAPE_SEQUENCE_LENGTH];
        unsigned int tempHeadAndTailDifference;//used for precise events reference
        void enumerateDevicesForVIDAndPID(int invid, int inpid);


        bool checkIfKeyWasPressed(int keyIndex);
        bool checkIfKeyWasReleased(int keyIndex);


        #if defined(_WIN32)
        KeyForJoystick keysForJoystick[8];

        #endif

        void setJoystickLeds(uint8_t state);
        void turnONJoystickLed(int ledIndex);
        void turnOFFJoystickLed(int ledIndex);
    private:
        uint8_t previousButtonState;
        uint8_t currentButtonState;
}; //class end

}//namespace
#endif // HIDUSBMANAGER_H
