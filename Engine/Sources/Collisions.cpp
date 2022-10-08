#include "Physics.h"
using namespace Core::Physics;
using namespace Core::Maths;


// ----- Collision backend functions ----- //

Vector3 FurthestPoint(const Primitive* prim, const Vector3& dir)
{
    Vector3 maxPoint;
    float   maxDist = -FLT_MAX;
    Mat4    worldMat = prim->transform->GetModelMat() * prim->transform->parentMat;

    for (Vector3& coord : PrimitiveBuffers::vertices[(int)prim->type])
    {
        Vector3 pos  = (coord.toVector4() * worldMat).toVector3();
        float   dist = pos & dir;
        if (dist > maxDist) {
            maxDist  = dist;
            maxPoint = pos;
        }
    }
    return maxPoint;
}

Vector3 SupportPoint(const Primitive* prim1, const Primitive* prim2, const Vector3& dir)
{
    return FurthestPoint(prim1, dir) - FurthestPoint(prim2, -dir);
}

bool SameDirection(const Vector3& vec1, const Vector3& vec2)
{
    return (vec1 & vec2) > 0;
}

bool Line(Vector3* points, int& pointsCount, Vector3& dir)
{
    Vector3 a  = points[0];
    Vector3 b  = points[1];
    Vector3 ab = b - a;
    Vector3 ao =   - a;

    if (SameDirection(ab, ao)) {
        dir = (ab ^ ao) ^ ab;
    }
    else {
        points[0] = a;
        points[1] = Vector3();
        points[2] = Vector3();
        points[3] = Vector3();
        pointsCount = 1;
        dir = ao;
    }

    return false;
}

bool Triangle(Vector3* points, int& pointsCount, Vector3& dir)
{
    Vector3 a   = points[0];
    Vector3 b   = points[1];
    Vector3 c   = points[2];
    Vector3 ab  = b - a;
    Vector3 ac  = c - a;
    Vector3 ao  =   - a;
    Vector3 abc = ab ^ ac;

    if (SameDirection(abc ^ ac, ao)) 
    {
        if (SameDirection(ac, ao)) {
            points[0] = a;
            points[1] = c;
            points[2] = Vector3();
            points[3] = Vector3();
            pointsCount = 2;
            dir = (ac ^ ao) ^ ac;
        }
        else {
            points[0] = a;
            points[1] = b;
            points[2] = Vector3();
            points[3] = Vector3();
            pointsCount = 2;
            return Line(points, pointsCount, dir);
        }
    }
    else 
    {
        if (SameDirection(ab ^ abc, ao)) {
            points[0] = a;
            points[1] = b;
            points[2] = Vector3();
            points[3] = Vector3();
            pointsCount = 2;
            return Line(points, pointsCount, dir);
        }
        else 
        {
            if (SameDirection(abc, ao)) {
                dir = abc;
            }
            else {
                points[0] = a;
                points[1] = c;
                points[2] = b;
                points[3] = Vector3();
                pointsCount = 3;
                dir = -abc;
            }
        }
    }

    return false;
}

bool Tetrahedron(Vector3* points, int& pointsCount, Vector3& dir)
{
    Vector3 a   = points[0];
    Vector3 b   = points[1];
    Vector3 c   = points[2];
    Vector3 d   = points[3];
    Vector3 ab  = b - a;
    Vector3 ac  = c - a;
    Vector3 ad  = d - a;
    Vector3 ao  =   - a;
    Vector3 abc = ab ^ ac;
    Vector3 acd = ac ^ ad;
    Vector3 adb = ad ^ ab;

    if (SameDirection(abc, ao)) {
        points[0] = a;
        points[1] = b;
        points[2] = c;
        points[3] = Vector3();
        pointsCount = 3;
        return Triangle(points, pointsCount, dir);
    }

    if (SameDirection(acd, ao)) {
        points[0] = a;
        points[1] = c;
        points[2] = d;
        points[3] = Vector3();
        pointsCount = 3;
        return Triangle(points, pointsCount, dir);
    }

    if (SameDirection(adb, ao)) {
        points[0] = a;
        points[1] = d;
        points[2] = b;
        points[3] = Vector3();
        pointsCount = 3;
        return Triangle(points, pointsCount, dir);
    }

    return true;
}

bool NextSimplex(Vector3* points, int& pointsCount, Vector3& dir)
{
    switch (pointsCount) 
    {
        case 2:  return Line       (points, pointsCount, dir);
        case 3:  return Triangle   (points, pointsCount, dir);
        case 4:  return Tetrahedron(points, pointsCount, dir);
        default: return false;
    }
}

std::pair<std::vector<Vector4>, size_t> GetFaceNormals(std::vector<Vector3>& polytope, std::vector<size_t>& faces)
{
    std::vector<Vector4> normals;
    size_t minTriangle = 0;
    float  minDistance = FLT_MAX;

    for (size_t i = 0; i < faces.size(); i += 3) {
        Vector3 a = polytope[faces[i    ]];
        Vector3 b = polytope[faces[i + 1]];
        Vector3 c = polytope[faces[i + 2]];

        Vector3 normal = ((b - a) ^ (c - a)).getNormalized();
        float distance = (normal & a);

        if (distance < 0) {
            normal   *= -1;
            distance *= -1;
        }

        normals.emplace_back(normal, distance);

        if (distance < minDistance) {
            minTriangle = i / 3;
            minDistance = distance;
        }
    }

    return { normals, minTriangle };
}

void AddIfUniqueEdge(std::vector<std::pair<size_t, size_t>>& edges, std::vector<size_t>& faces, const size_t& a, const size_t& b)
{
    auto reverse = std::find(              //    0--<--3
        edges.begin(),                     //   / \ B /   A: 2-0
        edges.end(),                       //  / A \ /    B: 0-2
        std::make_pair(faces[b], faces[a]) // 1-->--2
    );

    if (reverse != edges.end()) {
        edges.erase(reverse);
    }

    else {
        edges.emplace_back(faces[a], faces[b]);
    }
}

Vector3 EPA(Vector3* points, int& pointsCount, const Primitive* prim1, const Primitive* prim2)
{
    std::vector<Vector3> polytope(points, points + pointsCount);
    std::vector<size_t>  faces = {
        0, 1, 2,
        0, 3, 1,
        0, 2, 3,
        1, 3, 2
    };

    auto [normals, minFace] = GetFaceNormals(polytope, faces);

    if (normals.size() <= 0)
        return Vector3();

    Vector3 minNormal;
    float   minDistance = FLT_MAX;

    int i = 0;
    for (i = 0; i < 50 && minDistance == FLT_MAX; i++) 
    {
        minNormal   = normals[minFace].toVector3();
        minDistance = normals[minFace].w;

        Vector3 support = SupportPoint(prim1, prim2, minNormal);
        float sDistance = (minNormal & support);

        if (abs(sDistance - minDistance) > 0.001f) 
        {
            minDistance = FLT_MAX;
            std::vector<std::pair<size_t, size_t>> uniqueEdges;

            for (size_t i = 0; i < normals.size(); i++) 
            {
                if (SameDirection(normals[i].toVector3(), support)) {
                    size_t f = i * 3;

                    AddIfUniqueEdge(uniqueEdges, faces, f,     f + 1);
                    AddIfUniqueEdge(uniqueEdges, faces, f + 1, f + 2);
                    AddIfUniqueEdge(uniqueEdges, faces, f + 2, f    );

                    faces[f + 2] = faces.back(); faces.pop_back();
                    faces[f + 1] = faces.back(); faces.pop_back();
                    faces[f    ] = faces.back(); faces.pop_back();

                    normals[i] = normals.back(); normals.pop_back();

                    i--;
                }
            }

            std::vector<size_t> newFaces;
            for (auto [edgeIndex1, edgeIndex2] : uniqueEdges) {
                newFaces.push_back(edgeIndex1);
                newFaces.push_back(edgeIndex2);
                newFaces.push_back(polytope.size());
            }

            polytope.push_back(support);

            auto [newNormals, newMinFace] = GetFaceNormals(polytope, newFaces);
            float oldMinDistance = FLT_MAX;
            for (size_t i = 0; i < normals.size(); i++) {
                if (normals[i].w < oldMinDistance) {
                    oldMinDistance = normals[i].w;
                    minFace = i;
                }
            }

            if (newMinFace < newNormals.size() && newNormals[newMinFace].w < oldMinDistance) {
                minFace = newMinFace + normals.size();
            }

            faces  .insert(faces  .end(), newFaces  .begin(), newFaces  .end());
            normals.insert(normals.end(), newNormals.begin(), newNormals.end());
        }
    }
    if (minDistance >= FLT_MAX)
        return Vector3();

    return Vector3(minNormal.x, -minNormal.y, -minNormal.z).getNormalized() * (minDistance + 0.001f);
}


// ----- High level collision functions ----- //

bool Collisions::BoundingSpheres(const Primitive* prim1, const Primitive* prim2)
{
    if (prim1->transform == nullptr || prim2->transform == nullptr)
        return false;
    
    Vector3 worldPos1 = (prim1->transform->GetPosition().toVector4() * prim1->transform->parentMat).toVector3();
    Vector3 worldPos2 = (prim2->transform->GetPosition().toVector4() * prim2->transform->parentMat).toVector3();

    // Similar computation to Collisions::Spheres().
    Vector3 vecDist = Vector3(worldPos1, worldPos2);
    float radiusDist = prim1->boundingSphere.radius + prim2->boundingSphere.radius;
    return (vecDist & vecDist) <= radiusDist * radiusDist;
}

bool Collisions::Spheres(const Primitive* prim1, const Primitive* prim2)
{
    if (prim1->transform == nullptr || prim2->transform == nullptr)
        return false;
    
    Vector3 worldPos1 = (prim1->transform->GetPosition().toVector4() * prim1->transform->parentMat).toVector3();
    Vector3 worldPos2 = (prim2->transform->GetPosition().toVector4() * prim2->transform->parentMat).toVector3();

    // Compare the squared distance between the two spheres' positions (vecDist dot itself)
    // to the squared sum of the spheres' radii (radiusDist squared).
    Vector3 vecDist = Vector3(worldPos1, worldPos2);
    float radiusDist = (prim1->transform->GetScale().x + prim2->transform->GetScale().x) / 2;
    return (vecDist & vecDist) <= radiusDist * radiusDist;
}

bool Collisions::Primitives(const Primitive* prim1, const Primitive* prim2)
{
    if (prim1->transform == nullptr || prim2->transform == nullptr)
        return false;

    // If the two prims are spheres, don't even check bounding boxes.
    if (prim1->type == PrimitiveTypes::Sphere && prim2->type == PrimitiveTypes::Sphere)
        return Collisions::Spheres(prim1, prim2);

    // Check collisions between the primitives' bounding spheres.
    if (Collisions::BoundingSpheres(prim1, prim2))
    {
        // GJK algorithm.
        Vector3 points[4];
        int pointsCount = 0;

        // Find the first support point and add it to the points array.
        Vector3 support = SupportPoint(prim1, prim2, Vector3(0, 0, 1));
        points[0] = support;
        pointsCount++;

        // The next direction is towards the origin.
        Vector3 dir = -support;

        for (int i = 0; i < 50; i++) // Max number of iterations is 50.
        {
            support = SupportPoint(prim1, prim2, dir);

            // No collision if the support point has not passed the origin.
            if ((support & dir) <= 0)
                return false;

            // Add the support point to the front of the points array.
            points[3] = points[2];
            points[2] = points[1];
            points[1] = points[0];
            points[0] = support;
            if (pointsCount < 4)
                pointsCount++;

            if (NextSimplex(points, pointsCount, dir))
                return true;
        }
    }
    return false;
}

Vector3 Collisions::PrimitivesResolution(const Primitive* prim1, const Primitive* prim2)
{
    if (prim1->transform == nullptr || prim2->transform == nullptr)
        return Vector3();

    // Check collisions between the primitives' bounding spheres.
    if (Collisions::BoundingSpheres(prim1, prim2))
    {
        // GJK algorithm.
        Vector3 points[4];
        int pointsCount = 0;

        // Find the first support point and add it to the points array.
        Vector3 support = SupportPoint(prim1, prim2, Vector3(prim1->transform->GetPosition(), prim2->transform->GetPosition()));
        points[0] = support;
        pointsCount++;

        // The next direction is towards the origin.
        Vector3 dir = -support;

        for (int i = 0; i < 50; i++) // Max number of iterations is 50.
        {
            support = SupportPoint(prim1, prim2, dir);

            // No collision if the support point has not passed the origin.
            if ((support & dir) <= 0)
                return Vector3();

            // Add the support point to the front of the points array.
            points[3] = points[2];
            points[2] = points[1];
            points[1] = points[0];
            points[0] = support;
            if (pointsCount < 4)
                pointsCount++;

            if (NextSimplex(points, pointsCount, dir))
                return EPA(points, pointsCount, prim1, prim2);
        }
    }
    return Vector3();
}
