#pragma once

#include <string>
#include <vector>
#include "IResource.h"


namespace Resources
{
    class ResourceManager;
    class Mesh;

    class MtlFile : public IResource
    {
    private:
        ResourceManager& resourceManager;
        void ParseMtlColor(const std::string& line, float* colorValues);

    public:
        std::vector<IResource*> createdResources;

        MtlFile(const std::string& _name, ResourceManager& _resourceManager);

        void Load()         override;
        void SendToOpenGL() override;

        static ResourceTypes GetResourceType() { return ResourceTypes::MtlFile; }
    };
}