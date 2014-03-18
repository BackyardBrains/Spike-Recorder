#ifndef BACKYARDBRAINS_WIDGETS_SIZEPOLICY_H
#define BACKYARDBRAINS_WIDGETS_SIZEPOLICY_H

#include "global.h"
#include "Rect.h"
#include "Size.h"

namespace BackyardBrains {

namespace Widgets {

class SizePolicy
{
public:
	enum PolicyFlag
	{
		GrowFlag   = 1,
		ExpandFlag = 2,
		ShrinkFlag = 4,
		IgnoreFlag = 8
	};
	enum Policy
	{
		Fixed            = 0,
		Minimum          = GrowFlag,
		Maximum          = ShrinkFlag,
		Preferred        = GrowFlag | ShrinkFlag,
		Expanding        = GrowFlag | ShrinkFlag | ExpandFlag,
		MinimumExpanding = GrowFlag | ExpandFlag,
		Ignored          = ShrinkFlag | GrowFlag | IgnoreFlag
	};
public:
	SizePolicy() : _horizontal(Fixed), _vertical(Fixed) {}
	SizePolicy(Policy horiz, Policy vert) : _horizontal(horiz), _vertical(vert) {}

	Orientations expandingDirections() const
	{
		Orientations result = 0;
		if (verticalPolicy() & ExpandFlag)
			result |= Vertical;
		if (horizontalPolicy() & ExpandFlag)
			result |= Horizontal;
		return result;
	}

	Policy horizontalPolicy() const {return _horizontal;}
	Policy verticalPolicy() const {return _vertical;}
	void setHorizontalPolicy(Policy policy) {_horizontal = policy;}
	void setVerticalPolicy(Policy policy) {_vertical = policy;}
private:
	Policy _horizontal;
	Policy _vertical;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
