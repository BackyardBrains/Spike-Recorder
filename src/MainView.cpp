#include "MainView.h"
#include "widgets/Application.h"
#include "widgets/PushButton.h"
#include "widgets/TextureGL.h"
#include "widgets/ScrollBar.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/FileDialog.h"
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"
#include "widgets/ToolTip.h"
#include "engine/SpikeSorter.h"
#include "engine/FileRecorder.h"
#include "engine/AnalysisManager.h"
#include "Paths.h"
#include "MainView.h"
#include "AudioView.h"
#include "ConfigView.h"
#include "AnalysisView.h"
#include "RecordingBar.h"
#include "ThresholdPanel.h"
#include "Log.h"
#include "FFTView.h"
#include <SDL_opengl.h>
#include <SDL.h>
#include <cerrno>
#include <cstring>
#include "CalibrationWindow.h"
#include <sstream>
#include "FirmwareUpdateView.h"


namespace BackyardBrains {


MainView::MainView(RecordingManager &mngr, AnalysisManager &anaman, FileRecorder &fileRec, Widget *parent) : Widget(parent), _manager(mngr), _anaman(anaman), _fileRec(fileRec), _anaView(NULL) {
	_audioView = new AudioView(this, _manager, _anaman);

	    //setup timer that will periodicaly check for USB HID devices

	timerUSB = clock();
	_manager.scanForHIDDevices();
	_manager.triggered.connect(this,&MainView::triggerEvent);
	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_manager.devicesChanged.connect(_audioView, &AudioView::updateChannels);


	_configButton = new Widgets::PushButton(this);
	_configButton->setNormalTex(Widgets::TextureGL::get("data/config.bmp"));
	_configButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.bmp"));
	_configButton->clicked.connect(this, &MainView::configPressed);
    
    _p300Button = new Widgets::PushButton(this);
    _p300Button->setNormalTex(Widgets::TextureGL::get("data/p300normal.bmp"));
    _p300Button->setHoverTex(Widgets::TextureGL::get("data/p300high.bmp"));
    _p300Button->clicked.connect(this, &MainView::p300Pressed);
    _p300Button->setRightPadding(5);
    _p300Button->setSizeHint(Widgets::Size(53,48));
    
    _p300AudioButton = new Widgets::PushButton(this);
    _p300AudioButton->setNormalTex(Widgets::TextureGL::get("data/p300audio-normal.bmp"));
    _p300AudioButton->setHoverTex(Widgets::TextureGL::get("data/p300audio-high.bmp"));
    _p300AudioButton->clicked.connect(this, &MainView::p300AudioStimulationPressed);
    _p300AudioButton->setRightPadding(5);
    _p300AudioButton->setSizeHint(Widgets::Size(53,48));
    
    
	threshButton = new Widgets::PushButton(this);
	threshButton->setNormalTex(Widgets::TextureGL::get("data/thresh.bmp"));
	threshButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.bmp"));
	threshButton->clicked.connect(this, &MainView::threshPressed);
	threshButton->setRightPadding(5);
	threshButton->setSizeHint(Widgets::Size(53,48));

	_analysisButton = new Widgets::PushButton(this);
	_analysisButton->setNormalTex(Widgets::TextureGL::get("data/analysis.bmp"));
	_analysisButton->setHoverTex(Widgets::TextureGL::get("data/analysishigh.bmp"));
	_analysisButton->clicked.connect(this, &MainView::analysisPressed);


	_analysisButton->setVisible(false);

	_muscleHIDButton = new Widgets::PushButton(this);
    _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprocon.bmp"));
    _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgproconhigh.bmp"));
    _muscleHIDButton->clicked.connect(this, &MainView::muscleHIDPressed);
	_muscleHIDButton->setVisible(false);
	_muscleHIDButton->setRightPadding(5);
	_muscleHIDButton->setSizeHint(Widgets::Size(0,0));


    _neuronHIDButton = new Widgets::PushButton(this);
    _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprocon.bmp"));
    _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronproconhigh.bmp"));
    _neuronHIDButton->clicked.connect(this, &MainView::neuronHIDPressed);
    _neuronHIDButton->setVisible(false);
    _neuronHIDButton->setRightPadding(5);
    _neuronHIDButton->setSizeHint(Widgets::Size(0,0));


	_recordButton = new Widgets::PushButton(this);
	_recordButton->setNormalTex(Widgets::TextureGL::get("data/rec.bmp"));
	_recordButton->setHoverTex(Widgets::TextureGL::get("data/rechigh.bmp"));
	_recordButton->clicked.connect(this, &MainView::recordPressed);


	_fftButton = new Widgets::PushButton(this);
	_fftButton->setNormalTex(Widgets::TextureGL::get("data/fft.bmp"));
	_fftButton->setHoverTex(Widgets::TextureGL::get("data/ffthigh.bmp"));
	_fftButton->clicked.connect(this, &MainView::fftPressed);
    _fftButton->setRightPadding(5);
    _fftButton->setSizeHint(Widgets::Size(53,48));
    //_fftButton->setSizeHint(Widgets::Size(164,164));


	_fileButton = new Widgets::PushButton(this);
	_fileButton->setNormalTex(Widgets::TextureGL::get("data/file.bmp"));
	_fileButton->setHoverTex(Widgets::TextureGL::get("data/filehigh.bmp"));
	_fileButton->clicked.connect(this, &MainView::filePressed);

	_pauseButton = new Widgets::PushButton(this);
	_pauseButton->clicked.connect(this, &MainView::pausePressed);
	_manager.pauseChanged.connect(this, &MainView::pausePressed);
	_pauseButton->setNormalTex(Widgets::TextureGL::get("data/pause.bmp"));
	_pauseButton->setHoverTex(Widgets::TextureGL::get("data/pausehigh.bmp"));
	_pauseButton->setSizeHint(Widgets::Size(64,64));


    _alphaFeedbackButton = new Widgets::PushButton(this);
    _alphaFeedbackButton->setNormalTex(Widgets::TextureGL::get("data/speakeroff.bmp"));
    _alphaFeedbackButton->setSizeHint(Widgets::Size(50,50));
    _alphaFeedbackButton->clicked.connect(this, &MainView::alphaFeedbackPressed);
    _alphaFeedbackButton->setVisible(false);
     alphaFeedbackAcive = false;



	_backwardButton = new Widgets::PushButton(this);
	_backwardButton->setNormalTex(Widgets::TextureGL::get("data/backward.bmp"));
	_backwardButton->setHoverTex(Widgets::TextureGL::get("data/backwardhigh.bmp"));
	_backwardButton->setSizeHint(Widgets::Size(32,32));
	_backwardButton->clicked.connect(this, &MainView::backwardPressed);

	_forwardButton = new Widgets::PushButton(this);
	_forwardButton->setNormalTex(Widgets::TextureGL::get("data/forward.bmp"));
	_forwardButton->setHoverTex(Widgets::TextureGL::get("data/forwardhigh.bmp"));
	_forwardButton->setSizeHint(Widgets::Size(32,32));
	_forwardButton->clicked.connect(this, &MainView::forwardPressed);

	_seekBar = new Widgets::ScrollBar(Widgets::Horizontal, this);
	_seekBar->setVisible(false);
	_seekBar->setRange(0,1000);
	_seekBar->setPageStep(25);
	_seekBar->setValue(0);
	_seekBar->valueChanged.connect(_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(_seekBar, &Widgets::ScrollBar::updateValue);

	_threshavgGroup = new ThresholdPanel(_manager, _anaman, this);
	_threshavgGroup->setVisible(false);
    _threshavgGroup->setMouseTracking(true);
	_recBar = new RecordingBar(_fileRec, this);

	_fftView = new FFTView(*_audioView, _manager, _anaman, this);

	Widgets::BoxLayout *topBar = new Widgets::BoxLayout(Widgets::Horizontal);
	topBar->addSpacing(10);
	topBar->addWidget(_configButton);
	topBar->addSpacing(5);

	topBar->addWidget(threshButton);
	//topBar->addSpacing(5);
	topBar->addWidget(_fftButton);
    //topBar->addSpacing(5);
    topBar->addWidget(_muscleHIDButton);
    topBar->addWidget(_neuronHIDButton);



    shieldsButtonBoxLayout = new Widgets::BoxLayout(Widgets::Horizontal);
    //shieldsButtonBoxLayout->addWidget(_plantSSButton);
    //shieldsButtonBoxLayout->addWidget(_muscleSSButton);
    //shieldsButtonBoxLayout->addWidget(_heartSSButton);
    topBar->addLayout(shieldsButtonBoxLayout);
    topBar->addWidget(_p300Button);
    topBar->addWidget(_p300AudioButton);
    
    //topBar->addSpacing(5);
	topBar->addWidget(_analysisButton);
    topBar->addSpacing(30);
	topBar->addWidget(_threshavgGroup, Widgets::AlignVCenter);
	topBar->addStretch();
    topBar->addWidget(_alphaFeedbackButton);
    topBar->addSpacing(10);
	topBar->addWidget(_recordButton);
	topBar->addSpacing(5);
	topBar->addWidget(_fileButton);
	topBar->addSpacing(10);


	Widgets::BoxLayout * const seekBarBox = new Widgets::BoxLayout(Widgets::Horizontal);
	seekBarBox->setAlignment(Widgets::AlignHCenter);

	seekBarBox->addWidget(_backwardButton,Widgets::AlignVCenter);
	seekBarBox->addSpacing(10);
	seekBarBox->addWidget(_pauseButton);
	seekBarBox->addSpacing(10);
	seekBarBox->addWidget(_forwardButton,Widgets::AlignVCenter);

	Widgets::BoxLayout * const vbox = new Widgets::BoxLayout(Widgets::Vertical, this);

	vbox->addWidget(_recBar);
	vbox->addSpacing(10);
	vbox->addLayout(topBar);
	vbox->addSpacing(10);
	vbox->addWidget(_audioView, Widgets::AlignVCenter);
	vbox->addWidget(_fftView);
	vbox->addSpacing(10);
	vbox->addWidget(_seekBar);
	vbox->addSpacing(10);
	vbox->addLayout(seekBarBox);
	vbox->addSpacing(10);

	vbox->update();
    topBar->update();
 	Widgets::Application::getInstance()->updateLayout();

	_audioView->standardSettings();

}

MainView::~MainView() {
	delete _anaView;
}

void MainView::triggerEvent()
{
    if(((ThresholdPanel *)_threshavgGroup)->ekgOn())
    {
        _manager.sendEKGImpuls();
    }

}


void MainView::pausePressed() {
	if(_manager.paused()) {
		_manager.setPaused(false);
		_pauseButton->setNormalTex(Widgets::TextureGL::get("data/pause.bmp"));
		_pauseButton->setHoverTex(Widgets::TextureGL::get("data/pausehigh.bmp"));
		_seekBar->setVisible(false);
	} else {
		_manager.setPaused(true);
		_pauseButton->setNormalTex(Widgets::TextureGL::get("data/play.bmp"));
		_pauseButton->setHoverTex(Widgets::TextureGL::get("data/playhigh.bmp"));
		_seekBar->setVisible(true);
	}
}


void MainView::ekgPressed() {
}


void MainView::alphaFeedbackPressed()
{
    if(alphaFeedbackAcive)
    {
        _alphaFeedbackButton->setNormalTex(Widgets::TextureGL::get("data/speakeroff.bmp"));
        alphaFeedbackAcive = false;
        _manager.player().setVolume(0);
    }
    else
    {
        _alphaFeedbackButton->setNormalTex(Widgets::TextureGL::get("data/speaker.bmp"));
        alphaFeedbackAcive = true;
    }
}
void MainView::forwardPressed() {
	if(_manager.fileMode()) { // end file mode when in file mode
		//delete _anaView;
		_manager.initRecordingDevices();
		_recordButton->setVisible(true);
		_analysisButton->setVisible(false);
		_anaView = NULL;
	} else {
		_audioView->setOffset(0);
	}

	if(_manager.paused())
		pausePressed();
}

void MainView::backwardPressed() {
    _audioView->setOffset(_audioView->offset()-5*_manager.sampleRate());
    if(_manager.paused())
        pausePressed();
}


void MainView::threshPressed() {
	if(!_manager.threshMode()) {

        threshButton->setNormalTex(Widgets::TextureGL::get("data/threshcrossed.bmp"));
        threshButton->setHoverTex(Widgets::TextureGL::get("data/threshcrossed.bmp"));
		_fftView->setActive(false);
        _fftButton->setNormalTex(Widgets::TextureGL::get("data/fft.bmp"));
        _fftButton->setHoverTex(Widgets::TextureGL::get("data/ffthigh.bmp"));
		_manager.setThreshMode(true);
		_threshavgGroup->setVisible(true);
		_threshavgGroup->setSizeHint(Widgets::Size(400,32));
		_fftButton->setSizeHint(Widgets::Size(0,0));
		_fftButton->setVisible(false);
	} else {
        threshButton->setNormalTex(Widgets::TextureGL::get("data/thresh.bmp"));
        threshButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.bmp"));
		_manager.setThreshMode(false);
		_fftButton->setVisible(true);
		_fftButton->setSizeHint(Widgets::Size(53,48));
		_threshavgGroup->setSizeHint(Widgets::Size());
		_threshavgGroup->setVisible(false);
	}
	Widgets::Application::getInstance()->updateLayout();
}

void MainView::recordPressed() {
	if(!_fileRec.recording()) {
		if(_audioView->channelCount() == 0) {
			Widgets::ErrorBox *box = new Widgets::ErrorBox("Error: At least one channel has to be open to record anything.");
			box->setGeometry(Widgets::Rect(this->width()/2-200, this->height()/2-40, 400, 80));
			Widgets::Application::getInstance()->addPopup(box);
			return;
		}

        //in case we are looking at some point in past we want to go to present signal and start recording
        forwardPressed();

		char buf[64];
		time_t t = time(NULL);
		strftime(buf, sizeof(buf), "%Y-%m-%d_%H.%M.%S.wav", localtime(&t));
		std::string filename = getRecordingPath()+"/BYB_Recording_"+buf;
        Log::msg("Record in file: %s",filename.c_str());
		if(!_fileRec.start(filename.c_str())) {
			const char *error = strerror(errno);
			std::stringstream s;
			s << "Error: Failed to open '" << filename.c_str() << "' for recording: " << error << ".";
			Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());

			box->setGeometry(Widgets::Rect(this->width()/2-200, this->height()/2-40, 400, 80));
			Widgets::Application::getInstance()->addPopup(box);
			Log::msg("\n\n%s\n\n",s.str().c_str());
			return;
		}
		//disable buttons during recording
		_configButton->setSizeHint(Widgets::Size(0, 48));
		_configButton->setVisible(false);
		_fileButton->setSizeHint(Widgets::Size(0, 48));
		_fileButton->setVisible(false);
		_pauseButton->setVisible(false);
		_forwardButton->setVisible(false);
		_backwardButton->setVisible(false);

		_recBar->setActive(true);
	} else {
		MetadataChunk m;
		_audioView->constructMetadata(&m);
		_manager.constructMetadata(&m);

		_fileRec.stop(&m);

		//enable buttons when we stopped recording
		_configButton->setVisible(true);
		_configButton->setSizeHint(Widgets::Size(48, 48));
		_fileButton->setVisible(true);
		_fileButton->setSizeHint(Widgets::Size(48, 48));
		_pauseButton->setVisible(true);
		_forwardButton->setVisible(true);
		_backwardButton->setVisible(true);

		_recBar->setActive(false);

		std::stringstream s;
		s << "Saved recording to '" << _fileRec.filename() << "'.";
		Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
		box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
		Widgets::Application::getInstance()->addPopup(box);
		Log::msg("%s",s.str().c_str());
	}

	Widgets::Application::getInstance()->updateLayout();
}

void MainView::fftPressed() {
	if(_fftView->active()) {
		_fftView->setActive(false);
        _fftButton->setNormalTex(Widgets::TextureGL::get("data/fft.bmp"));
        _fftButton->setHoverTex(Widgets::TextureGL::get("data/ffthigh.bmp"));
	} else {
		_fftView->setActive(true);
        _fftButton->setNormalTex(Widgets::TextureGL::get("data/fftcrossed.bmp"));
        _fftButton->setHoverTex(Widgets::TextureGL::get("data/fftcrossed.bmp"));
	}

	Widgets::Application::getInstance()->updateLayout();
}

void MainView::filePressed() {
	Widgets::FileDialog d(Widgets::FileDialog::OpenFile);

	d.open();
	while(d.isOpen())
		SDL_Delay(16);
	std::string str;
	int s = d.getResultState();
	if(s != Widgets::FileDialog::SUCCESS)
		return;

	//delete _anaView;
	_anaView = NULL;
	bool rc = _manager.loadFile(d.getResultFilename().c_str());

	if(rc == false) {
		std::stringstream s;
		s << "Error: Failed to open '" << d.getResultFilename().c_str() << "'. Wrong format perhaps?";
		Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
		box->setGeometry(Widgets::Rect(width()/2-200, height()/2-40, 400, 80));
		Widgets::Application::getInstance()->addPopup(box);
		Log::msg("\n\n%s\n\n",s.str().c_str());
		return;
	}
	Log::msg("Loading metadata, if present...");
	MetadataChunk m;
	const char *mdatastr = _manager.fileMetadataString();
    
	if(mdatastr)
    {
		FileRecorder::parseMetadataStr(&m, mdatastr, _manager);
    }
    FileRecorder::parseMarkerTextFile(m.markers, FileRecorder::eventTxtFilename(d.getResultFilename().c_str()), _manager.sampleRate());
    _manager.applyMetadata(m);
    _audioView->applyMetadata(m);


	_recordButton->setVisible(false);
	_analysisButton->setVisible(true);
	//_analysisButton->setSizeHint(Widgets::Size(48,48));
	_fftView->setActive(false);
    _fftButton->setNormalTex(Widgets::TextureGL::get("data/fft.bmp"));
    _fftButton->setHoverTex(Widgets::TextureGL::get("data/ffthigh.bmp"));

	Widgets::ToolTip *tip = new Widgets::ToolTip("Click to return to live mode \x1f", 2000);
	tip->setGeometry(Widgets::Rect(width()/2-190, height()-150, 280, 40));
	tip->setMouseTracking(false);
	Widgets::Application::getInstance()->updateLayout();
	Widgets::Application::getInstance()->addPopup(tip);
    _manager.fileIsLoadedAndFirstActionDidNotYetOccurred = true;
}

void MainView::configPressed() {


    Log::msg("Config pressed, refresh serial port...");
    //_manager.refreshSerialPorts();
    Log::msg("Create config view...");
	ConfigView *c = new ConfigView(_manager, *_audioView);
	Log::msg("Set mouse track...");
	c->setMouseTracking(true);
	c->setDeleteOnClose(true);
	c->setGeometry(rect());
	Log::msg("Add Config Window...");
	Widgets::Application::getInstance()->addWindow(c);
}

void MainView::p300Pressed()
{
    if(_manager.getP300HardwareStatus())
    {
        _manager.setP300OnHardware(false);
        _p300Button->setNormalTex(Widgets::TextureGL::get("data/p300normal.bmp"));
        _p300Button->setHoverTex(Widgets::TextureGL::get("data/p300high.bmp"));
    }
    else
    {
        _manager.setP300OnHardware(true);
        _p300Button->setNormalTex(Widgets::TextureGL::get("data/p300selected.bmp"));
        _p300Button->setHoverTex(Widgets::TextureGL::get("data/p300selected.bmp"));
    }
    
}

void MainView::p300AudioStimulationPressed()
{
    if(_manager.getP300AudioStimulationHardwareStatus())
    {
        _manager.setP300SoundStimmulationOnHardware(false);
        _p300AudioButton->setNormalTex(Widgets::TextureGL::get("data/p300audio-normal.bmp"));
        _p300AudioButton->setHoverTex(Widgets::TextureGL::get("data/p300audio-high.bmp"));
    }
    else
    {
        _manager.setP300SoundStimmulationOnHardware(true);
        _p300AudioButton->setNormalTex(Widgets::TextureGL::get("data/p300audio-selected.bmp"));
        _p300AudioButton->setHoverTex(Widgets::TextureGL::get("data/p300audio-selected.bmp"));
    }
}

void MainView::analysisPressed() {


    _anaView = new AnalysisView(_manager, _anaman);


	_anaView->setDeleteOnClose(true);
	_anaView->setGeometry(rect());
	Widgets::Application::getInstance()->addWindow(_anaView);

	if(!_manager.paused())
		pausePressed();
}

    void MainView::plantPressed(Widgets::MouseEvent *mouseEv, Widgets::PushButton* buttonInst)
    {
        Log::msg("plantPressed function");
        connectToShieldForButton(buttonInst);
        //connectToFirstShieldOfType(ArduinoSerial::plant);
    }

    void MainView::musclePressed(Widgets::MouseEvent *mouseEv, Widgets::PushButton* buttonInst)
    {
        Log::msg("musclePressed function");
        connectToShieldForButton(buttonInst);
        //connectToFirstShieldOfType(ArduinoSerial::muscle);
    }
    void MainView::neuronPressed(Widgets::MouseEvent *mouseEv, Widgets::PushButton* buttonInst)
    {
        Log::msg("neuronPressed function");
        connectToShieldForButton(buttonInst);
    }
    void MainView::heartPressed(Widgets::MouseEvent *mouseEv, Widgets::PushButton* buttonInst)
    {
        Log::msg("heartPressed function");
        connectToShieldForButton(buttonInst);
        //connectToFirstShieldOfType(ArduinoSerial::heart);
    }

    void MainView::connectToShieldForButton(Widgets::PushButton* buttonInst)
    {
        std::list<SerialPortIndicator> ::iterator buttonsIterator;
        std::cout<<"\n connectToShieldForButton\n";

        //find port name
        bool foundButton = false;
        ArduinoSerial::SerialPort selectedPort;

        for(buttonsIterator = shieldButtons.begin();buttonsIterator!=shieldButtons.end();buttonsIterator++)
        {
            if(buttonsIterator->button == buttonInst)
            {
                selectedPort = buttonsIterator->serialPort;
                foundButton = true;
            }

        }

        if(foundButton == false)
        {
            return;
        }

        //find index of port in list
        std::list<ArduinoSerial::SerialPort> sps =  _manager.serailPorts();
        std::list<ArduinoSerial::SerialPort>::iterator it;
        int portIndex = 0;
        for(it = sps.begin();it!=sps.end();it++)
        {
            std::size_t found;
            found  = it->portName.find(selectedPort.portName);

            if (found!=std::string::npos)
            {
                break;
            }
            portIndex++;
        }

        //end HID mode if in HID mode
        if(_manager.hidMode())
        {
            _manager.disconnectFromHID();
            _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprocon.bmp"));
            _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgproconhigh.bmp"));
            _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprocon.bmp"));
            _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronproconhigh.bmp"));

        }

        // end file mode when in file mode
        if(_manager.fileMode()) {
            _recordButton->setVisible(true);
            _analysisButton->setVisible(false);
            _audioView->setOffset(0);
        } else {
            _audioView->setOffset(0);
        }


        //if we are in serial mode already disconnect from current serial port
        if(_manager.serialMode())
        {

                std::cout<<"disconnect on Button click (connectToShieldForButton)\n";

                std::size_t found;
                found  = _manager.getCurrentPort().portName.find(selectedPort.portName);

                //_manager.setP300OnHardware(false);
                //_manager.setP300SoundStimmulationOnHardware(false);
        
            
                _manager.setSerialNumberOfChannels(1);
                _manager.disconnectFromSerial();

                //if current serial port was one that we clicked than we don;t have
                //to do anything else, just return from function
                if (found!=std::string::npos)
                {
                    return;
                }

        }

        bool connected = _manager.initSerial(selectedPort.portName.c_str(), selectedPort.baudRate);
        //Used so that config knows which one is selected in dropdown
        _manager.changeSerialPort(portIndex);

        if(!connected)
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

        if(_manager.paused())
            pausePressed();

    }




    //
    // Not used currently
    //
    void MainView::connectToFirstShieldOfType(ArduinoSerial::SerialDevice deviceType)
    {
        std::list<ArduinoSerial::SerialPort> sps =  _manager.serailPorts();
        std::list<ArduinoSerial::SerialPort>::iterator it;
        int portIndex = 0;
        std::cout<<"\n connectToFirstShieldOfType\n";
        if(_manager.hidMode())
        {
            _manager.disconnectFromHID();
            _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprocon.bmp"));
            _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgproconhigh.bmp"));
            _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprocon.bmp"));
            _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronproconhigh.bmp"));


        }
        if(_manager.serialMode())
        {
            if(_manager.getCurrentPort().deviceType == deviceType)
            {
                std::cout<<"1 disconnect \n";
                //if we are connected to serial and device type is the same as this one
                //just disconnect!
                _manager.setSerialNumberOfChannels(1);
                _manager.disconnectFromSerial();

                return;
            }
        }

        if(_manager.fileMode()) { // end file mode when in file mode
            _recordButton->setVisible(true);
            _analysisButton->setVisible(false);
            _audioView->setOffset(0);
        } else {
            _audioView->setOffset(0);
        }

        for(it = sps.begin();it!=sps.end();it++)
        {
            if(it->deviceType == deviceType)
            {
                //we found first shield of right device type

                std::size_t found;
                found  = _manager.getCurrentPort().portName.find(it->portName);

                if (found!=std::string::npos)
                {
                    //if we just want to turn off current shield
                    if(_manager.serialMode())
                    {
                        std::cout<<"2 disconnect \n";
                        _manager.setSerialNumberOfChannels(1);
                        _manager.disconnectFromSerial();
                    }
                    break;
                }
                //if we want to turn on some different shield
                if(_manager.serialMode())
                {
                    std::cout<<"3 disconnect \n";
                    _manager.setSerialNumberOfChannels(1);
                    _manager.disconnectFromSerial();
                }

                bool connected = _manager.initSerial(it->portName.c_str(),it->baudRate);
                _manager.changeSerialPort(portIndex);
                if(!connected)
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
                break;
            }
            portIndex++;
        }


        if(_manager.paused())
            pausePressed();

    }

void MainView::muscleHIDPressed()
{
    //connect/diconnect
    std::cout<<"HId mode:"<<_manager.hidMode()<<" Currently connected type: "<<_manager.currentlyConnectedHIDBoardType()<<"\n";
    if(_manager.hidMode() && (_manager.currentlyConnectedHIDBoardType() == HID_BOARD_TYPE_MUSCLE))
    {
        _manager.disconnectFromHID();
        _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprocon.bmp"));
        _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgproconhigh.bmp"));
    }
    else
    {
        if(_manager.hidMode() && (_manager.currentlyConnectedHIDBoardType() == HID_BOARD_TYPE_NEURON))
        {
            _manager.disconnectFromHID();
            _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprocon.bmp"));
            _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronproconhigh.bmp"));
            while(_manager.currentlyConnectedHIDBoardType() != HID_BOARD_TYPE_NONE)
            {

            }
        }

        if(_manager.serialMode())
        {
            _manager.setSerialNumberOfChannels(1);
            _manager.disconnectFromSerial();
        }

        if(_manager.fileMode()) { // end file mode when in file mode
            //delete _anaView;
            _manager.initRecordingDevices();
            _recordButton->setVisible(true);
            _analysisButton->setVisible(false);
            _anaView = NULL;
        } else {
            _audioView->setOffset(0);
        }

        if(_manager.paused())
        {
            _manager.setPaused(false);
        }

        if(!_manager.initHIDUSB(HID_BOARD_TYPE_MUSCLE))
        {
            std::cout<<"Can't open HID Muscle device. \n";


            Widgets::ErrorBox *box = new Widgets::ErrorBox(_manager.hidError.c_str());
            box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
            Widgets::Application::getInstance()->addPopup(box);
        }
        if(_manager.currentlyConnectedHIDBoardType()==HID_BOARD_TYPE_MUSCLE)
        {
            _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprodiscon.bmp"));
            _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgprodiscon.bmp"));
        }


    }

}

void MainView::neuronHIDPressed()
{
    //connect/diconnect

    if(_manager.hidMode() && (_manager.currentlyConnectedHIDBoardType() == HID_BOARD_TYPE_NEURON))
    {
        _manager.disconnectFromHID();
        _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprocon.bmp"));
        _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronproconhigh.bmp"));
    }
    else
    {
        if(_manager.hidMode() && (_manager.currentlyConnectedHIDBoardType() == HID_BOARD_TYPE_MUSCLE))
        {
            _manager.disconnectFromHID();
            _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprocon.bmp"));
            _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgproconhigh.bmp"));
            while(_manager.currentlyConnectedHIDBoardType() != HID_BOARD_TYPE_NONE)
            {

            }
        }

        if(_manager.serialMode())
        {
            _manager.setSerialNumberOfChannels(1);
            _manager.disconnectFromSerial();
        }

        if(_manager.fileMode()) { // end file mode when in file mode
            //delete _anaView;
            _manager.initRecordingDevices();
            _recordButton->setVisible(true);
            _analysisButton->setVisible(false);
            _anaView = NULL;
        } else {
            _audioView->setOffset(0);
        }

        if(_manager.paused())
        {
            _manager.setPaused(false);
        }

        if(!_manager.initHIDUSB(HID_BOARD_TYPE_NEURON))
        {
            std::cout<<"Can't open HID Muscle device. \n";


            Widgets::ErrorBox *box = new Widgets::ErrorBox(_manager.hidError.c_str());
            box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
            Widgets::Application::getInstance()->addPopup(box);
        }
        if(_manager.currentlyConnectedHIDBoardType()==HID_BOARD_TYPE_NEURON)
        {
            _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprodiscon.bmp"));
            _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronprodiscon.bmp"));
        }


    }
}
//
// Draws time label (current time and file length) when in file mode
// must be here and not in audio view since audio view shrinks when FFT is opened
//
void MainView::drawTimeLabelsForFile()
{

    int64_t fullTime = _manager.fileLength();
    int64_t fullMiliseconds = fullTime/(_manager.sampleRate()/1000);
    fullMiliseconds = fullMiliseconds%1000;
    int64_t fullSeconds = fullTime/_manager.sampleRate();
    int64_t fullMinutes = fullSeconds/60;
    fullSeconds = fullSeconds%60;
    std::stringstream fullS;
    if(fullMinutes<1)
    {
        fullS<<"00:";
    }
    else if (fullMinutes<10)
    {
        fullS<<"0"<<fullMinutes<<":";
    }
    else
    {
        fullS<<fullMinutes<<":";
    }

    if(fullSeconds<10)
    {
        fullS<<"0"<<fullSeconds<<" ";
    }
    else
    {
        fullS<<fullSeconds<<" ";
    }

    if(fullMiliseconds<10)
    {
        fullS<<"00"<<fullMiliseconds;
    }
    else if (fullMiliseconds<100)
    {
        fullS<<"0"<<fullMiliseconds;
    }
    else
    {
        fullS<<fullMiliseconds;
    }

    Widgets::Painter::setColor(Widgets::Colors::white);
    Widgets::Application::font()->draw(fullS.str().c_str(), width()-15, height()-70, Widgets::AlignRight);





    int64_t time = _manager.pos();
    int64_t miliseconds = time/(_manager.sampleRate()/1000);
    miliseconds = miliseconds%1000;
    int64_t seconds = time/_manager.sampleRate();
    int64_t minutes = seconds/60;
    seconds = seconds%60;
    std::stringstream o;

    if(minutes<1)
    {
        o<<"00:";
    }
    else if (minutes<10)
    {
        o<<"0"<<minutes<<":";
    }
    else
    {
        o<<minutes<<":";
    }

    if(seconds<10)
    {
        o<<"0"<<seconds<<" ";
    }
    else
    {
        o<<seconds<<" ";
    }

    if(miliseconds<10)
    {
        o<<"00"<<miliseconds;
    }
    else if (miliseconds<100)
    {
        o<<"0"<<miliseconds;
    }
    else
    {
        o<<miliseconds;
    }

    Widgets::Application::font()->draw(o.str().c_str(), 15, height()-70, Widgets::AlignLeft);

}





void MainView::paintEvent()
{


    if(_manager.shouldStartFirmwareUpdatePresentation)
    {
        int typeOfFirmwareUpgradeProcesss = TYPE_MSP430_UPDATER;
        if(_manager.bootloaderState()!=BOOTLOADER_STAGE_OFF)
        {
            typeOfFirmwareUpgradeProcesss = TYPE_STM32_BOOTLOADER;
        }
        FirmwareUpdateView *c = new FirmwareUpdateView(_manager, *_audioView, typeOfFirmwareUpgradeProcesss, nullptr);
        c->setDeleteOnClose(true);
        c->setGeometry(rect());
        Widgets::Application::getInstance()->addWindow(c);
        _manager.shouldStartFirmwareUpdatePresentation = false;
    }

    if(_manager.isHIDBoardTypeAvailable(HID_BOARD_TYPE_MUSCLE))
    {
        _muscleHIDButton->setSizeHint(Widgets::Size(53,48));
        if(_muscleHIDButton->isHidden())
        {

            _muscleHIDButton->setVisible(true);
            if(!_manager.ignoreHIDReconnect())
            {
                    muscleHIDPressed();
            }
        }

        Widgets::Application::getInstance()->updateLayout();
    }
    else
    {
        _muscleHIDButton->setVisible(false);
        _muscleHIDButton->setSizeHint(Widgets::Size(0,0));
        Widgets::Application::getInstance()->updateLayout();
    }

    if(_manager.isHIDBoardTypeAvailable(HID_BOARD_TYPE_NEURON))
    {
        _neuronHIDButton->setSizeHint(Widgets::Size(53,48));
        if(_neuronHIDButton->isHidden())
        {
            _neuronHIDButton->setVisible(true);
            if(!_manager.ignoreHIDReconnect())
            {
                neuronHIDPressed();
            }
        }

        Widgets::Application::getInstance()->updateLayout();
    }
    else
    {
        _neuronHIDButton->setVisible(false);
        _neuronHIDButton->setSizeHint(Widgets::Size(0,0));
        Widgets::Application::getInstance()->updateLayout();
    }

    //-------------- Check if we have new serial buttons ----------------------------------------------------------
    //-------------- Compare buttons and ports in “_portScanningArduinoSerial.ports” ----------------

    //get ports from “_portScanningArduinoSerial.ports”
    std::list<ArduinoSerial::SerialPort> sps =  _manager.serailPorts();
    std::list<ArduinoSerial::SerialPort>::iterator it;


    std::list<ArduinoSerial::SerialPort>::iterator serialPortsIterator;
    std::list<SerialPortIndicator> ::iterator buttonsIterator;

    bool thereWasChange = false;

    //--------------------- add new serial buttons ------------------------------------
    //first check if we have button for all serial ports
    //and refresh serial port data
    SerialPortIndicator lastNewButton;
    bool foundButtonForPort;
    for(serialPortsIterator = sps.begin();serialPortsIterator!=sps.end();serialPortsIterator++)
    {
        if(serialPortsIterator->deviceType == ArduinoSerial::unknown)
        {
            continue;
        }
        foundButtonForPort = false;
        for(buttonsIterator = shieldButtons.begin();buttonsIterator!=shieldButtons.end();buttonsIterator++)
        {
            std::size_t found;
            found  = serialPortsIterator->portName.find(buttonsIterator->serialPort.portName);
            if (found!=std::string::npos)
            {
                foundButtonForPort = true;
                buttonsIterator->serialPort =  *serialPortsIterator;
                break;
            }

        }
        //make button if we do not have it
        if(foundButtonForPort == false)
        {
            thereWasChange = true;
            SerialPortIndicator newButtonData;
            newButtonData.serialPort = *serialPortsIterator;
            shieldButtons.push_back(newButtonData);
            lastNewButton = newButtonData;
        }
    }


    //----------------- remove nonexisting serial buttons ----------------------------------
    //than we remove buttons that do not have serial port anymore
    bool foundPortForButton;

    for(buttonsIterator = shieldButtons.begin();buttonsIterator!=shieldButtons.end();buttonsIterator++)
    {
        foundPortForButton = false;
        for(serialPortsIterator = sps.begin();serialPortsIterator!=sps.end();serialPortsIterator++)
        {
            std::size_t found;
            found  = serialPortsIterator->portName.find(buttonsIterator->serialPort.portName);
            if (found!=std::string::npos)
            {
                foundPortForButton = true;
                break;
            }
        }

        if(foundPortForButton==false)
        {
            buttonsIterator = shieldButtons.erase(buttonsIterator);
            buttonsIterator--;
            thereWasChange = true;
        }
    }
    //TODO: if adding device add new type to this places:
    //------------------- prepare to update serial buttons -----------------------------------
    int numberOfMuscle = 0;
    int numberOfHeart = 0;
    int numberOfPlant = 0;
    int numberOfNeuron = 0;
    for(buttonsIterator = shieldButtons.begin();buttonsIterator!=shieldButtons.end();buttonsIterator++)
    {


        if (buttonsIterator->serialPort.deviceType == ArduinoSerial::plant)
        {
            numberOfPlant++;
        }
        if (buttonsIterator->serialPort.deviceType == ArduinoSerial::neuronOneChannel)
        {
            numberOfNeuron++;
        }
        else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::muscle || buttonsIterator->serialPort.deviceType == ArduinoSerial::muscleusb || buttonsIterator->serialPort.deviceType == ArduinoSerial::hhibox ||  buttonsIterator->serialPort.deviceType == ArduinoSerial::sbpromusclecdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuroncdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuronmfi || buttonsIterator->serialPort.deviceType == ArduinoSerial::unibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::extclaw)
        {
            numberOfMuscle++;
        }
        else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::heart || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartOneChannel || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartPro || buttonsIterator->serialPort.deviceType == ArduinoSerial::humansb)
        {
            numberOfHeart++;
        }

    }


    //check if only one USB device is present
    //if yes we will use generic USB icon for any device that
    //is connected to USB
    int numberOfUSBDevicesConnected = 0;
    if(_manager.isHIDBoardTypeAvailable(HID_BOARD_TYPE_MUSCLE) || _manager.currentlyConnectedHIDBoardType()==HID_BOARD_TYPE_MUSCLE)
    {numberOfUSBDevicesConnected++;}
    if(_manager.isHIDBoardTypeAvailable(HID_BOARD_TYPE_NEURON) || _manager.currentlyConnectedHIDBoardType()==HID_BOARD_TYPE_NEURON)
    {numberOfUSBDevicesConnected++;}
    numberOfUSBDevicesConnected += numberOfMuscle;
    numberOfUSBDevicesConnected += numberOfHeart;
    numberOfUSBDevicesConnected += numberOfPlant;
    numberOfUSBDevicesConnected += numberOfNeuron;
    bool showGenericUSBButton = false;
    if(numberOfUSBDevicesConnected==1)
    {
        showGenericUSBButton = true;
    }
   // std::cout<<"Number of USB: "<<numberOfUSBDevicesConnected<<"\n";

    // --------------------- Update all serial buttons -----------------------------

    //Now we have sinchronized port and button list
    //check if we had changes and update screen

    int currentPlant = 0;
    int currentMuscle = 0;
    int currentHeart = 0;
    int currentNeuron = 0;
    if(thereWasChange)
    {
        shieldsButtonBoxLayout->removeAll();
        for(buttonsIterator = shieldButtons.begin();buttonsIterator!=shieldButtons.end();buttonsIterator++)
        {
            Widgets::PushButton * newButton = new Widgets::PushButton(this);
            newButton->setRightPadding(5);
            newButton->setSizeHint(Widgets::Size(53,48));
            newButton->setVisible(true);
//TODO: if adding device add new type to this places:
            if(buttonsIterator->serialPort.deviceType == ArduinoSerial::plant)
            {
                currentPlant++;
                if(currentPlant>6)
                {
                    numberOfPlant = 1;
                }

                if(numberOfPlant>1)
                {
                    std::stringstream s;

                    std::size_t found;
                    found  = _manager.getCurrentPort().portName.find(buttonsIterator->serialPort.portName);
                    if (found!=std::string::npos)
                    {
                        s << "data/dconnp"<<currentPlant<<".bmp";
                    }
                    else
                    {
                        s << "data/connp"<<currentPlant<<".bmp";
                    }
                    newButton->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    newButton->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(showGenericUSBButton)
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                    }
                    else
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/plantcon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/plantconhigh.bmp"));
                    }
                }
                newButton->clickedWithRef.connect(this, &MainView::plantPressed);

            }
            else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::neuronOneChannel)
            {
                currentNeuron++;

                if(currentNeuron>6)
                {
                    numberOfNeuron = 1;
                }

                if(numberOfNeuron>1)
                {
                    std::stringstream s;

                    std::size_t found;
                    found  = _manager.getCurrentPort().portName.find(buttonsIterator->serialPort.portName);
                    if (found!=std::string::npos)
                    {
                        s << "data/dconnn"<<currentNeuron<<".bmp";//TODO
                    }
                    else
                    {
                        s << "data/connn"<<currentNeuron<<".bmp";//TODO
                    }
                    newButton->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    newButton->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(showGenericUSBButton)
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                    }
                    else
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/neuroncon.bmp"));//TODO
                        newButton->setHoverTex(Widgets::TextureGL::get("data/neuroncon.bmp"));//TODO
                    }
                }

                newButton->clickedWithRef.connect(this, &MainView::neuronPressed);

            }
            else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::muscle || buttonsIterator->serialPort.deviceType == ArduinoSerial::muscleusb || buttonsIterator->serialPort.deviceType == ArduinoSerial::hhibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbpromusclecdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuroncdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuronmfi || buttonsIterator->serialPort.deviceType == ArduinoSerial::unibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::extclaw)
            {
                currentMuscle++;

                if(currentMuscle>6)
                {
                    numberOfMuscle = 1;
                }

                if(numberOfMuscle>1)
                {
                    std::stringstream s;

                    std::size_t found;
                    found  = _manager.getCurrentPort().portName.find(buttonsIterator->serialPort.portName);
                    if (found!=std::string::npos)
                    {
                        s << "data/dconnm"<<currentMuscle<<".bmp";
                    }
                    else
                    {
                        s << "data/connm"<<currentMuscle<<".bmp";
                    }
                    newButton->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    newButton->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(showGenericUSBButton)
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                    }
                    else
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/musclecon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/muscleconhigh.bmp"));
                    }
                }

                newButton->clickedWithRef.connect(this, &MainView::musclePressed);

            }
            else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::heart || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartOneChannel || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartPro || buttonsIterator->serialPort.deviceType == ArduinoSerial::humansb)
            {
                currentHeart++;


                if(currentHeart>6)
                {
                    numberOfHeart = 1;
                }

                if(numberOfHeart>1)
                {
                    std::stringstream s;

                    std::size_t found;
                    found  = _manager.getCurrentPort().portName.find(buttonsIterator->serialPort.portName);
                    if (found!=std::string::npos)
                    {
                        s << "data/dconne"<<currentHeart<<".bmp";
                    }
                    else
                    {
                        s << "data/conne"<<currentHeart<<".bmp";
                    }
                    newButton->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    newButton->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(showGenericUSBButton)
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                    }
                    else
                    {
                        newButton->setNormalTex(Widgets::TextureGL::get("data/heartcon.bmp"));
                        newButton->setHoverTex(Widgets::TextureGL::get("data/heartconhigh.bmp"));
                    }
                }

                newButton->clickedWithRef.connect(this, &MainView::heartPressed);

            }
            buttonsIterator->button = newButton;
            shieldsButtonBoxLayout->addWidget(newButton);


            // -------------------------------- Autoconnect to serial ---------------------------------------------
            // If we have new button
            // CONNECT TO IT!!!
            //
            std::size_t foundTemp;
            foundTemp  = lastNewButton.serialPort.portName.find(buttonsIterator->serialPort.portName);
            if (foundTemp!=std::string::npos)
            {
//TODO: if adding device add new type to this places:
                if(buttonsIterator->serialPort.deviceType == ArduinoSerial::plant)
                {
                    plantPressed(NULL, buttonsIterator->button);
                }
                else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::muscle || buttonsIterator->serialPort.deviceType == ArduinoSerial::muscleusb || buttonsIterator->serialPort.deviceType == ArduinoSerial::hhibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbpromusclecdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuroncdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuronmfi || buttonsIterator->serialPort.deviceType == ArduinoSerial::unibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::extclaw)
                {
                    musclePressed(NULL,buttonsIterator->button );
                }
                else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::neuronOneChannel)
                {
                    neuronPressed(NULL,buttonsIterator->button );
                }
                else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::heart || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartOneChannel || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartPro || buttonsIterator->serialPort.deviceType == ArduinoSerial::humansb)
                {
                   heartPressed(NULL,buttonsIterator->button );
                }
            }

        }

        Widgets::Application::getInstance()->updateLayout();
    }
    else//if there was no change just update icons on button
    {
//TODO: if adding device add new type to this places:
        //refresh icons
        for(buttonsIterator = shieldButtons.begin();buttonsIterator!=shieldButtons.end();buttonsIterator++)
        {
            bool buttonIsActive = false;
            std::size_t found;
            found  = _manager.getCurrentPort().portName.find(buttonsIterator->serialPort.portName);
            if (found!=std::string::npos)
            {
                buttonIsActive = true;
            }


            if(buttonsIterator->serialPort.deviceType == ArduinoSerial::plant)
            {
                currentPlant++;
                if(currentPlant>6)
                {
                    numberOfPlant = 1;
                }

                if(numberOfPlant>1)
                {
                    std::stringstream s;


                    if (buttonIsActive)
                    {
                        s << "data/dconnp"<<currentPlant<<".bmp";
                    }
                    else
                    {
                        s << "data/connp"<<currentPlant<<".bmp";
                    }
                    buttonsIterator->button->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    buttonsIterator->button->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(buttonIsActive)
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/plantdiscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/plantdiscon.bmp"));
                        }
                    }
                    else
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/plantcon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/plantconhigh.bmp"));
                        }
                    }

                }

            }
            else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::neuronOneChannel)
            {
                currentNeuron++;

                if(currentNeuron>6)
                {
                    numberOfNeuron = 1;
                }

                if(numberOfNeuron>1)
                {
                    std::stringstream s;


                    if (buttonIsActive)
                    {
                        s << "data/dconnn"<<currentNeuron<<".bmp";//TODO
                    }
                    else
                    {
                        s << "data/connn"<<currentNeuron<<".bmp";//TODO
                    }
                    buttonsIterator->button->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    buttonsIterator->button->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(buttonIsActive)
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/neurondiscon.bmp"));//TODO
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/neurondiscon.bmp"));//TODO
                        }
                    }
                    else
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/neuroncon.bmp"));//TODO
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/neuroncon.bmp"));//TODO
                        }
                    }
                }


            }
            else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::muscle || buttonsIterator->serialPort.deviceType == ArduinoSerial::muscleusb || buttonsIterator->serialPort.deviceType == ArduinoSerial::hhibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbpromusclecdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuroncdc || buttonsIterator->serialPort.deviceType == ArduinoSerial::sbproneuronmfi || buttonsIterator->serialPort.deviceType == ArduinoSerial::unibox || buttonsIterator->serialPort.deviceType == ArduinoSerial::extclaw)
            {
                currentMuscle++;

                if(currentMuscle>6)
                {
                    numberOfMuscle = 1;
                }

                if(numberOfMuscle>1)
                {
                    std::stringstream s;


                    if (buttonIsActive)
                    {
                        s << "data/dconnm"<<currentMuscle<<".bmp";
                    }
                    else
                    {
                        s << "data/connm"<<currentMuscle<<".bmp";
                    }
                    buttonsIterator->button->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    buttonsIterator->button->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(buttonIsActive)
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/musclediscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/musclediscon.bmp"));
                        }
                    }
                    else
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/musclecon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/muscleconhigh.bmp"));
                        }
                    }
                }


            }
            else if(buttonsIterator->serialPort.deviceType == ArduinoSerial::heart || buttonsIterator->serialPort.deviceType == ArduinoSerial::heartOneChannel ||  buttonsIterator->serialPort.deviceType == ArduinoSerial::heartPro ||  buttonsIterator->serialPort.deviceType == ArduinoSerial::humansb)
            {
                currentHeart++;


                if(currentHeart>6)
                {
                    numberOfHeart = 1;
                }

                if(numberOfHeart>1)
                {
                    std::stringstream s;


                    if (buttonIsActive)
                    {
                        s << "data/dconne"<<currentHeart<<".bmp";
                    }
                    else
                    {
                        s << "data/conne"<<currentHeart<<".bmp";
                    }
                    buttonsIterator->button->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
                    buttonsIterator->button->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));

                }
                else
                {
                    if(buttonIsActive)
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/heartdiscon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/heartdiscon.bmp"));
                        }
                    }
                    else
                    {
                        if(showGenericUSBButton)
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
                        }
                        else
                        {
                            buttonsIterator->button->setNormalTex(Widgets::TextureGL::get("data/heartcon.bmp"));
                            buttonsIterator->button->setHoverTex(Widgets::TextureGL::get("data/heartconhigh.bmp"));
                        }
                    }
                }

            }


        }
    }

    //------ P300 on Human SB control buttons -----
    if(_manager.serialMode() && _manager.getCurrentPort().deviceType == ArduinoSerial::humansb)
    {
        _p300Button->setVisible(true);
        if(_manager.getP300HardwareStatus())
        {
            _p300Button->setNormalTex(Widgets::TextureGL::get("data/p300selected.bmp"));
            _p300Button->setHoverTex(Widgets::TextureGL::get("data/p300selected.bmp"));
            _p300AudioButton->setVisible(true);
            if(_manager.getP300AudioStimulationHardwareStatus())
            {
                _p300AudioButton->setNormalTex(Widgets::TextureGL::get("data/p300audio-selected.bmp"));
                _p300AudioButton->setHoverTex(Widgets::TextureGL::get("data/p300audio-selected.bmp"));
            }
            else
            {
                _p300AudioButton->setNormalTex(Widgets::TextureGL::get("data/p300audio-normal.bmp"));
                _p300AudioButton->setHoverTex(Widgets::TextureGL::get("data/p300audio-high.bmp"));
            }
        }
        else
        {
            _p300Button->setNormalTex(Widgets::TextureGL::get("data/p300normal.bmp"));
            _p300Button->setHoverTex(Widgets::TextureGL::get("data/p300high.bmp"));
            _p300AudioButton->setVisible(false);
        }
    }
    else
    {
        _p300Button->setVisible(false);
        _p300AudioButton->setVisible(false);
    }

    
    //------- alpha wave feedback -----------------
    
    if(_fftView->active() && _manager.serialMode())
    {
        _alphaFeedbackButton->setVisible(true);

        _manager.alphaWavePower = _anaman.fft.lowPassAlphaWaves;

        if(SDL_GetTicks() - timerForAlphaWaveMessages>150)
        {
            timerForAlphaWaveMessages = SDL_GetTicks();
            _manager.sendAlphaWavePowerToSerial();
        }


        if(alphaFeedbackAcive)
        {

            _manager.turnAlphaFeedbackON();

        }
        else
        {
            _manager.turnAlphaFeedbackOFF();
        }
    }
    else
    {
        _manager.turnAlphaFeedbackOFF();
        _alphaFeedbackButton->setVisible(false);
    }

    
    //-------------- HID buttons connect/disconnect------------
    
    
    if(_manager.hidMode())
    {


        if(_manager.currentlyConnectedHIDBoardType()==HID_BOARD_TYPE_MUSCLE)
        {
            if(showGenericUSBButton)
            {
                _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
            }
            else
            {
                _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprodiscon.bmp"));
                _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgprodiscon.bmp"));
            }
            _muscleHIDButton->setSizeHint(Widgets::Size(53,48));
            _muscleHIDButton->setVisible(true);
        }
        else
        {
            if(showGenericUSBButton)
            {
                _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
                _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
            }
            else
            {
                _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprodiscon.bmp"));
                _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronprodiscon.bmp"));
            }
            _neuronHIDButton->setSizeHint(Widgets::Size(53,48));
            _neuronHIDButton->setVisible(true);
        }
        Widgets::Application::getInstance()->updateLayout();


    }
    else
    {
        if(showGenericUSBButton)
        {
            _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
            _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
            _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
            _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
        }
        else
        {
            _muscleHIDButton->setNormalTex(Widgets::TextureGL::get("data/emgprocon.bmp"));
            _muscleHIDButton->setHoverTex(Widgets::TextureGL::get("data/emgproconhigh.bmp"));
            _neuronHIDButton->setNormalTex(Widgets::TextureGL::get("data/neuronprocon.bmp"));
            _neuronHIDButton->setHoverTex(Widgets::TextureGL::get("data/neuronproconhigh.bmp"));
        }
    }

    if(_manager.fileMode())
    {
        drawTimeLabelsForFile();
    }
}


void MainView::keyPressEvent(Widgets::KeyboardEvent *e) {

	if(e->key() >= Widgets::Key0 && e->key() <= Widgets::Key9) {
		int mnum = e->key()-Widgets::Key0;
		int64_t offset = 0;
		if(!_manager.fileMode())
			offset = _audioView->offset();
		_manager.addMarker(std::string(1, mnum+'0'), offset);

	}
	if(!_manager.threshMode())
    {
        if(e->key()==275 || e->key() == 1073741903)//right 37
        {
            if(_manager.fileMode())
            {
                _audioView->navigateFilePosition(true);
            }
            else
            {
                if(_manager.paused())
                {
                    _audioView->navigateCurrentRecordingPosition(true);
                }
                else
                {
                    forwardPressed();
                }
            }
        }
        if(e->key()==276 || e->key()== 1073741904)//left 39
        {
            if(_manager.fileMode())
            {
                _audioView->navigateFilePosition(false);
            }
            else
            {
                if(_manager.paused())
                {
                    _audioView->navigateCurrentRecordingPosition(false);
                }
                else
                {
                    backwardPressed();
                }

            }
        }
    }

}

}
