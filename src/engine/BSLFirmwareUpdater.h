#ifndef BSLFIRMWAREUPDATER_H
#define BSLFIRMWAREUPDATER_H


//

#ifdef _WIN32
	#include <windows.h>
	#include "mingw.thread.h"
	#include "BSL430_dll.h"
    #include "CRC.h"
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

    protected:


    private:
}; //class end

}//namespace
#endif // BSLFIRMWAREUPDATER_H
