#include "Game.h"
#include "widgets/PushButton.h"
#include "widgets/DropDownList.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/ScrollBar.h"
#include "widgets/FileDialog.h"
#include "AudioView.h"
#include "ConfigView.h"
#include <SDL_opengl.h>
#include <SDL.h>
#include <iostream>

namespace BackyardBrains {

Game::Game()
{
	std::cout << "Loading Resources...\n";
	loadResources();
	std::cout << "Creating Window...\n";
	createWindow(800,600);

	_audioView = new AudioView(mainWidget(), &_manager);
	_manager.channelCountChanged.connect(_audioView, &AudioView::updateView);

	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));

	Widgets::PushButton *configButton = new Widgets::PushButton(mainWidget());
	configButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	configButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));
	configButton->clicked.connect(this, &Game::configPressed);
	Widgets::PushButton *threshButton = new Widgets::PushButton(mainWidget());
	threshButton->setNormalTex(Widgets::TextureGL::get("data/thresh.png"));
	threshButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.png"));
	threshButton->clicked.connect(_audioView, &AudioView::toggleThreshMode);
	Widgets::PushButton *recordButton = new Widgets::PushButton(mainWidget());
	recordButton->setNormalTex(Widgets::TextureGL::get("data/rec.png"));
	recordButton->setHoverTex(Widgets::TextureGL::get("data/rechigh.png"));
	Widgets::PushButton *fileButton = new Widgets::PushButton(mainWidget());
	fileButton->setNormalTex(Widgets::TextureGL::get("data/file.png"));
	fileButton->setHoverTex(Widgets::TextureGL::get("data/filehigh.png"));
	fileButton->clicked.connect(this, &Game::filePressed);

	_pauseButton = new Widgets::PushButton(mainWidget());
	_pauseButton->clicked.connect(this, &Game::pausePressed);
	_pauseButton->setNormalTex(Widgets::TextureGL::get("data/pause.png"));
	_pauseButton->setHoverTex(Widgets::TextureGL::get("data/pausehigh.png"));
	_pauseButton->setCustomSize(Widgets::Size(64,64));

	Widgets::PushButton * const backwardButton = new Widgets::PushButton(mainWidget());
	backwardButton->setNormalTex(Widgets::TextureGL::get("data/backward.png"));
	backwardButton->setHoverTex(Widgets::TextureGL::get("data/backwardhigh.png"));
	backwardButton->setCustomSize(Widgets::Size(32,32));
	backwardButton->clicked.connect(this, &Game::backwardPressed);

	Widgets::PushButton * const forwardButton = new Widgets::PushButton(mainWidget());
	forwardButton->setNormalTex(Widgets::TextureGL::get("data/forward.png"));
	forwardButton->setHoverTex(Widgets::TextureGL::get("data/forwardhigh.png"));
	forwardButton->setCustomSize(Widgets::Size(32,32));
	forwardButton->clicked.connect(this, &Game::forwardPressed);

	_seekBar = new Widgets::ScrollBar(Widgets::Horizontal,mainWidget());
	_seekBar->setVisible(false);
	_seekBar->setRange(0,1000);
	_seekBar->setPageStep(30);
	_seekBar->setValue(1000);
	_seekBar->valueChanged.connect(_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(_seekBar, &Widgets::ScrollBar::updateValue);

	Widgets::BoxLayout *topBar = new Widgets::BoxLayout(Widgets::Horizontal);
	topBar->addSpacing(10);
	topBar->addWidget(configButton);
	topBar->addSpacing(5);
	topBar->addWidget(threshButton);
	topBar->addStretch();
	topBar->addWidget(recordButton);
	topBar->addSpacing(5);
	topBar->addWidget(fileButton);
	topBar->addSpacing(10);


	Widgets::BoxLayout * const seekBarBox = new Widgets::BoxLayout(Widgets::Horizontal);
	seekBarBox->setAlignment(Widgets::AlignHCenter);

	seekBarBox->addWidget(backwardButton,Widgets::AlignVCenter);
	seekBarBox->addSpacing(10);
	seekBarBox->addWidget(_pauseButton);
	seekBarBox->addSpacing(10);
	seekBarBox->addWidget(forwardButton,Widgets::AlignVCenter);

	Widgets::BoxLayout * const vbox = new Widgets::BoxLayout(Widgets::Vertical, mainWidget());

	vbox->addSpacing(10);
	vbox->addLayout(topBar);
	vbox->addSpacing(10);
	vbox->addWidget(_audioView, Widgets::AlignVCenter);
	vbox->addWidget(_seekBar);
	vbox->addSpacing(10);
	vbox->addLayout(seekBarBox);
	vbox->addSpacing(10);
	vbox->update();
	vbox->setGeometry(Widgets::Rect(0, 0, 800, 600));

	setWindowTitle("BYB Spike Recorder");

	std::cout << "Starting GUI...\n";

 	_manager.setChannelCount(3);
	_manager.setChannelVirtualDevice(0, 0);
	_manager.setChannelVirtualDevice(1, 2);
	_manager.setChannelVirtualDevice(2, 3);
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
	_audioView->setOffset(_audioView->offset()-5*RecordingManager::SAMPLE_RATE);
	if(_manager.paused())
		pausePressed();
}
void Game::forwardPressed() {
	_audioView->setOffset(0);
	if(_manager.paused())
		pausePressed();
}

void Game::filePressed() {
	Widgets::FileDialog d(Widgets::FileDialog::SaveFile);

	d.open();
	while(d.isOpen())
		SDL_Delay(16);
	std::string str;
	int s = d.getResultState();
	str = d.getResultFilename();
	test(s, str);
}

void Game::configPressed() {
	ConfigView *c = new ConfigView(&_manager);
	c->setDeleteOnClose(true);
	c->setGeometry(mainWidget()->rect());
	addWindow(c);
}

void Game::test(int state, std::string str) {
	std::cout << state << " '" << str << "'\n";
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

	Widgets::TextureGL::load("data/pin.png");
	Widgets::TextureGL::load("data/threshpin.png");
}

void Game::advance() {
	_manager.advance();
}

} // namespace BackyardBrains
