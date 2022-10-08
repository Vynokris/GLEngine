#pragma once

#include <type_traits>
#include <string>
#include "Debug.h"
#include "LightManager.h"
using namespace Render;


template <typename T> inline LightTypes FindLightType()
{
    // Make sure the given type is supported.
    bool isSupported = std::is_base_of<ILight, T>::value;
    Assert(isSupported, "Light type not supported.");

    // Find what light type T is.
    LightTypes type = LightTypes::Directional;
    {
        bool isPoint = std::is_same<PointLight, T>::value;
        bool isSpot  = std::is_same<SpotLight,  T>::value;
        if      (isPoint) type = LightTypes::Point;
        else if (isSpot ) type = LightTypes::Spot;
    }
    return type;
}

template <typename T> inline T* LightManager::Create()
{
    T* assignedLight = nullptr;
    switch (FindLightType<T>())
    {
    // Assign directional lights.
    case LightTypes::Directional:
        for (int i = 0; i < MAX_DIR_LIGHTS; i++) {
            if (dirLights[i] == nullptr) {
                dirLights[i] = (DirLight*)(new T(i));
                assignedLight = (T*)dirLights[i];
            break;
            }
        }
        break;

    // Assign point lights.
    case LightTypes::Point:
        for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
            if (pointLights[i] == nullptr) {
                pointLights[i] = (PointLight*)(new T(i));
                assignedLight = (T*)pointLights[i];
                break;
            }
        }
        break;

    // Assign spot lights.
    default:
        for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
            if (spotLights[i] == nullptr) {
                spotLights[i] = (SpotLight*)(new T(i));
                assignedLight = (T*)spotLights[i];
                break;
            }
        }
        break;
    }

    if (assignedLight == nullptr)
        DebugLogWarning("Unable to create more lights: maximum reached.");

    return assignedLight;
}

template <typename T> inline T* LightManager::Get(const unsigned int& id)
{
    switch (FindLightType<T>())
    {
    // Find directional lights.
    case LightTypes::Directional:
        if (id >= MAX_DIR_LIGHTS) {
            DebugLogWarning("Light id " + std::to_string(id) + " is higher than the maximum number of directional lights (" + std::to_string(MAX_DIR_LIGHTS) + ").");
            return nullptr;
        }
        if (dirLights[id] == nullptr) {
            DebugLogWarning("Trying to get a light that is not assigned (id: " + std::to_string(id) + ").");
        }
        return (T*)dirLights[id];

    // Find point lights.
    case LightTypes::Point:
        if (id >= MAX_POINT_LIGHTS) {
            DebugLogWarning("Light id " + std::to_string(id) + " is higher than the maximum number of point lights (" + std::to_string(MAX_POINT_LIGHTS) + ").");
            return nullptr;
        }
        if (pointLights[id] == nullptr) {
            DebugLogWarning("Trying to get a light that is not assigned (id: " + std::to_string(id) + ").");
        }
        return (T*)pointLights[id];

    // Find spot lights.
    default:
        if (id >= MAX_SPOT_LIGHTS) {
            DebugLogWarning("Light id " + std::to_string(id) + " is higher than the maximum number of spot lights (" + std::to_string(MAX_SPOT_LIGHTS) + ").");
            return nullptr;
        }
        if (spotLights[id] == nullptr) {
            DebugLogWarning("Trying to get a light that is not assigned (id: " + std::to_string(id) + ").");
        }
        return (T*)spotLights[id];
    }
}

template <typename T> inline T** LightManager::GetLights()
{
    switch (FindLightType<T>())
    {
    // Find directional lights.
    case LightTypes::Directional:
        return (T**)&dirLights[0];

    // Find point lights.
    case LightTypes::Point:
        return (T**)&pointLights[0];

    // Find spot lights.
    default:
        return (T**)&spotLights[0];
    }
}

template <typename T> inline void LightManager::Delete(const unsigned int& id)
{
    switch (FindLightType<T>())
    {
    // Assign directional lights.
    case LightTypes::Directional:
        if (id >= MAX_DIR_LIGHTS) return;
        if (dirLights[id] != nullptr) {
            delete dirLights[id];
            dirLights[id] = nullptr;
        }
        return;

    // Assign point lights.
    case LightTypes::Point:
        if (id >= MAX_POINT_LIGHTS) return;
        if (pointLights[id] != nullptr) {
            delete pointLights[id];
            pointLights[id] = nullptr;
        }
        return;

    // Assign spot lights.
    default:
        if (id >= MAX_SPOT_LIGHTS) return;
        if (spotLights[id] != nullptr) {
            delete spotLights[id];
            spotLights[id] = nullptr;
        }
        return;
    }
}
