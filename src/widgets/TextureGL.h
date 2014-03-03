#ifndef BACKYARDBRAINS_WIDGETS_TEXTUREGL_H
#define BACKYARDBRAINS_WIDGETS_TEXTUREGL_H

#include <string>
#include <list>

#include <SDL_opengl.h>

namespace BackyardBrains {

namespace Widgets {

class TextureGL
{
public:

	~TextureGL();

	static void reloadAll();

	static void load(const char *f);
	void unload();
	void reload();

	void bind() const;

	static const TextureGL *get(const char *f);
private:
	TextureGL();
	std::string filename;
	GLuint id;

	typedef std::list<TextureGL> InstanceSet;
	static InstanceSet instances;
};

} // namespace Widgets

} // namespace BackyardBrains

#endif
