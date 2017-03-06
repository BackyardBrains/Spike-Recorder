#include "MainView.h"
#include "widgets/Application.h"
#include "widgets/PushButton.h"
#include "widgets/BoxLayout.h"
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

#if defined(_WIN32)
#include "FirmwareUpdateView.h"
#endif // defined

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

	_usbButton = new Widgets::PushButton(this);
	_usbButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
	_usbButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
	_usbButton->clicked.connect(this, &MainView::usbPressed);
	_usbButton->setVisible(false);
	_usbButton->setRightPadding(5);
	_usbButton->setSizeHint(Widgets::Size(0,0));


    _plantSSButton = new Widgets::PushButton(this);
    _plantSSButton->setNormalTex(Widgets::TextureGL::get("data/plantcon.bmp"));
    _plantSSButton->setHoverTex(Widgets::TextureGL::get("data/plantconhigh.bmp"));
    _plantSSButton->clicked.connect(this, &MainView::plantPressed);
    _plantSSButton->setVisible(false);
    _plantSSButton->setRightPadding(5);
    _plantSSButton->setSizeHint(Widgets::Size(0,0));


    _muscleSSButton = new Widgets::PushButton(this);
    _muscleSSButton->setNormalTex(Widgets::TextureGL::get("data/musclecon.bmp"));
    _muscleSSButton->setHoverTex(Widgets::TextureGL::get("data/muscleconhigh.bmp"));
    _muscleSSButton->clicked.connect(this, &MainView::musclePressed);
    _muscleSSButton->setVisible(false);
    _muscleSSButton->setRightPadding(5);
    _muscleSSButton->setSizeHint(Widgets::Size(0,0));


    _heartSSButton = new Widgets::PushButton(this);
    _heartSSButton->setNormalTex(Widgets::TextureGL::get("data/heartcon.bmp"));
    _heartSSButton->setHoverTex(Widgets::TextureGL::get("data/heartconhigh.bmp"));
    _heartSSButton->clicked.connect(this, &MainView::heartPressed);
    _heartSSButton->setVisible(false);
    _heartSSButton->setRightPadding(5);
    _heartSSButton->setSizeHint(Widgets::Size(0,0));

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



	Widgets::PushButton * const backwardButton = new Widgets::PushButton(this);
	backwardButton->setNormalTex(Widgets::TextureGL::get("data/backward.bmp"));
	backwardButton->setHoverTex(Widgets::TextureGL::get("data/backwardhigh.bmp"));
	backwardButton->setSizeHint(Widgets::Size(32,32));
	backwardButton->clicked.connect(this, &MainView::backwardPressed);

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
    topBar->addWidget(_usbButton);
    topBar->addWidget(_plantSSButton);
    topBar->addWidget(_muscleSSButton);
    topBar->addWidget(_heartSSButton);
	//topBar->addSpacing(5);
	topBar->addWidget(_analysisButton);
    topBar->addSpacing(5);
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

	seekBarBox->addWidget(backwardButton,Widgets::AlignVCenter);
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

		char buf[64];
		time_t t = time(NULL);
		strftime(buf, sizeof(buf), "%Y-%m-%d_%H.%M.%S.wav", localtime(&t));
		std::string filename = getRecordingPath()+"/BYB_Recording_"+buf;

		if(!_fileRec.start(filename.c_str())) {
			const char *error = strerror(errno);
			std::stringstream s;
			s << "Error: Failed to open '" << filename.c_str() << "' for recording: " << error << ".";
			Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
			box->setGeometry(Widgets::Rect(this->width()/2-200, this->height()/2-40, 400, 80));
			Widgets::Application::getInstance()->addPopup(box);
			return;
		}
		_configButton->setSizeHint(Widgets::Size(0, 48));
		_configButton->setVisible(false);
		_fileButton->setSizeHint(Widgets::Size(0, 48));
		_fileButton->setVisible(false);
		_recBar->setActive(true);
	} else {
		MetadataChunk m;
		_audioView->constructMetadata(&m);
		_manager.constructMetadata(&m);

		_fileRec.stop(&m);
		_configButton->setVisible(true);
		_configButton->setSizeHint(Widgets::Size(48, 48));
		_fileButton->setVisible(true);
		_fileButton->setSizeHint(Widgets::Size(48, 48));
		_recBar->setActive(false);

		std::stringstream s;
		s << "Saved recording to '" << _fileRec.filename() << "'.";
		Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
		box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
		Widgets::Application::getInstance()->addPopup(box);
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
    _manager.refreshSerialPorts();
    Log::msg("Create config view...");
	ConfigView *c = new ConfigView(_manager, *_audioView);
	Log::msg("Set mouse track...");
	c->setMouseTracking(true);
	c->setDeleteOnClose(true);
	c->setGeometry(rect());
	Log::msg("Add Config Window...");
	Widgets::Application::getInstance()->addWindow(c);



}

void MainView::analysisPressed() {


    _anaView = new AnalysisView(_manager, _anaman);


	_anaView->setDeleteOnClose(true);
	_anaView->setGeometry(rect());
	Widgets::Application::getInstance()->addWindow(_anaView);

	if(!_manager.paused())
		pausePressed();
}

    void MainView::plantPressed()
    {
        connectToFirstShieldOfType(ArduinoSerial::plant);
    }



    void MainView::musclePressed()
    {
        connectToFirstShieldOfType(ArduinoSerial::muscle);
    }
    void MainView::heartPressed()
    {
        connectToFirstShieldOfType(ArduinoSerial::heart);
    }



    void MainView::connectToFirstShieldOfType(ArduinoSerial::SerialDevice deviceType)
    {
        std::list<ArduinoSerial::SerialPort> sps =  _manager.serailPorts();
        std::list<ArduinoSerial::SerialPort>::iterator it;
        int portIndex = 0;
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
                        _manager.setSerialNumberOfChannels(1);
                        _manager.disconnectFromSerial();
                    }
                    break;
                }
                //if we want to turn on some different shield
                if(_manager.serialMode())
                {
                    _manager.setSerialNumberOfChannels(1);
                    _manager.disconnectFromSerial();
                }

                bool connected = _manager.initSerial(it->portName.c_str());
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




    }

void MainView::usbPressed()
{
    //connect/diconnect

    if(_manager.hidMode())
    {
        // _manager.setSerialNumberOfChannels(1);
        _manager.disconnectFromHID();
        _usbButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
        _usbButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));

    }
    else
    {
        if(_manager.serialMode())
        {
            _manager.setSerialNumberOfChannels(1);
            _manager.disconnectFromSerial();
        }
        
        if(_manager.fileMode()) { // end file mode when in file mode
            delete _anaView;
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

        if(!_manager.initHIDUSB())
        {
            std::cout<<"Can't open HID device. \n";


            Widgets::ErrorBox *box = new Widgets::ErrorBox(_manager.hidError.c_str());
            box->setGeometry(Widgets::Rect(this->width()/2-250, this->height()/2-40, 500, 80));
            Widgets::Application::getInstance()->addPopup(box);
        }
        _usbButton->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
        _usbButton->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));

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


    #if defined(_WIN32)
    if(_manager.shouldStartFirmwareUpdatePresentation)
    {
        FirmwareUpdateView *c = new FirmwareUpdateView(_manager, *_audioView);
        c->setDeleteOnClose(true);
        c->setGeometry(rect());
        Widgets::Application::getInstance()->addWindow(c);
        _manager.shouldStartFirmwareUpdatePresentation = false;
    }
    #endif
    if(_manager.hidDevicePresent())
    {
        //_usbButton->setSizeHint(Widgets::Size(48,48));
        _usbButton->setSizeHint(Widgets::Size(53,48));
        _usbButton->setVisible(true);
        Widgets::Application::getInstance()->updateLayout();

    }
    else
    {
        _usbButton->setVisible(false);
        _usbButton->setSizeHint(Widgets::Size(0,0));
    }


    std::list<ArduinoSerial::SerialPort> sps =  _manager.serailPorts();
    std::list<ArduinoSerial::SerialPort>::iterator it;
    int tempIndex = 0;
    bool plantExis = false;
    bool muscleExist = false;
    bool heartExis = false;

    for(it = sps.begin();it!=sps.end();it++)
    {
        if(it->deviceType == ArduinoSerial::plant)
        {
            plantExis = true;
        }
        if(it->deviceType == ArduinoSerial::muscle)
        {
            muscleExist = true;
        }
        if(it->deviceType == ArduinoSerial::heart)
        {
            heartExis = true;
        }
    }

    if(plantExis)
    {

        if(_manager.serialMode() && _manager.getCurrentPort().deviceType == ArduinoSerial::plant)
        {
            _plantSSButton->setNormalTex(Widgets::TextureGL::get("data/plantdiscon.bmp"));
            _plantSSButton->setHoverTex(Widgets::TextureGL::get("data/plantdiscon.bmp"));
        }
        else
        {
            _plantSSButton->setNormalTex(Widgets::TextureGL::get("data/plantcon.bmp"));
            _plantSSButton->setHoverTex(Widgets::TextureGL::get("data/plantconhigh.bmp"));
        }
        _plantSSButton->setSizeHint(Widgets::Size(53,48));
        _plantSSButton->setVisible(true);
        Widgets::Application::getInstance()->updateLayout();
    }
    else
    {
        _plantSSButton->setVisible(false);
        _plantSSButton->setSizeHint(Widgets::Size(0,0));
    }

    if(muscleExist)
    {
        if(_manager.serialMode() && _manager.getCurrentPort().deviceType == ArduinoSerial::muscle)
        {
            _muscleSSButton->setNormalTex(Widgets::TextureGL::get("data/musclediscon.bmp"));
            _muscleSSButton->setHoverTex(Widgets::TextureGL::get("data/musclediscon.bmp"));
        }
        else
        {
            _muscleSSButton->setNormalTex(Widgets::TextureGL::get("data/musclecon.bmp"));
            _muscleSSButton->setHoverTex(Widgets::TextureGL::get("data/muscleconhigh.bmp"));
        }


        _muscleSSButton->setSizeHint(Widgets::Size(53,48));
        _muscleSSButton->setVisible(true);
        Widgets::Application::getInstance()->updateLayout();
    }
    else
    {
        _muscleSSButton->setVisible(false);
        _muscleSSButton->setSizeHint(Widgets::Size(0,0));
    }

    if(heartExis)
    {
        if(_manager.serialMode() && _manager.getCurrentPort().deviceType == ArduinoSerial::heart)
        {
            _heartSSButton->setNormalTex(Widgets::TextureGL::get("data/heartdiscon.bmp"));
            _heartSSButton->setHoverTex(Widgets::TextureGL::get("data/heartdiscon.bmp"));
        }
        else
        {
            _heartSSButton->setNormalTex(Widgets::TextureGL::get("data/heartcon.bmp"));
            _heartSSButton->setHoverTex(Widgets::TextureGL::get("data/heartconhigh.bmp"));
        }



        _heartSSButton->setSizeHint(Widgets::Size(53,48));
        _heartSSButton->setVisible(true);
        Widgets::Application::getInstance()->updateLayout();
    }
    else
    {
        _heartSSButton->setVisible(false);
        _heartSSButton->setSizeHint(Widgets::Size(0,0));
    }


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

    if(_manager.hidMode())
    {
        _usbButton->setNormalTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));
        _usbButton->setHoverTex(Widgets::TextureGL::get("data/usbdiscon.bmp"));

        _usbButton->setSizeHint(Widgets::Size(53,48));
        //_usbButton->setSizeHint(Widgets::Size(48,48));
        _usbButton->setVisible(true);
        Widgets::Application::getInstance()->updateLayout();

    }
    else
    {
        _usbButton->setNormalTex(Widgets::TextureGL::get("data/usbcon.bmp"));
        _usbButton->setHoverTex(Widgets::TextureGL::get("data/usbconhigh.bmp"));
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
