#include "ToolTip.h"
#include "BitmapFontGL.h"
#include "TextureGL.h"
#include "Application.h"
#include "Painter.h"

#include <SDL.h>
#include <sstream>
#include <cassert>

namespace BackyardBrains {

namespace Widgets {

ToolTip::ToolTip(const char *text, int lifeTime) : _text(text), _lifeTime(lifeTime) {
	setSizeHint(Size(20, 20));
	_creationTime = SDL_GetTicks();
}

void ToolTip::paintEvent() {
	assert(_lifeTime != 0);
	float t = std::min(1.0f, std::max(0.f,(SDL_GetTicks() - _creationTime)/(float)_lifeTime*1.5f-0.5f));
	if(_lifeTime < 0)
		t = 0.f;

	Widgets::Color bg = Widgets::Colors::widgetbgdark;
	bg.a = 200.f*(1.f-t);
	Painter::setColor(bg);
	Painter::drawRect(rect());

	const BitmapFontGL &font = *Application::font();

	Widgets::Color textclr = Colors::white;
	textclr.a = 255.f*(1.f-t);
	Painter::setColor(textclr);
	font.drawMultiline(_text.c_str(),10,10,width()-20);

}

void ToolTip::advance() {
	if(_lifeTime > 0 && SDL_GetTicks()-_creationTime > (unsigned int) _lifeTime)
		close();
}

}

}
