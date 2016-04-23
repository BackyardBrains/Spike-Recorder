#include "CalibrationWindow.h"

#include "widgets/TextureGL.h"
#include "widgets/Painter.h"

#include "engine/RecordingManager.h"
#include "AudioView.h"

#define WIDTH 200
#define HEIGHT 200

#define X_PROGRESS 40
#define Y_PROGRESS 100
#define WIDTH_PROGRESS 300
namespace BackyardBrains {
    CalibrationWindow::CalibrationWindow(RecordingManager &mngr, AudioView &audioView, Widget *parent) : Widget(parent), _manager(mngr), _audioView(audioView) {


        inCalibration = false;
        startOfCalibration = 0;

        setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Fixed));

        setSizeHint(Widgets::Size(WIDTH, HEIGHT));




        Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);

        hbuttonbox = new Widgets::BoxLayout(Widgets::Horizontal);

        Widgets::Label *topLabel = new Widgets::Label(this);
        topLabel->setText("Calibration");
        topLabel->updateSize();

        explanationLabel1 = new Widgets::Label(this);
        explanationLabel1->setText("Connect your device to calibrator and click");
        explanationLabel1->updateSize();

        explanationLabel2 = new Widgets::Label(this);
        explanationLabel2->setText("on Calibrate button when you are ready.");
        explanationLabel2->updateSize();


        cancelButton = new Widgets::PushButton(this);
        cancelButton->clicked.connect(this, &CalibrationWindow::closePressed);
        cancelButton->setNormalTex(Widgets::TextureGL::get("data/cancel-normal.bmp"));
        cancelButton->setHoverTex(Widgets::TextureGL::get("data/cancel-high.bmp"));
        cancelButton->setSize(Widgets::Size(80,26));

        okButton = new Widgets::PushButton(this);
        okButton->clicked.connect(this, &CalibrationWindow::closePressed);
        okButton->setNormalTex(Widgets::TextureGL::get("data/okbtn-normal.bmp"));
        okButton->setHoverTex(Widgets::TextureGL::get("data/okbtn-high.bmp"));
        okButton->setSize(Widgets::Size(80,26));
        okButton->setVisible(false);

        calibrateButton = new Widgets::PushButton(this);
        calibrateButton->clicked.connect(this, &CalibrationWindow::calibratePressed);
        calibrateButton->setNormalTex(Widgets::TextureGL::get("data/calibratebtn-normal.bmp"));
        calibrateButton->setHoverTex(Widgets::TextureGL::get("data/calibratebtn-high.bmp"));
        calibrateButton->setSize(Widgets::Size(80,26));


        hbuttonbox->addSpacing(70);
        hbuttonbox->addWidget(cancelButton);
        hbuttonbox->addWidget(okButton);
        hbuttonbox->addWidget(calibrateButton);

        hbuttonbox->update();

        vbox->addSpacing(10);
        vbox->addWidget(topLabel, Widgets::AlignHCenter);
        vbox->addSpacing(10);
        vbox->addWidget(explanationLabel1, Widgets::AlignHCenter);
        vbox->addWidget(explanationLabel2, Widgets::AlignHCenter);
        vbox->addSpacing(20);
        vbox->addLayout(hbuttonbox);

        vbox->update();

    }

    void CalibrationWindow::paintEvent()
    {
        	Widgets::Color bg = Widgets::Colors::widgetbg;
            bg.a = 250;
            Widgets::Painter::setColor(bg);
            Widgets::Painter::drawRect(rect());


            if(inCalibration)
            {

                if((SDL_GetTicks()-startOfCalibration) >= 2000)
                {
                    //end of calibration
                    inCalibration = false;
                    explanationLabel1->setText("Calibration finished");
                    explanationLabel1->updateSize();
                    okButton->setVisible(true);

                    //calculate for two seconds of signal and 1mV peak to peak
                    float howManyOurUnitsIsOnemV = _audioView.calculateCalibrationCoeficient(2.0f,1.0);
                    float calibrationCoef = 1.0f/howManyOurUnitsIsOnemV;
                    _manager.setCalibrationCoeficient(calibrationCoef);
                }
                else
                {
                    //calibration in progress

                    Widgets::Color progressColor = Widgets::Colors::selectedstate;

                    Widgets::Painter::setColor(progressColor);
                    //(240, 20, 380, 160)
                    Widgets::Painter::drawRect(Widgets::Rect(X_PROGRESS, Y_PROGRESS, WIDTH_PROGRESS, 20));

                    Widgets::Color fillColor = Widgets::Colors::widgetbg;
                    Widgets::Painter::setColor(fillColor);
                    //(240, 20, 380, 160)
                    Widgets::Painter::drawRect(Widgets::Rect(X_PROGRESS+1, Y_PROGRESS+1, WIDTH_PROGRESS-2, 18));


                    Widgets::Painter::setColor(progressColor);
                    //(240, 20, 380, 160)
                    Widgets::Painter::drawRect(Widgets::Rect(X_PROGRESS+1, Y_PROGRESS+1, (WIDTH_PROGRESS-2)*((float)(SDL_GetTicks()-startOfCalibration))/2000.0, 18));
                }
            }


    }

    void CalibrationWindow::closePressed()
    {
        close();
    }

    void CalibrationWindow::calibratePressed()
    {
        _manager.resetCalibrationCoeficient();
        inCalibration = true;
        startOfCalibration = SDL_GetTicks();
        calibrateButton->setVisible(false);
        cancelButton->setVisible(false);
        explanationLabel1->setText("Calibration in progress...");
        explanationLabel1->updateSize();
        explanationLabel2->setVisible(false);
    }



}//namespace
