#include "Transform.h"
#include "Maths.h"
using namespace Core::Maths;

Transform::Transform(bool _isCamera)
{
    isCamera = _isCamera;
    pos = Vector3();
    rot = Vector3();
    scale = Vector3(1, 1, 1);
    UpdateModelMat();
}

// ----- Position ----- //
Vector3 Transform::GetPosition() const { return pos; }
void    Transform::SetPosition(const Vector3& position) { pos  = position; UpdateModelMat(); }
void    Transform::Move       (const Vector3& movement) { pos += movement; UpdateModelMat(); }

// ----- Forward ----- //
Vector3 Transform::Forward() const                    { return Vector3(rot, 1); }
Vector3 Transform::Up()      const                    { return Vector3({ rot.x - PI/2, rot.y, rot.z }, 1); }
Vector3 Transform::Right()   const                    { return Vector3({ 0, rot.y + PI/2, rot.z }, 1); }
void    Transform::SetForward(const Vector3& forward) { rot.x = forward.getXAngle(); rot.y = forward.getYAngle(); UpdateModelMat(); }

// ----- Rotation ----- //
Vector3 Transform::GetRotation() const                  { return rot; }
void    Transform::SetRotation(const Vector3& rotation) { rot = rotation; UpdateModelMat(); }
void    Transform::Rotate     (const Vector3& rotation) 
{
    rot.x = fmod(rot.x + rotation.x, 2*PI);
    rot.y = fmod(rot.y + rotation.y, 2*PI);
    rot.z = fmod(rot.z + rotation.z, 2*PI);
    UpdateModelMat(); 
}

// ----- Scale ----- //
Vector3 Transform::GetScale()        const         { return scale;   }
Vector3 Transform::GetUniformScale() const         { return std::max(scale.x, std::max(scale.y, scale.z)); }
void    Transform::SetScale(const Vector3& _scale) { scale = _scale; UpdateModelMat(); }

// ----- Matrices ----- //
Mat4 Transform::GetModelMat() const { return modelMat; }
void Transform::UpdateModelMat() 
{
    if (!isCamera)
        modelMat = GetTransformMatrix(pos, rot, scale);
    else
        modelMat = GetTransformMatrix(-pos, rot, { 1, 1, 1 }, true);
}
