#ifndef BACKYARDBRAINS_WIDGETS_SIZE_H
#define BACKYARDBRAINS_WIDGETS_SIZE_H

#include <algorithm>

namespace BackyardBrains {

namespace Widgets {

template <typename T>
struct SizeBase
{
	SizeBase() : w(0), h(0)
	{
	}
	SizeBase(T ww, T hh) : w(ww), h(hh)
	{
	}
	/*SizeBase(const Size &other) : w(other.w), h(other.h)
	{
	}*/
	bool operator==(const SizeBase &other) const
	{
		return w == other.w && h == other.h;
	}
	bool operator!=(const SizeBase &other) const
	{
		return w != other.w || h != other.h;
	}
	SizeBase boundedTo(const SizeBase &other) const
	{
		return SizeBase(std::min(w, other.w), std::min(h, other.h));
	}
	SizeBase expandedTo(const SizeBase &other) const
	{
		return SizeBase(std::max(w, other.w), std::max(h, other.h));
	}
	bool isValid() const {return w > static_cast<T>(0) && h > static_cast<T>(0);}
	bool isNull() const {return w == static_cast<T>(0) && h == static_cast<T>(0);}
	T w;
	T h;
};

typedef SizeBase<int> Size;
typedef SizeBase<float> SizeF;

} // namespace Widgets

} // namespace BackyardBrains

#endif
