
/*
    This class FirmwareUpdater is used to update MSP430 devices 
    like old HID Muscle and Neuron SpikerBox Pro.
    Class takes info about available firmware versions from server 
    https://backyardbrains.com/products/firmwares/sbpro/compatibility.xml
    and offers users to update firmware.
    Works only on Windows since actual communiation with boothloader uses DLL for Windows only

*/
#ifndef FIRMWAREUPDATER_H
#define FIRMWAREUPDATER_H
#include "constants.h"
#include "tinyxml2.h"
#include <string>
#include <list>
#include "BYBFirmwareVO.h"
#include <iostream>

namespace BackyardBrains {
    class FirmwareUpdater
    {
        public:
            FirmwareUpdater();
            std::string errorString;
             std::list<BYBFirmwareVO> firmwares;
             int downloadFirmware(BYBFirmwareVO * firmwareInfo);
        protected:
        private:
            void LoadXMLFile();
            bool checkNodeName(tinyxml2::XMLNode * node, const char * name);
            tinyxml2::XMLNode * findChildWithName(tinyxml2::XMLNode * parentNode, const char * name);
            void logError(const char * errorMessage);
            static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream, void *p);
            size_t write_data_impl(void *ptr, size_t size, size_t nmemb, FILE *stream);
            typedef std::list<BYBFirmwareVO> listBYBFirmwareVO;
            void downloadCompatibilityXML();

    };
}
#endif // FIRMWAREUPDATER_H
