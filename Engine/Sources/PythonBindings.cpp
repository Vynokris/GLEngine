#include <python/Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <imgui/imgui.h>
#include "PyOpaqueClasses.h"
#include "ObjectScript.h"
namespace py = pybind11;
using namespace Maths;
using namespace Scenes;
using namespace Render;
using namespace Core::Physics;

PYBIND11_MODULE(engine, m)
{
    m.doc() = "Python bindings used for scripting in my C++ engine.";

    // ----- Arithmetic ----- //

    m.attr("PI") = PI;

    // Degrees/Radians conversions.
    m.def("degToRad", &degToRad, "Converts the given angle from degrees to radians.", py::arg("angle"));
    m.def("radToDeg", &radToDeg, "Converts the given angle from radians to degrees.", py::arg("angle"));

    // Clamp.
    m.def("clamp",      &clamp,      "Clamps the given value to be superior or equal to the minimum value and inferior or equal to the maximum value.", py::arg("value"), py::arg("minimum"), py::arg("maximum"));
    m.def("clampUnder", &clampUnder, "Clamps the given value to be inferior or equal to the maximum value.", py::arg("value"), py::arg("maximum"));
    m.def("clampAbove", &clampAbove, "Clamps the given value to be superior or equal to the minimum value.", py::arg("value"), py::arg("minimum"));

    // Lerp.
    m.def("lerp",    &lerp,    "Compute linear interpolation between start and end for the parameter val (if 0 <= val <= 1: start <= return <= end).", py::arg("value"), py::arg("start"), py::arg("end"));
    m.def("getLerp", &getLerp, "Compute the linear interpolation factor that returns val when lerping between start and end.",                         py::arg("value"), py::arg("start"), py::arg("end"));
    m.def("remap",   &remap,   "Remaps the given value from one range to another.", py::arg("value"), py::arg("inputStart"), py::arg("inputEnd"), py::arg("outputStart"), py::arg("outputEnd"));

    // Powers of 2.
    m.def("isPowerOf2",       &isPowerOf2,       "Returns True if the given value is a power of 2.",                       py::arg("value"));
    m.def("getPowerOf2Under", &getPowerOf2Under, "Returns the closest power of 2 that is inferior or equal to the value.", py::arg("value"));
    m.def("getPowerOf2Above", &getPowerOf2Above, "Returns the closest power of 2 that is superior or equal to the value.", py::arg("value"));


    // ----- Color ----- //

    py::class_<RGBA>(m, "RGBA")
        .def(py::init<float, float, float, float>())

        .def_readwrite("r", &RGBA::r)
        .def_readwrite("g", &RGBA::g)
        .def_readwrite("b", &RGBA::b)
        .def_readwrite("a", &RGBA::a)
        .def("ToRGB", &RGBA::rgb, "Creates and returns an RGB color from this RGBA color.")

        .def(py::self +  py::self)
        .def(py::self +  float())
        .def(py::self += py::self)
        .def(py::self += float())
        .def(py::self -  py::self)
        .def(py::self -  float())
        .def(py::self -= py::self)
        .def(py::self -= float())
        .def(py::self *  py::self)
        .def(py::self *  float())
        .def(py::self *= py::self)
        .def(py::self *= float())
        .def(py::self /  py::self)
        .def(py::self /  float())
        .def(py::self /= py::self)
        .def(py::self /= float())
        .def(py::self == py::self)
        .def(py::self != py::self);
    
    py::class_<RGB>(m, "RGB")
        .def(py::init<float, float, float>())

        .def_readwrite("r", &RGB::r)
        .def_readwrite("g", &RGB::g)
        .def_readwrite("b", &RGB::b)
        .def("ToRGBA", &RGB::rgba, "Creates and returns an RGBA color from this RGB color.");

    py::class_<HSV>(m, "HSV")
        .def(py::init<float, float, float>())

        .def_readwrite("h", &HSV::h)
        .def_readwrite("s", &HSV::s)
        .def_readwrite("v", &HSV::v);

    m.def("hueFromRGB", &hueFromRGB, "Returns the given RGB color's hue.", py::arg("rgb"));
    m.def("colorLerp",  &colorLerp,  "Computes linear interpolation between two colors with the given value.", py::arg("value"), py::arg("start"), py::arg("end"));
    m.def("blendHSV",   &blendHSV,   "Returns the blended color of the two give HSV colors.", py::arg("hsv1"), py::arg("hsv2"));
    m.def("colorShift", &colorShift, "Returns the given color with its hue shifted by the given amount.", py::arg("rgba"), py::arg("hueShift"));
    m.def("RGBAtoHSV",  &RGBAtoHSV,  "Converts the given RGBA color to HSV and returns it.", py::arg("rgba"));
    m.def("HSVtoRGBA",  &HSVtoRGBA,  "Converts the given HSV color to RGBA and returns it.", py::arg("hsv"), py::arg("alpha") = 1);


    // ----- Vector2 ----- //

    py::class_<Vector2>(m, "Vector2")
        .def(py::init<float, float>())
        
        // Properties.
        .def_readwrite("x", &Vector2::x)
        .def_readwrite("y", &Vector2::y)

        // Comparison.
        .def(py::self == py::self)
        .def(py::self != py::self)

        // Addition.
        .def(py::self + py::self)
        .def(py::self + float())
        .def(py::self += py::self)
        .def(py::self += float())

        // Substraction.
        .def(py::self - py::self)
        .def(py::self - float())
        .def(py::self -= py::self)
        .def(py::self -= float())

        // Multiplication.
        .def(py::self * py::self)
        .def(py::self * float())
        .def(py::self *= py::self)
        .def(py::self *= float())

        // Division.
        .def(py::self / py::self)
        .def(py::self / float())
        .def(py::self /= py::self)
        .def(py::self /= float())

        // Dot and cross products.
        .def("__and__", [](const Vector2 &a, const Vector2& b) {
            return a & b;
        }, "Computes a dot product between two 2D vectors.", py::is_operator())
        .def("__xor__", [](const Vector2 &a, const Vector2& b) {
            return a ^ b;
        }, "Computes a cross product between two 2D vectors.", py::is_operator())
        
        // Negation.
        .def(-py::self)

        // Length.
        .def("getLength", &Vector2::getLength, "Returns the vector's length.")
        .def("setLength", &Vector2::setLength, "Modifies the vector's length.", py::arg("length"))

        // Normalizing.
        .def("normalize",     &Vector2::normalize,     "Normalizes the vector.")
        .def("getNormalized", &Vector2::getNormalized, "Returns a normalized copy of the vector.")

        // Copysign.
        .def("copySign",      &Vector2::copySign,      "Copies the signs from the source vector to the destination vector.", py::arg("source"))
        .def("getCopiedSign", &Vector2::getCopiedSign, "Returns a new vector with the source vector's signs and the destination vector's values.", py::arg("source"))

        // Normal.
        .def("normal", &Vector2::getNormal, "Returns the vector's normal.")

        // Distance from point.
        .def("distanceFromPoint", &Vector2::getDistanceFromPoint, "Interprets the vector as a point and returns the distance to another point.", py::arg("point"))

        // Angle.
        .def("getAngle",            &Vector2::getAngle,            "Returns the angle (in radians) of the given vector.")
        .def("getAngleWithVector2", &Vector2::getAngleWithVector2, "Returns the angle (in radians) between two vectors.", py::arg("vector"))

        // Rotation.
        .def("rotate",     &Vector2::rotate,     "Rotates the given vector by the given angle (in radians).", py::arg("angle"))
        .def("getRotated", &Vector2::getRotated, "Returns a copy of the given vector rotated by the given angle (in radians).", py::arg("angle"))
        .def("rotateAsPoint",     &Vector2::rotateAsPoint,     "Rotates the vector (interpreted as a point) around the given pivot point by the given angle (in radians).", py::arg("pivot"), py::arg("angle"))
        .def("getRotatedAsPoint", &Vector2::getRotatedAsPoint, "Returns a copy of the vector (interpreted as a point) rotated around the given pivot point by the given angle (in radians).", py::arg("pivot"), py::arg("angle"))

        // String cast.
        .def("__repr__", &Vector2::toString);

    // Other Vector2 constructors.
    m.def("Vector2FromPoints", [](const Vector2& p1, const Vector2& p2    ){ return Vector2(p1, p2);               }, "Returns a Vector2 that starts at the first point and ends at the second.", py::arg("point1"), py::arg("point2"));
    m.def("Vector2FromAngles", [](const float& angles, const float& length){ return Vector2(angles, length, true); }, "Returns a Vector2 of the given length, pointing in the direction of the given angle.", py::arg("angle"), py::arg("length"));

    // Point 2 lerp.
    m.def("point2Lerp", &point2Lerp, "Calculates linear interpolation for a value from a start point to an end point.", py::arg("value"), py::arg("start"), py::arg("end"));


    // ----- Vector3 ----- //

    py::class_<Vector3>(m, "Vector3")
        .def(py::init<float, float, float>())
        
        // Properties.
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z)

        // Comparison.
        .def(py::self == py::self)
        .def(py::self != py::self)

        // Addition.
        .def(py::self + py::self)
        .def(py::self + float())
        .def(py::self += py::self)
        .def(py::self += float())

        // Substraction.
        .def(py::self - py::self)
        .def(py::self - float())
        .def(py::self -= py::self)
        .def(py::self -= float())

        // Multiplication.
        .def(py::self * py::self)
        .def(py::self * float())
        .def(py::self *= py::self)
        .def(py::self *= float())

        // Division.
        .def(py::self / py::self)
        .def(py::self / float())
        .def(py::self /= py::self)
        .def(py::self /= float())

        // Dot and cross products.
        .def("__and__", [](const Vector3 &a, const Vector3& b) {
            return a & b;
        }, "Computes a dot product between two 2D vectors.", py::is_operator())
        .def("__xor__", [](const Vector3 &a, const Vector3& b) {
            return a ^ b;
        }, "Computes a cross product between two 2D vectors.", py::is_operator())
        
        // Negation.
        .def(-py::self)

        // Length.
        .def("getLength", &Vector3::getLength, "Returns the vector's length.")
        .def("setLength", &Vector3::setLength, "Modifies the vector's length.", py::arg("length"))

        // Normalizing.
        .def("normalize",     &Vector3::normalize,     "Normalizes the vector.")
        .def("getNormalized", &Vector3::getNormalized, "Returns a normalized copy of the vector.")

        // Copysign.
        .def("copySign",      &Vector3::copySign,      "Copies the signs from the source vector to the destination vector.", py::arg("source"))
        .def("getCopiedSign", &Vector3::getCopiedSign, "Returns a new vector with the source vector's signs and the destination vector's values.", py::arg("source"))

        // Distance from point.
        .def("distanceFromPoint", &Vector3::getDistanceFromPoint, "Interprets the vector as a point and returns the distance to another point.", py::arg("point"))

        // Angle.
        .def("getXAngle",            &Vector3::getXAngle,            "Returns the angle (in radians) of the given vector on the X axis.")
        .def("getYAngle",            &Vector3::getYAngle,            "Returns the angle (in radians) of the given vector on the Y axis.")
        .def("getXAngleWithVector3", &Vector3::getXAngleWithVector3, "Returns the angle (in radians) between two vectors on the X axis.", py::arg("vector"))
        .def("getYAngleWithVector3", &Vector3::getYAngleWithVector3, "Returns the angle (in radians) between two vectors on the Y axis.", py::arg("vector"))

        // Rotation.
        .def("rotate",     &Vector3::rotate,     "Rotates the given vector by the given angles (in radians).", py::arg("angles"))
        .def("getRotated", &Vector3::getRotated, "Returns a copy of the given vector rotated by the given angles (in radians).", py::arg("angles"))

        // Vector4 cast.
        .def("toVector4", &Vector3::toVector4, "Creates a Vector4 from this vector.")

        // String cast.
        .def("__repr__", &Vector3::toString);

    // Other Vector3 constructors.
    m.def("Vector3FromPoints", [](const Vector3& p1, const Vector3& p2){ return Vector3(p1, p2); }, "Returns a Vector3 that starts at the first point and ends at the second.", py::arg("point1"), py::arg("point2"));
    m.def("Vector3FromAngles", [](const Vector3& angles, const float& length){ return Vector3(angles, length); }, "Returns a Vector3 of the given length, pointing in the direction of the given angles.", py::arg("angles"), py::arg("length"));

    // Point 3 lerp.
    m.def("point3Lerp", &point3Lerp, "Calculates linear interpolation for a value from a start point to an end point.", py::arg("value"), py::arg("start"), py::arg("end"));

    // Spherical coordinates.
    m.def("getSphericalCoords", &getSphericalCoords, "Returns the coordinates of a point on a sphere of the given radius, using the given angles.", py::arg("radius"), py::arg("pitch"), py::arg("yaw"));


    // ----- Vector4 ----- //

    py::class_<Vector4>(m, "Vector4")
        .def(py::init<float, float, float, float>())

        // Properties.
        .def_readwrite("x", &Vector4::x)
        .def_readwrite("y", &Vector4::y)
        .def_readwrite("z", &Vector4::z)
        .def_readwrite("w", &Vector4::w)

        // Comparison.
        .def(py::self == py::self)
        .def(py::self != py::self)

        // Addition.
        .def(py::self + py::self)
        .def(py::self + float())
        .def(py::self += py::self)
        .def(py::self += float())

        // Substraction.
        .def(py::self - py::self)
        .def(py::self - float())
        .def(py::self -= py::self)
        .def(py::self -= float())

        // Multiplication.
        .def(py::self * py::self)
        .def(py::self * float())
        .def(py::self * Mat4())
        .def(py::self *= py::self)
        .def(py::self *= float())

        // Division.
        .def(py::self / py::self)
        .def(py::self / float())
        .def(py::self /= py::self)
        .def(py::self /= float())

        // Dot and cross products.
        .def("__and__", [](const Vector4 &a, const Vector4& b) {
            return a & b;
        }, "Computes a dot product between two 2D vectors.", py::is_operator())
        .def("__xor__", [](const Vector4 &a, const Vector4& b) {
            return a ^ b;
        }, "Computes a cross product between two 2D vectors.", py::is_operator())
        
        // Negation.
        .def(-py::self)

        // Length.
        .def("getLength", &Vector4::getLength, "Returns the vector's length.")
        .def("setLength", &Vector4::setLength, "Modifies the vector's length.", py::arg("length"))

        // Normalizing.
        .def("normalize",     &Vector4::normalize,     "Normalizes the vector.")
        .def("getNormalized", &Vector4::getNormalized, "Returns a normalized copy of the vector.")

        // Homogenizing.
        .def("homogenize",     &Vector4::homogenize,     "Homogenizes the vector by dividing it by its w component.")
        .def("getHomogenized", &Vector4::getHomogenized, "Returns a homogenized copy of the vector.")

        // Copysign.
        .def("copySign",      &Vector4::copySign,      "Copies the signs from the source vector to the destination vector.", py::arg("source"))
        .def("getCopiedSign", &Vector4::getCopiedSign, "Returns a new vector with the source vector's signs and the destination vector's values.", py::arg("source"))

        // Distance from point.
        .def("distanceFromPoint", &Vector4::getDistanceFromPoint, "Interprets the vector as a point and returns the distance to another point.", py::arg("point"))
            
        // Angle.
        .def("getXAngle",            &Vector4::getXAngle,            "Returns the angle (in radians) of the given vector on the X axis.")
        .def("getYAngle",            &Vector4::getYAngle,            "Returns the angle (in radians) of the given vector on the Y axis.")
        .def("getXAngleWithVector4", &Vector4::getXAngleWithVector4, "Returns the angle (in radians) between two vectors on the X axis.", py::arg("vector"))
        .def("getYAngleWithVector4", &Vector4::getYAngleWithVector4, "Returns the angle (in radians) between two vectors on the Y axis.", py::arg("vector"))

        // Rotation.
        .def("rotate",     &Vector4::rotate,     "Rotates the given vector by the given angles (in radians).", py::arg("angles"))
        .def("getRotated", &Vector4::getRotated, "Returns a copy of the given vector rotated by the given angles (in radians).", py::arg("angles"))

        // Vector4 cast.
        .def("toVector3", &Vector4::toVector3, "Creates a Vector3 from this vector.")

        // String cast.
        .def("__repr__", &Vector4::toString);


    // ----- Matrix ----- //

    py::class_<Matrix<2, 2>>(m, "Mat2")
        .def(py::init<bool>())

        // .def_readwrite("m", &Matrix<2, 2>::m)
        .def("__getitem__", [](const Matrix<2, 2>& mat, const int& idx) { return mat.m[idx]; })

        .def(py::self +  py::self)
        .def(py::self +  float())
        .def(py::self += py::self)
        .def(py::self += float())
        .def(py::self -  py::self)
        .def(py::self -  float())
        .def(py::self -= py::self)
        .def(py::self -= float())
        .def(py::self *  py::self)
        .def(py::self *  float())
        .def(py::self *= py::self)
        .def(py::self *= float())
        .def(py::self /  float())
        .def(py::self /= float())
        // .def(-py::self)
        .def("__xor__", [](const Matrix<2, 2> &a, const float& b) {
            return a ^ b;
        }, py::is_operator())
        
        .def("getValue",    &Matrix<2, 2>::getMatrixValue, py::arg("y"), py::arg("x"))
        .def("isIdentity",  &Matrix<2, 2>::isIdentity)
        .def("determinant", &Matrix<2, 2>::det2)
        .def("inverse",     &Matrix<2, 2>::inv2)
        .def("transpose",   &Matrix<2, 2>::transpose)
        .def("__repr__",    [](const Matrix<2, 2>& mat) { return mat.printStr(false); });

    py::class_<Matrix<3, 3>>(m, "Mat3")
        .def(py::init<bool>())

        // .def_readwrite("m", &Matrix<3, 3>::m)
        .def("__getitem__", [](const Matrix<3, 3>& mat, const int& idx) { return mat.m[idx]; })

        .def(py::self +  py::self)
        .def(py::self +  float())
        .def(py::self += py::self)
        .def(py::self += float())
        .def(py::self -  py::self)
        .def(py::self -  float())
        .def(py::self -= py::self)
        .def(py::self -= float())
        .def(py::self *  py::self)
        .def(py::self *  float())
        .def(py::self *= py::self)
        .def(py::self *= float())
        .def(py::self /  float())
        .def(py::self /= float())
        // .def(-py::self)
        .def("__xor__", [](const Matrix<3, 3> &a, const float& b) {
            return a ^ b;
        }, py::is_operator())
        
        .def("getValue",    &Matrix<3, 3>::getMatrixValue, py::arg("y"), py::arg("x"))
        .def("isIdentity",  &Matrix<3, 3>::isIdentity)
        .def("determinant", &Matrix<3, 3>::det3)
        .def("inverse",     &Matrix<3, 3>::inv3)
        .def("transpose",   &Matrix<3, 3>::transpose)
        .def("__repr__",    [](const Matrix<3, 3>& mat) { return mat.printStr(false); });

    py::class_<Matrix<4, 4>>(m, "Mat4")
        .def(py::init<bool>())

        // .def_readwrite("m", &Matrix<4, 4>::m)
        .def("__getitem__", [](const Matrix<4, 4>& mat, const int& idx) { return mat.m[idx]; })

        .def(py::self +  py::self)
        .def(py::self +  float())
        .def(py::self += py::self)
        .def(py::self += float())
        .def(py::self -  py::self)
        .def(py::self -  float())
        .def(py::self -= py::self)
        .def(py::self -= float())
        .def(py::self *  py::self)
        .def(py::self *  float())
        .def(py::self *= py::self)
        .def(py::self *= float())
        .def(py::self /  float())
        .def(py::self /= float())
        // .def(-py::self)
        .def("__xor__", [](const Matrix<4, 4> &a, const float& b) {
            return a ^ b;
        }, py::is_operator())
        
        .def("getValue",    &Matrix<4, 4>::getMatrixValue, py::arg("y"), py::arg("x"))
        .def("isIdentity",  &Matrix<4, 4>::isIdentity)
        .def("determinant", &Matrix<4, 4>::det4)
        .def("inverse",     &Matrix<4, 4>::inv4)
        .def("transpose",   &Matrix<4, 4>::transpose)
        .def("__repr__",    [](const Matrix<4, 4>& mat) { return mat.printStr(false); });

    m.def("GetTranslationMatrix", &GetTranslationMatrix, "", py::arg("translation"));
    m.def("GetScaleMatrix",       &GetScaleMatrix,       "", py::arg("scale"));
    m.def("GetXRotationMatrix",   &GetXRotationMatrix,   "", py::arg("angle"));
    m.def("GetYRotationMatrix",   &GetYRotationMatrix,   "", py::arg("angle"));
    m.def("GetZRotationMatrix",   &GetZRotationMatrix,   "", py::arg("angle"));
    m.def("GetRotationMatrix",    &GetRotationMatrix,    "", py::arg("angles"),   py::arg("reverse") = false);
    m.def("GetTransformMatrix",   &GetTransformMatrix,   "", py::arg("position"), py::arg("rotation"), py::arg("scale"), 
            py::arg("reverse") = false, py::arg("transformNormals") = false);


    // ----- Transform ----- //

    py::class_<Transform>(m, "Transform")
        .def(py::init<bool>())

        // Position.
        .def("GetPosition", &Transform::GetPosition, "Returns the transform's position as a Vector3.")
        .def("SetPosition", &Transform::SetPosition, "Modifies the transform's position.", py::arg("position"))
        .def("Move",        &Transform::Move,        "Moves the transform's position by the given Vector3.", py::arg("movement"))

        // Forward.
        .def("Forward",    &Transform::Forward,    "Returns the transform's forward direction.")
        .def("Up",         &Transform::Up,         "Returns the transform's upwards direction.")
        .def("Right",      &Transform::Right,      "Returns the transform's right direction.")
        .def("SetForward", &Transform::SetForward, "Rotates the transform so that it points in the given direction.", py::arg("forward"))

        // Rotation.
        .def("GetRotation", &Transform::GetRotation, "Returns the transform's rotation.")
        .def("SetRotation", &Transform::SetRotation, "Modifies the transform's rotation.", py::arg("rotation"))
        .def("Rotate",      &Transform::Rotate,      "Rotates the transform by the given angles.", py::arg("angles"))

        // Scale.
        .def("GetScale",        &Transform::GetScale, "Returns the transform's scale.")
        .def("GetUniformScale", &Transform::GetUniformScale, "Returns a Vector3 with all its components set to the scale's highest component.")
        .def("SetScale",        &Transform::SetScale, "Modifies the transform's scale.", py::arg("scale"))

        // Matrix.
        .def("GetModelMat", &Transform::GetModelMat, "Returns the transform's model matrix.");


    // ----- Rigidbody ----- //

    py::class_<Rigidbody>(m, "Rigidbody")
        .def(py::init<Transform*>())
        .def("IsGrounded",   &Rigidbody::IsGrounded,   "Returns True if the rigidbody is on the ground.")
        .def("IsKinematic",  &Rigidbody::IsKinematic,  "Returns True if the rigidbody has physics updates enabled.")
        .def("SetKinematic", &Rigidbody::SetKinematic, "Enables or disables physics updates for the rigidbody.", py::arg("kinematic"))
        
        // Velocity.
        .def("GetVelocity", &Rigidbody::GetVelocity, "Returns the rigidbody's velocity.")
        .def("SetVelocity", &Rigidbody::SetVelocity, "Modifies the rigidbody's velocity.", py::arg("velocity"))
        .def("AddVelocity", &Rigidbody::AddVelocity, "Adds velocity to the rigidbody.",    py::arg("velocity"))

        // Acceleration.
        .def("GetAcceleration", &Rigidbody::GetAcceleration, "Returns the rigidbody's acceleration.")
        .def("SetAcceleration", &Rigidbody::SetAcceleration, "Modifies the rigidbody's acceleration.", py::arg("acceleration"))
        .def("AddAcceleration", &Rigidbody::AddAcceleration, "Adds acceleration to the rigidbody.",    py::arg("acceleration"));


    // ----- Primitive ----- //

    py::enum_<PrimitiveTypes>(m, "PrimitiveTypes")
        .value("Spere",   PrimitiveTypes::Sphere)
        .value("Cube",    PrimitiveTypes::Cube)
        .value("Capsule", PrimitiveTypes::Capsule)
        .export_values();

    py::class_<BoundingSphere>(m, "BoundingSphere")
        .def("Update", &BoundingSphere::Update, "Updates the bounding spere's radius to fit around the given SceneNode's collider.", py::arg("collider"), py::arg("sceneNode"));

    py::class_<Primitive>(m, "Primitive")
        .def(py::init<PrimitiveTypes>())

        .def_readonly("type", &Primitive::type)
        .def_readwrite("transform", &Primitive::transform)
        .def_readwrite("boundingSphere", &Primitive::boundingSphere)

        .def("GetShaderProgram", &Primitive::GetShaderProgram, "Returns the primitive's shader program.", py::return_value_policy::reference)
        .def("GetMaterial",      &Primitive::GetMaterial,      "Returns the primitive's material.",       py::return_value_policy::reference)

        .def("SetShaderProgram", &Primitive::SetShaderProgram, "Sets the primitive's shader program to the given one.", py::arg("shaderProgram"))
        .def("SetMaterial",      &Primitive::SetMaterial,      "Sets the primitive's material to the given one.",       py::arg("material"));


    // ----- Debug ----- //
    
    m.def("DebugLog",        [](const char* message, const char* file, const char* function, const long& line){ Core::Debug::Log::ToFile(Core::Debug::LogTypes::Info,    file, function, line, message, true); }, 
            "Send an info log containing the given message.",   py::arg("message"), py::arg("file") = "python file", py::arg("function") = "\0", py::arg("line") = -1);

    m.def("DebugLogWarning", [](const char* message, const char* file, const char* function, const long& line){ Core::Debug::Log::ToFile(Core::Debug::LogTypes::Warning, file, function, line, message, true); }, 
            "Send a warning log containing the given message.", py::arg("message"), py::arg("file") = "python file", py::arg("function") = "\0", py::arg("line") = -1);

    m.def("DebugLogError",   [](const char* message, const char* file, const char* function, const long& line){ Core::Debug::Log::ToFile(Core::Debug::LogTypes::Error,   file, function, line, message, true); }, 
            "Send an error log containing the given message.",  py::arg("message"), py::arg("file") = "python file", py::arg("function") = "\0", py::arg("line") = -1);


    // ----- IResource ----- //

    py::class_<IResource>(m, "BasicResource")
        .def("GetName",         &IResource::GetName,         "Returns the resource's name.")
        .def("GetType",         &IResource::GetType,         "Returns the resource's type.")
        .def("IsLoaded",        &IResource::IsLoaded,        "Returns True if the resource is loaded.")
        .def("WasSentToOpenGL", &IResource::WasSentToOpenGL, "Returns True if the resource was sent to OpenGL.");


    // ----- Textures ----- //

    py::class_<Texture, IResource>(m, "Texture")
        .def(py::init<std::string>())
        .def("GetWidth",  &Texture::GetWidth,  "Returns the texture's width.")
        .def("GetHeight", &Texture::GetHeight, "Returns the texture's height.");

    py::class_<DynamicTexture, IResource>(m, "DynamicTexture")
        .def(py::init<std::string>())
        .def("GetWidth",  &DynamicTexture::GetWidth,  "Returns the texture's width.")
        .def("GetHeight", &DynamicTexture::GetHeight, "Returns the texture's height.")
        
        .def("GetPixel", &DynamicTexture::GetPixel, "Returns the RGBA color of the pixel at the given position.", py::arg("x"), py::arg("y"))
        .def("SetPixel", &DynamicTexture::SetPixel, "Sets the RGBA color of the pixel at the given position.", 
                py::arg("x"), py::arg("y"), py::arg("color"), py::arg("updateTexture") = true)
        .def("UpdateTexture", &DynamicTexture::UpdateTexture, "Sends the texture's pixels to OpenGL.");

    py::class_<RenderTexture, IResource>(m, "RenderTexture")
        .def(py::init<std::string>())
        .def_readwrite("clearColor", &RenderTexture::clearColor)

        .def("GetWidth",  &RenderTexture::GetWidth,  "Returns the rendertexture's width.")
        .def("GetHeight", &RenderTexture::GetHeight, "Returns the rendertexture's height.")
        
        .def("SetWidth",  &RenderTexture::SetWidth,  "Modifies the rendertexture's width.",  py::arg("width"))
        .def("SetHeight", &RenderTexture::SetHeight, "Modifies the rendertexture's height.", py::arg("height"))
        .def("SetSize",   &RenderTexture::SetSize,   "Modifies the rendertexture's size.",   py::arg("width"), py::arg("height"))
        
        .def("BeginRender", &RenderTexture::BeginRender, "Starts rendering to the rendertexture.")
        .def("EndRender",   &RenderTexture::EndRender,   "Stops rendering to the rendertexture (defaults back to rendering to screen).");


    // ----- Cubemap ----- //

    py::enum_<CubeSides>(m, "CubeSides")
        .value("Right",  CubeSides::Right)
        .value("Left",   CubeSides::Left)
        .value("Top",    CubeSides::Top)
        .value("Bottom", CubeSides::Bottom)
        .value("Front",  CubeSides::Front)
        .value("Back",   CubeSides::Back)
        .export_values();

    py::class_<Cubemap, IResource>(m, "Cubemap")
        .def(py::init<std::string>())
        
        .def("SetTexture", &Cubemap::SetTexture, "Sets the cubemap's texture on the given side.", py::arg("side"), py::arg("name"));


    // ----- Material ----- //

    py::class_<Material, IResource>(m, "Material")
        .def_readwrite("ambient",      &Material::ambient)
        .def_readwrite("diffuse",      &Material::diffuse)
        .def_readwrite("specular",     &Material::specular)
        .def_readwrite("emission",     &Material::emission)
        .def_readwrite("shininess",    &Material::shininess)
        .def_readwrite("transparency", &Material::transparency)
        
        .def_readwrite("ambientTexture",  &Material::ambientTexture)
        .def_readwrite("diffuseTexture",  &Material::diffuseTexture)
        .def_readwrite("specularTexture", &Material::specularTexture)
        .def_readwrite("emissionTexture", &Material::emissionTexture)
        .def_readwrite("shininessMap",    &Material::shininessMap)
        .def_readwrite("alphaMap",        &Material::alphaMap)
        .def_readwrite("normalMap",       &Material::normalMap)

        .def("SetParams", &Material::SetParams, "Sets the material's parameters to the given ones.", 
                py::arg("ambient"), py::arg("diffuse"), py::arg("specular"), py::arg("emission"), py::arg("shininess"));


    // ----- SubMesh ---- //

    py::class_<SubMesh>(m, "SubMesh")
        .def(py::init<std::string, ShaderProgram*>())
        .def("GetName",          &SubMesh::GetName,          "Returns the sub-mesh's name.")
        .def("GetVertexCount",   &SubMesh::GetVertexCount,   "Returns the number of vertices stored in the sub-mesh.")
        .def("IsLoaded",         &SubMesh::IsLoaded,         "Returns True if the sub-mesh is loaded.")
        .def("WasSentToOpenGL",  &SubMesh::WasSentToOpenGL,  "Returns True if the sub-mesh was sent to OpenGL.")
        .def("GetShaderProgram", &SubMesh::GetShaderProgram, "Returns the sub-mesh's shader program.", py::return_value_policy::reference)
        .def("GetMaterial",      &SubMesh::GetMaterial,      "Returns the sub-mesh's material.", py::return_value_policy::reference)
        .def("SetShaderProgram", &SubMesh::SetShaderProgram, "Sets the sub-mesh's shader program to the given one.", py::arg("shaderProgram"))
        .def("SetMaterial",      &SubMesh::SetMaterial,      "Sets the sub-mesh's material to the given one.", py::arg("material"));


    // ----- Mesh ----- //

    py::class_<Mesh, IResource>(m, "MeshGroup")
        .def(py::init<std::string>())
        .def_readwrite("subMeshes",     &Mesh::subMeshes)

        .def("AreAllSubMeshesLoaded",   &Mesh::AreAllSubMeshesLoaded,   "Returns True if all of the sub-meshes of this mesh are loaded.")
        .def("AreAllSubMeshesInOpenGL", &Mesh::AreAllSubMeshesInOpenGL, "Returns True if all of the sub-meshes of this mesh have been sent to OpenGL.");


    // ----- Shaders ----- //

    py::class_<VertexShader,   IResource>(m, "VertexShader"  ).def(py::init<std::string>());
    py::class_<FragmentShader, IResource>(m, "FragmentShader").def(py::init<std::string>());
    py::class_<ComputeShader,  IResource>(m, "ComputeShader" ).def(py::init<std::string>());
    py::class_<ShaderProgram,  IResource>(m, "ShaderProgram")
        .def(py::init<std::string>())
        .def("AttatchShader", &ShaderProgram::AttachShader, "Link the given shader to this shader program for compilation.", py::arg("shader"))
        .def("Compile",       &ShaderProgram::SendToOpenGL, "Compile all attached shaders and send this shader program to OpenGL.");


    // ----- ObjFile and MtlFile ----- //

    py::class_<ObjFile, IResource>(m, "ObjFile")
        .def(py::init<std::string, ResourceManager&>())
        .def_readonly("createdResources", &ObjFile::createdResources);
    py::class_<MtlFile, IResource>(m, "MtlFile")
        .def(py::init<std::string, ResourceManager&>())
        .def_readonly("createdResources", &MtlFile::createdResources);
          

    // ----- Time Manager ----- //
    
    py::class_<Time>(m, "Time")
        .def("IsVsyncOn",    &Time::IsVsyncOn,    "Returns True if vertical synchronisation is enabled.")
        .def("SetVsyncOn",   &Time::SetVsyncOn,   "Enables/disables vertical synchronisation.", py::arg("vsyncOn"))
        .def("GetTargetFPS", &Time::GetTargetFPS, "Returns the engine's FPS cap.")
        .def("SetTargetFPS", &Time::SetTargetFPS, "Modifies the engine's FPS cap.", py::arg("fps"))
        .def("FPS",          &Time::FPS,          "Returns the engine's current FPS.")
        .def("DeltaTime",    &Time::DeltaTime,    "Returns the engine's current delta time.");


    // ----- Resource Manager ----- //

    py::enum_<ResourceTypes>(m, "ResourceTypes")
        .value("TextureResource",        ResourceTypes::Texture)
        .value("DynamicTextureResource", ResourceTypes::DynamicTexture)
        .value("MaterialResource",       ResourceTypes::Material)
        .value("MeshResource",           ResourceTypes::Mesh)
        .value("VertexShaderResource",   ResourceTypes::VertexShader)
        .value("FragmentShaderResource", ResourceTypes::FragmentShader)
        .value("ComputeShaderResource",  ResourceTypes::ComputeShader)
        .value("ShaderProgramResource",  ResourceTypes::ShaderProgram)
        .value("ObjFileResource",        ResourceTypes::ObjFile)
        .value("MtlFileResource",        ResourceTypes::MtlFile)
        .export_values();
    
    py::class_<ResourceManager>(m, "ResourceManager")
        .def("CreateTexture",        [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::Texture,        name }); }, "Creates a new texture with the given name, loads it and returns it.",                         py::arg("name"), py::return_value_policy::reference)
        .def("CreateDynamicTexture", [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::DynamicTexture, name }); }, "Creates a new dynamic texture with the given name, loads it and returns it.",                 py::arg("name"), py::return_value_policy::reference)
        .def("CreateMaterial",       [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::Material,       name }); }, "Creates a new material with the given name, loads it and returns it.",                        py::arg("name"), py::return_value_policy::reference)
        .def("CreateMesh",           [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::Mesh,           name }); }, "Creates a new mesh with the given name, loads it and returns it.",                            py::arg("name"), py::return_value_policy::reference)
        .def("CreateVertexShader",   [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::VertexShader,   name }); }, "Creates a new vertex shader with the given name, loads it and returns it.",                   py::arg("name"), py::return_value_policy::reference)
        .def("CreateFragmentShader", [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::FragmentShader, name }); }, "Creates a new fragment shader with the given name, loads it and returns it.",                 py::arg("name"), py::return_value_policy::reference)
        .def("CreateComputeShader",  [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::ComputeShader,  name }); }, "Creates a new compute shader with the given name, loads it and returns it.",                  py::arg("name"), py::return_value_policy::reference)
        .def("CreateShaderProgram",  [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::ShaderProgram,  name }); }, "Creates a new shader program with the given name, loads it and returns it.",                  py::arg("name"), py::return_value_policy::reference)
        .def("CreateObjFile",        [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::ObjFile,        name }); }, "Loads all the meshes and materials from the given .obj file, and returns an ObjFile object.", py::arg("name"), py::return_value_policy::reference)
        .def("CreateMtlFile",        [](ResourceManager& self, const std::string& name){ self.pyResourceCreationQueue.push_back({ ResourceTypes::MtlFile,        name }); }, "Loads all the materials from the given .mtl file, and returns an MtlFile object.",            py::arg("name"), py::return_value_policy::reference)

        .def("GetTexture",        &ResourceManager::Get<Texture>,        "Returns a texture with the given name.",         py::arg("name"), py::return_value_policy::reference)
        .def("GetDynamicTexture", &ResourceManager::Get<DynamicTexture>, "Returns a dynamic texture with the given name.", py::arg("name"), py::return_value_policy::reference)
        .def("GetMaterial",       &ResourceManager::Get<Material>,       "Returns a material with the given name.",        py::arg("name"), py::return_value_policy::reference)
        .def("GetMesh",           &ResourceManager::Get<Mesh>,           "Returns a mesh with the given name.",            py::arg("name"), py::return_value_policy::reference)
        .def("GetVertexShader",   &ResourceManager::Get<VertexShader>,   "Returns a vertex shader with the given name.",   py::arg("name"), py::return_value_policy::reference)
        .def("GetFragmentShader", &ResourceManager::Get<FragmentShader>, "Returns a fragment shader with the given name.", py::arg("name"), py::return_value_policy::reference)
        .def("GetComputeShader",  &ResourceManager::Get<ComputeShader>,  "Returns a compute shader with the given name.",  py::arg("name"), py::return_value_policy::reference)
        .def("GetShaderProgram",  &ResourceManager::Get<ShaderProgram>,  "Returns a shader program with the given name.",  py::arg("name"), py::return_value_policy::reference)
        .def("GetObjFile",        &ResourceManager::Get<ObjFile>,        "Returns an ObjFile object with the given name.", py::arg("name"), py::return_value_policy::reference)
        .def("GetMtlFile",        &ResourceManager::Get<MtlFile>,        "Returns an MtlFile object with the given name.", py::arg("name"), py::return_value_policy::reference)
        
        .def("FindTexture",        &ResourceManager::Find<Texture>,        "Searches for a texture which has a name that contains the search term, and returns the first one found.",         py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindDynamicTexture", &ResourceManager::Find<DynamicTexture>, "Searches for a dynamic texture which has a name that contains the search term, and returns the first one found.", py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindMaterial",       &ResourceManager::Find<Material>,       "Searches for a material which has a name that contains the search term, and returns the first one found.",        py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindMesh",           &ResourceManager::Find<Mesh>,           "Searches for a mesh which has a name that contains the search term, and returns the first one found.",            py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindVertexShader",   &ResourceManager::Find<VertexShader>,   "Searches for a vertex shader which has a name that contains the search term, and returns the first one found.",   py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindFragmentShader", &ResourceManager::Find<FragmentShader>, "Searches for a fragment shader which has a name that contains the search term, and returns the first one found.", py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindComputeShader",  &ResourceManager::Find<ComputeShader>,  "Searches for a compute shader which has a name that contains the search term, and returns the first one found.",  py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindShaderProgram",  &ResourceManager::Find<ShaderProgram>,  "Searches for a shader program which has a name that contains the search term, and returns the first one found.",  py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindObjFile",        &ResourceManager::Find<ObjFile>,        "Searches for an ObjFile object which has a name that contains the search term, and returns the first one found.", py::arg("searchTerm"), py::return_value_policy::reference)
        .def("FindMtlFile",        &ResourceManager::Find<MtlFile>,        "Searches for an MtlFile object which has a name that contains the search term, and returns the first one found.", py::arg("searchTerm"), py::return_value_policy::reference)

        .def("Delete", &ResourceManager::Delete, "Deletes the resource with the given name.", py::arg("name"))

        .def("AreAllResourcesLoaded",   &ResourceManager::AreAllResourcesLoaded,   "Returns True if all the resources have been loaded.")
        .def("AreAllResourcesInOpenGL", &ResourceManager::AreAllResourcesInOpenGL, "Returns True if all the resources have been sent to OpenGL.");


    // ----- Light Manager ----- //

    py::enum_<LightTypes>(m, "LightTypes")
        .value("Directional", LightTypes::Directional)
        .value("Point",       LightTypes::Point)
        .value("Spot",        LightTypes::Spot)
        .export_values();

    py::class_<DirLight>(m, "DirLight")
        .def_readwrite("ambient",  &DirLight::ambient)
        .def_readwrite("diffuse",  &DirLight::diffuse)
        .def_readwrite("specular", &DirLight::specular)
        .def_readwrite("dir",      &DirLight::dir)

        .def("GetType",   &DirLight::GetType, "Returns the light's type.")
        .def("GetId",     &DirLight::GetId,   "Returns the light's id.")
        .def("SetParams", &DirLight::SetParams, "Sets the light's parameters to the given ones.", 
                py::arg("ambient") = RGB(), py::arg("diffuse") = RGB(1, 1, 1), py::arg("specular") = RGB(), py::arg("dir") = Vector3(1, 0, 0));

    py::class_<PointLight>(m, "PointLight")
        .def_readwrite("ambient",   &PointLight::ambient)
        .def_readwrite("diffuse",   &PointLight::diffuse)
        .def_readwrite("specular",  &PointLight::specular)
        .def_readwrite("constant",  &PointLight::constant)
        .def_readwrite("linear",    &PointLight::linear)
        .def_readwrite("quadratic", &PointLight::quadratic)
        .def_readwrite("pos",       &PointLight::pos)

        .def("GetType",   &PointLight::GetType, "Returns the light's type.")
        .def("GetId",     &PointLight::GetId,   "Returns the light's id.")
        .def("SetParams", &PointLight::SetParams, "Sets the light's parameters to the given ones.", 
                py::arg("ambient")  = RGB(), py::arg("diffuse") = RGB(1, 1, 1), py::arg("specular")  = RGB(), 
                py::arg("constant") = 1,     py::arg("linear")  = 0.025f,       py::arg("quadratic") = 0.01f, py::arg("pos") = Vector3(0, 0, 0));

    py::class_<SpotLight>(m, "SpotLight")
        .def_readwrite("ambient",   &SpotLight::ambient)
        .def_readwrite("diffuse",   &SpotLight::diffuse)
        .def_readwrite("specular",  &SpotLight::specular)
        .def_readwrite("outerCone", &SpotLight::outerCone)
        .def_readwrite("innerCone", &SpotLight::innerCone)
        .def_readwrite("pos",       &SpotLight::pos)
        .def_readwrite("dir",       &SpotLight::dir)

        .def("GetType",   &SpotLight::GetType, "Returns the light's type.")
        .def("GetId",     &SpotLight::GetId,   "Returns the light's id.")
        .def("SetParams", &SpotLight::SetParams, "Sets the light's parameters to the given ones.", 
                py::arg("ambient")   = RGB(), py::arg("diffuse")    = RGB(1, 1, 1), py::arg("specular") = RGB(), 
                py::arg("outerCone") = PI/6,  py::arg("innerCone")  = PI/12,        py::arg("pos")      = Vector3(0, 0, 0), py::arg("dir") = Vector3(1, 0, 0));

    py::class_<LightManager> lightManagerClass = py::class_<LightManager>(m, "LightManager")
        .def("CreateDirLight",   [](LightManager& self){ return self.Create<DirLight>(); },   "Creates and returns a new directional light.", py::return_value_policy::reference)
        .def("CreatePointLight", [](LightManager& self){ return self.Create<PointLight>(); }, "Creates and returns a new point light.",       py::return_value_policy::reference)
        .def("CreateSpotLight",  [](LightManager& self){ return self.Create<SpotLight>(); },  "Creates and returns a new spot light.",        py::return_value_policy::reference)

        .def("GetDirLight",    [](LightManager& self, const unsigned int& id){ return self.Get<DirLight>(id); },   "Returns the directional light with the given id.", py::arg("id"), py::return_value_policy::reference)
        .def("GetPointLight",  [](LightManager& self, const unsigned int& id){ return self.Get<PointLight>(id); }, "Returns the point light with the given id.",       py::arg("id"), py::return_value_policy::reference)
        .def("GetSpotLight",   [](LightManager& self, const unsigned int& id){ return self.Get<SpotLight>(id); },  "Returns the spot light with the given id.",        py::arg("id"), py::return_value_policy::reference)

        .def("DeleteDirLight",   [](LightManager& self, const unsigned int& id){ return self.Delete<DirLight>(id); },   "Deletes the directional light with the given id.", py::arg("id"))
        .def("DeletePointLight", [](LightManager& self, const unsigned int& id){ return self.Delete<PointLight>(id); }, "Deletes the point light with the given id.",       py::arg("id"))
        .def("DeleteSpotLight",  [](LightManager& self, const unsigned int& id){ return self.Delete<SpotLight>(id); },  "Deletes the spot light with the given id.",        py::arg("id"))
        .def("ClearLights",      &LightManager::ClearLights,                                                            "Deletes all of the lights.");

    lightManagerClass.attr("maxDirLights")   = MAX_DIR_LIGHTS;
    lightManagerClass.attr("maxPointLights") = MAX_POINT_LIGHTS;
    lightManagerClass.attr("maxSpotLights")  = MAX_SPOT_LIGHTS;


    // ----- Camera Manager ----- //
    
    py::class_<CameraParams>(m, "CameraParams")
        .def("__init__", [](CameraParams& self, const unsigned int& w, const unsigned int& h, const float& near, const float& far, const float& fov)
            { 
                self.width = w;
                self.height = h;
                self.near = near;
                self.far = far;
                self.fov = fov;
            })

        .def_readwrite("width",  &CameraParams::width)
        .def_readwrite("height", &CameraParams::height)
        .def_readwrite("near",   &CameraParams::near)
        .def_readwrite("far",    &CameraParams::far)
        .def_readwrite("fov",    &CameraParams::fov)
        .def_readonly ("aspect", &CameraParams::aspect);

    py::class_<Camera>(m, "Camera")
        .def(py::init<CameraParams>())
        .def_readwrite("transform", &Camera::transform)
        .def("ChangeParameters",    &Camera::ChangeParameters,  "Modifies the camera's parameters to the given ones.", py::arg("parameters"))
        .def("GetParameters",       &Camera::GetParameters,     "Returns the camera's parameters.")
        .def("GetWorldTransform",   &Camera::GetWorldTransform, "Returns the camera's world transform matrix (4x4).")
        .def("GetProjectionMat",    &Camera::GetProjectionMat,  "Returns the camera's projection matrix (4x4).")
        .def("GetViewMat",          &Camera::GetViewMat,        "Returns the camera's view matrix (4x4).");

    py::class_<CameraManager>(m, "CameraManager")
        .def_readwrite("cameras",     &CameraManager::cameras)
        .def_readwrite("sceneCamera", &CameraManager::sceneCamera)
        
        .def("Create", &CameraManager::Create, "Creates a new scene camera. If scaleToScreen is True, the camera's width and height are always set to the screen size.", 
                py::arg("cameraParams"), py::arg("scaleToScreen"), py::return_value_policy::reference);


    // ----- Post Processor ----- //

    py::class_<PostProcessor>(m, "PostProcessor")
        .def(py::init<PostProcessor>())
        
        .def_readwrite("negative",    &PostProcessor::negative)
        .def_readwrite("blur",        &PostProcessor::blur)
        .def_readwrite("toonShading", &PostProcessor::toonShading)
        .def_readwrite("blurRadius",  &PostProcessor::blurRadius)
        .def_readwrite("toonLevels",  &PostProcessor::toonLevels)
        
        .def("SetFramebufferSize", &PostProcessor::SetFramebufferSize, "Modifies the framebuffer's size.", py::arg("width"), py::arg("height"))
        .def("SetClearColor",      &PostProcessor::SetClearColor,      "Modifies the background color.",   py::arg("color"));


    // ----- SceneNode ----- //

    py::class_<SceneNode>(m, "SceneNode")
        .def(py::init<size_t, std::string>())
        
        .def_readonly ("id",        &SceneNode::id)
        .def_readonly ("name",      &SceneNode::name)
        .def_readonly ("type",      &SceneNode::type)
        .def_readwrite("scripts",   &SceneNode::scripts)
        .def_readwrite("colliders", &SceneNode::colliders)
        .def_readwrite("rigidbody", &SceneNode::rigidbody)
        .def_readwrite("transform", &SceneNode::transform)
        
        .def("Parent",        [](SceneNode& node){ return node.parent; }, "Returns this node's parent.", py::return_value_policy::reference)
        .def("RemoveChild",      &SceneNode::RemoveChild, "Removes the node with the given id from this node's children.", py::arg("childId"))
        .def("MoveNode",         &SceneNode::MoveNode,    "Modifies this node's parent.", py::arg("newParent"))
        .def("FindInChildrenId", [](SceneNode& node, const size_t&      searchId  ){ return node.FindInChildrenId(searchId  ); }, 
             "If this node is parent of a node with the given id, return it.",              py::arg("searchId" ), py::return_value_policy::reference)
        .def("FindInChildren",   [](SceneNode& node, const std::string& searchName){ return node.FindInChildren  (searchName); }, 
             "Returns the first node that has the given name and is a child of this node.", py::arg("seachName"), py::return_value_policy::reference)
        .def("AddRigidbody", &SceneNode::AddRigidbody, "Adds a rigidbody to this node.", py::arg("isKinematic") = false)
        .def("AddScript",    &SceneNode::AddScript,   "Adds a new C++ script to this node.",    py::arg("script"),   py::arg("app"))
        .def("AddPyScript",  &SceneNode::AddPyScript, "Adds a new Python script to this node.", py::arg("filename"), py::arg("app"))
        .def("GetCppScript", &SceneNode::GetCppScript, 
             "Returns the script at the given index as an ObjectScript.\nReturns None if there is no script with the given index.", py::arg("index"), py::return_value_policy::reference)
        .def("GetPyScript",  [](SceneNode& self, const size_t& id){ PyScript* script = self.GetPyScript(id); if (!script) return (py::object*)nullptr; return &script->scriptObject; }, 
             "Returns the script at the given index as a user defined Python script.\nReturns None if there is no script with the given index.", py::arg("index"), py::return_value_policy::reference);


    // ----- Scene Objects ----- //

    py::class_<SceneModel,          SceneNode>(m, "SceneModel"         ).def_readwrite("mesh",      &SceneModel::meshGroup);
    py::class_<SceneInstancedModel, SceneNode>(m, "SceneInstancedModel").def_readwrite("mesh",      &SceneInstancedModel::meshGroup).def_readwrite("instanceCount", &SceneInstancedModel::instanceCount).def_readwrite("instanceTransforms", &SceneInstancedModel::instanceTransforms);
    py::class_<SceneSkybox,         SceneNode>(m, "SceneSkybox"        ).def_readwrite("cubemap",   &SceneSkybox::cubemap);
    py::class_<SceneCamera,         SceneNode>(m, "SceneCamera"        ).def_readwrite("camera",    &SceneCamera::camera);
    py::class_<SceneDirLight,       SceneNode>(m, "SceneDirLight"      ).def_readwrite("light",     &SceneDirLight::light);
    py::class_<ScenePointLight,     SceneNode>(m, "ScenePointLight"    ).def_readwrite("light",     &ScenePointLight::light);
    py::class_<SceneSpotLight,      SceneNode>(m, "SceneSpotLight"     ).def_readwrite("light",     &SceneSpotLight::light);
    py::class_<ScenePrimitive,      SceneNode>(m, "ScenePrimitive"     ).def_readwrite("primitive", &ScenePrimitive::primitive);


    // ----- Scene Graph ----- //

    py::class_<SceneGraph>(m, "SceneGraph")
        .def_readonly ("totalVertexCount", &SceneGraph::totalVertexCount)
        .def_readwrite("root",             &SceneGraph::root)
        
        .def("AddEmpty",      &SceneGraph::AddEmpty,      "Adds a new empty object to the scene and returns it.",      py::arg("name"),                       py::arg("parent") = py::none())
        .def("AddModel",      &SceneGraph::AddModel,      "Adds a new model to the scene and returns it.",             py::arg("name"), py::arg("mesh"),      py::arg("parent") = py::none())
        .def("AddCamera",     &SceneGraph::AddCamera,     "Adds a new camera to the scene and returns it.",            py::arg("name"), py::arg("camera"),    py::arg("parent") = py::none())
        .def("AddDirLight",   &SceneGraph::AddDirLight,   "Adds a new directional light to the scene and returns it.", py::arg("name"), py::arg("light"),     py::arg("parent") = py::none())
        .def("AddPointLight", &SceneGraph::AddPointLight, "Adds a new point light to the scene and returns it.",       py::arg("name"), py::arg("light"),     py::arg("parent") = py::none())
        .def("AddSpotLight",  &SceneGraph::AddSpotLight,  "Adds a new spot light to the scene and returns it.",        py::arg("name"), py::arg("light"),     py::arg("parent") = py::none())
        .def("AddPrimitive",  &SceneGraph::AddPrimitive,  "Adds a new primitive to the scene and returns it.",         py::arg("name"), py::arg("primitive"), py::arg("parent") = py::none())

        .def("AddCollider", [](SceneGraph& self, SceneNode* node, const PrimitiveTypes& type, 
                               const Vector3& pos = { 0 }, const Vector3& rot = { 0 }, const Vector3& scale = { 1 })
                              { return self.AddCollider(node, type, pos, rot, scale); }, 
             "Adds a new collider to the given node and returns it as a Primitive.", 
             py::arg("node"), py::arg("primitiveType"), py::arg("position") = Vector3(), py::arg("rotation") = Vector3(), py::arg("scale") = Vector3(1), py::return_value_policy::reference)

        .def("FindId", &SceneGraph::FindId, "If a node has the given id, return it. Returns None if no node is found.",          py::arg("searchId"),   py::return_value_policy::reference)
        .def("Find",   &SceneGraph::Find,   "Returns the first node that has the given name. Returns None if no node is found.", py::arg("searchName"), py::return_value_policy::reference);


    // ----- Object Script ----- //

    py::class_<ObjectScript>(m, "ObjectScript")
        .def(py::init())

        .def_readwrite("object",    &ObjectScript::object)
        .def_readwrite("transform", &ObjectScript::transform)
        .def_readwrite("rigidbody", &ObjectScript::rigidbody)
        .def_readwrite("inputs",    &ObjectScript::inputs)
        .def_readwrite("time",      &ObjectScript::time)
        .def_readwrite("app",       &ObjectScript::app)

        .def("Start",      &ObjectScript::Start,      "Function called when the scene is in play mode, or when the object is added to the scene while in play mode.")
        .def("LateStart",  &ObjectScript::LateStart,  "Similar to the Start function, but is called after all other objects' Start functions are done.")
        .def("Update",     &ObjectScript::Update,     "Function called every frame when the scene is in play mode.")
        .def("LateUpdate", &ObjectScript::LateUpdate, "Similar to the Update function, but is called after all other objects' Update functions are done.")

        .def("OnCollisionEnter", &ObjectScript::OnCollisionEnter, "Called once whenever the object collides with another object.")
        .def("OnCollisionStay",  &ObjectScript::OnCollisionStay,  "Called every frame when the object is in collision with another object.")
        .def("OnCollisionExit",  &ObjectScript::OnCollisionExit,  "Called once whenever the object stops colliding with another object.")

        .def("IsPyScript",    &ObjectScript::IsPyScript,    "Returns True if this script is coded in Python, False if it is coded in C++.")
        .def("GetAsPyScript", &ObjectScript::GetAsPyScript, "Returns this script as a PyScript object. Returns None if this script wasn't coded in Python.");


    // ----- PyScript ----- //

    py::class_<PyScript, ObjectScript>(m, "PyScript")
        .def(py::init<std::string>())
        .def_readwrite("scriptObject", &PyScript::scriptObject);


    // ----- App ----- //

    py::class_<AppInputs>(m, "AppInputs")
        .def_readonly("exitKeyPressed",  &AppInputs::exitKeyPressed)
        .def_readonly("mouseRightClick", &AppInputs::mouseRightClick)
        .def_readonly("mouseLeftClick",  &AppInputs::mouseLeftClick)
        .def_readonly("mouseScroll",     &AppInputs::mouseScroll)
        .def_readonly("mousePos",        &AppInputs::mousePos)
        .def_readonly("mouseDelta",      &AppInputs::mouseDelta)
        .def_readonly("movement",        &AppInputs::movement);

    py::class_<App>(m, "App")
        .def_readwrite("inputs",          &App::inputs)
        .def_readwrite("time",            &App::time)
        .def_readonly ("resourceManager", &App::resourceManager)
        .def_readwrite("lightManager",    &App::lightManager)
        .def_readwrite("cameraManager",   &App::cameraManager)
        .def_readwrite("sceneGraph",      &App::sceneGraph)

        .def("GetWindowWidth",  &App::GetWindowW, "Returns the width of the engine's window.")
        .def("GetWindowHeight", &App::GetWindowH, "Returns the height of the engine's window.")
        .def("SetWindowSize",   [](App& self, const int& width, const int& height){ self.SetWindowSize(width, height); }, "Sets the engine window's size to the given one.", py::arg("width"), py::arg("height"));
}
