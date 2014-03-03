#include "BitmapFontGL.h"
#include <cstring>

namespace BackyardBrains {

namespace Widgets {

static const float stride = 1.0/16.0;
static const int strideX = 8;
static const int strideY = 16;

BitmapFontGL::BitmapFontGL() {
	characters = TextureGL::get("data/ascii.png");
}

int BitmapFontGL::characterWidth() const
{
	return strideX;
}

int BitmapFontGL::characterHeight() const
{
	return strideY;
}

// TODO use display list calling directly of the text and strlen(text) to accelerate drawing all of the quads
void BitmapFontGL::draw(const char *text, int xx, int yy, Alignment alignment) const
{
	int x = xx;
	int y = yy;

	const int len = strlen(text);
	if (alignment & AlignHCenter)
		x -= (strideX*len)/2;
	else if (alignment & AlignRight)
		x -= strideX*len;
	if (alignment & AlignVCenter)
		y -= strideY/2;
	else if (alignment & AlignBottom)
		y -= strideY;

	characters->bind();
	glBegin(GL_QUADS);
	for (; *text; ++text)
	{
		if (*text == '\n')
		{
			x = xx;
			y += strideY;
		}
		else
		{
			float texX = static_cast<float>(*text & 0xF)/16.0;
			float texY = static_cast<float>(*text >>  4)/16.0;
			glTexCoord2f(texX, texY);
			glVertex2i(x, y);

			glTexCoord2f(texX, texY + stride);
			glVertex2i(x, y + strideY);

			glTexCoord2f(texX + stride, texY + stride);
			glVertex2i(x + strideX, y + strideY);

			glTexCoord2f(texX + stride, texY);
			glVertex2i(x + strideX, y + 0);
			x += strideX;
		}

	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Widgets

} // namespace BackyardBrains
