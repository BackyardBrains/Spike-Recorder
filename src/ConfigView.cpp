#include "ConfigView.h"
#include "engine/RecordingManager.h"
#include "widgets/Painter.h"
#include "widgets/Color.h"
#include "widgets/BoxLayout.h"
#include "widgets/PushButton.h"
#include "widgets/TextureGL.h"
#include "widgets/Application.h"
#include "widgets/BitmapFontGL.h"
#include "ColorDropDownList.h"

namespace BackyardBrains {

ConfigView::ConfigView(RecordingManager *mngr, Widget *parent) : Widget(parent), _manager(mngr) {
	Widgets::PushButton *closeButton = new Widgets::PushButton(this);
	closeButton->clicked.connect(this, &ConfigView::closePressed);
	closeButton->setNormalTex(Widgets::TextureGL::get("data/config.png"));
	closeButton->setHoverTex(Widgets::TextureGL::get("data/confighigh.png"));

	ColorDropDownList *clrs = new ColorDropDownList(this);
	std::vector<Widgets::Color> c(3);
	c[0] = Widgets::Color(225,252,90);
	c[1] = Widgets::Color(255,138,91);
	c[2] = Widgets::Color(106,106,233);
	clrs->setContent(c);

	Widgets::BoxLayout *vbox = new Widgets::BoxLayout(Widgets::Vertical, this);
	Widgets::BoxLayout *hbox = new Widgets::BoxLayout(Widgets::Horizontal);
	hbox->addSpacing(10);
	hbox->addWidget(closeButton);
	vbox->addSpacing(10);
	vbox->addLayout(hbox);
	vbox->addSpacing(20);
	vbox->addWidget(clrs, Widgets::AlignCenter);

	vbox->update();

}

void ConfigView::paintEvent() {
	Widgets::Painter::setColor(Widgets::Color(20,20,20,250));
	Widgets::Painter::drawRect(rect());
	Widgets::Painter::setColor(Widgets::Colors::white);

	Widgets::Application::font()->draw("Config", 100, 35, Widgets::AlignCenter);
	
}

void ConfigView::closePressed() {
	close();
}

}
