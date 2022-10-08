#include "CameraManager.h"
using namespace Render;

std::vector<Render::Camera*> CameraManager::screenScaleCameras;

CameraManager::~CameraManager()
{
    ClearCameras();
}

Camera* CameraManager::Create(const CameraParams& params, const bool& scaleToScreen)
{
    Camera* camera = new Camera(params);
    cameras.push_back(camera);
    if (scaleToScreen)
        screenScaleCameras.push_back(camera);
    return camera;
}

void CameraManager::ClearCameras()
{
    // Free all cameras.
    for (Camera* camera : cameras)
        delete camera;
    cameras.clear();
    sceneCamera = nullptr;

    // Clear screen scale cameras but keep the engine camera.
    screenScaleCameras.clear();
    if (engineCamera != nullptr)
        screenScaleCameras.push_back(engineCamera);
}
