#include "AbstractScrollArea.h"

namespace BackyardBrains {

namespace Widgets {

AbstractScrollArea::AbstractScrollArea(Widget *parent) : Widget(parent)
{
}

AbstractScrollArea::~AbstractScrollArea()
{
}

void AbstractScrollArea::paintEvent()
{
	// Painter::setColor(Colors::white);
	// Painter::drawRect(rect());
}

} // namespace Widgets

} // namespace BackyardBrains
