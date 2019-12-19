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
             std::list<BYBFirmwareVO> hidFirmwares;
             std::list<BYBFirmwareVO> serialFirmwares;
             int downloadFirmware(BYBFirmwareVO * firmwareInfo);
        protected:
        private:
            void LoadAndParseHIDXMLFile();
            bool checkNodeName(tinyxml2::XMLNode * node, const char * name);
            tinyxml2::XMLNode * findChildWithName(tinyxml2::XMLNode * parentNode, const char * name);
            void logError(const char * errorMessage);
            static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream, void *p);
            size_t write_data_impl(void *ptr, size_t size, size_t nmemb, FILE *stream);
            typedef std::list<BYBFirmwareVO> listBYBFirmwareVO;
            void downloadCompatibilityXML();
            int downloadSerialCompatibilityXML();
            void loadAndParseSerialXMLFile();
            char outfilename[FILENAME_MAX];

    };
}
#endif // FIRMWAREUPDATER_H
