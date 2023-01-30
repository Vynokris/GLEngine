#pragma once

#include "Maths.h"

struct GLFWwindow;
namespace Core
{
    struct AppInputs;
}

namespace Render
{
    struct CameraParams
    {
        unsigned int width, height;
        float near = 0.1f, far = 1000, fov = 80.f, aspect = 1;
    };

    class Camera
    {
    private:
        Core::Maths::Mat4 projectionMat;
        Core::Maths::Mat4 viewProjMat;
        CameraParams      params;

    public:
        Core::Maths::Transform* transform;

        Camera(const CameraParams& parameters);
        void ChangeParameters(const CameraParams& parameters);
        virtual ~Camera() {}

        CameraParams      GetParameters()     const;
        Core::Maths::Mat4 GetWorldTransform() const;
        Core::Maths::Mat4 GetProjectionMat()  const;
        Core::Maths::Mat4 GetViewMat()        const;
    };

    class EngineCamera : public Camera
    {
    public:
        float moveSpeed = 5.5f, turnSpeed = 0.5f;

        EngineCamera(const CameraParams& parameters);
        ~EngineCamera();

        void ProcessInputs(Core::AppInputs& inputs, GLFWwindow* window);
    };
}
