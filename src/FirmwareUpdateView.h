#ifndef FIRMWAREUPDATEVIEW_H
#define FIRMWAREUPDATEVIEW_H

#include <stdio.h>
#include "widgets/Widget.h"
#include "widgets/PushButton.h"
#include "widgets/Label.h"
#include <time.h>

#define TYPE_MSP430_UPDATER 0
#define TYPE_STM32_BOOTLOADER 1

namespace BackyardBrains {
    class RecordingManager;
    class AudioView;
    class FirmwareUpdateView : public Widgets::Widget
    {
        public:
            FirmwareUpdateView(RecordingManager &mngr, AudioView &audioView, int typeOfUpdater, Widget *parent = NULL);
        protected:
        private:
        int typeOfFirmwareUpdater = TYPE_MSP430_UPDATER;
        Widgets::PushButton *closeButton;
        RecordingManager &_manager;
        AudioView &_audioView;
        Widgets::Label *infoLabel;
        void paintEvent();
        void closePressed();
        clock_t timerUSB;
        void testPowerOfDevice();
        bool firmwareUpdateStarted = false;
    };
}
#endif // FIRMWAREUPDATEVIEW_H
