#pragma once
#include "Arithmetic.h"
#include "Color.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "Transform.h"

#ifndef PI
#define PI 3.14159265358979323846f
#endif

namespace Core::Maths
{
    // A point in 3D space with rendering data.
    struct Vertex
    {
        Vector3 pos;
        Vector2 uv;
        Vector3 normal;
    };

    // Holds indices for the vertex's data.
    struct TangentVertex
    {
        Vector3 pos;
        Vector2 uv;
        Vector3 normal;
        Vector3 tangent;
        Vector3 bitangent;
    };

    struct VertexIndices
    {
        uint32_t pos;
        uint32_t uv;
        uint32_t normal;
    };
}