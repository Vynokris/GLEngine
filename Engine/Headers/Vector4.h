#pragma once
#include <string>

namespace Core::Maths
{
    class Vector3;

    // Point in 3D space with homogeneous coordinates.
    class Vector4
    {
    public:
        // Attributes.
        float x, y, z, w;

        // -- Constructors & Destructor -- //
        Vector4();                                                                                                // Null vector.
        Vector4(const float& _x, const float& _y, const float& _z, const float& _w);                              // Vector with 3 coordinates.
        Vector4(const Vector3& vec, const float& _w);                                                             // Vector4 from vector3.
        Vector4(const Vector4& p1, const Vector4& p2, const float& _w);                                           // Vector4 from 2 points.
        Vector4(const float& theta, const float& phi, const float& length, const float& _w, const bool& isAngle); // Vector4 from angles (useless bool).
        ~Vector4() {}

        // -- Operators -- //

                              void    operator= (const Vector4& other);
        template <typename T> bool    operator==(const T& val) const;
        template <typename T> bool    operator!=(const T& val) const;
        template <typename T> Vector4 operator+ (const T& val) const;
        template <typename T> Vector4 operator- (const T& val) const;
        template <typename T> Vector4 operator* (const T& val) const;
        template <typename T> Vector4 operator/ (const T& val) const;
        template <typename T> void    operator+=(const T& val);
        template <typename T> void    operator-=(const T& val);
        template <typename T> void    operator*=(const T& val);
        template <typename T> void    operator/=(const T& val);
                              float   operator& (const Vector4& val) const;
                              Vector3 operator^ (const Vector4& val) const;
                              Vector4 operator- ()                   const;

        // -- Methods -- //

        // Returns the length of this vector.
        float getLength() const;
        // Modifies the length of this vector to correspond to the given value.
        void setLength(const float& length);

        // Normalizes the given vector so that its length is 1.
        void    normalize();
        Vector4 getNormalized() const;

        // Homogenizes the vector4 by dividing it by w.
        void    homogenize();
        Vector4 getHomogenized() const;

        // Copies the signs from the source vector to the destination vector.
        void    copySign(Vector4 source);
        Vector4 getCopiedSign(Vector4 source) const;

        // Interprets the vector as a point and returns the distance to another point.
        float getDistanceFromPoint(const Vector4& p) const;

        // Returns the angle (in radians) of this vector.
        float getXAngle() const;
        float getYAngle() const;

        // Returns the angle (in radians) between two vectors.
        float getXAngleWithVector4(const Vector4& v) const;
        float getYAngleWithVector4(const Vector4& v) const;

        // Rotates the given vector by the given angle.
        void    rotate(const Vector3& angles);
        Vector4 getRotated(const Vector3& angles) const;

        // Creates a Vector3 from this vector.
        Vector3 toVector3(bool homogenizeVec = false) const;
        
        // Returns a string containing the values of the vector formatted in the following way: (x, y, z, w).
        std::string toString() { return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + ")"; }
    };
}

#include "Vector4.inl"
