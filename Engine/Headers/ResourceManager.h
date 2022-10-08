#pragma once

#include <unordered_map>
#include "IResource.h"
#include "Textures.h"
#include "TextureSampler.h"
#include "Material.h"
#include "SubMesh.h"
#include "Mesh.h"
#include "Shader.h"
#include "ObjFile.h"
#include "MtlFile.h"
#include "ThreadManager.h"

namespace Resources
{
    class ResourceManager
    {
    private:
        static bool asyncLoad;
        static Resources::TextureSampler* sampler;
        std::unordered_map<std::string, IResource*> resources;
		std::atomic_flag resourceLock = ATOMIC_FLAG_INIT;

    public:
        Core::ThreadManager threadManager;
        std::vector<std::pair<ResourceTypes, std::string>> pyResourceCreationQueue;

        ResourceManager();
        ~ResourceManager();

		ResourceManager(const ResourceManager&)			   = delete;
		ResourceManager(ResourceManager&&)				   = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager& operator=(ResourceManager&&)      = delete;

        template <typename T> T* Create(const std::string& name);
        template <typename T> T* Get   (const std::string& name);
        template <typename T> T* Find  (const std::string& searchTerm);
        void                     Delete(const std::string& name);
        bool                     Reset ();

        void CreateSampler();
        static TextureSampler* GetSampler() { return sampler; }

        std::unordered_map<std::string, IResource*>& GetResources() { return resources; }

        bool AreAllResourcesLoaded();
        bool AreAllResourcesInOpenGL();
        static void SetAsyncLoading(const bool& async) { asyncLoad = async; }
        static bool AsyncLoading()                     { return asyncLoad;  }

        void CheckForNewPyResources();
    };
}

#include "ResourceManager.inl"
