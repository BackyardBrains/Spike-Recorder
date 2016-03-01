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

#include <sstream>

#if defined(_WIN32)
#include "FirmwareUpdateView.h"
#endif // defined

namespace BackyardBrains {


MainView::MainView(RecordingManager &mngr, AnalysisManager &anaman, FileRecorder &fileRec, Widget *parent) : Widget(parent), _manager(mngr), _anaman(anaman), _fileRec(fileRec), _anaView(NULL) {
	_audioView = new AudioView(this, _manager);

	    //setup timer that will periodicaly check for USB HID devices

	timerUSB = clock();
	_manager.scanForHIDDevices();
	_manager.triggered.connect(this,&MainView::triggerEvent);
	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_manager.deviceReload.connect(_audioView, &AudioView::standardSettings);


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
	_seekBar->setValue(1000);
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
	//topBar->addSpacing(5);
	topBar->addWidget(_analysisButton);
    topBar->addSpacing(5);
	topBar->addWidget(_threshavgGroup, Widgets::AlignVCenter);
	topBar->addStretch();
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

void MainView::backwardPressed() {
	_audioView->setOffset(_audioView->offset()-5*_manager.sampleRate());
	if(_manager.paused())
		pausePressed();
}

void MainView::ekgPressed() {
}

void MainView::forwardPressed() {
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
		pausePressed();
}

void MainView::threshPressed() {
	if(!_manager.threshMode()) {

        threshButton->setNormalTex(Widgets::TextureGL::get("data/threshcrossed.bmp"));
        threshButton->setHoverTex(Widgets::TextureGL::get("data/threshcrossed.bmp"));
		_fftView->setActive(false);
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

	delete _anaView;
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
		FileRecorder::parseMetadataStr(&m, mdatastr);
	FileRecorder::parseMarkerTextFile(m.markers, FileRecorder::eventTxtFilename(d.getResultFilename().c_str()), _manager.sampleRate());
	_manager.applyMetadata(m);
	_audioView->applyMetadata(m);

	_recordButton->setVisible(false);
	_analysisButton->setVisible(true);
	//_analysisButton->setSizeHint(Widgets::Size(48,48));
	_fftView->setActive(false);

	Widgets::ToolTip *tip = new Widgets::ToolTip("Click to return to live mode \x1f", 2000);
	tip->setGeometry(Widgets::Rect(width()/2-190, height()-150, 280, 40));
	tip->setMouseTracking(false);
	Widgets::Application::getInstance()->updateLayout();
	Widgets::Application::getInstance()->addPopup(tip);
}

void MainView::configPressed() {
    _manager.refreshSerialPorts();
	ConfigView *c = new ConfigView(_manager, *_audioView);
	c->setDeleteOnClose(true);
	c->setGeometry(rect());
	Widgets::Application::getInstance()->addWindow(c);
}

void MainView::analysisPressed() {
	if(_anaView == NULL)
		_anaView = new AnalysisView(_manager);

	_anaView->setDeleteOnClose(false);
	_anaView->setGeometry(rect());
	Widgets::Application::getInstance()->addWindow(_anaView);

	if(!_manager.paused())
		pausePressed();
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
        if(e->key()==275)//right 37
        {
            if(_manager.fileMode())
            {
                _audioView->navigateFilePosition(true);
            }
            else
            {
                forwardPressed();
            }
        }
        if(e->key()==276)//let 39
        {
            if(_manager.fileMode())
            {
                _audioView->navigateFilePosition(false);
            }
            else
            {
                backwardPressed();
            }
        }
    }

}

}
