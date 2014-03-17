#include "Game.h"
#include "widgets/PushButton.h"
#include "widgets/DropDownList.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/ScrollBar.h"
#include "widgets/FileDialog.h"
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"
#include "AudioView.h"
#include "ConfigView.h"
#include "RecordingBar.h"
#include <SDL_opengl.h>
#include <SDL.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sstream>

namespace BackyardBrains {

Widgets::Widget *Game::makeThreshavgGroup() {
 	Widgets::Widget *group = new Widgets::Widget(mainWidget());
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

Game::Game() : _fileRec(_manager) {
	std::cout << "Loading Resources...\n";
	loadResources();
	std::cout << "Creating Window...\n";
	createWindow(800,600);

	_audioView = new AudioView(mainWidget(), _manager);

	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_manager.deviceReload.connect(_audioView, &AudioView::standardSettings);

	_configButton = new Widgets::PushButton(mainWidget());
	_configButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	_configButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));
	_configButton->clicked.connect(this, &Game::configPressed);
	Widgets::PushButton *threshButton = new Widgets::PushButton(mainWidget());
	threshButton->setNormalTex(Widgets::TextureGL::get("data/thresh.png"));
	threshButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.png"));
	threshButton->clicked.connect(this, &Game::threshPressed);
	_recordButton = new Widgets::PushButton(mainWidget());
	_recordButton->setNormalTex(Widgets::TextureGL::get("data/rec.png"));
	_recordButton->setHoverTex(Widgets::TextureGL::get("data/rechigh.png"));
	_recordButton->clicked.connect(this, &Game::recordPressed);
	_fileButton = new Widgets::PushButton(mainWidget());
	_fileButton->setNormalTex(Widgets::TextureGL::get("data/file.png"));
	_fileButton->setHoverTex(Widgets::TextureGL::get("data/filehigh.png"));
	_fileButton->clicked.connect(this, &Game::filePressed);

	_pauseButton = new Widgets::PushButton(mainWidget());
	_pauseButton->clicked.connect(this, &Game::pausePressed);
	_manager.pauseChanged.connect(this, &Game::pausePressed);
	_pauseButton->setNormalTex(Widgets::TextureGL::get("data/pause.png"));
	_pauseButton->setHoverTex(Widgets::TextureGL::get("data/pausehigh.png"));
	_pauseButton->setSizeHint(Widgets::Size(64,64));

	Widgets::PushButton * const backwardButton = new Widgets::PushButton(mainWidget());
	backwardButton->setNormalTex(Widgets::TextureGL::get("data/backward.png"));
	backwardButton->setHoverTex(Widgets::TextureGL::get("data/backwardhigh.png"));
	backwardButton->setSizeHint(Widgets::Size(32,32));
	backwardButton->clicked.connect(this, &Game::backwardPressed);

	_forwardButton = new Widgets::PushButton(mainWidget());
	_forwardButton->setNormalTex(Widgets::TextureGL::get("data/forward.png"));
	_forwardButton->setHoverTex(Widgets::TextureGL::get("data/forwardhigh.png"));
	_forwardButton->setSizeHint(Widgets::Size(32,32));
	_forwardButton->clicked.connect(this, &Game::forwardPressed);

	_seekBar = new Widgets::ScrollBar(Widgets::Horizontal,mainWidget());
	_seekBar->setVisible(false);
	_seekBar->setRange(0,1000);
	_seekBar->setPageStep(25);
	_seekBar->setValue(1000);
	_seekBar->valueChanged.connect(_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(_seekBar, &Widgets::ScrollBar::updateValue);

	_threshavgGroup = makeThreshavgGroup();

	_recBar = new RecordingBar(_fileRec, mainWidget());

	Widgets::BoxLayout *topBar = new Widgets::BoxLayout(Widgets::Horizontal);
	topBar->addSpacing(10);
	topBar->addWidget(_configButton);
	topBar->addSpacing(5);
	topBar->addWidget(threshButton);
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

	Widgets::BoxLayout * const vbox = new Widgets::BoxLayout(Widgets::Vertical, mainWidget());

	vbox->addWidget(_recBar);
	vbox->addSpacing(10);
	vbox->addLayout(topBar);
	vbox->addSpacing(10);
	vbox->addWidget(_audioView, Widgets::AlignVCenter);
	vbox->addWidget(_seekBar);
	vbox->addSpacing(10);
	vbox->addLayout(seekBarBox);
	vbox->addSpacing(10);

	updateLayout();

	_audioView->standardSettings();

	setWindowTitle("BYB Spike Recorder");

	std::cout << "Starting GUI...\n";

}

Game::~Game() {
	
}

void Game::pausePressed() {
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

void Game::backwardPressed() {
	_audioView->setOffset(_audioView->offset()-5*_manager.sampleRate());
	if(_manager.paused())
		pausePressed();
}
void Game::forwardPressed() {
	if(_manager.fileMode()) {
		_audioView->setOffset(_manager.fileLength()-1);
	} else {
		_audioView->setOffset(0);
	}

	if(_manager.paused())
		pausePressed();
}

void Game::threshPressed() {
	if(!_manager.threshMode()) {
		_manager.setThreshMode(true);
		_threshavgGroup->setVisible(true);
	} else {
		_manager.setThreshMode(false);
		_threshavgGroup->setVisible(false);
	}
}

void Game::recordPressed() {
	if(!_fileRec.recording()) {
		if(_audioView->channelCount() == 0) {
			Widgets::ErrorBox *box = new Widgets::ErrorBox("Error: At least one channel has to be open to record anything.");
			box->setGeometry(Widgets::Rect(mainWidget()->width()/2-200, mainWidget()->height()/2-40, 400, 80));
			addPopup(box);
			return;
		}

		Widgets::FileDialog d(Widgets::FileDialog::SaveFile);

		d.open();
		while(d.isOpen())
			SDL_Delay(16);
		std::string str;
		int s = d.getResultState();

		if(s != Widgets::FileDialog::SUCCESS)
			return;
		if(!_fileRec.start(d.getResultFilename().c_str())) {
			const char *error = strerror(errno);
			std::stringstream s;
			s << "Error: Failed to open '" << d.getResultFilename().c_str() << "' for recording: " << error << ".";
			Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
			box->setGeometry(Widgets::Rect(mainWidget()->width()/2-200, mainWidget()->height()/2-40, 400, 80));
			addPopup(box);
			return;
		}
		_configButton->setSizeHint(Widgets::Size(0, 48));
		_configButton->setVisible(false);
		_fileButton->setSizeHint(Widgets::Size(0, 48));
		_fileButton->setVisible(false);
		_recBar->setActive(true);
	} else {
		MetadataChunk *m = new MetadataChunk;
		_audioView->constructMetaData(m);
		_manager.constructMetaData(m);
		_fileRec.setMetaData(m);

		_fileRec.stop();
		_configButton->setVisible(true);
		_configButton->setSizeHint(Widgets::Size(48, 48));
		_fileButton->setVisible(true);
		_fileButton->setSizeHint(Widgets::Size(48, 48));
		_recBar->setActive(false);
	}

	updateLayout();
}

void Game::filePressed() {
	if(!_manager.fileMode()) {
		Widgets::FileDialog d(Widgets::FileDialog::OpenFile);

		d.open();
		while(d.isOpen())
			SDL_Delay(16);
		std::string str;
		int s = d.getResultState();
		if(s != Widgets::FileDialog::SUCCESS)
			return;

		bool rc = _manager.loadFile(d.getResultFilename().c_str());

		if(rc == false) {
			std::stringstream s;
			s << "Error: Failed to open '" << d.getResultFilename().c_str() << "'. Wrong format perhaps?";
			Widgets::ErrorBox *box = new Widgets::ErrorBox(s.str().c_str());
			box->setGeometry(Widgets::Rect(mainWidget()->width()/2-200, mainWidget()->height()/2-40, 400, 80));
			addPopup(box);
			return;
		}

		MetadataChunk m;
		const char *mdatastr = _manager.fileMetaDataString();
		if(mdatastr) {
			FileRecorder::parseMetaDataStr(&m, mdatastr);
			_manager.applyMetaData(m);
			_audioView->applyMetaData(m);
		}
		_fileButton->setNormalTex(Widgets::TextureGL::get("data/filestop.png"));
		_fileButton->setHoverTex(Widgets::TextureGL::get("data/filestophigh.png"));

		_recordButton->setVisible(false);
		_forwardButton->setVisible(false);
	} else {
		_manager.initRecordingDevices();
		_fileButton->setNormalTex(Widgets::TextureGL::get("data/file.png"));
		_fileButton->setHoverTex(Widgets::TextureGL::get("data/filehigh.png"));
		_recordButton->setVisible(true);
		_forwardButton->setVisible(true);
	}
}

void Game::configPressed() {
	ConfigView *c = new ConfigView(_manager, *_audioView);
	c->setDeleteOnClose(true);
	c->setGeometry(mainWidget()->rect());
	addWindow(c);
}


void Game::loadResources() {
	Widgets::TextureGL::load("data/pause.png");
	Widgets::TextureGL::load("data/pausehigh.png");
	Widgets::TextureGL::load("data/play.png");
	Widgets::TextureGL::load("data/playhigh.png");

	Widgets::TextureGL::load("data/forward.png");
	Widgets::TextureGL::load("data/forwardhigh.png");
	Widgets::TextureGL::load("data/backward.png");
	Widgets::TextureGL::load("data/backwardhigh.png");

	Widgets::TextureGL::load("data/config.png");
	Widgets::TextureGL::load("data/confighigh.png");
	Widgets::TextureGL::load("data/thresh.png");
	Widgets::TextureGL::load("data/threshhigh.png");
	Widgets::TextureGL::load("data/rec.png");
	Widgets::TextureGL::load("data/rechigh.png");
	Widgets::TextureGL::load("data/file.png");
	Widgets::TextureGL::load("data/filehigh.png");
	Widgets::TextureGL::load("data/filestop.png");
	Widgets::TextureGL::load("data/filestophigh.png");

	Widgets::TextureGL::load("data/pin.png");
	Widgets::TextureGL::load("data/threshpin.png");
	Widgets::TextureGL::load("data/dropdown.png");
}

void Game::advance() {
	static uint32_t t = 0;
	uint32_t newt = SDL_GetTicks();
	_manager.advance((newt-t)*_manager.sampleRate()/1000);
	_fileRec.advance();

	t = newt;
}

void Game::keyPressEvent(Widgets::KeyboardEvent *e) {
	if(e->key() >= Widgets::Key0 && e->key() <= Widgets::Key9) {
		int mnum = e->key()-Widgets::Key0;
		_manager.setMarker(mnum);
	}
}

} // namespace BackyardBrains
