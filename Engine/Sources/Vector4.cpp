#include "Maths.h"
#include "Vector4.h"
#include <cmath>
using namespace Core;
using namespace Core::Maths;


// Constructors.
Vector4::Vector4() : x(0), y(0), z(0), w(1) {}; // Null vector.
Vector4::Vector4(const float& _x, const float& _y, const float& _z, const float& _w) : x(_x), y(_y), z(_z), w(_w) {}; // Vector with 3 coordinates.
Vector4::Vector4(const Vector4& p1, const Vector4& p2, const float& _w) : x(p2.x - p1.x), y(p2.y - p1.y), z(p2.z - p1.z), w(_w) {}; // Vector from 2 points.
Vector4::Vector4(const Vector3& vec, const float& _w) : x(vec.x), y(vec.y), z(vec.z), w(_w) {}; // Vector from Segment3.
Vector4::Vector4(const float& theta, const float& phi, const float& length, const float& _w, const bool& isAngle) // Vector from angle (useless bool).
{
    x = length * sinf(theta) * cosf(phi);
    y = length * cosf(theta);
    z = length * sinf(theta) * sinf(phi);
    w = _w;
}

// ---------- VECTOR4 OPERATORS ---------- //

// Copy constructor.
void Vector4::operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; }

// Vector4 dot product.
float Vector4::operator&(const Vector4& v)   const { return (x * v.x) + (y * v.y) + (z * v.z); }

// Vector4 cross product.
Vector3 Vector4::operator^(const Vector4& v) const { return Vector3((y * v.z - z * v.y), (z * v.x - x * v.z), (x * v.y - y * v.x)); }

// Vector4 negation.
Vector4 Vector4::operator-()                 const { return { -x, -y, -z, w }; }

// ------------ VECTOR4 METHODS ----------- //

// Returns the length of the given vector.
float Vector4::getLength()        const { return sqrt(sqpow(x) + sqpow(y) + sqpow(z)); }

// Modifies the length of the given vector to correspond to the given value.
void Vector4::setLength(const float& length) { normalize(); (*this) *= length; }

// Normalizes the given vector so that its length is 1.
void    Vector4::normalize()     { (*this) = getNormalized(); }
Vector4 Vector4::getNormalized() const
{
    float length = getLength();
    return Vector4(x / length, y / length, z / length, w);
}

// Homogenizes the vector4 by dividing it by w.
void    Vector4::homogenize()           { *this = getHomogenized(); }
Vector4 Vector4::getHomogenized() const { return Vector4(x / w, y / w, z / w, w / w); }

// Copies the signs from the source vector to the destination vector.
void    Vector4::copySign(Vector4 source)            { *(this) = getCopiedSign(source); }
Vector4 Vector4::getCopiedSign(Vector4 source) const { return Vector4(std::copysign(x, source.x), std::copysign(y, source.y), std::copysign(z, source.z), w); }

// Interprets the vector as a point and returns the distance to another point.
float Vector4::getDistanceFromPoint(const Vector4& p) const { return Vector4(*this, p, w).getLength(); }

// Returns the angle (in radians) of the given vector.
float Vector4::getXAngle() const { return asin(-y); }
float Vector4::getYAngle() const { return atan2(x, z); }

// Returns the angle (in radians) between two vectors.
float Vector4::getXAngleWithVector4(const Vector4& v) const
{
    float this_angle = getXAngle();
    float v_angle = v.getXAngle();
    return (this_angle >= v_angle ? (this_angle - v_angle) : (v_angle - this_angle));
}
float Vector4::getYAngleWithVector4(const Vector4& v) const
{
    float this_angle = getYAngle();
    float v_angle = v.getYAngle();
    return (this_angle >= v_angle ? (this_angle - v_angle) : (v_angle - this_angle));
}

// Rotates the given vector by the given angle.
void Vector4::rotate(const Vector3& angles) 
{ 
    *this = getRotated(angles);
}
Vector4 Vector4::getRotated(const Vector3& angles) const
{
    return Vector4({ getXAngle() + angles.x, getYAngle() + angles.y, angles.z }, getLength());
}

// Creates a Vector3 from this vector.
Vector3 Vector4::toVector3(bool homogenizeVec) const
{
    if (homogenizeVec)
        return Vector3(x / w, y / w, z / w);
    else
        return Vector3(x, y, z);
}
