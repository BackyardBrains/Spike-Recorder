//
//  RTConfigView.cpp
//  SpikeRecorder
//
//  Created by Stanislav Mircic on 8/13/15.
//  Copyright (c) 2015 BackyardBrains. All rights reserved.
//

#include "RTConfigView.h"
#include "ConfigView.h"
#include "engine/RecordingManager.h"
#include "engine/BASSErrors.h"
#include "widgets/Painter.h"
#include "widgets/Color.h"
#include "widgets/BoxLayout.h"
#include "widgets/PushButton.h"
#include "widgets/TextureGL.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/ErrorBox.h"
#include "widgets/Label.h"
#include "widgets/ErrorBox.h"
#include "DropDownList.h"
#include "AudioView.h"
#include "ColorDropDownList.h"
#include "Log.h"
#include <bass.h>
#include <sstream>

namespace BackyardBrains {

    RTConfigView::RTConfigView(RecordingManager &mngr, AudioView &audioView, Widget *parent) : Widget(parent), _manager(mngr), _audioView(audioView) {

        //create close button
        Widgets::PushButton *closeButton = new Widgets::PushButton(this);
        closeButton->clicked.connect(this, &RTConfigView::closePressed);
        closeButton->setNormalTex(Widgets::TextureGL::get("data/rtimer.png"));
        closeButton->setHoverTex(Widgets::TextureGL::get("data/rtimer.png"));

        //create window Title label
        Widgets::Label *topLabel = new Widgets::Label(this);
        topLabel->setText("Reaction Timer Configuration");
        topLabel->updateSize();

        //create main group
        Widgets::Widget *group = new Widgets::Widget(this);
        group->setSizeHint(Widgets::Size(500,400));

        //create main vertical layout and attach to group
        Widgets::BoxLayout *gvbox = new Widgets::BoxLayout(Widgets::Vertical, group);

        //create horizontal layout for stimulation config
        Widgets::BoxLayout *repeatbox = new Widgets::BoxLayout(Widgets::Horizontal);
        //create label
        Widgets::Label *repeatLabel = new Widgets::Label(group);
        repeatLabel->setText("Repeat stimmulation:");
        repeatLabel->updateSize();
        //create check box
        _repeatCKBox = new Widgets::PushButton(group);
        if(_manager.isRTRepeating())
            _repeatCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.png"));
        else
            _repeatCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.png"));
        _repeatCKBox->setSizeHint(Widgets::Size(16,16));
        _repeatCKBox->clicked.connect(this, &RTConfigView::repeatPressed);

        //add label and checkbox to horizontal layout
        repeatbox->addWidget(repeatLabel);
        repeatbox->addSpacing(10);
        repeatbox->addWidget(_repeatCKBox, Widgets::AlignVCenter);
        repeatbox->addSpacing(50);

        //add horizontal layout to main vertical
        gvbox->addLayout(repeatbox);
        gvbox->addSpacing(40);
        gvbox->update();

        //create master vertical box
        Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);

        //create title horizontal box
        Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
        hbox->addSpacing(10);
        hbox->addWidget(closeButton);
        hbox->addSpacing(17);
        hbox->addWidget(topLabel, Widgets::AlignVCenter);

        //add things to master vertical box
        vbox->addSpacing(10);
        vbox->addLayout(hbox);//add title box
        vbox->addSpacing(20);
        vbox->addWidget(group, Widgets::AlignCenter); //add content
        vbox->update();

    }

    void RTConfigView::paintEvent() {
        Widgets::Color bg = Widgets::Colors::background;
        bg.a = 250;
        Widgets::Painter::setColor(bg);
        Widgets::Painter::drawRect(rect());
        if(_manager.isRTRepeating()) {
            _repeatCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.png"));
            //_manager.
        } else {
            _repeatCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.png"));


        }
    }


    void RTConfigView::closePressed() {
        close();
    }

    void RTConfigView::repeatPressed() {
        _manager.swapRTRepeating();
    }
}
