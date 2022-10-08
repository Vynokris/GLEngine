#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <STB_Image/stb_image.h>

#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#pragma warning(disable : 4996)

#include "Maths.h"
#include "ResourceManager.h"
using namespace Core::Maths;
using namespace Resources;


bool                       ResourceManager::asyncLoad = false;
Resources::TextureSampler* ResourceManager::sampler = nullptr;

ResourceManager::ResourceManager() 
    : threadManager(10)
{
    stbi_set_flip_vertically_on_load(true);
}

ResourceManager::~ResourceManager()
{
    while (resourceLock.test_and_set()) {}
    for (auto& it : resources) 
        delete it.second;
    resources.clear();
    delete sampler;
	resourceLock.clear();
}

// Attempts to delete the resource that has the specified name.
void ResourceManager::Delete(const std::string& name)
{
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) <= 0) {
        DebugLogWarning("Resource not found: " + name);
        resourceLock.clear();
        return;
    }
    if (!resources[name]->WasSentToOpenGL()) {
        DebugLogWarning("Unable to delete currently loading resource: " + name);
        resourceLock.clear();
        return;
    }
    delete resources[name];
    resources.erase(name);
    resourceLock.clear();
}

// Delete all of the loaded resources if all are loaded.
bool ResourceManager::Reset()
{
    if (!AreAllResourcesInOpenGL())
        return false;

    for (auto& it : resources)
        delete it.second;
    resources.clear();
    return true;
}

void ResourceManager::CreateSampler()
{
    if (sampler == nullptr) {
        sampler = new TextureSampler();
        sampler->SetDefaultParams();
    }
}

bool ResourceManager::AreAllResourcesLoaded()
{
    while (resourceLock.test_and_set()) {}
    for (auto& it : resources) {
        if (!it.second->IsLoaded()) {
	        resourceLock.clear();
            return false;
        }
    }
	resourceLock.clear();
   return true;
}

bool ResourceManager::AreAllResourcesInOpenGL()
{
    while (resourceLock.test_and_set()) {}
    for (auto& it : resources) {
        if (!it.second->WasSentToOpenGL()) {
	        resourceLock.clear();
            return false;
        }
    }
	resourceLock.clear();
   return true;
}

void ResourceManager::CheckForNewPyResources()
{
    while (pyResourceCreationQueue.size() > 0)
    {
        switch (pyResourceCreationQueue[0].first)
        {
        case ResourceTypes::Texture:        Create<Texture       >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::DynamicTexture: Create<DynamicTexture>(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::Material:       Create<Material      >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::Mesh:           Create<Mesh          >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::VertexShader:   Create<VertexShader  >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::FragmentShader: Create<FragmentShader>(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::ComputeShader:  Create<ComputeShader >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::ShaderProgram:  Create<ShaderProgram >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::ObjFile:        Create<ObjFile       >(pyResourceCreationQueue[0].second); break;
        case ResourceTypes::MtlFile:        Create<MtlFile       >(pyResourceCreationQueue[0].second); break;
        }
        pyResourceCreationQueue.erase(pyResourceCreationQueue.begin());
    }
}
