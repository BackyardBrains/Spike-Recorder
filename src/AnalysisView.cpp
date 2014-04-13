#include "AnalysisView.h"
#include "widgets/PushButton.h"
#include "widgets/BoxLayout.h"
#include "widgets/TextureGL.h"
#include "widgets/Painter.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "widgets/Label.h"
#include "AudioView.h"

namespace BackyardBrains {

AnalysisView::AnalysisView(RecordingManager &mngr, Widgets::Widget *parent) : Widgets::Widget(parent), _manager(mngr) {
	_audioView = new AudioView(this, _manager);
	_audioView->setSizePolicy(Widgets::SizePolicy(Widgets::SizePolicy::Expanding, Widgets::SizePolicy::Expanding));
	_audioView->addChannel(0);


	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect((Widgets::Widget *)this, &Widgets::Widget::close);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));

	Widgets::Label *label = new Widgets::Label(this);
	label->setText("Spike Analysis");
	label->updateSize();



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

	vbox->update();
}

void AnalysisView::paintEvent() {
	const Widgets::Color bg = Widgets::Colors::background;

	Widgets::Painter::setColor(bg);
	Widgets::Painter::drawRect(rect());

}

}
