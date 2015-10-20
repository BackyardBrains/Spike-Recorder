#ifndef FIRMWAREUPDATER_H
#define FIRMWAREUPDATER_H
#include "constants.h"

namespace BackyardBrains {
    class FirmwareUpdater
    {
        public:
            FirmwareUpdater();

        protected:
        private:
            void LoadXMLFile();
    };
}
#endif // FIRMWAREUPDATER_H
