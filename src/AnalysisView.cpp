#include "AnalysisView.h"
#include "widgets/PushButton.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Label.h"
#include "AnalysisAudioView.h"

namespace BackyardBrains {

AnalysisView::AnalysisView(RecordingManager &mngr, Widgets::Widget *parent) : Widgets::Widget(parent), _manager(mngr) {
	_audioView = new AnalysisAudioView(mngr, _spikes, this);
	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_audioView->addChannel(0);


	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &AnalysisView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/analysis.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/analysishigh.png"));

	Widgets::Label *label = new Widgets::Label(this);
	label->setText("Spike Analysis");
	label->updateSize();

	Widgets::ScrollBar *seekBar = new Widgets::ScrollBar(Widgets::Horizontal, this);
	seekBar->setRange(0,1000);
	seekBar->setPageStep(25);
	seekBar->valueChanged.connect((AudioView *)_audioView, &AudioView::setRelOffset);
 	_audioView->relOffsetChanged.connect(seekBar, &Widgets::ScrollBar::updateValue);
	seekBar->setValue(0);

	Widgets::PushButton *saveButton = new Widgets::PushButton(this);
	saveButton->setNormalTex(Widgets::TextureGL::get("data/thresh.png"));
	saveButton->setHoverTex(Widgets::TextureGL::get("data/threshhigh.png"));
	saveButton->setSizeHint(Widgets::Size(64,64));
	Widgets::BoxLayout *saveBox = new Widgets::BoxLayout(Widgets::Horizontal);

	saveBox->addWidget(saveButton);
	saveBox->setAlignment(Widgets::AlignCenter);
	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
	hbox->addSpacing(10);
	hbox->addWidget(closeButton);
	hbox->addSpacing(17);
	hbox->addWidget(label, Widgets::AlignVCenter);
	vbox->addSpacing(10);
	vbox->addLayout(hbox);
	vbox->addSpacing(20);
	vbox->addWidget(_audioView, Widgets::AlignCenter);
	vbox->addWidget(seekBar);
	vbox->addSpacing(10);
	vbox->addLayout(saveBox);
	vbox->addSpacing(20);

	vbox->update();

	_spikes.findSpikes(_manager.fileName(), _manager.selectedVDevice(), _manager.recordingDevices()[_manager.selectedVDevice()].threshold, _manager.sampleRate()/100 /* 10 ms */);

	_wasThreshMode = _manager.threshMode();
	_manager.setThreshMode(false);
	_manager.setPos(0);
}



void AnalysisView::paintEvent() {
	const Widgets::Color bg = Widgets::Colors::background;

	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect());

}

void AnalysisView::closePressed() {
	_manager.setThreshMode(_wasThreshMode);
	close();
}

}
