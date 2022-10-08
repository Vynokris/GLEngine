#pragma once

#include <vector>
#include <array>
#include <string>
#include <sstream>
#include "IResource.h"

namespace Core::Maths
{
    struct TangentVertex;
}

namespace Resources
{
    class Material;
    class ShaderProgram;

    class SubMesh
    {
    private:
        std::string      name;
        unsigned int     vertexCount  = 0;
        std::atomic_bool loaded       = false;
        std::atomic_bool sentToOpenGL = false;
        const ShaderProgram* shaderProgram = nullptr;
              Material*      material      = nullptr;

        std::vector<Core::Maths::TangentVertex> vertices;
        std::vector<unsigned int>               indices;

        unsigned int VBO = 0;
        unsigned int EBO = 0;

    public:
        unsigned int VAO = 0;

        SubMesh(const std::string& _name, const ShaderProgram* _shaderProgram);
        ~SubMesh();

        void LoadVertices(std::stringstream& fileContents, const std::array<std::vector<float>, 3>& vertexData);
        bool SendVerticesToOpenGL(size_t& totalVertexCount);

        std::string          GetName()          const { return name;                }
        unsigned int         GetVertexCount()   const { return vertexCount;         }
        bool                 IsLoaded()         const { return loaded.load();       }
        void                 SetLoadingDone()         { loaded.store(true);         }
        bool                 WasSentToOpenGL()  const { return sentToOpenGL.load(); }
        const ShaderProgram* GetShaderProgram() const { return shaderProgram;       }
              Material*      GetMaterial()            { return material;            }
        const std::vector<Core::Maths::TangentVertex>& GetVertices() const { return vertices; }

        void SetShaderProgram(const ShaderProgram* _shaderProgram) { shaderProgram = _shaderProgram; }
        void SetMaterial     (      Material*      _material)      { material = _material; }
    };
}