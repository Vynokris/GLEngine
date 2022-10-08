#pragma once
#include <chrono>

namespace Core
{
    class Time
    {
    private:
        bool  vsync;
        int   targetFps;
        float targetDeltaTime, lastFrameDuration;
        std::chrono::steady_clock::time_point frameStartTime;

    public:
        Time(const bool& _vsync = true, const int& fps = 60);

        bool  IsVsyncOn() const;
        void  SetVsyncOn(const bool& vsyncOn);
        int   GetTargetFPS() const;
        void  SetTargetFPS(int _fps);
        float FPS()               const;
        float DeltaTime()         const;
        bool  CanStartNextFrame() const;
        void  NewFrame();
    };
}