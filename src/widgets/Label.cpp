#include "Label.h"
#include "BitmapFontGL.h"
#include "Application.h"
#include "Painter.h"

#include <sstream>

namespace BackyardBrains {

namespace Widgets {

const int Label::PADDING = 4;

Label::Label(Widget *parent) : Widget(parent) {
	setSizeHint(Size(Application::font()->characterWidth()*10+2*PADDING, Application::font()->characterHeight()+2*PADDING));
}

void Label::setText(const char *text) {
	_text = text;
}

void Label::updateSize() {
	setSizeHint(Size(Application::font()->characterWidth()*_text.size()+2*PADDING, Application::font()->characterHeight()+2*PADDING));
}

void Label::setText(int num) {
	std::stringstream o;
	o << num;
	_text = o.str();
}

void Label::setText(float num) {
	std::stringstream o;
	o << num;
	_text = o.str();
}

void Label::paintEvent() {
	Painter::setColor(Colors::white);
	Application::font()->draw(_text.c_str(), width()/2, height()/2, AlignCenter);
}

}

}
