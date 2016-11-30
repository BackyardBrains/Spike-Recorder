#include "ConfigView.h"
#include "engine/RecordingManager.h"
#include "engine/BASSErrors.h"
#include "widgets/Painter.h"
#include "widgets/Color.h"

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
    SetupScreen();
}

//----------------------------------- START OF CONFIG ----------------------------------------------
//--------------------------------------------------------------------------------------------------

void ConfigView::SetupScreen()
{
    //weAreOnTouchScreen = Widgets::Application::getInstance()->areWeOnTouchscreen();


        Log::msg("Create close button...");
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &ConfigView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/configcrossed.bmp"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/configcrossed.bmp"));

    Log::msg("Create top label...");
	Widgets::Label *topLabel = new Widgets::Label(this);
	topLabel->setText("Config");
	topLabel->updateSize();
    Log::msg("Make colors");
	std::vector<Widgets::Color> c(AudioView::COLORS, AudioView::COLORS+AudioView::COLOR_NUM);
	Log::msg("Access virtual devices");
	_clrs.resize(_manager.virtualDevices().size());
	_catchers.reserve(_clrs.size());

    Log::msg("Make box layout");
	Widgets::Widget *group = new Widgets::Widget(this);
	group->setSizeHint(Widgets::Size(500,2500));
	Widgets::BoxLayout *gvbox = new Widgets::BoxLayout(Widgets::Vertical, group);



    //-------------- Mute check box ----------------------------------------

    Log::msg("Check file mode");
	if(!_manager.fileMode()) {
        Log::msg("Make mute checkbox");
		Widgets::BoxLayout *mutehbox = new Widgets::BoxLayout(Widgets::Horizontal);
		Widgets::Label *muteLabel = new Widgets::Label(group);
		muteLabel->setText("Mute Speakers:");
		muteLabel->updateSize();

		_muteCKBox = new Widgets::PushButton(group);
        Log::msg("Check state for mute checkbox");
		if(_manager.player().volume() == 0)
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
		else
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));

        if(weAreOnTouchScreen)
        {
            _muteCKBox->setSizeHint(Widgets::Size(40,40));
        }
        else
        {
            _muteCKBox->setSizeHint(Widgets::Size(19,19));
        }

		_muteCKBox->clicked.connect(this, &ConfigView::mutePressed);
        Log::msg("Add mute label to box");
		mutehbox->addWidget(muteLabel);
		mutehbox->addSpacing(10);
		Log::msg("Add mute checkbox");
		mutehbox->addWidget(_muteCKBox, Widgets::AlignVCenter);
		mutehbox->addSpacing(50);
		Log::msg("Add layout for mute checkbox");
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
    Log::msg("Check file mode 2");
    if(!_manager.fileMode()) {
        Log::msg("Add filter label");
        Widgets::Label *filterMainLabel = new Widgets::Label(group);
		filterMainLabel->setText("Set band-pass filter cutoff frequencies");
		filterMainLabel->updateSize();

        Log::msg("Create text inputs");
        lowValueTI = new Widgets::TextInput(group, 50);
        lowValueTI->textEditingEnded.connect(this, &ConfigView::lowFilterTIValueChanged);
        highValueTI = new Widgets::TextInput(group, 50);
        highValueTI->textEditingEnded.connect(this, &ConfigView::highFilterTIValueChanged);

        Log::msg("Create range selector");
        rangeSelector = new Widgets::RangeSelector(group);
        rangeSelector->setRange(0,_manager.sampleRate()/2);
        Log::msg("Set parameters on range selector");
        highValueTI->setInt(_manager.lowCornerFrequency());
        lowValueTI->setInt(_manager.highCornerFrequency());
        Log::msg("Connect range selector");
        rangeSelector->lowValueChanged.connect(this, &ConfigView::lowFilterValueChanged);
        rangeSelector->highValueChanged.connect(this, &ConfigView::highFilterValueChanged);
        Log::msg("Update values range selector");
        rangeSelector->updateHighLogValue(_manager.lowCornerFrequency());
        rangeSelector->updateLowLogValue(_manager.highCornerFrequency());

        Log::msg("Create label for range boxes");
        Widgets::BoxLayout *labelsBox = new Widgets::BoxLayout(Widgets::Horizontal);

        Widgets::Label *lowFilterLabel = new Widgets::Label(group);
		lowFilterLabel->setText("Low");
		lowFilterLabel->updateSize();

        Widgets::Label *highFilterLabel = new Widgets::Label(group);
		highFilterLabel->setText("High");
		highFilterLabel->updateSize();
        Log::msg("Add filter labels to label boxes");
        labelsBox->addSpacing(8);
        labelsBox->addWidget(lowFilterLabel, Widgets::AlignBottom);
		labelsBox->addSpacing(415);
		labelsBox->addWidget(highFilterLabel, Widgets::AlignBottom);
        Log::msg("Add layout label box");
        gvbox->addLayout(labelsBox);

        Widgets::BoxLayout *valuesBox = new Widgets::BoxLayout(Widgets::Horizontal);
        Log::msg("Add components to value box");
		valuesBox->addWidget(lowValueTI);
		valuesBox->addSpacing(40);
		valuesBox->addWidget(filterMainLabel);
		valuesBox->addSpacing(40);
		valuesBox->addWidget(highValueTI);
		Log::msg("Add components ro gvbox");
		gvbox->addLayout(valuesBox);
        gvbox->addWidget(rangeSelector);
		gvbox->addSpacing(20);
    }




    //----------- 50Hz/60Hz noise filter ------------------------------------

    Log::msg("Check file mode 3");
    if(!_manager.fileMode()) {
        Log::msg("Create filterbox");
		Widgets::BoxLayout *filterhbox = new Widgets::BoxLayout(Widgets::Horizontal);
        Log::msg("Create filter label for notch");
		Widgets::Label *filterLabel = new Widgets::Label(group);
		filterLabel->setText("Enable Notch filter:");
		filterLabel->updateSize();
        Log::msg("Create 50Hz label");
		Widgets::Label *fiftyHzLabel = new Widgets::Label(group);
		fiftyHzLabel->setText("50Hz");
		fiftyHzLabel->updateSize();
        Log::msg("Create 60Hz label");
        Widgets::Label *sixtyHzLabel = new Widgets::Label(group);
		sixtyHzLabel->setText("60Hz");
		sixtyHzLabel->updateSize();
        Log::msg("Create 50 hz push button");
		_50hzFilter = new Widgets::PushButton(group);
		if(_manager.fiftyHzFilterEnabled())
        {
            Log::msg("50 hz enabled");
            std::cout<<"\n\n-----50hz enabled init";
			_50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        }
		else
        {
            Log::msg("50 hz disabled");
            std::cout<<"\n\n-------50hz disabled init";
			_50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        }
        Log::msg("Wire up 50 hz");
		_50hzFilter->setSizeHint(Widgets::Size(19,19));
		_50hzFilter->clicked.connect(this, &ConfigView::fiftyHzPressed);

        Log::msg("Create 60 hz filter button");
		_60hzFilter = new Widgets::PushButton(group);
		if(_manager.sixtyHzFilterEnabled())
        {
            Log::msg("60 Hz enabled");
            std::cout<<"\n\n-----6s0hz enabled init";
			_60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        }
		else
        {
            Log::msg("60 Hz disabled");
             std::cout<<"\n\n-------60hz disabled init";
			_60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        }
        Log::msg("Wire up 60 Hz");
		_60hzFilter->setSizeHint(Widgets::Size(19,19));
		_60hzFilter->clicked.connect(this, &ConfigView::sixtyHzPressed);

        Log::msg("Add components to filter box");
		filterhbox->addWidget(filterLabel);
		filterhbox->addSpacing(13);
		Log::msg("50 Hz components");
		filterhbox->addWidget(fiftyHzLabel);
		filterhbox->addSpacing(4);
		filterhbox->addWidget(_50hzFilter, Widgets::AlignVCenter);
		filterhbox->addSpacing(16);
		Log::msg("60 hz components");
		filterhbox->addWidget(sixtyHzLabel);
		filterhbox->addSpacing(4);
		filterhbox->addWidget(_60hzFilter, Widgets::AlignVCenter);
		filterhbox->addSpacing(50);
		Log::msg("Add filter box to gvbox");
		gvbox->addLayout(filterhbox);
		gvbox->addSpacing(40);
	}


    //----------- Color chooser for channels --------------------------------

    Log::msg("Color chooser start -------");
	for(unsigned int i = 0; i < _manager.virtualDevices().size(); i++) {
        Log::msg("Create dropdown");
		_clrs[i] = new ColorDropDownList(group);
		_clrs[i]->setContent(c);
        Log::msg("Signal catcher");
		_catchers.push_back(SignalCatcher(i, this));
		_clrs[i]->selectionChanged.connect(&_catchers[i], &SignalCatcher::catchColor);
        Log::msg("Create label");
		Widgets::Label *name = new Widgets::Label(group);
		Log::msg("get name of virtual device");
		name->setText(_manager.virtualDevices()[i].name.c_str());
		name->updateSize();
        Log::msg("Create box layout for color chooser");
		Widgets::BoxLayout *ghbox = new Widgets::BoxLayout(Widgets::Horizontal);
		Log::msg("Add to gvbox");
		ghbox->addWidget(_clrs[i]);
		ghbox->addSpacing(20);
		ghbox->addWidget(name,Widgets::AlignVCenter);
		gvbox->addLayout(ghbox);
		gvbox->addSpacing(15);
	}

    Log::msg("Set selection");
	for(int i = 0; i < _audioView.channelCount(); i++)
    {
        Log::msg("Set selection n");
        _clrs[_audioView.channelVirtualDevice(i)]->setSelection(_audioView.channelColor(i));
    }




    // -------- Serial configuration -----------------------------------------
    Log::msg("Set selection 4 - before serial");
    if(!_manager.fileMode())
    {
        Log::msg("Create label");
        //Serial  config widgets
        Widgets::Label *name2 = new Widgets::Label(group);
        name2->setText("Select port:");
        name2->updateSize();
        Log::msg("Add widget label");
        gvbox->addSpacing(0);
        gvbox->addWidget(name2, Widgets::AlignLeft);



        //Dropdown for select port
        Log::msg("Create box layout");
        Widgets::BoxLayout *serialHbox = new Widgets::BoxLayout(Widgets::Horizontal);
        Log::msg("Create dropdown");
        serialPortWidget = new DropDownList(group);
        Log::msg("Clear serial");
        serialPortWidget->clear();
        Log::msg("Get list of serials");
        std::list<std::string> sps =  _manager.serailPortsList();
        std::list<std::string>::iterator it;
        Log::msg("Iterate through serials");
        for(it = sps.begin();it!=sps.end();it++)
        {
            Log::msg("Serial n");
            serialPortWidget->addItem(it->c_str());
        }
        Log::msg("Set selection serial");
        serialPortWidget->setSelection(_manager.serialPortIndex());
        _catchers.push_back(SignalCatcher(_catchers.size(), this));
        Log::msg("Wire up serial");
        serialPortWidget->indexChanged.connect(&_catchers[_catchers.size()-1], &SignalCatcher::catchPort);
        Log::msg("Set disabled enabled");
        serialPortWidget->setDisabled(_manager.serialMode());
        Log::msg("Add widget serial port");
        serialHbox->addWidget(serialPortWidget);
        serialHbox->addSpacing(5);



        Log::msg("Create button for connect to serial");
        //Button for connect to serial
        _connectButton = new Widgets::PushButton(group);
        _connectButton->clicked.connect(this, &ConfigView::connectPressed);
        if(_manager.serialMode())
        {
            Log::msg("connected.bmp");
            _connectButton->setNormalTex(Widgets::TextureGL::get("data/connected.bmp"));
            _connectButton->setHoverTex(Widgets::TextureGL::get("data/connected.bmp"));
        }
        else
        {
            Log::msg("disconnected.bmp");
            _connectButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
            _connectButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
        }
        Log::msg("Set size for button");
        _connectButton->setSizeHint(Widgets::Size(26,26));
        Log::msg("Add button to serial H box");
        serialHbox->addWidget(_connectButton);
        serialHbox->update();
        Log::msg("Add to gvbox");
        gvbox->addSpacing(3);
        gvbox->addLayout(serialHbox);



        //-------------- Serial Number of channels chooser ----------------------------------------
        Log::msg("Check if in serial mode for Num. ch. dropdown");
        if(_manager.serialMode())
        {
                //Number of channels chooser
                Log::msg("Create box layout");
                Widgets::BoxLayout *numberOfChannelsHbox = new Widgets::BoxLayout(Widgets::Horizontal);

                Log::msg("Create label");
                Widgets::Label *numChannelsLabel = new Widgets::Label(group);
                numChannelsLabel->setText("Number of channels:");
                numChannelsLabel->updateSize();
                Log::msg("Add label to box");
                numberOfChannelsHbox->addWidget(numChannelsLabel);
                numberOfChannelsHbox->addSpacing(5);


                Log::msg("Create dropdown");
                numberOfChannelsWidget = new DropDownList(group, 50,30);
                numberOfChannelsWidget->clear();
                Log::msg("Add items");
                numberOfChannelsWidget->addItem("1");
                numberOfChannelsWidget->addItem("2");
                numberOfChannelsWidget->addItem("3");
                numberOfChannelsWidget->addItem("4");
                numberOfChannelsWidget->addItem("5");
                numberOfChannelsWidget->addItem("6");
                Log::msg("Set selection");
                numberOfChannelsWidget->setSelection(_manager.numberOfSerialChannels()-1);
                Log::msg("Signal catcher");
                _catchers.push_back(SignalCatcher(_catchers.size(), this));
                Log::msg("Connect other signal catchers");
                numberOfChannelsWidget->indexChanged.connect(&_catchers[_catchers.size()-1], &SignalCatcher::setNumOfChannelsHandler);
                Log::msg("Disable/enable");
                numberOfChannelsWidget->setDisabled(!_manager.serialMode());
                Log::msg("Add widget to num of channel HBox");
                numberOfChannelsHbox->addWidget(numberOfChannelsWidget);

                numberOfChannelsHbox->update();
                Log::msg("Add layout num of ch box");
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


        Log::msg("Check HID mode");
        if(_manager.hidMode())
        {
                    Log::msg("Crete box layout");
                     Widgets::BoxLayout *updateHbox = new Widgets::BoxLayout(Widgets::Horizontal);
                    //Add label
                    Widgets::Label *updateLabel = new Widgets::Label(group);
                    Log::msg("Create label");
                    std::stringstream labelTextString;
                    if(_manager.firmwareAvailable())
                    {
                        Log::msg("Firmware available");
                        labelTextString <<"Change firmware (device "<< _manager.currentHIDFirmwareType()<<" F:"<<_manager.currentHIDFirmwareVersion()<<" H:"<<_manager.currentHIDHardwareVersion()<<"):";
                    }
                    else
                    {
                        Log::msg("Connected device");
                        labelTextString <<"Connected device: "<< _manager.currentHIDFirmwareType()<<" F:"<<_manager.currentHIDFirmwareVersion()<<" H:"<<_manager.currentHIDHardwareVersion()<<"";
                    }
                    Log::msg("Set text");
                    updateLabel->setText(labelTextString.str().c_str());
                    updateLabel->updateSize();
                    updateHbox->addSpacing(0);
                    Log::msg("Add widget");
                    updateHbox->addWidget(updateLabel, Widgets::AlignLeft);
                    updateHbox->addSpacing(5);

                    Log::msg("Create update dropdown");
                    Widgets::BoxLayout *updateSelectionHbox = new Widgets::BoxLayout(Widgets::Horizontal);
                    if(_manager.firmwareAvailable())
                    {
                        //Add dropbox
                        Log::msg("Firmware available add dropbox");
                        firmwaresWidget = new DropDownList(group,400,30);
                        firmwaresWidget->clear();
                        Log::msg("Get list");
                        std::list<BYBFirmwareVO> fps =  _manager.firmwareList();
                        Log::msg("Iterate");
                        for( listBYBFirmwareVO::iterator ti = fps.begin();
                            ti != fps.end();
                            ti ++)
                        {
                            Log::msg("Crete stream");
                            std::stringstream sstm;
                             sstm <<((BYBFirmwareVO)(*ti)).description << " (V" << ((BYBFirmwareVO)(*ti)).version <<")";
                            Log::msg("Add item");
                            firmwaresWidget->addItem(sstm.str().c_str() );
                        }


                        Log::msg("Set selection");
                        firmwaresWidget->setSelection(0);

                        Log::msg("Set selected firmware");
                        selectedFirmware = new BYBFirmwareVO();
                        selectedFirmware->URL = std::string((_manager.firmwareList().front()).URL);
                        Log::msg("Set filepath");
                        selectedFirmware->filepath = std::string((_manager.firmwareList().front()).filepath);
                        selectedFirmware->id = (_manager.firmwareList().front()).id;
                        Log::msg("set disabled");
                        firmwaresWidget->setDisabled(false);



                        Log::msg("Catcher");
                        _catchers.push_back(SignalCatcher(_catchers.size(), this));
                        Log::msg("Wire up firmware selection");
                        firmwaresWidget->indexChanged.connect(&_catchers[_catchers.size()-1], &SignalCatcher::catchFirmwareSelection);

                        Log::msg("Add widget");
                        updateSelectionHbox->addWidget(firmwaresWidget);

                        Log::msg("Create button for update");
                        //Button for update HID device
                        _updateButton = new Widgets::PushButton(group);

                        _updateButton->clicked.connect(this, &ConfigView::firmwareUpdatePressed);
                        Log::msg("Set image for update button");
                        _updateButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                        _updateButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.bmp"));
                        _updateButton->setSizeHint(Widgets::Size(26,26));
                        Log::msg("Add button widget");
                        updateSelectionHbox->addSpacing(5);
                        updateSelectionHbox->addWidget(_updateButton);
                        updateSelectionHbox->update();
                    }

                    Log::msg("Add spacing");
                    gvbox->addSpacing(20);
                    gvbox->addLayout(updateHbox);
                    Log::msg("Firmware available?");
                    if(_manager.firmwareAvailable())
                    {
                        Log::msg("Add layout");
                        gvbox->addLayout(updateSelectionHbox);
                    }

            Log::msg("End");
        }
        #endif
        Log::msg("End in file mode");
    }//end if not file mode

    Log::msg("Update layout for gv box");
	gvbox->update();

    Log::msg("Create vbox and hbox");
	vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
	hbox->addSpacing(10);
	Log::msg("Add close button");
	hbox->addWidget(closeButton);
	hbox->addSpacing(17);
	Log::msg("Add top label");
	hbox->addWidget(topLabel, Widgets::AlignVCenter);
	vbox->addSpacing(10);
	Log::msg("Add hbox");
	vbox->addLayout(hbox);
	vbox->addSpacing(20);
	Log::msg("Add group");
	vbox->addWidget(group, Widgets::AlignCenter);

	vbox->update();

    Log::msg("End function");
}
//----------------------------------- END OF CONFIG ------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ConfigView::paintEvent() {
	Widgets::Color bg = Widgets::Colors::background;
	bg.a = 250;
	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect());

}


#if defined(_WIN32)

        void ConfigView::hidConnectPressed()
        {
            Log::msg("Hid connect pressed");
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
            Log::msg("Firmware selection changed");
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
            Log::msg("Firmware update pressed");
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
    Log::msg("Connect pressed");
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
    Log::msg("Close pressed");
	close();
}

void ConfigView::mutePressed() {
    Log::msg("Mute pressed");
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
    Log::msg("High filter value changed");
    highValueTI->setInt(hvalue);
    if(hvalue>=_manager.sampleRate()/2)
    {
        Log::msg("First if");
        _manager.enableLowPassFilterWithCornerFreq(_manager.sampleRate()/2);
        _manager.disableLowPassFilter();
    }
    else
    {
        Log::msg("In else");
        _manager.enableLowPassFilterWithCornerFreq(hvalue);
    }
}


//
// Band pass filtering value changed
//
void ConfigView::lowFilterValueChanged(int lvalue)
{
    Log::msg("Low filter value changed");
    lowValueTI->setInt(lvalue);
     if(lvalue<=0)
    {
        Log::msg("First if");
        _manager.enableHighPassFilterWithCornerFreq(0.0);
         _manager.disableHighPassFilter();
    }
    else
    {
        Log::msg("else");
        _manager.enableHighPassFilterWithCornerFreq(lvalue);

    }
}


void ConfigView::lowFilterTIValueChanged(std::string newString)
{
    Log::msg("lowFilterTIValueChanged");
    rangeSelector->updateLowLogValue(lowValueTI->getInt());
    lowValueTI->setInt(rangeSelector->getLowValue());
}

void ConfigView::highFilterTIValueChanged(std::string newString)
{
    Log::msg("highFilterTIValueChanged");
    rangeSelector->updateHighLogValue(highValueTI->getInt());
    highValueTI->setInt(rangeSelector->getHighValue());
}
//
// Enable/disable 50Hz filter checkbox pressed
//
void ConfigView::fiftyHzPressed() {

    weAreOnTouchScreen = !weAreOnTouchScreen;
    vbox->removeAll();
    SetupScreen();

    Log::msg("fiftyHzPressed");
    if(_manager.fiftyHzFilterEnabled())
    {
        Log::msg("Enabled");
        _manager.disable50HzFilter();
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
    else
    {
        Log::msg("Disabled");
        _manager.enable50HzFilter();
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
}


//
// Enable/disable 60Hz filter checkbox pressed
//
void ConfigView::sixtyHzPressed() {
    Log::msg("60 hz pressed");
    if(_manager.sixtyHzFilterEnabled())
    {
        Log::msg("if");
        _manager.disable60HzFilter();
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
    else
    {
        Log::msg("else");
        _manager.enable60HzFilter();
        _60hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.bmp"));
        _50hzFilter->setNormalTex(Widgets::TextureGL::get("data/ckboxon.bmp"));
    }
}


void ConfigView::serialPortChanged(int virtualDevice, int portidx)
{
    Log::msg("Serial port changed");
    _manager.changeSerialPort(portidx);
}

void ConfigView::setSerialNumberOfChannels(int numberOfChannels)
{
    Log::msg("Set serial num of channels pressed");
    _manager.setSerialNumberOfChannels(numberOfChannels);
    close();
}

void ConfigView::colorChanged(int virtualDevice, int coloridx) {
    Log::msg("Color changed");
	int channel = _audioView.virtualDeviceChannel(virtualDevice);
	if(channel < 0 && coloridx != 0) {
            Log::msg("Have channels");
		bool success = _manager.bindVirtualDevice(virtualDevice);

		if(success) {
		    Log::msg("success");
			_audioView.setChannelColor(_audioView.channelCount()-1, coloridx);
		} else {
		    Log::msg("else");
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
	    Log::msg("==0");
		_manager.unbindVirtualDevice(virtualDevice);
	} else {
	    Log::msg("else else");
		_audioView.setChannelColor(channel, coloridx);
	}
}


void ConfigView::calibratePressed()
{
    Log::msg("Calibration pressed");
    close();
    CalibrationWindow * c = new CalibrationWindow(_manager,_audioView);
    c->setMouseTracking(true);
	c->setDeleteOnClose(true);
	c->setGeometry(Widgets::Rect(240, 20, 380, 160));
	Widgets::Application::getInstance()->addWindow(c);

}


}
