#include "PushButton.h"
#include "Painter.h"
#include "BitmapFontGL.h"
#include <iostream>

// #include <map>

namespace BackyardBrains {

namespace Widgets {

PushButton::PushButton(Widget *parent) : Widget(parent), _normaltex(NULL), _hovertex(NULL), _size(48,48), _hover(false)
{
	// setMouseTracking(true);
}

Size PushButton::sizeHint() const
{
	return _size;
}

void PushButton::paintEvent()
{
	if(_normaltex != NULL) {
		if(_hover && _hovertex != NULL)
			_hovertex->bind();
		else
			_normaltex->bind();

		Painter::setColor(Colors::white);
		Painter::drawTexRect(rect());
		glBindTexture(GL_TEXTURE_2D, 0);
	} else {
		Painter::setColor(_hover ? Color(150,150,150) : Colors::gray);
		Painter::drawCircle(rect().center(), rect().width()/2, 15);
	}
}

void PushButton::setNormalTex(const TextureGL *tex) {
	_normaltex = tex;
}

void PushButton::setHoverTex(const TextureGL *tex) {
	_hovertex = tex;
}

void PushButton::setCustomSize(const Size &s) {
	_size = s;
}

void PushButton::mousePressEvent(MouseEvent *event)
{
	event->accept();

// 	std::cout << this << ": mousePressEvent:   " << event->pos().x << ',' << event->pos().y << std::endl;
}

void PushButton::mouseReleaseEvent(MouseEvent *event)
{
	if(event->button() == LeftButton && _hover)
		clicked.emit();
	// std::cout << this << ": mouseReleaseEvent: " << event->pos().x << ',' << event->pos().y << std::endl;
}

void PushButton::mouseMotionEvent(MouseEvent *event)
{
	// std::cout << this << ": mouseMotionEvent:  " << event->pos().x << ',' << event->pos().y << std::endl;
}

void PushButton::enterEvent() {
	_hover = true;
}

void PushButton::leaveEvent() {
	_hover = false;
}

} // namespace Widgets

} // namespace BackyardBrains
