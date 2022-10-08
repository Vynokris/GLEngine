#include "Light.h"
using namespace Core::Maths;
using namespace Render;

void DirLight::SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const Core::Maths::Vector3& _dir)
{
    ambient  = _ambient;
    diffuse  = _diffuse;
    specular = _specular;
    dir      = _dir;
}

void PointLight::SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const float& _constant, const float& _linear, const float& _quadratic, const Core::Maths::Vector3& _pos)
{
    ambient   = _ambient;
    diffuse   = _diffuse;
    specular  = _specular;
    constant  = _constant;
    linear    = _linear;
    quadratic = _quadratic;
    pos       = _pos;
}

void SpotLight::SetParams(const Core::Maths::RGB& _ambient, const Core::Maths::RGB& _diffuse, const Core::Maths::RGB& _specular, const Core::Maths::Vector3& _pos, const Core::Maths::Vector3& _dir, const float& _outerCone, const float& _innerCone)
{
    ambient   = _ambient;
    diffuse   = _diffuse;
    specular  = _specular;
    pos       = _pos;
    dir       = _dir;
    outerCone = _outerCone;
    innerCone = _innerCone;
}
