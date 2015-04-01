#include "MainView.h"
#include "widgets/Application.h"
#include "widgets/PushButton.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/ScrollBar.h"
#include "widgets/FileDialog.h"
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"
#include "widgets/ToolTip.h"
#include "engine/SpikeSorter.h"
#include "engine/FileRecorder.h"
#include "Paths.h"
#include "MainView.h"
#include "AudioView.h"
#include "ConfigView.h"
#include "AnalysisView.h"
#include "RecordingBar.h"
#include "Log.h"
#include "FFTView.h"
#include <SDL_opengl.h>
#include <SDL.h>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <sstream>

namespace BackyardBrains {

Widgets::Widget *MainView::makeThreshavgGroup() {
 	Widgets::Widget *group = new Widgets::Widget(this);
 	group->setVisible(false);


	Widgets::ScrollBar *bar = new Widgets::ScrollBar(Widgets::Horizontal,group);
	bar->setRange(1,50);
	bar->setPageStep(5);
	bar->valueChanged.connect(&_manager, &RecordingManager::setThreshAvgCount);
	bar->setSizeHint(Widgets::Size(250,20));
	bar->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Maximum));

	Widgets::Label *label = new Widgets::Label(group);
	label->setText("00");
	label->updateSize();
	bar->valueChanged.connect(label, &Widgets::Label::setText);
	bar->setValue(1);

	Widgets::BoxLayout *layout = new Widgets::BoxLayout(Widgets::Horizontal, group);
	layout->addWidget(bar);
	layout->addSpacing(10);
	layout->addWidget(label, Widgets::AlignVCenter);

	int width = bar->sizeHint().w+label->sizeHint().w;

	group->setSizeHint(Widgets::Size(width, 20));
	bar->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Expanding));

	return group;
}

MainView::MainView(RecordingManager &mngr, FileRecorder &fileRec, Widget *parent) : Widget(parent), _manager(mngr), _fileRec(fileRec), _anaView(NULL) {
	_audioView = new AudioView(this, _manager);

	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_manager.deviceReload.connect(_audioView, &AudioView::standardSettings);

	_configButton = new Widgets::PushButton(this);
	_configButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	_configButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));
	_configButton->clicked.connect(this, &MainView::configPressed);
	Widgets::PushButton *threshButton = new Widgets::PushButton(this);
	threshButton->setNormalTex(Widgets::TextureGL::get("data/thresh.png"));
	threshButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.png"));
	threshButton->clicked.connect(this, &MainView::threshPressed);

	_analysisButton = new Widgets::PushButton(this);
	_analysisButton->setNormalTex(Widgets::TextureGL::get("data/analysis.png"));
	_analysisButton->setHoverTex(Widgets::TextureGL::get("data/analysishigh.png"));
	_analysisButton->clicked.connect(this, &MainView::analysisPressed);
	_analysisButton->setVisible(false);

	_recordButton = new Widgets::PushButton(this);
	_recordButton->setNormalTex(Widgets::TextureGL::get("data/rec.png"));
	_recordButton->setHoverTex(Widgets::TextureGL::get("data/rechigh.png"));
	_recordButton->clicked.connect(this, &MainView::recordPressed);

	_fftButton = new Widgets::PushButton(this);
	_fftButton->setNormalTex(Widgets::TextureGL::get("data/fft.png"));
	_fftButton->setHoverTex(Widgets::TextureGL::get("data/ffthigh.png"));
	_fftButton->clicked.connect(this, &MainView::fftPressed);

	_fileButton = new Widgets::PushButton(this);
	_fileButton->setNormalTex(Widgets::TextureGL::get("data/file.png"));
	_fileButton->setHoverTex(Widgets::TextureGL::get("data/filehigh.png"));
	_fileButton->clicked.connect(this, &MainView::filePressed);

	_pauseButton = new Widgets::PushButton(this);
	_pauseButton->clicked.connect(this, &MainView::pausePressed);
	_manager.pauseChanged.connect(this, &MainView::pausePressed);
	_pauseButton->setNormalTex(Widgets::TextureGL::get("data/pause.png"));
	_pauseButton->setHoverTex(Widgets::TextureGL::get("data/pausehigh.png"));
	_pauseButton->setSizeHint(Widgets::Size(64,64));

	Widgets::PushButton * const backwardButton = new Widgets::PushButton(this);
	backwardButton->setNormalTex(Widgets::TextureGL::get("data/backward.png"));
	backwardButton->setHoverTex(Widgets::TextureGL::get("data/backwardhigh.png"));
	backwardButton->setSizeHint(Widgets::Size(32,32));
	backwardButton->clicked.connect(this, &MainView::backwardPressed);

	_forwardButton = new Widgets::PushButton(this);
	_forwardButton->setNormalTex(Widgets::TextureGL::get("data/forward.png"));
	_forwardButton->setHoverTex(Widgets::TextureGL::get("data/forwardhigh.png"));
	_forwardButton->setSizeHint(Widgets::Size(32,32));
	_forwardButton->clicked.connect(this, &MainView::forwardPressed);

	_seekBar = new Widgets::ScrollBar(Widgets::Horizontal, this);
	_seekBar->setVisible(false);
	_seekBar->setRange(0,1000);
	_seekBar->setPageStep(25);
	_seekBar->setValue(1000);
	_seekBar->valueChanged.connect(_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(_seekBar, &Widgets::ScrollBar::updateValue);

	_threshavgGroup = makeThreshavgGroup();

	_recBar = new RecordingBar(_fileRec, this);
	
	_fftView = new FFTView(*_audioView, _manager, this);

	Widgets::BoxLayout *topBar = new Widgets::BoxLayout(Widgets::Horizontal);
	topBar->addSpacing(10);
	topBar->addWidget(_configButton);
	topBar->addSpacing(5);
	topBar->addWidget(threshButton);
	topBar->addSpacing(5);
	topBar->addWidget(_fftButton);
	topBar->addSpacing(5);
	topBar->addWidget(_analysisButton);
	topBar->addSpacing(10);
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
// 	Widgets::Application::getInstance()->updateLayout();

	_audioView->standardSettings();

}

MainView::~MainView() {
	delete _anaView;
}

void MainView::pausePressed() {
	if(_manager.paused()) {
		_manager.setPaused(false);
		_pauseButton->setNormalTex(Widgets::TextureGL::get("data/pause.png"));
		_pauseButton->setHoverTex(Widgets::TextureGL::get("data/pausehigh.png"));
		_seekBar->setVisible(false);
	} else {
		_manager.setPaused(true);
		_pauseButton->setNormalTex(Widgets::TextureGL::get("data/play.png"));
		_pauseButton->setHoverTex(Widgets::TextureGL::get("data/playhigh.png"));
		_seekBar->setVisible(true);
	}
}

void MainView::backwardPressed() {
	_audioView->setOffset(_audioView->offset()-5*_manager.sampleRate());
	if(_manager.paused())
		pausePressed();
}
void MainView::forwardPressed() {
	if(_manager.fileMode()) { // end file mode when in file mode
		_manager.initRecordingDevices();
		_recordButton->setVisible(true);
		_analysisButton->setVisible(false);
		delete _anaView;
		_anaView = NULL;
	} else {
		_audioView->setOffset(0);
	}

	if(_manager.paused())
		pausePressed();
}

void MainView::threshPressed() {
	if(!_manager.threshMode()) {
		_fftView->setActive(false);
		_manager.setThreshMode(true);
		_threshavgGroup->setVisible(true);
		_fftButton->setVisible(false);
	} else {
		_manager.setThreshMode(false);
		_fftButton->setVisible(true);
		_threshavgGroup->setVisible(false);
	}
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
	} else {
		_fftView->setActive(true);
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
	_fftView->setActive(false);

	Widgets::ToolTip *tip = new Widgets::ToolTip("Click to return to live mode \x1f", 2000);
	tip->setGeometry(Widgets::Rect(width()/2-190, height()-150, 280, 40));
	tip->setMouseTracking(false);
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

void MainView::keyPressEvent(Widgets::KeyboardEvent *e) {
	if(e->key() >= Widgets::Key0 && e->key() <= Widgets::Key9) {
		int mnum = e->key()-Widgets::Key0;
		int64_t offset = 0;
		if(!_manager.fileMode())
			offset = _audioView->offset();
		_manager.addMarker(std::string(1, mnum+'0'), offset);
	}
}

}
