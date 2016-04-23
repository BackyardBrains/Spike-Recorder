#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H
#include <stdio.h>
#include "widgets/Widget.h"
#include "widgets/PushButton.h"
#include "widgets/Label.h"
#include "widgets/BoxLayout.h"


namespace BackyardBrains {
    class RecordingManager;
    class AudioView;


    class CalibrationWindow : public Widgets::Widget
    {
        public:
            CalibrationWindow(RecordingManager &mngr, AudioView &audioView, Widget *parent = NULL);
        protected:
        private:
        bool inCalibration;
        uint64_t startOfCalibration;

        Widgets::PushButton *calibrateButton;
        Widgets::PushButton *cancelButton;
        Widgets::PushButton *okButton;
        Widgets::Label *explanationLabel1;
        Widgets::Label *explanationLabel2;
        Widgets::BoxLayout *hbuttonbox;
        RecordingManager &_manager;
        AudioView &_audioView;
        Widgets::Label *infoLabel;
        void paintEvent();
        void closePressed();
        void calibratePressed();
    };
}
#endif // CALIBRATIONWINDOW_H
