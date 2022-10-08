#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <chrono>

#include <glad/glad.h>

#include "Maths.h"
#include "SubMesh.h"
#include "Material.h"
using namespace Core::Maths;
using namespace Resources;


VertexIndices ParseObjIndices(std::string indicesStr)
{
    VertexIndices vertex = { 0, 0, 0 };
    size_t start = 0;
    size_t end = indicesStr.find('/', start);
    for (int i = 0; i < 3 && indicesStr[start] != '\0'; i++)
    {
        // Find the current coordinate value.
        std::string index = indicesStr.substr(start, end - start);

        // Set the current index and skip double slashes.
        if (start != end)
            *((&vertex.pos) + i) = std::atoi(index.c_str()) - 1;

        // Update the start and end indices.
        start = end + 1;
        end = indicesStr.find('/', start);
    }
    return vertex;
}

void ParseObjTriangle(std::string line, std::array<std::vector<uint32_t>, 3>& indices)
{
    size_t start = 2;
    size_t end = line.find(' ', start);
    if (end == start) {
        start++;
        end = line.find(' ', start);
    }

    for (int i = 0; i < 3; i++)
    {
        // Find the current coordinate value.
        std::string indicesStr = line.substr(start, end - start) + '/';

        // Set the current index.
        VertexIndices vertexIndices = ParseObjIndices(indicesStr);
        for (int i = 0; i < 3; i++)
            indices[i].push_back(*((&vertexIndices.pos) + i));

        // Update the start and end line of the coord val.
        start = end + 1;
        end = line.find(' ', start);
    }
}

SubMesh::SubMesh(const std::string& _name, const ShaderProgram* _shaderProgram)
{
    name = _name;
    shaderProgram = _shaderProgram;
}

SubMesh::~SubMesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void SubMesh::LoadVertices(std::stringstream& fileContents, const std::array<std::vector<float>, 3>& vertexData)
{
    // Holds all vertex data as indices to the vertexData array.
    // Format: indices[0] = pos, indices[1] = uvs, indices[3] = normals.
    std::array<std::vector<uint32_t>, 3> vertexIndices;

    // Read file line by line to create vertex data.
    std::string line;
    while (std::getline(fileContents, line)) 
    {
        line += ' ';
        switch (line[0])
        {
        // Parse object indices.
        case 'f':
            ParseObjTriangle(line, vertexIndices);
            continue;
        // Skip comments, smooth lighting statements and empty lines.
        case '#':
        case 's':
        case ' ':
        case '\n':
        case '\0':
            continue;
        // Stop whenever another symbol is encountered.
        default:
            fileContents.seekg(fileContents.tellg() - std::streamoff(line.size()));
            break;
        }
        break;
    }

    // Get the first index.
    uint32_t startIndex = 0;
    if (indices.size() > 0)
        startIndex = indices[indices.size()-1] + 1;
    
    // Add all parsed data to the vertices array.
    for (size_t i = 0; i < vertexIndices[0].size(); i++)
    {
        // Get the current vertex's position.
        Vector3 curPos = Vector3(vertexData[0][vertexIndices[0][i] * 3], vertexData[0][vertexIndices[0][i] * 3+1], vertexData[0][vertexIndices[0][i] * 3+2]);

        // Get the current vertex's texture coordinates.
        Vector2 curUv;
        if (vertexData[1].size() > 0) curUv = Vector2(vertexData[1][vertexIndices[1][i] * 2], vertexData[1][vertexIndices[1][i] * 2+1]);

        // Get the current vertex's normal.
        Vector3 curNormal;
        if (vertexData[2].size() > 0) curNormal = Vector3(vertexData[2][vertexIndices[2][i] * 3], vertexData[2][vertexIndices[2][i] * 3+1], vertexData[2][vertexIndices[2][i] * 3+2]);

        // Get the current face's tangent and bitangent.
        static Vector3 curTangent, curBitangent;
        if (i % 3 == 0)
        {
            Vector3 edge1    = Vector3(vertexData[0][vertexIndices[0][i+1] * 3], vertexData[0][vertexIndices[0][i+1] * 3+1], vertexData[0][vertexIndices[0][i+1] * 3+2]) - curPos;
            Vector3 edge2    = Vector3(vertexData[0][vertexIndices[0][i+2] * 3], vertexData[0][vertexIndices[0][i+2] * 3+1], vertexData[0][vertexIndices[0][i+2] * 3+2]) - curPos;
            Vector2 deltaUv1 = Vector2(vertexData[1][vertexIndices[1][i+1] * 2], vertexData[1][vertexIndices[1][i+1] * 2+1]) - curUv;
            Vector2 deltaUv2 = Vector2(vertexData[1][vertexIndices[1][i+2] * 2], vertexData[1][vertexIndices[1][i+2] * 2+1]) - curUv;

            float f = 1.0f / (deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y);

            curTangent.x = f * (deltaUv2.y * edge1.x - deltaUv1.y * edge2.x);
            curTangent.y = f * (deltaUv2.y * edge1.y - deltaUv1.y * edge2.y);
            curTangent.z = f * (deltaUv2.y * edge1.z - deltaUv1.y * edge2.z);

            curBitangent.x = f * (-deltaUv2.x * edge1.x + deltaUv1.x * edge2.x);
            curBitangent.y = f * (-deltaUv2.x * edge1.y + deltaUv1.x * edge2.y);
            curBitangent.z = f * (-deltaUv2.x * edge1.z + deltaUv1.x * edge2.z);
        }

        // Create a new vertex with the computed data.
        vertices.push_back(TangentVertex{ curPos, curUv, curNormal, curTangent, curBitangent });
        indices .push_back(startIndex + (uint32_t)i);
    }
}

bool SubMesh::SendVerticesToOpenGL(size_t& totalVertexCount)
{
    if (vertices.size() <= 0 || !IsLoaded() || WasSentToOpenGL())
        return false;

    // Store the number of vertices in the model.
    vertexCount = (unsigned int)vertices.size();

    // Create and bind the Vertex Array Object.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create, bind and set the vertex buffer.
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(TangentVertex), vertices.data(), GL_STATIC_DRAW);

    // Create, bind and set the index buffer.
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set the position attribute pointer.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Set the uv attribute pointer.
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TangentVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set the normal attribute pointer.
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertex), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Set the tangent attribute pointer.
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertex), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Set the bitTangent attribute pointer.
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(TangentVertex), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);

    totalVertexCount += vertexCount;

    // Free vertex arrays.
    // vertices.clear();
    indices.clear();
    sentToOpenGL.store(true);
    return true;
}
