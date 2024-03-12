
/*
    This class BYBFirmwareVO is used as value object that is used with FirmwareUpdater class
    This is used to update MSP430 devices like old HID Muscle and Neuron SpikerBox Pro.
    Used only on Windows since actual communiation with boothloader uses DLL for Windows only

*/
#include "BYBFirmwareVO.h"

namespace BackyardBrains {

    BYBFirmwareVO::BYBFirmwareVO()
    {
        id = 0;
        location = REMOTE_FIRMWARE;
    }
}
