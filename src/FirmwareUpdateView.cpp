#include "FirmwareUpdateView.h"
#include "ConfigView.h"
#include "engine/RecordingManager.h"
#include "engine/BASSErrors.h"
#include "widgets/Painter.h"
#include "widgets/Color.h"
#include "widgets/BoxLayout.h"
#include "widgets/PushButton.h"
#include "widgets/TextureGL.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/ErrorBox.h"
#include "widgets/ErrorBox.h"
#include "DropDownList.h"
#include "AudioView.h"
#include "ColorDropDownList.h"
#include "Log.h"
#include <bass.h>
#include <sstream>

namespace BackyardBrains {

    FirmwareUpdateView::FirmwareUpdateView(RecordingManager &mngr, AudioView &audioView,int typeOfUpdater, Widget *parent) : Widget(parent), _manager(mngr), _audioView(audioView) {

        typeOfFirmwareUpdater = typeOfUpdater;
        
        
        //create close button
        timerUSB = clock();

        //create title
        Widgets::Label *topLabel = new Widgets::Label(this);
        topLabel->setText("Firmware update");
        topLabel->updateSize();

        //create content group
        Widgets::Widget *group = new Widgets::Widget(this);
        group->setSizeHint(Widgets::Size(500,400));

        //create main vertical layout for content group
        Widgets::BoxLayout *gvbox = new Widgets::BoxLayout(Widgets::Vertical, group);

        //create child content horizontal box
        Widgets::BoxLayout *repeatbox = new Widgets::BoxLayout(Widgets::Vertical);
        //create info label
        infoLabel = new Widgets::Label(group);
        infoLabel->setText("Firmware updating. Do not unplug device during updating.");
        infoLabel->updateSize();

         closeButton = new Widgets::PushButton(group);
        closeButton->clicked.connect(this, &FirmwareUpdateView::closePressed);
        closeButton->setNormalTex(Widgets::TextureGL::get("data/okbtn.bmp"));
        closeButton->setHoverTex(Widgets::TextureGL::get("data/okbtnhigh.bmp"));

        //add label to child horizontal layout
        repeatbox->addSpacing(180);
        repeatbox->addWidget(infoLabel,Widgets::AlignHCenter);
        repeatbox->addSpacing(150);
        repeatbox->addWidget(closeButton,Widgets::AlignHCenter);
        repeatbox->update();
        //add component to main vertical group in content box

        gvbox->addLayout(repeatbox);//add label and button
        gvbox->update();


        //make master vertical layout
        Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
        //create horizontal layout for title
        Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
        hbox->addSpacing(10);
        hbox->addWidget(topLabel, Widgets::AlignVCenter);
        hbox->addSpacing(30);
       // hbox->addWidget(closeButton, Widgets::AlignVCenter);
        hbox->update();


        //add title and content to master layout
        vbox->addSpacing(10);
        vbox->addLayout(hbox);
        vbox->addSpacing(20);
        vbox->addWidget(group, Widgets::AlignCenter);

        vbox->update();
        closeButton->setVisible(false);
    }

    //Update progress
    void FirmwareUpdateView::paintEvent() {

        Widgets::Color bg = Widgets::Colors::background;
        bg.a = 250;
        Widgets::Painter::setColor(bg);
        Widgets::Painter::drawRect(rect());


        if(typeOfFirmwareUpdater==TYPE_MSP430_UPDATER)
        {
            #if defined(_WIN32)
                if(!firmwareUpdateStarted)
                {
                    //SB Pro is turned OFF we can start firmware update
                    //start actual firmware update
                    
                    _manager.startActualFirmwareUpdateOnDevice();
                    
                    firmwareUpdateStarted = true;
                }
                 infoLabel->setText("Firmware updating. Do not unplug device during updating.");
                
                    int stage = _manager.getUSBFirmwareUpdateStage();
              
                int yoffset = 300;
                int widthofBar = 500;
                Widgets::Painter::setColor(Widgets::Color(200,200,200,255));
                Widgets::Painter::drawRect(Widgets::Rect(this->width()/2-(widthofBar/2)-5, yoffset-5, widthofBar+10, 50));
                if(stage>=0)
                {
                    Widgets::Painter::setColor(Widgets::Color(0,250,0,255));
                    Widgets::Painter::drawRect(Widgets::Rect(this->width()/2-(widthofBar/2), yoffset, (int)(((float)widthofBar)*(((float)stage)/20.0f)), 40));
                    
                }
                else
                {
                    Widgets::Painter::setColor(Widgets::Color(255,0,0,255));
                    Widgets::Painter::drawRect(Widgets::Rect(this->width()/2-(widthofBar/2), yoffset, widthofBar, 40));
                    closeButton->setVisible(true);
                    infoLabel->setText("Error while updating firmware. \n Unplug device, plug in again and try again.");
                }
                if(stage == 20)
                {
                    closeButton->setVisible(true);
                     infoLabel->setText("Firmware updated successfully!");
                }
            #endif
        }
        else if (typeOfFirmwareUpdater==TYPE_STM32_BOOTLOADER)
        {
            //_manager.progressOfBootloader
            int yoffset = 300;
            int widthofBar = 500;
            Widgets::Painter::setColor(Widgets::Color(200,200,200,255));
            Widgets::Painter::drawRect(Widgets::Rect(this->width()/2-(widthofBar/2)-5, yoffset-5, widthofBar+10, 50));
            
            
            if(_manager.progressOfBootloader()>=0)
            {
                infoLabel->setText("Firmware updating. Do not unplug device during updating.");
                Widgets::Painter::setColor(Widgets::Color(0,250,0,255));
                Widgets::Painter::drawRect(Widgets::Rect(this->width()/2-(widthofBar/2), yoffset, (int)(((float)widthofBar)*(((float)_manager.progressOfBootloader())/100.0f)), 40));
                closeButton->setVisible(false);
            }
            else
            {
                Widgets::Painter::setColor(Widgets::Color(255,0,0,255));
                Widgets::Painter::drawRect(Widgets::Rect(this->width()/2-(widthofBar/2), yoffset, widthofBar, 40));
                closeButton->setVisible(true);
                infoLabel->setText("Error while updating firmware. \n Unplug device, plug in again and try again.");
            }
            if(_manager.progressOfBootloader() == 100)
            {
                closeButton->setVisible(true);
                 infoLabel->setText("Firmware updated successfully!");
            }
            
        }
    }

    //Close view
    void FirmwareUpdateView::closePressed() {
        if(typeOfFirmwareUpdater==TYPE_MSP430_UPDATER)
        {
            #if defined(_WIN32)
                _manager.finishAndCleanFirmwareUpdate();
            #endif
        }
        close();
    }

    void FirmwareUpdateView::testPowerOfDevice()
    {
        clock_t end = clock();
        double elapsed_secs = double(end - timerUSB) / CLOCKS_PER_SEC;
        if(elapsed_secs>0.5)
        {
            timerUSB = end;
            if(typeOfFirmwareUpdater==TYPE_MSP430_UPDATER)
            {
                #if defined(_WIN32)
                    _manager.askForPowerStateHIDDevice();//ask HID device if power is OFF
                #endif
            }
        }


    }

}
