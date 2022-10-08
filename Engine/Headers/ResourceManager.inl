#pragma once

#include <type_traits>
#include <string>
#include <thread>
#include "Debug.h"
#include "Material.h"
using namespace Resources;


// Creates a new resource of the specified type and returns it.
template <typename T> inline T* ResourceManager::Create(const std::string& name)
{
    // Make sure the given type is supported.
    bool isSupported = std::is_base_of<IResource, T>::value;
    Assert(isSupported, (std::string("Specified resource type not supported for: ") + name).c_str());
    
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) > 0) 
    {
        // Return any previous resource.
        if (resources[name]->GetType() == T::GetResourceType()) 
        {
            if (AsyncLoading())
                threadManager.AddTask(resources[name]);
            else
                resources[name]->Load();
	        resourceLock.clear();
            return (T*)resources[name];
        }

        // Delete any previous resource with a different type.
        else {
            DebugLogWarning("Resource created twice with different types: " + name);
            Delete(name);
        }
    }

    // Create the resource.
    resources[name] = (IResource*)(new T(name));
	resourceLock.clear();

    // Load and return the resource.
    if (AsyncLoading())
        threadManager.AddTask(resources[name]);
    else
        resources[name]->Load();
    return (T*)resources[name];
}

template <> inline ObjFile* ResourceManager::Create(const std::string& name)
{
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) > 0) 
    {
        // Return any previous resource.
        if (resources[name]->GetType() == ObjFile::GetResourceType()) {
            if (AsyncLoading())
                threadManager.AddTask(resources[name]);
            else
                resources[name]->Load();
	        resourceLock.clear();
            return (ObjFile*)resources[name];
        }

        // Delete any previous resource with a different type.
        else {
            DebugLogWarning("Resource created twice with different types: " + name);
            Delete(name);
        }
    }

    // Create the resource.
    resources[name] = (IResource*)(new ObjFile(name, *this));
	resourceLock.clear();
    
    // Load and return the resource.
    if (AsyncLoading())
        threadManager.AddTask(resources[name]);
    else
        resources[name]->Load();
    return (ObjFile*)resources[name];
}

template <> inline MtlFile* ResourceManager::Create(const std::string& name)
{
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) > 0) 
    {
        // Return any previous resource.
        if (resources[name]->GetType() == MtlFile::GetResourceType()) {
            resources[name]->Load();
	        resourceLock.clear();
            return (MtlFile*)resources[name];
        }

        // Delete any previous resource with a different type.
        else {
            DebugLogWarning("Resource created twice with different types: " + name);
            Delete(name);
        }
    }

    // Create the resource.
    resources[name] = (IResource*)(new MtlFile(name, *this));
	resourceLock.clear();
    
    // Load and return the resource.
    resources[name]->Load();
    return (MtlFile*)resources[name];
}

// Attempts to find a resource of the specified type with the specified name and return it.
template <typename T> inline T* ResourceManager::Get(const std::string& name)
{
    // Make sure the given type is supported.
    bool isSupported = std::is_base_of<IResource, T>::value;
    Assert(isSupported, (std::string("Specified resource type not supported for: ") + name).c_str());
    
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) <= 0)
    {
        DebugLogWarning("Not found resource was created: " + name);
        resources[name] = (IResource*)(new T(name));
    }
    if (resources[name]->GetType() != T::GetResourceType())
    {
	    resourceLock.clear();
        DebugLogWarning("Resource found with the wrong type: " + name);
        return nullptr;
    }
	resourceLock.clear();
    return (T*)resources[name];
}

template <> inline ObjFile* ResourceManager::Get(const std::string& name)
{
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) <= 0)
    {
        DebugLogWarning("Not found resource was created: " + name);
        resources[name] = (IResource*)(new ObjFile(name, *this));
    }
    if (resources[name]->GetType() != ObjFile::GetResourceType())
    {
	    resourceLock.clear();
        DebugLogWarning("Resource found with the wrong type: " + name);
        return nullptr;
    }
	resourceLock.clear();
    return (ObjFile*)resources[name];
}

template <> inline MtlFile* ResourceManager::Get(const std::string& name)
{
    while (resourceLock.test_and_set()) {}
    if (resources.count(name) <= 0)
    {
        DebugLogWarning("Not found resource was created: " + name);
        resources[name] = (IResource*)(new MtlFile(name, *this));
    }
    if (resources[name]->GetType() != MtlFile::GetResourceType())
    {
	    resourceLock.clear();
        DebugLogWarning("Resource found with the wrong type: " + name);
        return nullptr;
    }
	resourceLock.clear();
    return (MtlFile*)resources[name];
}

// Attempts to find a resource of the specified type which name contains the specified search term and return it.
template <typename T> inline T* ResourceManager::Find(const std::string& searchTerm)
{
    // Make sure the given type is supported.
    bool isSupported = std::is_base_of<IResource, T>::value;
    Assert(isSupported, (std::string("Specified resource type not supported for: ") + searchTerm).c_str());
    
    T* resource = nullptr;
    for (auto& it : resources) 
    {
        if (it.first.find(searchTerm) != std::string::npos) 
        {
            if (it.second->GetType() != T::GetResourceType())
                continue;
            resource = (T*)it.second;
            break;
        }
    }
    if (resource == nullptr) {
        DebugLogWarning("Resource not found: " + searchTerm);
        return nullptr;
    }
    return resource;
}
