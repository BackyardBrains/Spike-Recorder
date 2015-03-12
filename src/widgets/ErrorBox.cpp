#include "ErrorBox.h"
#include "BitmapFontGL.h"
#include "TextureGL.h"
#include "Application.h"
#include "Painter.h"

namespace BackyardBrains {

namespace Widgets {

ErrorBox::ErrorBox(const char *text) : text(text) {
	setSizeHint(Size(20, 20));
}

void ErrorBox::paintEvent() {
	Widgets::Color bg = Colors::background;
	bg.a = 200;
	Painter::setColor(bg);
	Painter::drawRect(rect());

	Painter::setColor(Colors::white);
	const BitmapFontGL &font = *Application::font();
	font.drawMultiline(text.c_str(), 10, 10, width()-20);
}

void ErrorBox::mousePressEvent(MouseEvent *event) {
	close();
}

}

}
