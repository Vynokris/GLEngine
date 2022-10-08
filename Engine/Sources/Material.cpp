#include <glad/glad.h>

#include "ResourceManager.h"
#include "Material.h"
using namespace Resources;
using namespace Core::Maths;


Material::Material(const std::string& _name)
{
    name = _name;
    type = ResourceTypes::Material;
}

void Material::Load()
{
    SetLoadingDone();
}

void Material::SendToOpenGL()
{
    SetOpenGLTransferDone();
}

void Material::SetParams(const RGB& _ambient, const RGB& _diffuse, const RGB& _specular, const Core::Maths::RGB& _emission, const float& _shininess)
{
    ambient   = _ambient;
    diffuse   = _diffuse;
    specular  = _specular;
    emission  = _emission;
    shininess = _shininess;
}

void Material::SendDataToShader(const unsigned int& shaderProgram, const unsigned int& sampler) const
{
    // Send material parameters to shader.
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"     ), 1, &ambient.r);
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"     ), 1, &diffuse.r);
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"    ), 1, &specular.r);
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.emission"    ), 1, &emission.r);
    glUniform1f (glGetUniformLocation(shaderProgram, "material.shininess"   ),     shininess);
    glUniform1f (glGetUniformLocation(shaderProgram, "material.transparency"),     transparency);

    // Cull back faces of non-transparent models.
    if (transparency >= 1 && alphaMap == nullptr)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);

    // Send ambient texture to shader.
    if (ambientTexture != nullptr && ambientTexture->WasSentToOpenGL()) {
        glBindTextureUnit(1, ambientTexture->GetId());
        glBindSampler    (1, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "ambientTexture"),    1);
        glUniform1i(glGetUniformLocation(shaderProgram, "useAmbientTexture"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useAmbientTexture"), 0);
    }

    // Send diffuse texture to shader.
    if (diffuseTexture != nullptr && diffuseTexture->WasSentToOpenGL()) {
        glBindTextureUnit(2, diffuseTexture->GetId());
        glBindSampler    (2, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTexture"),    2);
        glUniform1i(glGetUniformLocation(shaderProgram, "useDiffuseTexture"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useDiffuseTexture"), 0);
    }

    // Send specular texture to shader.
    if (specularTexture != nullptr && specularTexture->WasSentToOpenGL()) {
        glBindTextureUnit(3, specularTexture->GetId());
        glBindSampler    (3, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "specularTexture"),    3);
        glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularTexture"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularTexture"), 0);
    }

    // Send emission texture to shader.
    if (emissionTexture != nullptr && emissionTexture->WasSentToOpenGL()) {
        glBindTextureUnit(4, emissionTexture->GetId());
        glBindSampler    (4, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "emissionTexture"),    4);
        glUniform1i(glGetUniformLocation(shaderProgram, "useEmissionTexture"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useEmissionTexture"), 0);
    }

    // Send shininess map to shader.
    if (shininessMap != nullptr && shininessMap->WasSentToOpenGL()) {
        glBindTextureUnit(5, shininessMap->GetId());
        glBindSampler    (5, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "shininessMap"),    5);
        glUniform1i(glGetUniformLocation(shaderProgram, "useShininessMap"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useShininessMap"), 0);
    }

    // Send alpha map to shader.
    if (alphaMap != nullptr && alphaMap->WasSentToOpenGL()) {
        glBindTextureUnit(6, alphaMap->GetId());
        glBindSampler    (6, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "alphaMap"),    6);
        glUniform1i(glGetUniformLocation(shaderProgram, "useAlphaMap"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useAlphaMap"), 0);
    }

    // Send normal map to shader.
    if (normalMap != nullptr && normalMap->WasSentToOpenGL()) {
        glBindTextureUnit(7, normalMap->GetId());
        glBindSampler    (7, sampler);
        glUniform1i(glGetUniformLocation(shaderProgram, "normalMap"),    7);
        glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMap"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(shaderProgram, "useNormalMap"), 0);
    }
}
