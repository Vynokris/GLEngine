#include <glad/glad.h>
#include <STB_Image/stb_image.h>

#include "Cubemap.h"
#include "Textures.h"
#include "Debug.h"
using namespace Resources;

Resources::Cubemap::Cubemap(const std::string& _name)
{
	name = _name;
	type = ResourceTypes::Cubemap;
}

Resources::Cubemap::~Cubemap()
{
	glDeleteTextures(1, &id);
}

void Resources::Cubemap::Load()
{
	SetLoadingDone();
}

void Resources::Cubemap::SendToOpenGL()
{
    if (!IsLoaded() || WasSentToOpenGL())
        return;

	// Generate cubemap texture id.
    DebugLog("Sending cubemap to openGL: " + name);
	glGenTextures(1, &id);
    if (id == 0) {
        DebugLogWarning("Unable to generate an OpenGL cubemap texture ID.");
        return;
    }
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	int width, height, nrChannels;
	unsigned char *data;
	for (int i = 0; i < 6; i++)
	{
		stbi_set_flip_vertically_on_load_thread(false);
		data = stbi_load(sideTextureNames[i].c_str(), &width, &height, &nrChannels, 0);
		stbi_set_flip_vertically_on_load_thread(true);
		if (!data)
		{
			DebugLogWarning("Unable to load cubemap side texture: " + sideTextureNames[i]);
			glDeleteTextures(1, &id);
			return;
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, (nrChannels == 3 ? GL_RGB : GL_RGBA), width, height, 0, (nrChannels == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);	
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,     GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,     GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,     GL_REPEAT);
	
    SetOpenGLTransferDone();
}

void Resources::Cubemap::SetTexture(const CubeSides& side, std::string name)
{
	sideTextureNames[(int)side] = name;
}
