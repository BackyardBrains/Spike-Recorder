#include "BitmapFontGL.h"
#include "Log.h"
#include <cstring>
#include <string>

namespace BackyardBrains {

namespace Widgets {

static const float stride = 1.0/16.0;
static const int strideX = 8;
static const int strideY = 16;

BitmapFontGL::BitmapFontGL() {
	Log::msg("Initializing font...");
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

void BitmapFontGL::drawMultiline(const char *t, int x, int y, int width, Alignment alignment) const {
	std::string text(t);
	
	std::string::iterator lstart = text.begin();
	std::string::iterator lbreak = text.begin();
	int i = 0;
	int line = 0;
	int linelen = std::max(1,width/characterWidth());

	for(std::string::iterator it = text.begin(); it != text.end(); it++) {
		if(*it == ' ' || *it == '\n') {
			lbreak = it;
		}

		if(i >= linelen || *it == '\n') {
			if(lbreak < lstart) {
				lbreak = it;
			}
			draw(std::string(lstart, lbreak).c_str(), 10, 10+line*(characterHeight()+2), alignment);
			lstart = lbreak+1;
			i = it-lstart;
			line++;
		} else {
			i++;
		}
	}

	draw(std::string(lstart, text.end()).c_str(), x, y+line*(characterHeight()+2), alignment);
}

} // namespace Widgets

} // namespace BackyardBrains
