#include "ThresholdPanel.h"
#include "engine/RecordingManager.h"
#include "widgets/PushButton.h"
#include "widgets/Label.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/ScrollBar.h"
#include "widgets/SwitchLayout.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Painter.h"
#include "Log.h"
#include <sstream>
#include <SDL.h>

namespace BackyardBrains {

ThresholdPanel::ThresholdPanel(RecordingManager &manager, Widgets::Widget *parent) : Widgets::Widget(parent) {
	_ekgButton = new Widgets::PushButton(this);
	_ekgButton->setNormalTex(Widgets::TextureGL::get("data/ekg.png"));
	_ekgButton->setHoverTex(Widgets::TextureGL::get("data/ekghigh.png"));
	_ekgButton->setSizeHint(Widgets::Size(32,32));
	_ekgButton->clicked.connect(this, &ThresholdPanel::ekgPressed);

	_ekgWidget = new EkgWidget(this);
	manager.triggered.connect(_ekgWidget,&EkgWidget::beat);
	_speakerButton = new Widgets::PushButton(this);
	_speakerButton->setNormalTex(Widgets::TextureGL::get("data/speakeroff.png"));
	_speakerButton->setSizeHint(Widgets::Size(20,20));
	_speakerButton->clicked.connect(this, &ThresholdPanel::speakerPressed);

	_avg = new Widgets::ScrollBar(Widgets::Horizontal,this);
	_avg->setRange(1,50);
	_avg->setPageStep(5);
	_avg->valueChanged.connect(&manager, &RecordingManager::setThreshAvgCount);
	_avg->setSizeHint(Widgets::Size(250,20));
	_avg->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Maximum));

	Widgets::Label *label = new Widgets::Label(this);
	label->setText("00");
	label->updateSize();

	_avg->valueChanged.connect(label, &Widgets::Label::setText);
	_avg->setValue(1);

	_switchLayout = new Widgets::SwitchLayout();

	Widgets::BoxLayout *avgBar = new Widgets::BoxLayout(Widgets::Horizontal);
	avgBar->addWidget(_avg, Widgets::AlignVCenter);
	avgBar->addSpacing(10);
	avgBar->addWidget(label, Widgets::AlignVCenter);

	Widgets::BoxLayout *ekgBar = new Widgets::BoxLayout(Widgets::Horizontal);
	ekgBar->addWidget(_ekgWidget, Widgets::AlignVCenter);
	ekgBar->addWidget(_speakerButton, Widgets::AlignVCenter);

	_switchLayout->addLayout(avgBar);
	_switchLayout->addLayout(ekgBar);

	Widgets::BoxLayout *layout = new Widgets::BoxLayout(Widgets::Horizontal, this);
	layout->addWidget(_ekgButton, Widgets::AlignVCenter);
	layout->addSpacing(10);
	layout->addLayout(_switchLayout);
	layout->update();

	setSizeHint(layout->sizeHint());
}

void ThresholdPanel::speakerPressed() {
	bool state = _ekgWidget->sound();
	_ekgWidget->setSound(!state);

	if(state) {
		_speakerButton->setNormalTex(Widgets::TextureGL::get("data/speakeroff.png"));
	} else {
		_speakerButton->setNormalTex(Widgets::TextureGL::get("data/speaker.png"));
	}
}

void ThresholdPanel::ekgPressed() {
	int state = _switchLayout->selected();
	_switchLayout->setSelected(!state);

	if(state == 0) {
		_ekgButton->setNormalTex(Widgets::TextureGL::get("data/thresh.png"));
		_ekgButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.png"));

		_avg->setValue(1);
	} else {
		_ekgButton->setNormalTex(Widgets::TextureGL::get("data/ekg.png"));
		_ekgButton->setHoverTex(Widgets::TextureGL::get("data/ekghigh.png"));
		_ekgWidget->setSound(0);
	}
}

EkgWidget::EkgWidget(Widget *parent) : Widget(parent) {
	_frequency = 0;
	_lastTime = 0;
	_beatt = 1.f;
	_sound = false;
	setSizeHint(Widgets::Size(300,32));

	_beepSample = BASS_SampleLoad(false, "data/ekg.wav",0,0,10,0);
}

bool EkgWidget::sound() const {
	return _sound;
}

void EkgWidget::setSound(bool sound) {
	_sound = sound;
}

void EkgWidget::paintEvent() {
	std::stringstream s;
	s << "f = " << (int)(_frequency*60+0.5) <<"/min  \x7ft = " << 1/_frequency << " s";
	Widgets::Application::font()->draw(s.str().c_str(), 0, height()/2, Widgets::AlignVCenter);
	Widgets::TextureGL::get("data/heart.png")->bind();
	glPushMatrix();
	glTranslatef(width()-height()/2-10,height()/2,0);
	glScalef(_beatt*1.2, _beatt*1.2, 1);
	Widgets::Painter::drawTexRect(Widgets::Rect(-height()/2,-height()/2, height(), height()));
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D,0);
}

void EkgWidget::beat() {
	unsigned int time = SDL_GetTicks();
	float nfreq = 1000.f/(time-_lastTime);
	_frequency += 0.3*(nfreq-_frequency); // exponential moving average. this may need tweaking
	
	_beatt = 1.f;

	_lastTime = time;

	if(_beepSample != (DWORD)-1 && _sound) {
		HCHANNEL chan = BASS_SampleGetChannel(_beepSample, false);
		BASS_ChannelPlay(chan, true);
	}
}

void EkgWidget::advance() {
	_beatt += (0.6-_beatt)*0.1;
}
}
