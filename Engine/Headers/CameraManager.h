#pragma once
#include <vector>
#include "Camera.h"

namespace Render
{
    class CameraManager
    {
    public:
        std::vector<Camera*> cameras;
        static std::vector<Camera*> screenScaleCameras;
        EngineCamera* engineCamera = nullptr;
        Camera*       sceneCamera  = nullptr;

        ~CameraManager();
        Camera* Create(const CameraParams& params, const bool& scaleToScreen = false);
        void ClearCameras();
    };
}