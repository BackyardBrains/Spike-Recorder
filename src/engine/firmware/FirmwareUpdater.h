#ifndef FIRMWAREUPDATER_H
#define FIRMWAREUPDATER_H
#include "constants.h"
#include "tinyxml2.h"
#include <string>
#include <list>
#include "BYBFirmwareVO.h"

namespace BackyardBrains {
    class FirmwareUpdater
    {
        public:
            FirmwareUpdater();
            std::string errorString;
             std::list<BYBFirmwareVO> firmwares;
        protected:
        private:
            void LoadXMLFile();
            bool checkNodeName(tinyxml2::XMLNode * node, const char * name);
            tinyxml2::XMLNode * findChildWithName(tinyxml2::XMLNode * parentNode, const char * name);
            void logError(const char * errorMessage);
    };
}
#endif // FIRMWAREUPDATER_H
