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
    
    _manager = &manager;
    _triggerButton = new Widgets::PushButton(this);
    _triggerButton->setNormalTex(Widgets::TextureGL::get("data/trigger.png"));
    _triggerButton->setHoverTex(Widgets::TextureGL::get("data/triggerhigh.png"));
    _triggerButton->setSizeHint(Widgets::Size(32,32));
    _triggerButton->clicked.connect(this, &ThresholdPanel::triggerPressed);
    
	_ekgButton = new Widgets::PushButton(this);
	_ekgButton->setNormalTex(Widgets::TextureGL::get("data/ekg.png"));
	_ekgButton->setHoverTex(Widgets::TextureGL::get("data/ekghigh.png"));
	_ekgButton->setSizeHint(Widgets::Size(32,32));
	_ekgButton->clicked.connect(this, &ThresholdPanel::ekgPressed);

	_ekgWidget = new EkgWidget(this);
	manager.triggered.connect(_ekgWidget,&EkgWidget::beat);
	manager.thresholdChanged.connect(_ekgWidget, &EkgWidget::reset);
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
    layout->addWidget(_triggerButton, Widgets::AlignCenter);
    layout->addSpacing(10);
	layout->addWidget(_ekgButton, Widgets::AlignVCenter);
	layout->addSpacing(10);
	layout->addLayout(_switchLayout);
	layout->update();

}

    
BOOL ThresholdPanel::ekgOn()
{
    return _switchLayout->selected();
}
    
void ThresholdPanel::triggerPressed()
{
    triggerOpened = !triggerOpened;
}
    
void ThresholdPanel::paintEvent() {
    

    if(triggerOpened)
    {
        int widthOfCell = 74;
        int Xposition = _triggerButton->pos().x-15;
        
        
        //draw background
        
        Widgets::Painter::setColor(Widgets::Colors::widgetbg);
        Widgets::Painter::drawRect(Widgets::Rect(Xposition,_triggerButton->pos().y+40, widthOfCell, 270));
        Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
        Widgets::Painter::drawRect(Widgets::Rect(Xposition+2,_triggerButton->pos().y+42, widthOfCell-4, 266));
        
        Widgets::Painter::setColor(Widgets::Colors::white);
        int YOffset =_triggerButton->pos().y+50;

        //draw Signal label
        
        int increment = 26;
        std::stringstream o;
        o << "Signal";
        if(_manager->getThresholdSource()==0)
        {
            Widgets::Painter::setColor(Widgets::Colors::selectedstate);
            Widgets::Painter::drawRect(Widgets::Rect(Xposition+2, YOffset-5, widthOfCell-4, increment));
            Widgets::Painter::setColor(Widgets::Colors::black);
        }
        Widgets::Application::font()->draw(o.str().c_str(), Xposition+10, YOffset, Widgets::AlignLeft);
        
        //Draw events labels
        
        YOffset += increment;
        for(int i=1;i<10;i++)
        {
            
            if(_manager->getThresholdSource()==i)
            {
                Widgets::Painter::setColor(Widgets::Colors::selectedstate);
                Widgets::Painter::drawRect(Widgets::Rect(Xposition+2, YOffset-5, widthOfCell-4, increment));
                Widgets::Painter::setColor(Widgets::Colors::black);
            }
            else
            {
                Widgets::Painter::setColor(Widgets::Colors::white);
            }
            o.str("");
            o << "Event " << i;            
            Widgets::Application::font()->draw(o.str().c_str(), Xposition+10, YOffset, Widgets::AlignLeft);
            YOffset += increment;
        }
    }
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
	reset();
	_sound = false;
	setSizeHint(Widgets::Size(300,32));

	_beepSample = BASS_SampleLoad(false, "data/ekg.wav",0,0,10,0);
}

void EkgWidget::reset() {
	_beatt = 1.f;
	_frequency = 0;
	_lastTime = 0;
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
