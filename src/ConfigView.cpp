#include "ConfigView.h"
#include "engine/RecordingManager.h"
#include "widgets/Painter.h"
#include "widgets/Color.h"
#include "widgets/BoxLayout.h"
#include "widgets/PushButton.h"
#include "widgets/TextureGL.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Label.h"
#include "AudioView.h"
#include "ColorDropDownList.h"

namespace BackyardBrains {

ConfigView::ConfigView(RecordingManager &mngr, AudioView &audioView, Widget *parent) : Widget(parent), _manager(mngr), _audioView(audioView) {
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &ConfigView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));

	std::vector<Widgets::Color> c(AudioView::COLORS, AudioView::COLORS+AudioView::COLOR_NUM);
	std::vector<ColorDropDownList *> clrs(_manager.recordingDevices().size());
	_catchers.reserve(clrs.size());

	Widgets::Widget *group = new Widgets::Widget(this);
	group->setSizeHint(Widgets::Size(500,400));
	Widgets::BoxLayout *gvbox = new Widgets::BoxLayout(Widgets::Vertical, group);

	if(!_manager.fileMode()) {
		Widgets::BoxLayout *mutehbox = new Widgets::BoxLayout(Widgets::Horizontal);
		Widgets::Label *muteLabel = new Widgets::Label(group);
		muteLabel->setText("Mute Speakers:");
		muteLabel->updateSize();

		_muteCKBox = new Widgets::PushButton(group);
		if(_manager.player().volume() == 0)
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.png"));
		else
			_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.png"));
		_muteCKBox->setSizeHint(Widgets::Size(16,16));
		_muteCKBox->clicked.connect(this, &ConfigView::mutePressed);

		mutehbox->addWidget(muteLabel);
		mutehbox->addSpacing(10);
		mutehbox->addWidget(_muteCKBox, Widgets::AlignVCenter);
		mutehbox->addSpacing(50);
		gvbox->addLayout(mutehbox);
		gvbox->addSpacing(40);
	}

	for(unsigned int i = 0; i < _manager.recordingDevices().size(); i++) {
		if(!_manager.recordingDevices()[i].enabled)
			continue;
		clrs[i] = new ColorDropDownList(group);
		clrs[i]->setContent(c);

		_catchers.push_back(SignalCatcher(i, this));
		clrs[i]->selectionChanged.connect(&_catchers[i], &SignalCatcher::catchColor);
		Widgets::Label *name = new Widgets::Label(group);
		name->setText(_manager.recordingDevices()[i].name.c_str());
		name->updateSize();

		Widgets::BoxLayout *ghbox = new Widgets::BoxLayout(Widgets::Horizontal);
		ghbox->addWidget(clrs[i]);
		ghbox->addSpacing(20);
		ghbox->addWidget(name,Widgets::AlignVCenter);
		gvbox->addLayout(ghbox);
		gvbox->addSpacing(15);
	}

	for(int i = 0; i < audioView.channelCount(); i++)
		clrs[audioView.channelVirtualDevice(i)]->setSelection(audioView.channelColor(i));

	gvbox->update();

	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
	hbox->addSpacing(10);
	hbox->addWidget(closeButton);
	vbox->addSpacing(10);
	vbox->addLayout(hbox);
	vbox->addSpacing(20);
	vbox->addWidget(group, Widgets::AlignCenter);

	vbox->update();

}

void ConfigView::paintEvent() {
	Widgets::Color bg = Widgets::Colors::background;
	bg.a = 250;
	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect());
	Widgets::Painter::setColor(Widgets::Colors::white);

	Widgets::Application::font()->draw("Config", 100, 35, Widgets::AlignCenter);
	
}

void ConfigView::closePressed() {
	close();
}

void ConfigView::mutePressed() {
	if(_manager.player().volume() == 0) {
		_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxoff.png"));
		_manager.player().setVolume(100);
	} else {
		_muteCKBox->setNormalTex(Widgets::TextureGL::get("data/ckboxon.png"));
		_manager.player().setVolume(0);
	}
}

void ConfigView::colorChanged(int virtualDevice, int coloridx) {
	int channel = _audioView.virtualDeviceChannel(virtualDevice);
	if(channel < 0 && coloridx != 0) {
		int nchan = _audioView.addChannel(virtualDevice);
		_audioView.setChannelColor(nchan, coloridx);
	} else if(coloridx == 0) {
		_audioView.removeChannel(virtualDevice);
	} else {
		_audioView.setChannelColor(channel, coloridx);
	}
}

}
