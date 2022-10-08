#pragma once
#include <vector>
#include "Maths.h"
#include "Collisions.h"

namespace Core::Physics
{
    class Primitive;
    class Collisions
    {
    public:
        static bool    BoundingSpheres(const Primitive* prim1, const Primitive* prim2);
        static bool    Spheres        (const Primitive* prim1, const Primitive* prim2);
        static bool    Primitives     (const Primitive* prim1, const Primitive* prim2);
        static Vector3 PrimitivesResolution(const Primitive* prim1, const Primitive* prim2);
    };
};