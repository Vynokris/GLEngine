#include <imgui/imgui.h>
#include <GLFW/glfw3.h>
#include "App.h"
#include "Maths.h"
#include "Camera.h"
using namespace Render;
using namespace Core::Maths;


Camera::Camera(const CameraParams& parameters)
{
    ChangeParameters(parameters);
}

void Camera::ChangeParameters(const CameraParams& parameters)
{
    params = CameraParams(parameters);
    params.aspect = (float)params.width / params.height;

    // Update the projection matrix.
    float yScale = 1 / tan(degToRad(params.fov / 2));
    float xScale = yScale / params.aspect;
    projectionMat = Mat4(
        -xScale, 0, 0, 0,
        0, yScale, 0, 0,
        0, 0, params.far / (params.far - params.near), 1,
        0, 0, -params.far * params.near / (params.far - params.near), 0
    );
}

CameraParams Camera::GetParameters() const { return params; }
Mat4 Camera::GetWorldTransform()     const { return GetViewMat().inv4(); }
Mat4 Camera::GetProjectionMat()      const { return projectionMat;       }
Mat4 Camera::GetViewMat()            const { Assert(transform, "Camera transform has not been set.");  return transform->GetModelMat(); }



// ----- Engine Camera ----- //

EngineCamera::EngineCamera(const CameraParams& parameters)
             : Camera(parameters)
{
    transform = new Transform(true);
}

EngineCamera::~EngineCamera()
{
    delete transform;
}

void EngineCamera::ProcessInputs(Core::AppInputs& inputs, GLFWwindow* window)
{
    if (inputs.mouseRightClick && transform)
    {
        float deltaTime = ((App*)glfwGetWindowUserPointer(window))->time.DeltaTime();

        // Rotate camera (Yaw locked between -90° and 90°, Pitch reset to 0 when it reaches 360°).
        Vector3 rotation = transform->GetRotation();
        transform->SetRotation(Vector3(clamp(rotation.x + inputs.mouseDelta.y * turnSpeed * deltaTime, -PI/2 + 0.001f, PI/2 - 0.001f), 
                                       fmodf(rotation.y - inputs.mouseDelta.x * turnSpeed * deltaTime, 2*PI), rotation.z));

        // Move camera in function of rotation.
        transform->Move((Vector4(inputs.movement, 1) * GetRotationMatrix(transform->GetRotation())).toVector3(true) * (moveSpeed * deltaTime));

        // Change camera speed according to scroll wheel.
        moveSpeed = clampAbove(moveSpeed + (float)inputs.mouseScroll / 20, 0.05f);

        // Hide cursor and disable its movement.
        if (ImGui::GetMouseCursor() == ImGuiMouseCursor_Arrow)
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        inputs.mousePos = inputs.mousePos - inputs.mouseDelta;
        glfwSetCursorPos(window, (double)inputs.mousePos.x, (double)inputs.mousePos.y);
    }
    else if (ImGui::GetMouseCursor() == ImGuiMouseCursor_None)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }
}
