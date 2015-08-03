#ifndef HIDUSBMANAGER_H
#define HIDUSBMANAGER_H

#define SIZE_OF_CIRC_BUFFER 4024
#define SIZE_OF_MESSAGES_BUFFER 64
#define ESCAPE_SEQUENCE_LENGTH 6
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
class HIDUsbManager
{
    public:
        HIDUsbManager();
        int openDevice(RecordingManager * managerin);
        int readDevice(int32_t * obuffer);
        std::list<std::string> list;
        void getAllDevicesList();
        void closeDevice();
        void askForCapabilities();
        void askForMaximumRatings();
        void setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate);
        int writeToDevice(const unsigned char *ptr, size_t len);
        int maxSamplingRate();
        int maxNumberOfChannels();
        int numberOfChannels();
        const char * currentDeviceName();
        std::string errorString;
        bool deviceOpened();
        int readOneBatch(int32_t * obuffer);
        int32_t *mainCircularBuffer;
        void stopDevice();
        void putInFirmwareUpdateMode();

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
        bool returnTailForOneAndCheck();
        bool checkIfNextByteExist();
        bool areWeAtTheEndOfFrame();
        bool checkIfHaveWholeFrame();
        void readThread(HIDUsbManager * ref);
        bool _deviceConnected;

        void testEscapeSequence(unsigned int newByte, int offset);
        void executeOneMessage(std::string typeOfMessage, std::string valueOfMessage, int offsetin);
        void executeContentOfMessageBuffer(int offset);
        int escapeSequenceDetectorIndex;
        int watchDogTimerForEscapeSequence;
        bool weAreInsideEscapeSequence;
        char messagesBuffer[SIZE_OF_MESSAGES_BUFFER];//contains payload inside escape sequence
        int messageBufferIndex;
        unsigned int escapeSequence[ESCAPE_SEQUENCE_LENGTH];
        unsigned int endOfescapeSequence[ESCAPE_SEQUENCE_LENGTH];
        unsigned int tempHeadAndTailDifference;//used for precise events reference

    private:
}; //class end

}//namespace
#endif // HIDUSBMANAGER_H
