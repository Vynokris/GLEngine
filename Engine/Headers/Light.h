#pragma once

#include "Maths.h"

namespace Render
{
    enum class LightTypes
    {
        Directional,
        Point,
        Spot,
    };

    class ILight
    {
    protected:
        LightTypes type;
        unsigned int id;

    public:
        Core::Maths::RGB ambient, diffuse = RGB(1, 1, 1), specular;

        ILight(const LightTypes& _type, const unsigned int& _id) { type = _type; id = _id; }
        LightTypes GetType() { return type; }
        unsigned int GetId() { return id; }
    };

    class DirLight : public ILight
    {
    public:
        Vector3 dir;
        DirLight(const unsigned int& _id) : ILight(LightTypes::Directional, _id) {}
        void SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const Core::Maths::Vector3& _dir);
    };

    class PointLight : public ILight
    {
    public:
        float constant = 1, linear = 0.025f, quadratic = 0.01f;
        Vector3 pos;
        PointLight(const unsigned int& _id) : ILight(LightTypes::Point, _id) {}
        void SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const float& _constant, const float& _linear, const float& _quadratic, const Core::Maths::Vector3& _pos);
    };

    class SpotLight : public ILight
    {
    public:
        float outerCone = PI/6, innerCone = PI/12;
        Vector3 pos, dir;
        SpotLight(const unsigned int& _id) : ILight(LightTypes::Spot, _id) {}
        void SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const Core::Maths::Vector3& _pos, const Core::Maths::Vector3& _dir, const float& _outerCone, const float& _innerCone);
    };
}
