#include "ObjFile.h"
#include "MtlFile.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include <filesystem>
#include <fstream>
using namespace Resources;


void ObjFile::ParseObjVertexValues(const std::string& line, std::vector<float>& values, const int& startIndex, const int& valCount)
{
    // Find the start and end of the first value.
    size_t start = startIndex;
    size_t end = line.find(' ', start);
    if (end == start) {
        start++;
        end = line.find(' ', start);
    }

    for (int i = 0; i < valCount; i++)
    {
        // Find the current coordinate value.
        std::string val = line.substr(start, end - start);

        // Set the current value.
        values.push_back(std::strtof(val.c_str(), nullptr));

        // Update the start and end indices.
        start = end + 1;
        end = line.find(' ', start);
    }
}

void ObjFile::ParseObjObjectLine(const std::string& line, Mesh*& meshGroup)
{
    // Let the previous mesh group send its vertices to OpenGL.
    if (meshGroup!= nullptr && meshGroup->subMeshes.size() > 0) {
        meshGroup->subMeshes.back()->SetLoadingDone();
        meshGroup->SetLoadingDone();
    }

    meshGroup = resourceManager.Create<Mesh>(line.substr(2, line.size()-3));
    createdResources.push_back(meshGroup);
}

void ObjFile::ParseObjGroupLine(const std::string& line, Mesh*& meshGroup, const ShaderProgram* meshShaderProgram)
{
    if (line[2] == '\0')
        return;

    // Make sure a mesh group was already created.
    if (meshGroup == nullptr) {
        meshGroup = resourceManager.Create<Mesh>("mesh_" + std::filesystem::path(name).stem().string());
        createdResources.push_back(meshGroup);
    }

    // Send all vertex data from the previous model to openGL.
    if (meshGroup->subMeshes.size() > 0) {
        meshGroup->subMeshes.back()->SetLoadingDone();
        meshGroup->SetLoadingDone();
    }

    // Create a model and add it to the mesh group.
    meshGroup->subMeshes.push_back(new SubMesh(line.substr(2, line.size() - 3), meshShaderProgram));
}

void ObjFile::ParseObjUsemtlLine(const std::string& line, Mesh*& meshGroup, const ShaderProgram* meshShaderProgram)
{
    // Make sure a mesh group was already created.
    if (meshGroup == nullptr) {
        meshGroup = resourceManager.Create<Mesh>("mesh_" + std::filesystem::path(name).stem().string());
        createdResources.push_back(meshGroup);
    }

    // Make sure a mesh was already created.
    if (meshGroup->subMeshes.size() <= 0)
        meshGroup->subMeshes.push_back(new SubMesh("submesh_" + line.substr(7, line.size() - 8), meshShaderProgram));

    // Make sure the current mesh doesn't already have a material.
    else if (meshGroup->subMeshes.back()->GetMaterial() != nullptr) {
        meshGroup->subMeshes.back()->SetLoadingDone();
        meshGroup->subMeshes.push_back(new SubMesh("submesh_" + line.substr(7, line.size() - 8), meshShaderProgram));
    }

    // Set the current model's material.
    meshGroup->subMeshes.back()->SetMaterial(resourceManager.Get<Material>(line.substr(7, line.size() - 8)));
}

void ObjFile::ParseObjIndices(const std::string& line, std::stringstream& fileContents, Mesh*& meshGroup, std::array<std::vector<float>, 3>& vertexData, const ShaderProgram* meshShaderProgram)
{
    // Make sure a mesh group was already created.
    if (meshGroup == nullptr) {
        meshGroup = resourceManager.Create<Mesh>("mesh_" + std::filesystem::path(name).stem().string());
        createdResources.push_back(meshGroup);
    }

    // Make sure a mesh was already created.
    if (meshGroup->subMeshes.size() <= 0)
        meshGroup->subMeshes.push_back(new SubMesh("submesh_" + std::filesystem::path(name).stem().string(), meshShaderProgram));

    // Let the current model parse its vertices.
    fileContents.seekg(fileContents.tellg() - std::streamoff(line.size()));
    meshGroup->subMeshes.back()->LoadVertices(fileContents, vertexData);
}

ObjFile::ObjFile(const std::string& _name, ResourceManager& _resourceManager)
    : resourceManager(_resourceManager)
{
    name = _name;
    type = ResourceTypes::ObjFile;
}

void ObjFile::Load()
{
    const ShaderProgram* shaderProgram = resourceManager.Get<ShaderProgram>("ShaderProgram"); // TEMP: Should be done automatically.

    // Start chrono.
    std::chrono::steady_clock::time_point chronoStart = std::chrono::high_resolution_clock::now();

    // Read file contents and extract them to the data string.
    std::stringstream fileContents;
    {
        std::fstream f(name, std::ios_base::in | std::ios_base::app);
        fileContents << f.rdbuf();
        f.close();
    }
    std::string filepath = std::filesystem::path(name).parent_path().string() + "/";

    // Define dynamic arrays for positions, uvs, and normals.
    // Format: vertexData[0] = positions, vertexData[1] = uvs, vertexData[2] = normals.
    std::array<std::vector<float>, 3> vertexData;

    // Store a pointer to the mesh group that is currently being created.
    Mesh* meshGroup = nullptr;

    // Read file line by line to create vertex data.
    std::string line;
    while (std::getline(fileContents, line))
    {
        line += ' ';
        switch (line[0])
        {
        case 'v':
            switch (line[1])
            {
            // Parse vertex coords and normals.
            case ' ':
                ParseObjVertexValues(line, vertexData[0], 2, 3);
                break;
            // Parse vertex UVs.
            case 't':
                ParseObjVertexValues(line, vertexData[1], 3, 2);
                break;
            // Parse vertex normals.
            case 'n':
                ParseObjVertexValues(line, vertexData[2], 3, 3);
                break;
            default:
                break;
            }
            break;

        case 'o':
            ParseObjObjectLine(line, meshGroup);
            break;

        case 'g':
            ParseObjGroupLine(line, meshGroup, shaderProgram);
            break;

        case 'm':
            createdResources.push_back(resourceManager.Create<MtlFile>(filepath + line.substr(7, line.size()-8)));
            break;

        case 'u':
            ParseObjUsemtlLine(line, meshGroup, shaderProgram);
            break;

        case 'f':
            ParseObjIndices(line, fileContents, meshGroup, vertexData, shaderProgram);
            break;

        default:
            break;
        }
    }
    if (meshGroup != nullptr && meshGroup->subMeshes.size() > 0) {
        meshGroup->subMeshes.back()->SetLoadingDone();
        meshGroup->SetLoadingDone();
    }
    else {
        DebugLogWarning("Mesh has no sub-meshes after being loaded from obj file: " + name);
    }

    // End chrono.
    std::chrono::steady_clock::time_point chronoEnd = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(chronoEnd - chronoStart);
    DebugLog((std::string("Loading file ") + name + std::string(" took ") + std::to_string(elapsed.count() * 1e-9) + " seconds.").c_str());
    SetLoadingDone();
}

void ObjFile::SendToOpenGL()
{
    SetOpenGLTransferDone();
}
