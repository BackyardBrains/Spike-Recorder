#include "ConfigView.h"
#include "engine/RecordingManager.h"
#include "widgets/Painter.h"
#include "widgets/Color.h"
#include "widgets/BoxLayout.h"
#include "widgets/PushButton.h"
#include "widgets/TextureGL.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"

namespace BackyardBrains {

ConfigView::ConfigView(RecordingManager *mngr, Widget *parent) : Widget(parent), _manager(mngr) {
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &ConfigView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));

	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
	hbox->addSpacing(10);
	hbox->addWidget(closeButton);
	vbox->addSpacing(10);
	vbox->addLayout(hbox);


}

void ConfigView::paintEvent() {
	Widgets::Painter::setColor(Widgets::Color(20,20,20,250));
	Widgets::Painter::drawRect(rect());
	Widgets::Painter::setColor(Widgets::Colors::white);

	Widgets::Application::getInstance()->font()->draw("Config", 100, 35, Widgets::AlignCenter);
	
}

void ConfigView::closePressed() {
	close();
}

}
