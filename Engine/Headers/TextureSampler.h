#pragma once

#include "IResource.h"
#include <vector>

namespace Resources
{
    class Texture;
    class DynamicTexture;

    class TextureSampler
    {
    private:
        unsigned int id;

    public:
        TextureSampler();
        ~TextureSampler();

        void SetDefaultParams();
        unsigned int GetMaxTextures();

        unsigned int GetId() { return id; }
    };
}