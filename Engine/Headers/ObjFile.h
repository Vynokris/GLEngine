#pragma once

#include <string>
#include <vector>
#include "IResource.h"


namespace Resources
{
    class ResourceManager;
    class Mesh;
    class ShaderProgram;

    class ObjFile : public IResource
    {
    private:
        ResourceManager& resourceManager;
        void ParseObjVertexValues(const std::string& line, std::vector<float>& values, const int& startIndex, const int& valCount);
        void ParseObjObjectLine  (const std::string& line, Mesh*& meshGroup);
        void ParseObjGroupLine   (const std::string& line, Mesh*& meshGroup, const ShaderProgram* meshShaderProgram);
        void ParseObjUsemtlLine  (const std::string& line, Mesh*& meshGroup, const ShaderProgram* meshShaderProgram);
        void ParseObjIndices     (const std::string& line, std::stringstream& fileContents, Mesh*& meshGroup, std::array<std::vector<float>, 3>& vertexData, const ShaderProgram* meshShaderProgram);

    public:
        std::vector<IResource*> createdResources;

        ObjFile(const std::string& _name, ResourceManager& _resourceManager);
        
        void Load()         override;
        void SendToOpenGL() override;

        static ResourceTypes GetResourceType() { return ResourceTypes::ObjFile; }
    };
}