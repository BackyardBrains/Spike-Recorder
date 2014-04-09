#ifndef BACKYARDBRAINS_WIDGETS_EVENT_H
#define BACKYARDBRAINS_WIDGETS_EVENT_H

#include "Point.h"
#include <SDL_keysym.h>

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
typedef int MouseButtons;

enum KeyCode {
	// Number keys

	Key0 = SDLK_0,
	Key1 = SDLK_1,
	Key2 = SDLK_2,
	Key3 = SDLK_3,
	Key4 = SDLK_4,
	Key5 = SDLK_5,
	Key6 = SDLK_6,
	Key7 = SDLK_7,
	Key8 = SDLK_8,
	Key9 = SDLK_9,

	// Alphabet

	Keya = SDLK_a,
	Keyb = SDLK_b,
	Keyc = SDLK_c,
	Keyd = SDLK_d,
	Keye = SDLK_e,
	Keyf = SDLK_f,
	Keyg = SDLK_g,
	Keyh = SDLK_h,
	Keyi = SDLK_i,
	Keyj = SDLK_j,
	Keyk = SDLK_k,
	Keyl = SDLK_l,
	Keym = SDLK_m,
	Keyn = SDLK_n,
	Keyo = SDLK_o,
	Keyp = SDLK_p,
	Keyq = SDLK_q,
	Keyr = SDLK_r,
	Keys = SDLK_s,
	Keyt = SDLK_t,
	Keyu = SDLK_u,
	Keyv = SDLK_v,
	Keyw = SDLK_w,
	Keyx = SDLK_x,
	Keyy = SDLK_y,
	Keyz = SDLK_z,
};

enum KeyModifier {
	KModNone = KMOD_NONE,
	KModLShift = KMOD_LSHIFT,
	KModRShift = KMOD_RSHIFT,
	KModLCtrl = KMOD_LCTRL,
	KModRCtrl = KMOD_RCTRL,
	KModLAlt = KMOD_LALT,
	KModRAlt = KMOD_RALT,
	KModLMeta = KMOD_LMETA,
	KModRMeta = KMOD_RMETA,
	KModNum = KMOD_NUM,
	KModCaps = KMOD_CAPS,
	KModMode = KMOD_MODE,

	KModCtrl = KMOD_LCTRL|KMOD_RCTRL,
	KModShift = KMOD_LSHIFT|KMOD_RSHIFT,
	KModAlt = KMOD_LALT|KMOD_RALT,
	KModMeta = KMOD_LMETA|KMOD_RMETA
};

typedef int KeyModifiers;

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

class KeyboardEvent : public Event {
public:
	KeyboardEvent(KeyCode key, int mod) : _key(key), _mod(mod) {
	}

	KeyCode key() const { return _key; }
	KeyModifiers mod() const { return _mod; }

private:
	KeyCode _key;
	KeyModifiers _mod;
};


} // namespace Widgets

} // namespace BackyardBrains

#endif
