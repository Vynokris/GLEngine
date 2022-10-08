#include <glad/glad.h>
#include "LightManager.h"
using namespace Render;

LightManager::LightManager()
{
    for (int i = 0; i < MAX_DIR_LIGHTS; i++)
        dirLights[i] = nullptr;

    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        pointLights[i] = nullptr;

    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        spotLights[i] = nullptr;
}

LightManager::~LightManager()
{
    ClearLights();
}

void LightManager::SendLightsToShader(const unsigned int& shaderProgramId) const
{
    // Send all directional light data.
    for (int i = 0; i < MAX_DIR_LIGHTS; i++)
    {
        std::string shaderLightName = std::string("dirLights[") + std::to_string(i) + "]";
        if (dirLights[i] != nullptr)
        {
            glUniform1i (glGetUniformLocation(shaderProgramId, (shaderLightName + ".assigned").c_str()), 1);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".ambient" ).c_str()), 1, &dirLights[i]->ambient.r);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".diffuse" ).c_str()), 1, &dirLights[i]->diffuse.r);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".specular").c_str()), 1, &dirLights[i]->specular.r);
            glUniform3f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".dir"     ).c_str()),    -dirLights[i]->dir.x, 
                                                                                                             dirLights[i]->dir.y, 
                                                                                                             dirLights[i]->dir.z);
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgramId, (shaderLightName + ".assigned").c_str()), 0);
        }
    }

    // Send all point light data.
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        std::string shaderLightName = std::string("pointLights[") + std::to_string(i) + "]";
        if (pointLights[i] != nullptr)
        {
            glUniform1i (glGetUniformLocation(shaderProgramId, (shaderLightName + ".assigned").c_str()), 1);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".ambient"  ).c_str()), 1, &pointLights[i]->ambient.r);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".diffuse"  ).c_str()), 1, &pointLights[i]->diffuse.r);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".specular" ).c_str()), 1, &pointLights[i]->specular.r);
            glUniform1f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".constant" ).c_str()),     pointLights[i]->constant);
            glUniform1f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".linear"   ).c_str()),     pointLights[i]->linear);
            glUniform1f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".quadratic").c_str()),     pointLights[i]->quadratic);
            glUniform3f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".pos"      ).c_str()),    -pointLights[i]->pos.x, 
                                                                                                              pointLights[i]->pos.y,
                                                                                                              pointLights[i]->pos.z);
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgramId, (shaderLightName + ".assigned").c_str()), 0);
        }
    }

    // Send all spot light data.
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        std::string shaderLightName = std::string("spotLights[") + std::to_string(i) + "]";
        if (spotLights[i] != nullptr)
        {
            glUniform1i (glGetUniformLocation(shaderProgramId, (shaderLightName + ".assigned" ).c_str()), 1);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".ambient"  ).c_str()), 1, &spotLights[i]->ambient.r);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".diffuse"  ).c_str()), 1, &spotLights[i]->diffuse.r);
            glUniform3fv(glGetUniformLocation(shaderProgramId, (shaderLightName + ".specular" ).c_str()), 1, &spotLights[i]->specular.r);
            glUniform1f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".outerCone").c_str()), cos(spotLights[i]->outerCone));
            glUniform1f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".innerCone").c_str()), cos(spotLights[i]->innerCone));
            glUniform3f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".pos"      ).c_str()),    -spotLights[i]->pos.x, 
                                                                                                              spotLights[i]->pos.y,
                                                                                                              spotLights[i]->pos.z);
            glUniform3f (glGetUniformLocation(shaderProgramId, (shaderLightName + ".dir"      ).c_str()),    -spotLights[i]->dir.x, 
                                                                                                              spotLights[i]->dir.y, 
                                                                                                              spotLights[i]->dir.z);
        }
        else
        {
            glUniform1i(glGetUniformLocation(shaderProgramId, (shaderLightName + ".assigned").c_str()), 0);
        }
    }
}

void LightManager::ClearLights()
{
    for (int i = 0; i < MAX_DIR_LIGHTS; i++) {
        if (dirLights[i] != nullptr) {
            delete dirLights[i];
            dirLights[i] = nullptr;
        }
    }

    for (int i = 0; i < MAX_POINT_LIGHTS; i++) {
        if (pointLights[i] != nullptr) {
            delete pointLights[i];
            pointLights[i] = nullptr;
        }
    }

    for (int i = 0; i < MAX_SPOT_LIGHTS; i++) {
        if (spotLights[i] != nullptr) {
            delete spotLights[i];
            spotLights[i] = nullptr;
        }
    }
}
