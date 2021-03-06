#ifndef BACKYARDBRAINS_WIDGETS_PAINTER_H
#define BACKYARDBRAINS_WIDGETS_PAINTER_H

#include "Color.h"
#include "Rect.h"
#include "Point.h"

namespace BackyardBrains {

namespace Widgets {

class Painter
{
public:
	Painter();
	~Painter();

	static void setColor(const Color &color);
    static void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, bool filled = true);
	static void drawRect(const Rect &rect, bool filled = true);
	static void drawRectFast(const Rect &rect);
	static void drawTexRect(const Rect &rect);
	static void drawCircle(const Point &center, int radius, int points = 16);
private:

};

} // namespace Widgets

} // namespace BackyardBrains

#endif
