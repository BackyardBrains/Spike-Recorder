#ifndef BACKYARDBRAINS_WIDGETS_COLOR_H
#define BACKYARDBRAINS_WIDGETS_COLOR_H

#include <stdint.h>
#include <climits>
#include "util.h"

namespace BackyardBrains {

namespace Widgets {

struct Color
{
	Color() : r(0), g(0), b(0), a(0)
	{
	}
	Color(uint8_t rr, uint8_t gg, uint8_t bb, uint8_t aa = 255) : r(rr), g(gg), b(bb), a(aa)
	{
	}
	Color(const Color &other) : r(other.r), g(other.g), b(other.b), a(other.a)
	{
	}
	Color & operator=(const Color &other)
	{
		r = other.r;
		g = other.g;
		b = other.b;
		a = other.a;
		return *this;
	}
	bool operator==(const Color &other) const
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}
	bool operator!=(const Color &other) const
	{
		return r != other.r || g != other.g || b != other.b || a != other.a;
	}
	void setRed(int rr) {r = BoundedValue(rr, 0, UCHAR_MAX);}
	void setGreen(int gg) {g = BoundedValue(gg, 0, UCHAR_MAX);}
	void setBlue(int bb) {b = BoundedValue(bb, 0, UCHAR_MAX);}
	void setAlpha(int aa) {a = BoundedValue(aa, 0, UCHAR_MAX);}
	float redF() const {return static_cast<float>(r)/static_cast<float>(UCHAR_MAX);}
	float greenF() const {return static_cast<float>(g)/static_cast<float>(UCHAR_MAX);}
	float blueF() const {return static_cast<float>(b)/static_cast<float>(UCHAR_MAX);}
	float alphaF() const {return static_cast<float>(a)/static_cast<float>(UCHAR_MAX);}
	void setRedF(float rr) {r = static_cast<uint8_t>(rr*static_cast<float>(UCHAR_MAX));}
	void setGreenF(float gg) {g = static_cast<uint8_t>(gg*static_cast<float>(UCHAR_MAX));}
	void setBlueF(float bb) {b = static_cast<uint8_t>(bb*static_cast<float>(UCHAR_MAX));}
	void setAlphaF(float aa) {a = static_cast<uint8_t>(aa*static_cast<float>(UCHAR_MAX));}
	uint8_t r, g, b, a;
};

namespace Colors {

static const Color red(255, 0, 0);
static const Color green(0, 255, 0);
static const Color blue(0, 0, 255);
static const Color white(255, 255, 255);
static const Color black(0, 0, 0);
static const Color yellow(255, 255, 0);
static const Color cyan(0, 255, 255);
static const Color violet(255, 0, 255);
static const Color gray(128, 128, 128);

static const Color background(25,25,25);
static const Color button(128,128,128);
static const Color buttonhigh(150,150,150);
static const Color widgetbg(80,80,80);
static const Color widgetbgdark(50,50,50);

} // namespace Colors

} // namespace Widgets

} // namespace BackyardBrains

#endif
