#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <STB_Image/stb_image.h>

#include <string>
#include <cstdio>
#include <functional>
#include <direct.h>
#pragma warning(disable : 4996)

#include "Maths.h"
#include "Arithmetic.h"
#include "ResourceManager.h"
#include "Textures.h"
using namespace Core::Maths;
using namespace Resources;



// ----- Static Texture ----- //

// Creates a texture out of the specified image file.
Texture::Texture(const std::string& _name)
{
    // Set texture variables and load texture.
    name = _name;
    type = ResourceTypes::Texture;
}

// Creates an empty texture of specified width and height.
Texture::Texture(const int& _width, const int& _height)
{
    name   = "";
    width  = _width;
    height = _height;
    type   = ResourceTypes::Texture;
    colorChannels = 4;

    // Create texture.
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

void Texture::Load()
{
    int w, h, nrChannels;

    // Load texture data from binary file.
    if (FILE* f = fopen(("Binaries/" + std::to_string(std::hash<std::string>{}(name)) + ".bin").c_str(), "rb"))
    {
        hashed = true;
        fread(&w, sizeof(int), 1, f);
        fread(&h, sizeof(int), 1, f);
        fread(&nrChannels, sizeof(int), 1, f);

        data = new unsigned char[w * h * nrChannels];
        fread(data, sizeof(unsigned char), w * h * nrChannels, f);
        fclose(f);
    }

    // Load texture data with stbi.
    else
    {
        data = stbi_load(name.c_str(), &w, &h, &nrChannels, 0);
    }

    // Check if the data was correctly loaded.
    if (!data)
    {
        DebugLogWarning("Unable to open texture file: " + name);
        glDeleteTextures(1, &id);
        return;
    }

    // Save image parameters.
    name   = name;
    width  = w;
    height = h;
    colorChannels = nrChannels;
    SetLoadingDone();
}

void Texture::SendToOpenGL()
{
    if (!IsLoaded() || WasSentToOpenGL())
        return;

    // Create OpenGL texture from data.
    DebugLog("Sending texture to openGL: " + name);
    glGenTextures(1, &id);
    if (id == 0) {
        DebugLogWarning("Unable to generate an OpenGL texture ID.");
        return;
    }
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, (colorChannels == 3 ? GL_RGB : GL_RGBA), width, height, 0, (colorChannels == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Save texture data to hashed binary file.
    if (!hashed)
    {
        _mkdir("Binaries");
        FILE* f = fopen(("Binaries/" + std::to_string(std::hash<std::string>{}(name)) + ".bin").c_str(), "wb");
        if (f != nullptr) {
            fwrite(&width, sizeof(int), 1, f);
            fwrite(&height, sizeof(int), 1, f);
            fwrite(&colorChannels, sizeof(int), 1, f);
            fwrite(data, sizeof(unsigned char), width * height * colorChannels, f);
            fclose(f);
        }
        stbi_image_free(data);
    }

    // Simply free texture data.
    else
    {
        delete[] data;
    }
    SetOpenGLTransferDone();
}

Texture::~Texture()
{
    glDeleteTextures(1, &id);
}



// ----- Dynamic Texture ----- //

DynamicTexture::DynamicTexture(const std::string& _name)
{
    // Set and declare texture variables.
    name = _name;
    type = ResourceTypes::DynamicTexture;
}

DynamicTexture::~DynamicTexture()
{
    glDeleteTextures(1, &id);
    stbi_image_free(data);
}

void DynamicTexture::Load()
{
    int w, h, nrChannels;

    // Load texture.
    data = stbi_load(name.c_str(), &w, &h, &nrChannels, 0);
    Assert(data != NULL, "Unable to open texture: " + name);

    // Save image parameters.
    width = w;
    height = h;
    colorChannels = nrChannels;
    SetLoadingDone();
}

void DynamicTexture::SendToOpenGL()
{
    if (!IsLoaded())
        return;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, (colorChannels == 3 ? GL_RGB : GL_RGBA), width, height, 0, (colorChannels == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    SetOpenGLTransferDone();
}

RGBA DynamicTexture::GetPixel(const int& x, const int& y)
{
    int index = (y * width + x) * colorChannels;
    return RGBA(data[index] / 255.f, data[index + 1] / 255.f, data[index + 2] / 255.f, (colorChannels == 3 ? 255 : data[index + 3]) / 255.f);
}

void DynamicTexture::SetPixel(const int& x, const int& y, const RGBA& color, bool updateTexture)
{
    unsigned char* pixelOffset = data + ((height - y) * width + x) * colorChannels;
    pixelOffset[0] = (unsigned int)(color.r * 255);
    pixelOffset[1] = (unsigned int)(color.g * 255);
    pixelOffset[2] = (unsigned int)(color.b * 255);
    if (colorChannels > 3)
        pixelOffset[3] = (unsigned int)(color.a * 255);
    if (updateTexture)
        UpdateTexture();
}

void DynamicTexture::UpdateTexture()
{
    glDeleteTextures(1, &id);
    SendToOpenGL();
}



// ----- Render Texture ----- //

RenderTexture::RenderTexture(const std::string& _name)
{
    name = _name;
    type = ResourceTypes::RenderTexture;
}

RenderTexture::~RenderTexture()
{
    glDeleteRenderbuffers(1, &rbo);
    glDeleteTextures(1, &id);
    glDeleteFramebuffers(1, &fbo);
}

void RenderTexture::Load()
{
    if (IsLoaded() || width <= 0 || height <= 0)
        return;
    SetLoadingDone();
}

void RenderTexture::SendToOpenGL()
{
    if (!IsLoaded() || WasSentToOpenGL())
        return;

    // Create Frame Buffer Object
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Create Framebuffer Texture
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);

	// Create Render Buffer Object
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		DebugLogError("Unable to create rendertexture framebuffer (error code: " + std::to_string(fboStatus) + ")");
        glDeleteRenderbuffers(1, &rbo);
        glDeleteTextures(1, &id);
        glDeleteFramebuffers(1, &fbo);
        return;
    }

    SetOpenGLTransferDone();
}

void RenderTexture::BeginRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Clear the framebuffer.
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderTexture::EndRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // TODO: this requires a rendertexture stack to work with nested render textures.
}

void RenderTexture::SetWidth(const int& _width )
{
    width = _width;
    if (WasSentToOpenGL())
    {
        glDeleteRenderbuffers(1, &rbo);
        glDeleteTextures(1, &id);
        glDeleteFramebuffers(1, &fbo);
        sentToOpenGL = false;
        SendToOpenGL();
    }
}

void RenderTexture::SetHeight(const int& _height)
{
    height = _height;
    if (WasSentToOpenGL())
    {
        glDeleteRenderbuffers(1, &rbo);
        glDeleteTextures(1, &id);
        glDeleteFramebuffers(1, &fbo);
        sentToOpenGL = false;
        SendToOpenGL();
    }
}

void RenderTexture::SetSize(const int& _width, const int& _height)
{
    width  = _width;
    height = _height;
    if (WasSentToOpenGL())
    {
        glDeleteRenderbuffers(1, &rbo);
        glDeleteTextures(1, &id);
        glDeleteFramebuffers(1, &fbo);
        sentToOpenGL = false;
        SendToOpenGL();
    }
}
