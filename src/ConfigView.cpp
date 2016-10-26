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
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"

#include "DropDownList.h"
#include "AudioView.h"
#include "ColorDropDownList.h"
#include "Log.h"
#include <bass.h>
#include <sstream>
#include "CalibrationWindow.h"


namespace BackyardBrains {

ConfigView::ConfigView(RecordingManager &mngr, AudioView &audioView, Widget *parent) : Widget(parent), _manager(mngr), _audioView(audioView) {
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &ConfigView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/configcrossed.bmp"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/configcrossed.bmp"));

	Widgets::Label *topLabel = new Widgets::Label(this);
	topLabel->setText("Config");
	topLabel->updateSize();

	std::vector<Widgets::Color> c(AudioView::COLORS, AudioView::COLORS+AudioView::COLOR_NUM);
	_clrs.resize(_manager.virtualDevices().size());
	_catchers.reserve(_clrs.size());

	Widgets::Widget *group = new Widgets::Widget(this);
	group->setSizeHint(Widgets::Size(500,600));
	Widgets::BoxLayout *gvbox = new Widgets::BoxLayout(Widgets::Vertical, group);



    //-------------- Mute check box ----------------------------------------

	if(!_manager.fileMode()) {
		Widgets::BoxLayout *mutehbox = new Widgets::BoxLayout(Widgets::Horizontal);
		Widgets::Label *muteLabel = new Widgets::Label(group);
		muteLabel->setText("Mute Speakers:");
		muteLabel->updateSize();

		_muteCKBox = new Widgets::PushButton(group);
		if(_manager.player().volume() == 0)
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
		else
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
		_muteCKBox->setSizeHint(Widgets::Size(19,19));
		_muteCKBox->clicked.connect(this, &ConfigView::mutePressed);

		mutehbox->addWidget(muteLabel);
		mutehbox->addSpacing(10);
		mutehbox->addWidget(_muteCKBox, Widgets::AlignVCenter);
		mutehbox->addSpacing(50);
		gvbox->addLayout(mutehbox);
		gvbox->addSpacing(30);
	}

    //---------- Calibrator code --------------------------------------
/*	if(!_manager.fileMode()) {
        Widgets::Label *calibrateMainLabel = new Widgets::Label(group);
		calibrateMainLabel->setText("Calibrate SpikeRecorder for current setup ");
		calibrateMainLabel->updateSize();

        Widgets::PushButton *calibrateButton = new Widgets::PushButton(group);
        calibrateButton->clicked.connect(this, &ConfigView::calibratePressed);
        calibrateButton->setNormalTex(Widgets::TextureGL::get("data/calibratebtn-normal.bmp"));
        calibrateButton->setHoverTex(Widgets::TextureGL::get("data/calibratebtn-high.bmp"));
        calibrateButton->setSize(Widgets::Size(80,26));

        Widgets::BoxLayout *hcalBox = new Widgets::BoxLayout(Widgets::Horizontal);
        hcalBox->addWidget(calibrateMainLabel);
        hcalBox->addSpacing(10);
        hcalBox->addWidget(calibrateButton);

        gvbox->addLayout(hcalBox);
        gvbox->addSpacing(40);
	}*/

    //---------- High/Low pass filter --------------------------------------
    if(!_manager.fileMode()) {

        Widgets::Label *filterMainLabel = new Widgets::Label(group);
		filterMainLabel->setText("Set band-pass filter cutoff frequencies");
		filterMainLabel->updateSize();


        lowValueTI = new Widgets::TextInput(group, 50);
        lowValueTI->textEditingEnded.connect(this, &ConfigView::lowFilterTIValueChanged);
        highValueTI = new Widgets::TextInput(group, 50);
        highValueTI->textEditingEnded.connect(this, &ConfigView::highFilterTIValueChanged);


        rangeSelector = new Widgets::RangeSelector(group);
        rangeSelector->setRange(0,_manager.sampleRate()/2);

        highValueTI->setInt(_manager.lowCornerFrequency());
        lowValueTI->setInt(_manager.highCornerFrequency());

        rangeSelector->lowValueChanged.connect(this, &ConfigView::lowFilterValueChanged);
        rangeSelector->highValueChanged.connect(this, &ConfigView::highFilterValueChanged);
        rangeSelector->updateHighLogValue(_manager.lowCornerFrequency());
        rangeSelector->updateLowLogValue(_manager.highCornerFrequency());


        Widgets::BoxLayout *labelsBox = new Widgets::BoxLayout(Widgets::Horizontal);

        Widgets::Label *lowFilterLabel = new Widgets::Label(group);
		lowFilterLabel->setText("Low");
		lowFilterLabel->updateSize();

        Widgets::Label *highFilterLabel = new Widgets::Label(group);
		highFilterLabel->setText("High");
		highFilterLabel->updateSize();

        labelsBox->addSpacing(8);
        labelsBox->addWidget(lowFilterLabel, Widgets::AlignBottom);
		labelsBox->addSpacing(415);
		labelsBox->addWidget(highFilterLabel, Widgets::AlignBottom);

        gvbox->addLayout(labelsBox);

        Widgets::BoxLayout *valuesBox = new Widgets::BoxLayout(Widgets::Horizontal);

		valuesBox->addWidget(lowValueTI);
		valuesBox->addSpacing(40);
		valuesBox->addWidget(filterMainLabel);
		valuesBox->addSpacing(40);
		valuesBox->addWidget(highValueTI);
		gvbox->addLayout(valuesBox);
        gvbox->addWidget(rangeSelector);
		gvbox->addSpacing(20);
    }




    //----------- 50Hz/60Hz noise filter ------------------------------------


    if(!_manager.fileMode()) {
		Widgets::BoxLayout *filterhbox = new Widgets::BoxLayout(Widgets::Horizontal);

		Widgets::Label *filterLabel = new Widgets::Label(group);
		filterLabel->setText("Enable Notch filter:");
		filterLabel->updateSize();

		Widgets::Label *fiftyHzLabel = new Widgets::Label(group);
		fiftyHzLabel->setText("50Hz");
		fiftyHzLabel->updateSize();

        Widgets::Label *sixtyHzLabel = new Widgets::Label(group);
		sixtyHzLabel->setText("60Hz");
		sixtyHzLabel->updateSize();

		_50hzFilter = new Widgets::PushButton(group);
		if(_manager.fiftyHzFilterEnabled())
        {
            std::cout<<"\n\n-----50hz enabled init";
			_50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        }
		else
        {
            std::cout<<"\n\n-------50hz disabled init";
			_50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        }
		_50hzFilter->setSizeHint(Widgets::Size(19,19));
		_50hzFilter->clicked.connect(this, &ConfigView::fiftyHzPressed);

		_60hzFilter = new Widgets::PushButton(group);
		if(_manager.sixtyHzFilterEnabled())
        {
            std::cout<<"\n\n-----6s0hz enabled init";
			_60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        }
		else
        {
             std::cout<<"\n\n-------60hz disabled init";
			_60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        }
		_60hzFilter->setSizeHint(Widgets::Size(19,19));
		_60hzFilter->clicked.connect(this, &ConfigView::sixtyHzPressed);

		filterhbox->addWidget(filterLabel);
		filterhbox->addSpacing(13);
		filterhbox->addWidget(fiftyHzLabel);
		filterhbox->addSpacing(4);
		filterhbox->addWidget(_50hzFilter, Widgets::AlignVCenter);
		filterhbox->addSpacing(16);
		filterhbox->addWidget(sixtyHzLabel);
		filterhbox->addSpacing(4);
		filterhbox->addWidget(_60hzFilter, Widgets::AlignVCenter);
		filterhbox->addSpacing(50);
		gvbox->addLayout(filterhbox);
		gvbox->addSpacing(40);
	}


    //----------- Color chooser for channels --------------------------------

	for(unsigned int i = 0; i < _manager.virtualDevices().size(); i++) {
		_clrs[i] = new ColorDropDownList(group);
		_clrs[i]->setContent(c);

		_catchers.push_back(SignalCatcher(i, this));
		_clrs[i]->selectionChanged.connect(&_catchers[i], &SignalCatcher::catchColor);
		Widgets::Label *name = new Widgets::Label(group);
		name->setText(_manager.virtualDevices()[i].name.c_str());
		name->updateSize();

		Widgets::BoxLayout *ghbox = new Widgets::BoxLayout(Widgets::Horizontal);
		ghbox->addWidget(_clrs[i]);
		ghbox->addSpacing(20);
		ghbox->addWidget(name,Widgets::AlignVCenter);
		gvbox->addLayout(ghbox);
		gvbox->addSpacing(15);
	}

	for(int i = 0; i < audioView.channelCount(); i++)
		_clrs[audioView.channelVirtualDevice(i)]->setSelection(audioView.channelColor(i));



    // -------- Serial configuration -----------------------------------------

    if(!_manager.fileMode())
    {
        //Serial  config widgets
        Widgets::Label *name2 = new Widgets::Label(group);
        name2->setText("Select port:");
        name2->updateSize();
        gvbox->addSpacing(0);
        gvbox->addWidget(name2, Widgets::AlignLeft);



        //Dropdown for select port
        Widgets::BoxLayout *serialHbox = new Widgets::BoxLayout(Widgets::Horizontal);
        serialPortWidget = new DropDownList(group);
        serialPortWidget->clear();
        std::list<std::string> sps =  _manager.serailPortsList();
        std::list<std::string>::iterator it;
        for(it = sps.begin();it!=sps.end();it++)
        {
            serialPortWidget->addItem(it->c_str());
        }
        serialPortWidget->setSelection(_manager.serialPortIndex());
        _catchers.push_back(SignalCatcher(_catchers.size(), this));
        serialPortWidget->indexChanged.connect(&_catchers[_catchers.size()-1], &SignalCatcher::catchPort);
        serialPortWidget->setDisabled(_manager.serialMode());

        serialHbox->addWidget(serialPortWidget);
        serialHbox->addSpacing(5);




        //Button for connect to serial
        _connectButton = new Widgets::PushButton(group);
        _connectButton->clicked.connect(this, &ConfigView::connectPressed);
        if(_manager.serialMode())
        {
            _connectButton->setNormalTex(Widgets::TextureGL::get("data/connected.bmp"));
            _connectButton->setHoverTex(Widgets::TextureGL::get("data/connected.bmp"));
        }
        else
        {
            _connectButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
            _connectButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
        }
        _connectButton->setSizeHint(Widgets::Size(26,26));
        serialHbox->addWidget(_connectButton);
        serialHbox->update();
        gvbox->addSpacing(3);
        gvbox->addLayout(serialHbox);



        //-------------- Serial Number of channels chooser ----------------------------------------

        if(_manager.serialMode())
        {
                //Number of channels chooser
                Widgets::BoxLayout *numberOfChannelsHbox = new Widgets::BoxLayout(Widgets::Horizontal);

                Widgets::Label *numChannelsLabel = new Widgets::Label(group);
                numChannelsLabel->setText("Number of channels:");
                numChannelsLabel->updateSize();
                numberOfChannelsHbox->addWidget(numChannelsLabel);
                numberOfChannelsHbox->addSpacing(5);



                numberOfChannelsWidget = new DropDownList(group, 50,30);
                numberOfChannelsWidget->clear();

                numberOfChannelsWidget->addItem("1");
                numberOfChannelsWidget->addItem("2");
                numberOfChannelsWidget->addItem("3");
                numberOfChannelsWidget->addItem("4");
                numberOfChannelsWidget->addItem("5");
                numberOfChannelsWidget->addItem("6");

                numberOfChannelsWidget->setSelection(_manager.numberOfSerialChannels()-1);
                _catchers.push_back(SignalCatcher(_catchers.size(), this));
                numberOfChannelsWidget->indexChanged.connect(&_catchers[_catchers.size()-1], &SignalCatcher::setNumOfChannelsHandler);
                numberOfChannelsWidget->setDisabled(!_manager.serialMode());

                numberOfChannelsHbox->addWidget(numberOfChannelsWidget);

                numberOfChannelsHbox->update();

                gvbox->addSpacing(10);
                gvbox->addLayout(numberOfChannelsHbox);
        }


        // -------------------- HID device connect (works only under windows)---------------------------------------


        #if defined(_WIN32)
            /* Widgets::BoxLayout *hidHbox = new Widgets::BoxLayout(Widgets::Horizontal);
            //USB  label
            Widgets::Label *nameUSB = new Widgets::Label(group);
            nameUSB->setText("Connect to USB device ");
            nameUSB->updateSize();
            hidHbox->addSpacing(0);
            hidHbox->addWidget(nameUSB, Widgets::AlignLeft);
            hidHbox->addSpacing(5);

            //Button for connect to HID device
            _hidButton = new Widgets::PushButton(group);

            _hidButton->clicked.connect(this, &ConfigView::hidConnectPressed);
            if(_manager.hidMode())
            {
                _hidButton->setNormalTex(Widgets::TextureGL::get("data/connected.bmp"));
                _hidButton->setHoverTex(Widgets::TextureGL::get("data/connected.bmp"));
            }
            else
            {
                _hidButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                _hidButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
            }
            _hidButton->setSizeHint(Widgets::Size(26,26));

            hidHbox->addWidget(_hidButton);
            hidHbox->update();
            gvbox->addSpacing(20);
            gvbox->addLayout(hidHbox);*/


        //--------------   Update firmware code (works only under Windows)----------------------



        if(_manager.hidMode())
        {
            if(_manager.firmwareAvailable())
            {
                     Widgets::BoxLayout *updateHbox = new Widgets::BoxLayout(Widgets::Horizontal);
                    //Add label
                    Widgets::Label *updateLabel = new Widgets::Label(group);

                    std::stringstream labelTextString;
                    labelTextString <<"Change firmware (current version V"<< _manager.currentHIDFirmwareVersion()<<"):";

                    updateLabel->setText(labelTextString.str().c_str());
                    updateLabel->updateSize();
                    updateHbox->addSpacing(0);
                    updateHbox->addWidget(updateLabel, Widgets::AlignLeft);
                    updateHbox->addSpacing(5);

                    //Add dropbox
                    firmwaresWidget = new DropDownList(group,400,30);
                    firmwaresWidget->clear();
                    std::list<BYBFirmwareVO> fps =  _manager.firmwareList();

                    for( listBYBFirmwareVO::iterator ti = fps.begin();
                        ti != fps.end();
                        ti ++)
                    {
                        std::stringstream sstm;
                         sstm <<((BYBFirmwareVO)(*ti)).description << " (V" << ((BYBFirmwareVO)(*ti)).version <<")";

                        firmwaresWidget->addItem(sstm.str().c_str() );
                    }


                    firmwaresWidget->setSelection(0);

                    selectedFirmware = new BYBFirmwareVO();
                    selectedFirmware->URL = std::string((_manager.firmwareList().front()).URL);
                    selectedFirmware->filepath = std::string((_manager.firmwareList().front()).filepath);
                    selectedFirmware->id = (_manager.firmwareList().front()).id;

                    firmwaresWidget->setDisabled(false);
                    _catchers.push_back(SignalCatcher(_catchers.size(), this));
                    firmwaresWidget->indexChanged.connect(&_catchers[_catchers.size()-1], &SignalCatcher::catchFirmwareSelection);
                     Widgets::BoxLayout *updateSelectionHbox = new Widgets::BoxLayout(Widgets::Horizontal);
                    updateSelectionHbox->addWidget(firmwaresWidget);



                    //Button for update HID device
                    _updateButton = new Widgets::PushButton(group);
                    _updateButton->clicked.connect(this, &ConfigView::firmwareUpdatePressed);

                    _updateButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                    _updateButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                    _updateButton->setSizeHint(Widgets::Size(26,26));
                    updateSelectionHbox->addSpacing(5);
                    updateSelectionHbox->addWidget(_updateButton);


                    updateSelectionHbox->update();
                    gvbox->addSpacing(20);
                    gvbox->addLayout(updateHbox);
                    gvbox->addLayout(updateSelectionHbox);
                    //gvbox->addWidget(firmwaresWidget);
            }
        }
        #endif

    }//end if not file mode

	gvbox->update();

	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
	hbox->addSpacing(10);
	hbox->addWidget(closeButton);
	hbox->addSpacing(17);
	hbox->addWidget(topLabel, Widgets::AlignVCenter);
	vbox->addSpacing(10);
	vbox->addLayout(hbox);
	vbox->addSpacing(20);
	vbox->addWidget(group, Widgets::AlignCenter);

	vbox->update();

}

void ConfigView::paintEvent() {
	Widgets::Color bg = Widgets::Colors::background;
	bg.a = 250;
	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect());

}


#if defined(_WIN32)

        void ConfigView::hidConnectPressed()
        {
            //connect/diconnect

            if(_manager.hidMode())
            {
                // _manager.setSerialNumberOfChannels(1);
                _manager.disconnectFromHID();

                _hidButton->setNormalTex(Widgets::TextureGL::get("data/connected.bmp"));
                _hidButton->setHoverTex(Widgets::TextureGL::get("data/connected.bmp"));
                close();
            }
            else
            {
                if(!_manager.initHIDUSB())
                {
                    std::cout<<"Can't open HID device. \n";


                    Widgets::ErrorBox *box = new Widgets::ErrorBox(_manager.hidError.c_str());
                    box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
                    Widgets::Application::getInstance()->addPopup(box);
                }
                _hidButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                _hidButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                close();
            }
        }

        //
        //Firmware selection drop-down selection changed
        //
        void ConfigView::firmwareSelectionChanged(int firmwareid)
        {
             std::list<BYBFirmwareVO> fps =  _manager.firmwareList();

            int i = 0;
            for( listBYBFirmwareVO::iterator ti = fps.begin();
                ti != fps.end();
                ti ++)
            {
                if(i==firmwareid)
                {
                    //selectedFirmware = (BYBFirmwareVO*)&(*ti);
                    selectedFirmware = new BYBFirmwareVO();
                    selectedFirmware->URL = std::string((*ti).URL);
                    selectedFirmware->filepath = std::string((*ti).filepath);
                    selectedFirmware->id = (*ti).id;
                    break;
                }
                i++;
            }
        }


        //
        // Start firmware update procedure
        //
        void ConfigView::firmwareUpdatePressed()
        {
            if(_manager.getUSBFirmwareUpdateStage() == 0)//avoid starting it twice
            {
                //send message to MSP to prepare for update
                std::cout<<"\n\n\n"<<"Firmware URL: "<<selectedFirmware->URL<<"\n\n\n";
                if(_manager.prepareForHIDFirmwareUpdate(selectedFirmware))
                {
                    std::cout<<"Error downloading firmware";

                    const char *error = "Error while downloading firmware.\n  Check your Internet connection.";
                    Widgets::ErrorBox *box = new Widgets::ErrorBox(error);
                    box->setGeometry(Widgets::Rect(this->width()/2-150, this->height()/2-40, 300, 80));
                    Widgets::Application::getInstance()->addPopup(box);

                }
                close();
            }
        }
#endif

//
// Connect/dsconnect from serial port
//
void ConfigView::connectPressed()
{
    if(_manager.serialMode())
    {
        _manager.setSerialNumberOfChannels(1);
        _manager.disconnectFromSerial();

    }
    else
    {
        if(!_manager.initSerial(serialPortWidget->item(serialPortWidget->selection()).c_str()))
        {
	    Log::error("Can't init serial port.");
	    const char *error = _manager.serialError.c_str();
	    if(strlen(error) == 0) {
		    error = "Error: Cannot init serial port.";
	    }

            Widgets::ErrorBox *box = new Widgets::ErrorBox(error);
            box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
            Widgets::Application::getInstance()->addPopup(box);
        }
    }
    if(_manager.serialMode())
    {
        _connectButton->setNormalTex(Widgets::TextureGL::get("data/connected.bmp"));
        _connectButton->setHoverTex(Widgets::TextureGL::get("data/connected.bmp"));
        close();
    }
    else
    {
        _connectButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
        _connectButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
        close();

    }
    serialPortWidget->setDisabled(_manager.serialMode());
}


void ConfigView::closePressed() {
	close();
}

void ConfigView::mutePressed() {
	if(_manager.player().volume() == 0) {
		_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
		_manager.player().setVolume(100);
	} else {
		_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
		_manager.player().setVolume(0);
	}
}

//
// Band pass filtering value changed
//
void ConfigView::highFilterValueChanged(int hvalue)
{
    highValueTI->setInt(hvalue);
    if(hvalue>=_manager.sampleRate()/2)
    {

        _manager.enableLowPassFilterWithCornerFreq(_manager.sampleRate()/2);
        _manager.disableLowPassFilter();
    }
    else
    {
        _manager.enableLowPassFilterWithCornerFreq(hvalue);
    }
}


//
// Band pass filtering value changed
//
void ConfigView::lowFilterValueChanged(int lvalue)
{
    lowValueTI->setInt(lvalue);
     if(lvalue<=0)
    {

        _manager.enableHighPassFilterWithCornerFreq(0.0);
         _manager.disableHighPassFilter();
    }
    else
    {
        _manager.enableHighPassFilterWithCornerFreq(lvalue);

    }
}


void ConfigView::lowFilterTIValueChanged(std::string newString)
{
    rangeSelector->updateLowLogValue(lowValueTI->getInt());
    lowValueTI->setInt(rangeSelector->getLowValue());
}

void ConfigView::highFilterTIValueChanged(std::string newString)
{
    rangeSelector->updateHighLogValue(highValueTI->getInt());
    highValueTI->setInt(rangeSelector->getHighValue());
}
//
// Enable/disable 50Hz filter checkbox pressed
//
void ConfigView::fiftyHzPressed() {
    if(_manager.fiftyHzFilterEnabled())
    {
        _manager.disable50HzFilter();
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
    else
    {
        _manager.enable50HzFilter();
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
}


//
// Enable/disable 60Hz filter checkbox pressed
//
void ConfigView::sixtyHzPressed() {
    if(_manager.sixtyHzFilterEnabled())
    {
        _manager.disable60HzFilter();
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
    else
    {
        _manager.enable60HzFilter();
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
}


void ConfigView::serialPortChanged(int virtualDevice, int portidx)
{
    _manager.changeSerialPort(portidx);
}

void ConfigView::setSerialNumberOfChannels(int numberOfChannels)
{
    _manager.setSerialNumberOfChannels(numberOfChannels);
    close();
}

void ConfigView::colorChanged(int virtualDevice, int coloridx) {
	int channel = _audioView.virtualDeviceChannel(virtualDevice);
	if(channel < 0 && coloridx != 0) {
		bool success = _manager.bindVirtualDevice(virtualDevice);
		if(success) {
			_audioView.setChannelColor(_audioView.channelCount()-1, coloridx);
		} else {
			_clrs[virtualDevice]->setSelectionSilent(0);

			std::stringstream s;
			s << "Error: Cannot open channel";
			if(BASS_ErrorGetCode())
				s << ": " << GetBassStrError();
			Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
			box->setGeometry(Widgets::Rect(this->width()/2-200, this->height()/2-40, 400, 80));
			Widgets::Application::getInstance()->addPopup(box);
		}
	} else if(coloridx == 0) {
		_manager.unbindVirtualDevice(virtualDevice);
	} else {
		_audioView.setChannelColor(channel, coloridx);
	}
}


void ConfigView::calibratePressed()
{
    close();
    CalibrationWindow * c = new CalibrationWindow(_manager,_audioView);
    c->setMouseTracking(true);
	c->setDeleteOnClose(true);
	c->setGeometry(Widgets::Rect(240, 20, 380, 160));
	Widgets::Application::getInstance()->addWindow(c);

}


}
