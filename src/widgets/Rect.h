#ifndef BACKYARDBRAINS_WIDGETS_RECT_H
#define BACKYARDBRAINS_WIDGETS_RECT_H

#include "Point.h"
#include "Size.h"

#include <algorithm>

namespace BackyardBrains {

namespace Widgets {

template <typename T>
struct RectBase
{
	RectBase()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
	RectBase(T xx, T yy, T ww, T hh)
	{
		x = xx;
		y = yy;
		w = ww;
		h = hh;
	}
	RectBase(const PointBase<T> &tl, const SizeBase<T> &size)
	{
		x = tl.x;
		y = tl.y;
		w = size.w;
		h = size.h;
	}
	RectBase(const RectBase &other)
	{
		x = other.x;
		y = other.y;
		w = other.w;
		h = other.h;
	}
	RectBase & operator=(const RectBase &other)
	{
		x = other.x;
		y = other.y;
		w = other.w;
		h = other.h;

		return *this;
	}
	RectBase operator+(const PointBase<T> &point) const
	{
		return RectBase(x + point.x, y + point.y, w, h);
	}
	RectBase operator-(const PointBase<T> &point) const
	{
		return RectBase(x - point.x, y - point.y, w, h);
	}
	bool operator==(const RectBase &other) const
	{
		return x == other.x && y == other.y && w == other.w && h == other.h;
	}
	bool operator!=(const RectBase &other) const
	{
		return x != other.x || y != other.y || w != other.w || h != other.h;
	}

	T left() const {return x;}
	T right() const {return x + w;}
	T top() const {return y;}
	T bottom() const {return y + h;}
	T centerX() const {return x + w/static_cast<T>(2);}
	T centerY() const {return y + h/static_cast<T>(2);}

	Point topLeft() const {return Point(left(), top());}
	Point topRight() const {return Point(right(), top());}
	Point bottomLeft() const {return Point(left(), bottom());}
	Point bottomRight() const {return Point(right(), bottom());}
	Point center() const {return Point(centerX(), centerY());}

	T width() const {return w;}
	T height() const {return h;}
	SizeBase<T> size() const {return SizeBase<T>(w, h);}

	bool contains(const Point &point) const
	{
		return point.x >= x && point.x < (x + w) && point.y >= y && point.y < (y + h);
	}
	bool contains(T xx, T yy) const
	{
		return xx >= x && xx < (x + w) && yy >= y && yy < (y + h);
	}
	bool isEmpty() const
	{
		return w == static_cast<T>(0) || h == static_cast<T>(0);
	}

	RectBase outset(T amount) const
	{
		return RectBase(x - amount, y - amount, w + amount*static_cast<T>(2), h + amount*static_cast<T>(2));
	}
	RectBase inset(T amount) const
	{
		return RectBase(x + amount, y + amount, w - amount*static_cast<T>(2), h - amount*static_cast<T>(2));
	}
	RectBase translated(const PointBase<T> &point) const
	{
		return RectBase(x + point.x, y + point.y, w, h);
	}

	RectBase intersected(const RectBase<T> &rect) const
	{
		const T l = std::max(left(), rect.left());
		const T t = std::max(top(), rect.top());
		const T r = std::min(right(), rect.right());
		const T b = std::min(bottom(), rect.bottom());
		return RectBase(l, t, r - l, b - t);
	}
	RectBase unioned(const RectBase<T> &rect) const
	{
		const T l = std::min(left(), rect.left());
		const T t = std::min(top(), rect.top());
		const T r = std::max(right(), rect.right());
		const T b = std::max(bottom(), rect.bottom());
		return RectBase(l, t, r - l, b - t);
	}
	RectBase adjusted(T dx, T dy, T dw, T dh) const
	{
		return RectBase(x + dx, y + dy, w + dw - dx, h + dh - dy);
	}
	T x, y, w, h;
};

typedef RectBase<int> Rect;
typedef RectBase<float> RectF;

} // namespace Widgets

} // namespace BackyardBrains

#endif
