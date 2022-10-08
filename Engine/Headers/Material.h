#pragma once

#include "IResource.h"
#include "Color.h"

namespace Resources
{
    class ResourceManager;
    class Texture;

    class Material : public IResource
    {
    public:
        Core::Maths::RGB ambient, diffuse, specular, emission;
        float shininess = 32, transparency = 1;

        Texture* ambientTexture  = nullptr;
        Texture* diffuseTexture  = nullptr;
        Texture* specularTexture = nullptr;
        Texture* emissionTexture = nullptr;
        Texture* shininessMap    = nullptr;
        Texture* alphaMap        = nullptr;
        Texture* normalMap       = nullptr;

        Material(const std::string& _name);
        void Load() override;
        void SendToOpenGL() override;
        void SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const Core::Maths::RGB& _emission, const float& _shininess);
        void SendDataToShader(const unsigned int& shaderProgramId, const unsigned int& sampler) const;

        static ResourceTypes GetResourceType() { return ResourceTypes::Material; }
    };
}