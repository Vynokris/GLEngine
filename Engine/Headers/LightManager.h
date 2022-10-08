#pragma once

#include "Light.h"

#define MAX_DIR_LIGHTS 5
#define MAX_POINT_LIGHTS 20
#define MAX_SPOT_LIGHTS 20
// Don't forget to change the values above in the shader too.

namespace Render
{
    class LightManager
    {
    private:
        DirLight*   dirLights  [MAX_DIR_LIGHTS]   = {};
        PointLight* pointLights[MAX_POINT_LIGHTS] = {};
        SpotLight*  spotLights [MAX_SPOT_LIGHTS]  = {};

    public:
        LightManager();
        ~LightManager();

        template <typename T> T*   Create();
        template <typename T> T*   Get   (const unsigned int& id);
        template <typename T> T**  GetLights();
        template <typename T> void Delete(const unsigned int& id);
                              void ClearLights();

        void SendLightsToShader(const unsigned int& shaderProgramId) const;
    };
}

#include "LightManager.inl"
