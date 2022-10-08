#include "SubMesh.h"
#include "Mesh.h"
using namespace Resources;


Mesh::Mesh(const std::string& _name)
{
    name = _name;
    type = ResourceTypes::Mesh;
}

Mesh::~Mesh()
{
    for (SubMesh* subMesh : subMeshes)
        delete subMesh;
}

void Mesh::Load()
{
    if (AreAllSubMeshesLoaded())
        SetLoadingDone();
}

void Mesh::SendToOpenGL()
{
    if (AreAllSubMeshesInOpenGL())
        SetOpenGLTransferDone();
}

bool Mesh::AreAllSubMeshesLoaded()
{
    if (subMeshes.size() <= 0)
        return false;

    for (int i = 0; i < subMeshes.size(); i++)
        if (!subMeshes[i]->IsLoaded())
            return false;
    return true;
}

bool Mesh::AreAllSubMeshesInOpenGL()
{
    if (subMeshes.size() <= 0)
        return false;

    for (int i = 0; i < subMeshes.size(); i++)
        if (!subMeshes[i]->WasSentToOpenGL())
            return false;
    return true;
}
