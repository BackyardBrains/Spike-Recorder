#include "PushButton.h"
#include "Painter.h"
#include "BitmapFontGL.h"

namespace BackyardBrains {

namespace Widgets {

PushButton::PushButton(Widget *parent) : Widget(parent), _normaltex(NULL), _hovertex(NULL), _hover(false)
{
	setSizeHint(Size(48,48));
	// setMouseTracking(true);
}

// Size PushButton::sizeHint() const
// {
// 	return Widget::sizeHint();
// }

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
		Painter::setColor(_hover ? Colors::buttonhigh : Colors::button);
		Painter::drawCircle(rect().center(), rect().width()/2, 15);
	}
}

void PushButton::setNormalTex(const TextureGL *tex) {
	_normaltex = tex;
}

void PushButton::setHoverTex(const TextureGL *tex) {
	_hovertex = tex;
}

void PushButton::mousePressEvent(MouseEvent *event)
{
	event->accept();
}

void PushButton::mouseReleaseEvent(MouseEvent *event)
{
	if(event->button() == LeftButton && _hover)
		clicked.emit();
}

void PushButton::mouseMotionEvent(MouseEvent *event)
{
}

void PushButton::enterEvent() {
	_hover = true;
}

void PushButton::leaveEvent() {
	_hover = false;
}

} // namespace Widgets

} // namespace BackyardBrains
