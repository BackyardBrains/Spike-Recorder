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

	_switchLayout->addLayout(avgBar);
	_switchLayout->addWidget(_ekgWidget);

	Widgets::BoxLayout *layout = new Widgets::BoxLayout(Widgets::Horizontal, this);
	layout->addWidget(_ekgButton, Widgets::AlignVCenter);
	layout->addSpacing(10);
	layout->addLayout(_switchLayout);
	layout->update();

	setSizeHint(layout->sizeHint());
//	bar->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Fixed, Widgets::SizePolicy::Expanding));

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
	}
}

EkgWidget::EkgWidget(Widget *parent) : Widget(parent) {
	_frequency = 0;
	_lastTime = 0;
	_beatt = 1.f;
	setSizeHint(Widgets::Size(70,20));
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
}

void EkgWidget::advance() {
	_beatt += (0.6-_beatt)*0.1;
}
}
