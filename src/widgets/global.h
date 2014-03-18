#ifndef BACKYARDBRAINS_WIDGETS_GLOBAL_H
#define BACKYARDBRAINS_WIDGETS_GLOBAL_H

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

typedef int Alignment;

enum Orientation
{
	Horizontal = 0x1,
	Vertical   = 0x2
};

typedef int Orientations;

} // namespace Widgets

} // namespace BackyardBrains

#endif
