#include "TextureGL.h"
#include "LoadTexture.h"
#include <cassert>
#include <cstdio>

namespace BackyardBrains {

namespace Widgets {

TextureGL::InstanceSet TextureGL::instances;

TextureGL::TextureGL() : id(0)
{
}

TextureGL::~TextureGL()
{
	unload();
}

void TextureGL::reloadAll()
{
	for (InstanceSet::iterator it = instances.begin(); it != instances.end(); ++it)
	{
		(*it).reload();
	}
}

void TextureGL::load(const char *f)
{
	TextureGL tex;
	fprintf(stderr,"Texture '%s' loaded.\n", f);
	tex.filename.assign(f);
	tex.id = LoadTexture(f);
	instances.push_back(tex);

}

void TextureGL::unload()
{
	if (id == 0)
		return;
	glDeleteTextures(1, &id);
	id = 0;
	// filename.assign("");
}

void TextureGL::reload() {
	if(id != 0)
		unload();

	id = LoadTexture(filename.c_str());
}

void TextureGL::bind() const
{
	glBindTexture(GL_TEXTURE_2D, id);
}

const TextureGL* TextureGL::get(const char *filename) {
	for(InstanceSet::iterator it = instances.begin(); it != instances.end(); it++)
		if((*it).filename == filename)
			return &*it;
	assert(false);
	return NULL;
}

} // namespace Widgets

} // namespace BackyardBrains
