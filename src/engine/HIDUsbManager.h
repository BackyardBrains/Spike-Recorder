#ifndef HIDUSBMANAGER_H
#define HIDUSBMANAGER_H

#define SIZE_OF_CIRC_BUFFER 4024
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"
#include <list>
#include <string>
#include <iostream>
#include <sys/time.h>

#ifdef _WIN32
	#include <windows.h>
	#include "mingw.thread.h"
#else
	#include <unistd.h>
	#include <thread>
    #include <functional>
#endif

namespace BackyardBrains {
class HIDUsbManager
{
    public:
        HIDUsbManager();
        int openDevice();
        int readDevice(int16_t * obuffer);
        std::list<std::string> list;
        void getAllDevicesList();
        void closeDevice();
        void askForCapabilities();
        void setNumberOfChannelsAndSamplingRate(int numberOfChannels, int samplingRate);
        int writeToDevice(const unsigned char *ptr, size_t len);
        int maxSamplingRate();
        int maxNumberOfChannels();
        int numberOfChannels();
        const char * currentDeviceName();
        std::string errorString;
        bool deviceOpened();
        int readOneBatch(int16_t * obuffer);
        int16_t *mainCircularBuffer;
        void stopDevice();
    protected:
        void startDevice();

        char circularBuffer[SIZE_OF_CIRC_BUFFER];
        std::thread t1;
        int mainHead;
        int mainTail;
        int cBufHead;
        int cBufTail;
        hid_device *handle;
        int _numberOfChannels;
        int _samplingRate;
        int serialCounter;
        bool returnTailForOneAndCheck();
        bool checkIfNextByteExist();
        bool areWeAtTheEndOfFrame();
        bool checkIfHaveWholeFrame();
        void readThread(HIDUsbManager * ref);

        bool _deviceConnected;
    private:
}; //class end

}//namespace
#endif // HIDUSBMANAGER_H
