#include "Painter.h"

#include <SDL_opengl.h>

#include <cmath>

namespace BackyardBrains {

namespace Widgets {

Painter::Painter()
{
}

Painter::~Painter()
{
}

void Painter::setColor(const Color &color)
{
	// glColor3b(color.r, color.b, color.g);
	glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void Painter::drawRect(const Rect &rect, bool filled)
{
	glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
	drawRectFast(rect);
	glEnd();
}

void Painter::drawTexRect(const Rect &rect) {
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2i(rect.left(), rect.top());
	glTexCoord2f(0, 1);
	glVertex2i(rect.left(), rect.bottom());
	glTexCoord2f(1, 1);
	glVertex2i(rect.right(), rect.bottom());
	glTexCoord2f(1, 0);
	glVertex2i(rect.right(), rect.top());
	glEnd();
}

void Painter::drawRectFast(const Rect &rect)
{
	glVertex2i(rect.left(), rect.top());
	glVertex2i(rect.left(), rect.bottom());
	glVertex2i(rect.right(), rect.bottom());
	glVertex2i(rect.right(), rect.top());
}

void Painter::drawCircle(const Point &center, int radius, int points)
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex2i(center.x, center.y);
	for (int i = 0; i <= points; i++)
	{
		glVertex2f(center.x + cos(i*2.0*M_PI/points)*radius, center.y - sin(i*2.0*M_PI/points)*radius);
	}
	glEnd();
}

} // namespace Widgets

} // namespace BackyardBrains
