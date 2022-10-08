#include <GLFW/glfw3.h>
#include "TimeManager.h"
using namespace Core;
using namespace std::chrono;


Time::Time(const bool& _vsync, const int& fps)
{
    frameStartTime = high_resolution_clock::now();
    SetTargetFPS(fps);

    vsync = _vsync;
    if (vsync)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);
}

bool Time::IsVsyncOn() const
{
    return vsync;
}

void Time::SetVsyncOn(const bool& vsyncOn)
{
    vsync = vsyncOn;
    if (vsync)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);
}

int Time::GetTargetFPS() const
{
    return targetFps;
}

void Time::SetTargetFPS(int _fps)
{
    targetFps = _fps;
    targetDeltaTime = 1.0f / targetFps;
}

float Time::FPS() const
{
    return 1 / DeltaTime();
}

float Time::DeltaTime() const
{
    return lastFrameDuration;
}

bool Time::CanStartNextFrame() const
{
    return (float)(duration_cast<nanoseconds>(high_resolution_clock::now() - frameStartTime).count() * 1e-9) > targetDeltaTime;
}

void Time::NewFrame()
{
    steady_clock::time_point lastFrameTime = frameStartTime;
    frameStartTime = high_resolution_clock::now();
    lastFrameDuration = (float)(duration_cast<nanoseconds>(frameStartTime - lastFrameTime).count() * 1e-9);
}
