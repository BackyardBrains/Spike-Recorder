#ifndef BACKYARDBRAINS_WIDGETS_EVENT_H
#define BACKYARDBRAINS_WIDGETS_EVENT_H

#include "Point.h"
#include "Flags.h"

namespace BackyardBrains {

namespace Widgets {

enum MouseButton
{
	NoButton     = 0,
	LeftButton   = 1,
	MiddleButton = 2,
	RightButton  = 4,
	WheelUpButton = 8,
	WheelDownButton = 16
};

DECLARE_FLAGS(MouseButtons, MouseButton);
DECLARE_OPERATORS_FOR_FLAGS(MouseButtons);

class Event
{
public:
	Event() : _accepted(false)
	{
	}
	virtual ~Event()
	{
	}
	void accept() {_accepted = true;}
	void ignore() {_accepted = false;}
	bool isAccepted() const {return _accepted;}
	void setAccepted(bool a) {_accepted = a;}
private:
	bool _accepted;
};

class MouseEvent : public Event
{
public:
	MouseEvent(MouseButton b, MouseButtons bs, const Point &p, const Point &lp) : _pos(p), _lastPos(lp), _button(b), _buttons(bs)
	{
	}
	const Point & pos() const {return _pos;}
	const Point & lastPos() const {return _lastPos;}
	MouseButton button() const {return _button;}
	MouseButtons buttons() const {return _buttons;}
private:
	Point _pos;
	Point _lastPos;
	MouseButton _button;
	MouseButtons _buttons;
};

class ResizeEvent : public Event
{
public:
	ResizeEvent(const Size &ssize, const Size &ooldSize) : _size(ssize), _oldSize(ooldSize)
	{
	}
	const Size & size() const {return _size;}
	const Size & oldSize() const {return _oldSize;}
private:
	Size _size;
	Size _oldSize;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
