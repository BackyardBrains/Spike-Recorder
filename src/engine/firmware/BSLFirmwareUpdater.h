/*
    This class BSLFirmwareUpdater is used to update MSP430 devices 
    like old HID Muscle and Neuron SpikerBox Pro.
    Works only on Windows since actual communiation with boothloader uses BSL430.dll for Windows only

*/

#ifndef BSLFIRMWAREUPDATER_H
#define BSLFIRMWAREUPDATER_H


//
#include <string>
#ifdef _WIN32
	#include <windows.h>
	#include "mingw.thread.h"
	#include "BSL430_dll.h"
    #include "CRC.h"
    #include "FirmwareUpdater.h"
#else
	#include <unistd.h>
	#include <thread>
    #include <functional>
#endif

namespace BackyardBrains {
class BSLFirmwareUpdater
{
    public:
        BSLFirmwareUpdater();
        std::string errorString;
        void customSelectedFirmware(const char * firmwareFilename);
        int currentStage = 0;
        std::string currentStageMessage;
        const char * firmwareFilenameGlobal;
        void updateThread(BSLFirmwareUpdater * ref);
    protected:
    std::thread t1;
    unsigned char * bslVersion;
    bool triggerForcedBOR();
    bool downloadStartUp(bool * massErased );
    void eraseDataSegment_File( const char * fileName );
    void sendNotification(const char * notificationMessage,int stage);
    void sendErrorNotification(const char * errorMessage);
    BYBFirmwareVO * newFirmwareObject;
    private:
}; //class end

}//namespace
#endif // BSLFIRMWAREUPDATER_H
