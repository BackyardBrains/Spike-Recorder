#ifndef FIRMWAREUPDATEVIEW_H
#define FIRMWAREUPDATEVIEW_H

#include <stdio.h>
#include "widgets/Widget.h"
#include "widgets/PushButton.h"
#include "widgets/Label.h"

namespace BackyardBrains {
    class RecordingManager;
    class AudioView;
    class FirmwareUpdateView : public Widgets::Widget
    {
        public:
            FirmwareUpdateView(RecordingManager &mngr, AudioView &audioView, Widget *parent = NULL);
        protected:
        private:
        Widgets::PushButton *closeButton;
        RecordingManager &_manager;
        AudioView &_audioView;
        Widgets::Label *infoLabel;
        void paintEvent();
        void closePressed();
    };
}
#endif // FIRMWAREUPDATEVIEW_H
