#pragma once

#include "IResource.h"

namespace Core::Maths
{
    // Forward declaration of color.
    class RGBA;
}

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
        unsigned int GetId()     { return id;     }
        int GetWidth()           { return width;  }
        int GetHeight()          { return height; }
        int GetColorChannels()   { return colorChannels; }
        static ResourceTypes GetResourceType() { return ResourceTypes::DynamicTexture; }

        Core::Maths::RGBA GetPixel(const int& x, const int& y);
        void SetPixel(const int& x, const int& y, const Core::Maths::RGBA& color, bool updateTexture = true);
        void UpdateTexture();
    };
}