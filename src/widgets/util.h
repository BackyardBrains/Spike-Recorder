#ifndef BACKYARDBRAINS_WIDGETS_UTIL_H
#define BACKYARDBRAINS_WIDGETS_UTIL_H

#include <algorithm>

namespace BackyardBrains {

namespace Widgets {

template <typename T>
const T BoundedValue(const T value, const T minVal, const T maxVal)
{
	return std::max(std::min(value, maxVal), minVal);
}

} // namespace Widgets

} // namespace BackyardBrains

#endif
