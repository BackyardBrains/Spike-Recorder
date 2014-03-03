#ifndef BACKYARDBRAINS_WIDGETS_ABSTRACTSCROLLAREA_H
#define BACKYARDBRAINS_WIDGETS_ABSTRACTSCROLLAREA_H

#include "Widget.h"

#include <string>

namespace BackyardBrains {

namespace Widgets {

class AbstractScrollArea : public Widget
{
public:
	AbstractScrollArea(Widget *parent = NULL);
	~AbstractScrollArea();

	// Size sizeHint() const;
private:
	void paintEvent();
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
