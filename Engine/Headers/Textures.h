#pragma once

#include "IResource.h"
#include "Color.h"

namespace Resources
{
    class TextureSampler;

    // Static, unchanging texture.
    class Texture : public IResource
    {
    private:
        unsigned int id = 0;
        bool hashed = false;
        int width, height, colorChannels;
        unsigned char* data;

    public:
        Texture(const std::string& _name);
        Texture(const int& _width, const int& _height);
        void Load() override;
        void SendToOpenGL() override;
        ~Texture();

        unsigned int GetId()     { return id;     }
        int GetWidth()           { return width;  }
        int GetHeight()          { return height; }
        int GetColorChannels()   { return colorChannels; }
        static ResourceTypes GetResourceType() { return ResourceTypes::Texture; }
    };

    // Dynamic texture with modifiable pixels.
    class DynamicTexture : public IResource
    {
    private:
        unsigned int id = 0;
        int width, height, colorChannels;
        unsigned char* data;

    public:
        DynamicTexture(const std::string& _name);
        ~DynamicTexture();
        
        void Load() override;
        void SendToOpenGL() override;

        Core::Maths::RGBA GetPixel(const int& x, const int& y);
        void SetPixel(const int& x, const int& y, const Core::Maths::RGBA& color, bool updateTexture = true);
        void UpdateTexture();

        unsigned int GetId()   { return id;     }
        int GetWidth()         { return width;  }
        int GetHeight()        { return height; }
        int GetColorChannels() { return colorChannels; }
        static ResourceTypes GetResourceType() { return ResourceTypes::DynamicTexture; }
    };

    class RenderTexture : public IResource
    {
    private:
        unsigned int fbo = 0, rbo = 0, id = 0;
        int width, height;

    public:
        Core::Maths::RGBA clearColor;

        RenderTexture(const std::string& _name);
        ~RenderTexture();

        void Load() override;
        void SendToOpenGL() override;

        void BeginRender();
        void EndRender();

        void SetWidth (const int& _width );
        void SetHeight(const int& _height);
        void SetSize  (const int& _width, const int& _height);

        unsigned int GetId    () { return id;     }
        int          GetWidth () { return width;  }
        int          GetHeight() { return height; }
        static ResourceTypes GetResourceType() { return ResourceTypes::RenderTexture; }
    };
}