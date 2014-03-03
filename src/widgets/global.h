#ifndef BACKYARDBRAINS_WIDGETS_GLOBAL_H
#define BACKYARDBRAINS_WIDGETS_GLOBAL_H

#include "Flags.h"

namespace BackyardBrains {

namespace Widgets {

enum AlignmentFlag
{
	AlignLeft        = 0x0001,
	AlignRight       = 0x0002,
	AlignHCenter     = 0x0004,
	//AlignJustify     = 0x0008,
	AlignTop         = 0x0020,
	AlignBottom      = 0x0040,
	AlignVCenter     = 0x0080,
	AlignCenter      = AlignVCenter | AlignHCenter,
	//AlignAbsolute    = 0x0010,
	AlignHorizontal_Mask = AlignLeft | AlignRight | AlignHCenter/* | AlignJustify | AlignAbsolute*/,
	AlignVertical_Mask   = AlignTop | AlignBottom | AlignVCenter
};

DECLARE_FLAGS(Alignment, AlignmentFlag);
DECLARE_OPERATORS_FOR_FLAGS(Alignment);

enum Orientation
{
	Horizontal = 0x1,
	Vertical   = 0x2
};

DECLARE_FLAGS(Orientations, Orientation);
DECLARE_OPERATORS_FOR_FLAGS(Orientations);

} // namespace Widgets

} // namespace BackyardBrains

#endif
