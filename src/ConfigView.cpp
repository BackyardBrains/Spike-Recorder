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

namespace BackyardBrains {

ConfigView::ConfigView(RecordingManager &mngr, AudioView &audioView, Widget *parent) : Widget(parent), _manager(mngr), _audioView(audioView) {
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &ConfigView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));

	Widgets::Label *topLabel = new Widgets::Label(this);
	topLabel->setText("Config");
	topLabel->updateSize();

	std::vector<Widgets::Color> c(AudioView::COLORS, AudioView::COLORS+AudioView::COLOR_NUM);
	_clrs.resize(_manager.recordingDevices().size());
	_catchers.reserve(_clrs.size());

	Widgets::Widget *group = new Widgets::Widget(this);
	group->setSizeHint(Widgets::Size(500,400));
	Widgets::BoxLayout *gvbox = new Widgets::BoxLayout(Widgets::Vertical, group);

	if(!_manager.fileMode()) {
		Widgets::BoxLayout *mutehbox = new Widgets::BoxLayout(Widgets::Horizontal);
		Widgets::Label *muteLabel = new Widgets::Label(group);
		muteLabel->setText("Mute Speakers while recording:");
		muteLabel->updateSize();

		_muteCKBox = new Widgets::PushButton(group);
		if(_manager.player().volume() == 0)
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.png"));
		else
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.png"));
		_muteCKBox->setSizeHint(Widgets::Size(16,16));
		_muteCKBox->clicked.connect(this, &ConfigView::mutePressed);

		mutehbox->addWidget(muteLabel);
		mutehbox->addSpacing(10);
		mutehbox->addWidget(_muteCKBox, Widgets::AlignVCenter);
		mutehbox->addSpacing(50);
		gvbox->addLayout(mutehbox);
		gvbox->addSpacing(40);
	}

	for(unsigned int i = 0; i < _manager.recordingDevices().size(); i++) {
		if(!_manager.recordingDevices()[i].enabled)
			continue;
		_clrs[i] = new ColorDropDownList(group);
		_clrs[i]->setContent(c);

		_catchers.push_back(SignalCatcher(i, this));
		_clrs[i]->selectionChanged.connect(&_catchers[i], &SignalCatcher::catchColor);
		Widgets::Label *name = new Widgets::Label(group);
		name->setText(_manager.recordingDevices()[i].name.c_str());
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



    // -------- Serial configuration

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
            _connectButton->setNormalTex(Widgets::TextureGL::get("data/connected.png"));
            _connectButton->setHoverTex(Widgets::TextureGL::get("data/connected.png"));
        }
        else
        {
            _connectButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.png"));
            _connectButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.png"));
        }
        _connectButton->setSizeHint(Widgets::Size(26,26));
        serialHbox->addWidget(_connectButton);
        serialHbox->update();
        gvbox->addSpacing(3);
        gvbox->addLayout(serialHbox);


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


        //HID device connect
        #if defined(_WIN32)
         Widgets::BoxLayout *hidHbox = new Widgets::BoxLayout(Widgets::Horizontal);
        //USB  label
        Widgets::Label *nameUSB = new Widgets::Label(group);
        nameUSB->setText("Connect to USB device ");
        nameUSB->updateSize();
        hidHbox->addSpacing(0);
        hidHbox->addWidget(nameUSB, Widgets::AlignLeft);
        hidHbox->addSpacing(5);
        #endif
        //Button for connect to HID device
        _hidButton = new Widgets::PushButton(group);
        #if defined(_WIN32)


        _hidButton->clicked.connect(this, &ConfigView::hidConnectPressed);
        if(_manager.hidMode())
        {
            _hidButton->setNormalTex(Widgets::TextureGL::get("data/connected.png"));
            _hidButton->setHoverTex(Widgets::TextureGL::get("data/connected.png"));
        }
        else
        {
            _hidButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.png"));
            _hidButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.png"));
        }
        _hidButton->setSizeHint(Widgets::Size(26,26));
       
        hidHbox->addWidget(_hidButton);
        hidHbox->update();
        gvbox->addSpacing(20);
        gvbox->addLayout(hidHbox);
        #endif // defined

        //--------------   Update firmware code (works only under Windows)

        #if defined(_WIN32)
        //TODO:add decision based on available update and current version of firmware
        if(_manager.hidMode())
        {
             Widgets::BoxLayout *updateHbox = new Widgets::BoxLayout(Widgets::Horizontal);
            //USB  label
            Widgets::Label *updateLabel = new Widgets::Label(group);
            updateLabel->setText("Update firmware on Backyard Brains' device ");
            updateLabel->updateSize();
            updateHbox->addSpacing(0);
            updateHbox->addWidget(updateLabel, Widgets::AlignLeft);
            updateHbox->addSpacing(5);

            //Button for connect to HID device
            _updateButton = new Widgets::PushButton(group);
            _updateButton->clicked.connect(this, &ConfigView::firmwareUpdatePressed);

            _updateButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.png"));
            _updateButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.png"));
            _updateButton->setSizeHint(Widgets::Size(26,26));
            updateHbox->addWidget(_updateButton);
            updateHbox->update();
            gvbox->addSpacing(20);
            gvbox->addLayout(updateHbox);

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

void ConfigView::hidConnectPressed()
{
    //connect/diconnect

    if(_manager.hidMode())
    {
       // _manager.setSerialNumberOfChannels(1);
        _manager.disconnectFromHID();

        _hidButton->setNormalTex(Widgets::TextureGL::get("data/connected.png"));
        _hidButton->setHoverTex(Widgets::TextureGL::get("data/connected.png"));
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
        _hidButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.png"));
        _hidButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.png"));
        close();
    }
}


#if defined(_WIN32)
//
// Start firmware update procedure
//
void ConfigView::firmwareUpdatePressed()
{
    if(_manager.getUSBFirmwareUpdateStage() == 0)//avoid starting it twice
    {
    //send message to MSP to prepare for update
        _manager.prepareForHIDFirmwareUpdate();
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
        _connectButton->setNormalTex(Widgets::TextureGL::get("data/connected.png"));
        _connectButton->setHoverTex(Widgets::TextureGL::get("data/connected.png"));
        close();
    }
    else
    {
        _connectButton->setNormalTex(Widgets::TextureGL::get("data/disconnected.png"));
        _connectButton->setHoverTex(Widgets::TextureGL::get("data/disconnected.png"));
        close();

    }
    serialPortWidget->setDisabled(_manager.serialMode());
}


void ConfigView::closePressed() {
	close();
}

void ConfigView::mutePressed() {
	if(_manager.player().volume() == 0) {
		_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.png"));
		_manager.player().setVolume(100);
	} else {
		_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.png"));
		_manager.player().setVolume(0);
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
		int nchan = _audioView.addChannel(virtualDevice);
		if(nchan != -1) {
			_audioView.setChannelColor(nchan, coloridx);
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
		_audioView.removeChannel(virtualDevice);
	} else {
		_audioView.setChannelColor(channel, coloridx);
	}
}

}
