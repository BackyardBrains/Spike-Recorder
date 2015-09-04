#include "Widget.h"
#include "widgets/LayoutItem.h"
#include "Application.h"

#include <SDL.h>
#include <SDL_opengl.h>

#include <algorithm>
#include <cassert>

namespace BackyardBrains {

namespace Widgets {


Widget::Widget(Widget *parent) : _parentWidget(parent), _layout(NULL) {
	setParentWidget(parent);
}

Widget::~Widget() {
	setLayout(NULL);
	for (WidgetVector::reverse_iterator it = _children.rbegin(); it != _children.rend(); ++it)
		delete *it;
	if (parentWidget()) {
		WidgetVector::reverse_iterator it = std::find(parentWidget()->_children.rbegin(), parentWidget()->_children.rend(), this);
		if (it != parentWidget()->_children.rend())
			parentWidget()->_children.erase(parentWidget()->_children.begin() + (&(*it) - parentWidget()->_children.data())); // TODO simplify this conversion from reverse_iterator to iterator
	}

	Application *app = Application::getInstance();
	if(app->hoverWidget() == this)
		app->hoverWidget() = NULL;
	if(app->keyboardGrabber() == this)
		app->keyboardGrabber() = NULL;
	if(app->mouseGrabber() == this)
		app->mouseGrabber() = NULL;
}

Widget * Widget::parentWidget() const {
	return _parentWidget;
}

void Widget::setParentWidget(Widget *w) {
	// TODO propogate this information to new parent, as well as old parent
	_parentWidget = w;
	// TODO notify of the addition for the purposes of layout/size updating
	if (parentWidget()) {
		parentWidget()->_children.push_back(this);
	}
}

Layout *Widget::layout() {
	return _layout;
}

void Widget::setLayout(Layout *newLayout) {
	if (newLayout == _layout)
		return;
	if (_layout) {
		Layout * const l = _layout;
		_layout = NULL;
		delete l;
	}
	_layout = newLayout;
	if (_layout) {
		_layout->setParentWidget(this);
	}
}

void Widget::setGeometry(const Rect &newRect) {
	const Size oldSize = _rect.size();
	_rect = newRect;
	
	if (_layout)
		_layout->setGeometry(newRect.translated(-geometry().topLeft()));
	const Size newSize = _rect.size();
	if (newSize != oldSize) {
		ResizeEvent event(newSize, oldSize);
		resizeEvent(&event);
	}
}

Rect Widget::geometry() const {
	return _rect;
}

void Widget::setRect(const Rect &newRect) {
	setGeometry(newRect.translated(geometry().topLeft()));
}

Rect Widget::rect() const {
	return Rect(0, 0, width(), height());
}

Point Widget::pos() const {
	return Point(_rect.x, _rect.y);
}

void Widget::move(const Point &newPoint) {
	setGeometry(Rect(newPoint, size()));
}

void Widget::setSize(const Size &newSize) {
	setGeometry(Rect(geometry().topLeft(), newSize));
}

Size Widget::size() const {
	return Size(width(), height());
}

int Widget::width() const {
	return _rect.w;
}

int Widget::height() const {
	return _rect.h;
}

Size Widget::sizeHint() const {
	return 	_sizeHint;
}

void Widget::setSizeHint(const Size &hint) {
	_sizeHint = hint;
}

SizePolicy Widget::sizePolicy() const {
	return _sizePolicy;
}

void Widget::setSizePolicy(const SizePolicy &newPolicy) {
	_sizePolicy = newPolicy;
}

Widget *Widget::_GetWidgetAt(const Point &point) {
	if (!geometry().contains(point) || !isVisible())
		return NULL;
	for (WidgetVector::const_reverse_iterator it = _children.rbegin(); it != _children.rend(); ++it) {
		Widget *result = (*it)->_GetWidgetAt(point - geometry().topLeft());
		if(result)
			return result;
	}
	return this;
}

void Widget::_CallAdvance() {
	advance();
	for (WidgetVector::iterator it = _children.begin(); it != _children.end(); ++it) {
		(*it)->_CallAdvance();
	}
}

void Widget::_DoPaintEvents(const Point &offset, const Rect &clipRect) {
	if(isVisible()) {
		const Point off = offset + geometry().topLeft();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(off.x, off.y, 0);
		//const SDL_Surface *const screen = SDL_GetVideoSurface();

		paintEvent();
		for (WidgetVector::iterator it = _children.begin(); it != _children.end(); ++it) {
			(*it)->_DoPaintEvents(off, (*it)->geometry().translated(off).intersected(clipRect));
		}
	}

}

void Widget::_DoGlResetEvents() {
	glResetEvent();
	for(WidgetVector::iterator it = _children.begin(); it != _children.end(); ++it)
		(*it)->_DoGlResetEvents();
}


Point Widget::mapToParent(const Point &point) const {
	Point result = point;
	if (parentWidget())
		result += geometry().topLeft();
	return result;
}

Point Widget::mapToGlobal(const Point &point) const {
	Point result = point;
	for (const Widget *current = this; current; current = current->parentWidget()) {
		result += current->geometry().topLeft();
	}
	return result;
}

void Widget::close() {
	_state.closed = true;
}

void Widget::unclose() {
	_state.closed = false;
}

bool Widget::closed() {
	return _state.closed;
}

void Widget::setVisible(bool visible) {
	_state.hidden = !visible;
}

void Widget::setHidden(bool hidden) {
	_state.hidden = hidden;
}

bool Widget::isVisible() const {
	return !_state.hidden;
}

bool Widget::isHidden() const {
	return _state.hidden;
}

void Widget::setMouseTracking(bool enable) {
	_state.mouseTracking = enable;
}

bool Widget::hasMouseTracking() const {
	return _state.mouseTracking;
}

void Widget::setDeleteOnClose(bool d) {
	_state.deleteOnClose = d;
}

bool Widget::getDeleteOnClose() const {
	return _state.deleteOnClose;
}

void Widget::advance() {
}

void Widget::resizeEvent(ResizeEvent *event) {
}

void Widget::glResetEvent() {
}

void Widget::paintEvent() {
}

void Widget::enterEvent() {
}

void Widget::leaveEvent() {
}

void Widget::mousePressEvent(MouseEvent *event) {
}

void Widget::mouseReleaseEvent(MouseEvent *event) {
}

void Widget::mouseMotionEvent(MouseEvent *event) {
}

void Widget::keyPressEvent(KeyboardEvent *event) {
}

void Widget::keyReleaseEvent(KeyboardEvent *event) {
}

Widget::WidgetVector &Widget::children() {
	return _children;
}


} // namespace Widgets

} // namespace BackyardBrains
