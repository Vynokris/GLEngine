#pragma once

#include <string>
#include <vector>
#include "IResource.h"


namespace Resources
{
    class SubMesh;

    class Mesh : public IResource
    {
    public:
        std::vector<SubMesh*> subMeshes;

        Mesh(const std::string& _name);
        ~Mesh();

        void Load()         override;
        void SendToOpenGL() override;
        bool AreAllSubMeshesLoaded();
        bool AreAllSubMeshesInOpenGL();

        static ResourceTypes GetResourceType() { return ResourceTypes::Mesh; }
    };
}