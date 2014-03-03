#include <SDL.h>
#include <SDL_image.h>
#include <SDL_opengl.h>
#include <cstring>

// #include "GLee.h"

namespace BackyardBrains {

namespace Widgets {

GLuint LoadTexture(const char *filename)
{
	GLuint texture = 0;

	SDL_Surface * const surface = IMG_Load(filename);
	if (surface)
	{
		// Check that the image's width is a power of 2
		if (((surface->w & (surface->w - 1)) != 0) ||
			((surface->h & (surface->h - 1)) != 0))
			fprintf(stderr, "warning: \"%s\"'s width is not a power of 2\n", filename);

		// get the number of channels in the SDL surface
		GLint nOfColors = surface->format->BytesPerPixel;
		GLenum texture_format;
		if (nOfColors == 4)     // contains an alpha channel
		{
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGBA;
			else
				texture_format = GL_BGRA;
		}
		else if (nOfColors == 3)     // no alpha channel
		{
			if (surface->format->Rmask == 0x000000ff)
				texture_format = GL_RGB;
			else
				texture_format = GL_BGR;
		}
		else
		{
			fprintf(stderr, "warning: \"%s\" is not truecolor...  this will probably break\n", filename);
			return -1;
		}

		// Have OpenGL generate a texture object handle for us
		glGenTextures(1, &texture);

		// Bind the texture object
		glBindTexture(GL_TEXTURE_2D, texture);

		// Set the texture's stretching properties
		//check whether extension string can be found
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		/*if (strstr((char*)glGetString(GL_EXTENSIONS), "GL_EXT_texture_filter_anisotropic"))
		{
			float maximumAnisotropy = 1.1;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximumAnisotropy);
		}*/

		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D(GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0, texture_format, GL_UNSIGNED_BYTE, surface->pixels);
	}
	else
	{
		fprintf(stderr, "SDL could not load \"%s\": %s\n", filename, SDL_GetError());
		return -1;
	}

	// Free the SDL_Surface only if it was successfully created
	if (surface)
		SDL_FreeSurface(surface);

	return texture;
}

} // namespace Widgets

} // namespace BackyardBrains
