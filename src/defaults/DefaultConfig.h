#ifndef DEFAULTCONFIG_H
#define DEFAULTCONFIG_H

#include "tinyxml2.h"
#include <string>
#include <list>
#include <iostream>
#include "Log.h"
#include "Paths.h"

namespace BackyardBrains {
    class DefaultConfig
    {
        public:
            DefaultConfig();
            std::string errorString;
            int configVersion;
            int firstChannelButton;
            int secondChannelButton;
            int thirdChannelButton;

            int loadDefaults();
            int saveDefaults();
        protected:
        private:

    };
}
#endif // DEFAULTCONFIG_H
