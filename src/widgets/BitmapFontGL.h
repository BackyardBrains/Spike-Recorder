#ifndef BACKYARDBRAINS_WIDGETS_BITMAPFONTGL_H
#define BACKYARDBRAINS_WIDGETS_BITMAPFONTGL_H

#include "global.h"
#include "TextureGL.h"

namespace BackyardBrains {

namespace Widgets {

class BitmapFontGL
{
public:
	BitmapFontGL();

	void draw(const char *text, int x, int y, Alignment alignment = 0) const;
	void drawMultiline(const char *text, int x, int y, int width, Alignment alignment = 0) const;
	int characterWidth() const;
	int characterHeight() const;
private:
	const TextureGL *characters;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
