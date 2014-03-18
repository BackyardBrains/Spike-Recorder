#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <cstring>

namespace BackyardBrains {

namespace Widgets {

GLuint LoadTexture(const char *filename)
{
	GLuint texture = 0;

	SDL_Surface * const surface = IMG_Load(filename);
	if (surface)
	{
		// Check that the image dimensions are a power of 2
		if (((surface->w & (surface->w - 1)) != 0) ||
			((surface->h & (surface->h - 1)) != 0))
			fprintf(stderr, "Warning: width of '%s' is not a power of 2\n", filename);


		GLint bytespp = surface->format->BytesPerPixel;
		GLenum texture_format;
		if(bytespp == 4) {
			if(surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGBA;
			else
				texture_format = GL_BGRA;
		}
		else if(bytespp == 3) {
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGB;
			else
				texture_format = GL_BGR;
		} else {
			fprintf(stderr, "Fatal: '%s' is not truecolor.\n", filename);
			exit(1);
		}

		glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, bytespp, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);

		SDL_FreeSurface(surface);
	}
	else
	{
		fprintf(stderr, "SDL could not load \"%s\": %s\n", filename, SDL_GetError());
		return -1;
	}



	return texture;
}

} // namespace Widgets

} // namespace BackyardBrains
