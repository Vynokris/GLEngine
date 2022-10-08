#include "MtlFile.h"
#include "Mesh.h"
#include "ResourceManager.h"
#include <filesystem>
#include <fstream>
using namespace Resources;


void MtlFile::ParseMtlColor(const std::string& line, float* colorValues)
{
    size_t start = 3;
    size_t end = line.find(' ', start);

    for (int i = 0; i < 3; i++)
    {
        // Find the current coordinate value.
        std::string val = line.substr(start, end - start);

        // Set the current color value.
        colorValues[i] = std::strtof(val.c_str(), nullptr);

        // Update the start and end indices.
        start = end + 1;
        end = line.find(' ', start);
    }
}

MtlFile::MtlFile(const std::string& _name, ResourceManager& _resourceManager)
    : resourceManager(_resourceManager)
{
    name = _name;
    type = ResourceTypes::MtlFile;
}

void MtlFile::Load()
{
    // Read file contents and extract them to the data string.
    std::stringstream fileContents;
    {
        std::fstream f(name, std::ios_base::in | std::ios_base::app);
        fileContents << f.rdbuf();
        f.close();
    }
    std::string filepath = std::filesystem::path(name).parent_path().string() + "/";

    // Read file line by line to create material data.
    std::string line;
    Material* curMaterial = nullptr;
    while (std::getline(fileContents, line)) 
    {
        // Create new material.
        if (line[0] == 'n' && line[1] == 'e' && line[2] == 'w')
        {
            curMaterial = resourceManager.Create<Material>(line.substr(7, line.size() - 7));
            createdResources.push_back(curMaterial);
            continue;
        }

        // Don't touch material values if it is not yet created.
        if (curMaterial == nullptr)
            continue;
        line += ' ';

        // Read color values.
        if (line[0] == 'K')
        {
            switch (line[1])
            {
            case 'a':
                ParseMtlColor(line, curMaterial->ambient.ptr());
                continue;
            case 'd':
                ParseMtlColor(line, curMaterial->diffuse.ptr());
                continue;
            case 's':
                ParseMtlColor(line, curMaterial->specular.ptr());
                continue;
            case 'e':
                ParseMtlColor(line, curMaterial->emission.ptr());
                continue;
            default:
                break;
            }
        }

        // Read shininess.
        if (line[0] == 'N' && line[1] == 's')
        {
            curMaterial->shininess = std::strtof(line.substr(3, line.size()-4).c_str(), nullptr);
            continue;
        }

        // Read transparency.
        if (line[0] == 'd')
        {
            curMaterial->transparency = std::strtof(line.substr(2, line.size()-3).c_str(), nullptr);
            continue;
        }
        if (line[0] == 'T' && line[1] == 'r')
        {
            curMaterial->transparency = 1 - std::strtof(line.substr(2, line.size()-3).c_str(), nullptr);
            continue;
        }

        // Read texture maps.
        if (line[0] == 'm' && line[1] == 'a' && line[2] == 'p')
        {
            std::string texName = line.substr(7, line.size()-8);
            if (line[4] == 'K')
            {
                switch (line[5])
                {
                // Ambient texture.
                case 'a':
                    curMaterial->ambientTexture = resourceManager.Create<Texture>(filepath + texName);
                    createdResources.push_back(curMaterial->ambientTexture);
                    continue;

                // Diffuse texture.
                case 'd':
                    curMaterial->diffuseTexture = resourceManager.Create<Texture>(filepath + texName);
                    createdResources.push_back(curMaterial->diffuseTexture);
                    continue;

                // Specular texture.
                case 's':
                    curMaterial->specularTexture = resourceManager.Create<Texture>(filepath + texName);
                    createdResources.push_back(curMaterial->specularTexture);
                    continue;

                // Emission texture.
                case 'e':
                    curMaterial->emissionTexture = resourceManager.Create<Texture>(filepath + texName);
                    createdResources.push_back(curMaterial->emissionTexture);
                    continue;

                default:
                    break;
                }
            }
            // Shininess map.
            if (line[4] == 'N' && line[5] == 's')
            {
                std::string texName = line.substr(7, line.size()-8);
                curMaterial->shininessMap = resourceManager.Create<Texture>(filepath + texName);
                createdResources.push_back(curMaterial->shininessMap);
                continue;
            }
            // Alpha map.
            if (line[4] == 'd')
            {
                std::string texName = line.substr(6, line.size()-7);
                curMaterial->alphaMap = resourceManager.Create<Texture>(filepath + texName);
                createdResources.push_back(curMaterial->alphaMap);
                continue;
            }
        }
        {
            // Normal map.
            size_t bumpIndex = line.find("bump ");
            if (bumpIndex == std::string::npos) bumpIndex = line.find("Bump ");
            if (bumpIndex != std::string::npos)
            {
                std::string texName = line.substr(bumpIndex+5, line.size()-(bumpIndex+5)-1);
                curMaterial->normalMap = resourceManager.Create<Texture>(filepath + texName);
                createdResources.push_back(curMaterial->normalMap);
                continue;
            }
        }
    }
    SetLoadingDone();
}

void MtlFile::SendToOpenGL()
{
    SetOpenGLTransferDone();
}
