#include <glad/glad.h>

#include <string>
#include "Textures.h"
#include "TextureSampler.h"
using namespace Resources;

TextureSampler::TextureSampler()
{
    glGenSamplers(1, &id);
	GetMaxTextures();
}

TextureSampler::~TextureSampler()
{
	glDeleteSamplers(1, &id);
}

void TextureSampler::SetDefaultParams()
{
	glSamplerParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameterf(id, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);
}

unsigned int TextureSampler::GetMaxTextures()
{
	GLint max = 0;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max);
	return max;
}
