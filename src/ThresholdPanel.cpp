#include "ThresholdPanel.h"
#include "engine/RecordingManager.h"
#include "engine/AnalysisManager.h"
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

ThresholdPanel::ThresholdPanel(RecordingManager &manager, AnalysisManager &anaman, Widgets::Widget *parent) : Widgets::Widget(parent) {

	_manager = &manager;
	_triggerButton = new Widgets::PushButton(this);
	setTriggerButtonImage();
	_triggerButton->setSizeHint(Widgets::Size(42,32));
	_triggerButton->setRightPadding(10);
	_triggerButton->clicked.connect(this, &ThresholdPanel::triggerPressed);

	_ekgButton = new Widgets::PushButton(this);
	_ekgButton->setNormalTex(Widgets::TextureGL::get("data/ekg.bmp"));
	_ekgButton->setHoverTex(Widgets::TextureGL::get("data/ekghigh.bmp"));
	_ekgButton->setSizeHint(Widgets::Size(42,32));
	_ekgButton->setRightPadding(10);
	_ekgButton->clicked.connect(this, &ThresholdPanel::ekgPressed);
	if(!(_manager->serialMode() || _manager->hidMode()))
	{
		_ekgButton->setVisible(false);
	}


	_ekgWidget = new EkgWidget(anaman, this);
	manager.triggered.connect(_ekgWidget,&EkgWidget::beat);
	_speakerButton = new Widgets::PushButton(this);
	_speakerButton->setNormalTex(Widgets::TextureGL::get("data/speakeroff.bmp"));
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
	layout->addWidget(_ekgButton);
	layout->addWidget(_triggerButton);


	// layout->addWidget(_thresholdWidget, Widgets::AlignTop);
	//layout->addSpacing(10);

	//layout->addSpacing(10);
	layout->addLayout(_switchLayout);
	layout->update();

}

    
void ThresholdPanel::setTriggerButtonImage()
{
    if(_manager->getThresholdSource()==0)
    {
        _triggerButton->setNormalTex(Widgets::TextureGL::get("data/trigger.bmp"));
        _triggerButton->setHoverTex(Widgets::TextureGL::get("data/triggerhigh.bmp"));
    }
    else
    {
        std::stringstream s;
        s << "data/e"<<_manager->getThresholdSource()<<".bmp";
        _triggerButton->setNormalTex(Widgets::TextureGL::get(s.str().c_str()));
        _triggerButton->setHoverTex(Widgets::TextureGL::get(s.str().c_str()));
    }

}
    
void ThresholdPanel::paintEvent()
{
    if(_manager->serialMode() || _manager->hidMode())
    {
        _ekgButton->setVisible(true);
        _ekgButton->setSizeHint(Widgets::Size(43,32));
    }
    else
    {
        int state = _switchLayout->selected();
       
        if(state) {
            _ekgButton->setNormalTex(Widgets::TextureGL::get("data/ekg.bmp"));
            _ekgButton->setHoverTex(Widgets::TextureGL::get("data/ekghigh.bmp"));
            _ekgWidget->setSound(0);
            _switchLayout->setSelected(!state);
        }
        _ekgButton->setVisible(false);
        _ekgButton->setSizeHint(Widgets::Size(0,0));
    }
}
    
    
BOOL ThresholdPanel::ekgOn()
{
    return _switchLayout->selected();
}
    
void ThresholdPanel::triggerPressed()
{
    
    triggerOpened = !triggerOpened;
    if(triggerOpened)
    {
        ThresholdWidget * _thresholdWidget = new ThresholdWidget(*_manager);
        _thresholdWidget->valueChanged.connect(this, &ThresholdPanel::triggerChanged);
        _thresholdWidget->setSizeHint(Widgets::Size(74,270));
        Widgets::Rect positionOfPopup = Widgets::Rect(_triggerButton->mapToGlobal(_triggerButton->rect().bottomLeft()), Widgets::Size(74, 270));
         std::cout<<"Position: "<<positionOfPopup.x<<"\n";
        positionOfPopup.x -=18;
        /*if(_manager->serialMode() || _manager->hidMode())
        {
            positionOfPopup.x +=43;
        }*/
        positionOfPopup.y +=8;
        _thresholdWidget->setMouseTracking(true);
        _thresholdWidget->setGeometry(positionOfPopup);
        Widgets::Application::getInstance()->addPopup(_thresholdWidget);
    }
    //_thresholdWidget->setVisible(triggerOpened);
}
    
void ThresholdPanel::triggerChanged(int value)
{
    _manager->setThresholdSource(value);
    setTriggerButtonImage();
    triggerOpened = false;
}
    
    
void ThresholdPanel::speakerPressed() {
	bool state = _ekgWidget->sound();
	_ekgWidget->setSound(!state);

	if(state) {
		_speakerButton->setNormalTex(Widgets::TextureGL::get("data/speakeroff.bmp"));
	} else {
		_speakerButton->setNormalTex(Widgets::TextureGL::get("data/speaker.bmp"));
	}
}

void ThresholdPanel::ekgPressed() {
	int state = _switchLayout->selected();
	_switchLayout->setSelected(!state);

	if(state == 0) {
		_ekgButton->setNormalTex(Widgets::TextureGL::get("data/thresh.bmp"));
		_ekgButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.bmp"));

		_avg->setValue(1);
        _triggerButton->setVisible(false);
	} else {
		_ekgButton->setNormalTex(Widgets::TextureGL::get("data/ekg.bmp"));
		_ekgButton->setHoverTex(Widgets::TextureGL::get("data/ekghigh.bmp"));
		_ekgWidget->setSound(0);
        _triggerButton->setVisible(true);
	}
}
    
    
//============================= EKG Widget ========================================

EkgWidget::EkgWidget(AnalysisManager &anaman, Widget *parent) : Widget(parent), _anaman(anaman) {
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
	s << "f = " << (int)(_anaman.ekg.frequency()*60+0.5) <<"/min  \x7ft = " << 1/_anaman.ekg.frequency() << " s";
	Widgets::Application::font()->draw(s.str().c_str(), 0, height()/2, Widgets::AlignVCenter);
	Widgets::TextureGL::get("data/heart.bmp")->bind();
	glPushMatrix();
	glTranslatef(width()-height()/2-10,height()/2,0);
	glScalef(_beatt*1.2, _beatt*1.2, 1);
	Widgets::Painter::drawTexRect(Widgets::Rect(-height()/2,-height()/2, height(), height()));
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D,0);
}

void EkgWidget::beat() {
	_beatt = 1.f;

	if(_beepSample != (DWORD)-1 && _sound) {
		HCHANNEL chan = BASS_SampleGetChannel(_beepSample, false);
		BASS_ChannelPlay(chan, true);
	}
}

void EkgWidget::advance() {
	_beatt += (0.6-_beatt)*0.1;
}
    
    
//=========================== Threshold widget =====================================
    
    ThresholdWidget::ThresholdWidget(RecordingManager &manager, Widget *parent) : Widget(parent) {
        _manager = &manager;
        setSizeHint(Widgets::Size(74,270));
    }
    
    void ThresholdWidget::paintEvent() {
        
        

            int widthOfCell = 74;
            int Xposition = 0;
            
            
            //draw background
            
            Widgets::Painter::setColor(Widgets::Colors::widgetbg);
            Widgets::Painter::drawRect(Widgets::Rect(Xposition,0, widthOfCell, 270));
            Widgets::Painter::setColor(Widgets::Colors::widgetbgdark);
            Widgets::Painter::drawRect(Widgets::Rect(Xposition+2,2, widthOfCell-4, 266));
            
            Widgets::Painter::setColor(Widgets::Colors::white);
            int YOffset =10;
            
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
            else if(mouseLastPositionY>YOffset && (mouseLastPositionY<(YOffset+increment)))
            {
                Widgets::Painter::setColor(Widgets::Colors::buttonhigh);
                Widgets::Painter::drawRect(Widgets::Rect(Xposition+2, YOffset-5, widthOfCell-4, increment));
                Widgets::Painter::setColor(Widgets::Colors::white);
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
                    if(mouseLastPositionY>YOffset && (mouseLastPositionY<(YOffset+increment)))
                    {
                        Widgets::Painter::setColor(Widgets::Colors::buttonhigh);
                        Widgets::Painter::drawRect(Widgets::Rect(Xposition+2, YOffset-5, widthOfCell-4, increment));
                
                    }
                     Widgets::Painter::setColor(Widgets::Colors::white);
                }
                o.str("");
                o << "Event " << i;
                Widgets::Application::font()->draw(o.str().c_str(), Xposition+10, YOffset, Widgets::AlignLeft);
                YOffset += increment;
            }
        mouseOver = false;
        
    }
    
    void ThresholdWidget::mousePressEvent(Widgets::MouseEvent *event) {

        int y = event->pos().y;
        
        int indexNumber = ((y-10)/26);
        valueChanged.emit(indexNumber);
        close();
    }
    
    void ThresholdWidget::mouseMotionEvent(Widgets::MouseEvent *event)
    {
        mouseLastPositionY = event->pos().y;
        mouseOver = true;
    }
   
    
    
    
    
}
