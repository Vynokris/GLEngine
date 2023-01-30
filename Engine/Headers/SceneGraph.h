#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "SceneNode.h"

namespace Core::Maths
{
    class Vector3;
    struct TangentVertex;
}

namespace Render
{
    class Camera;
    class DirLight;
    class PointLight;
    class LightManager;
}

namespace Scenes
{
    class SceneNode;

    class SceneGraph
    {
    private:
        size_t FindFirstFreeId();

    public:
        size_t totalVertexCount = 0;
        std::vector<size_t> assignedNodeIds;
        std::vector<Physics::Primitive*> sceneColliders;
        SceneNode* root = new SceneNode(0, "Root", nullptr);
        SceneNode* selectedNode = nullptr;

        ~SceneGraph();

        SceneNode*            AddEmpty          (const std::string& name,                                                          SceneNode* parent = nullptr);
        SceneModel*           AddModel          (const std::string& name, Resources::Mesh*    meshGroup,                           SceneNode* parent = nullptr);
        SceneInstancedModel*  AddInstancedModel (const std::string& name, Resources::Mesh*    meshGroup, const int& instanceCount, SceneNode* parent = nullptr);
        SceneSkybox*          AddSkybox         (const std::string& name, Resources::Cubemap* cubemap,                             SceneNode* parent = nullptr);
        SceneCamera*          AddCamera         (const std::string& name, Render::Camera*     camera,                              SceneNode* parent = nullptr);
        SceneDirLight*        AddDirLight       (const std::string& name, Render::DirLight*   light,                               SceneNode* parent = nullptr);
        ScenePointLight*      AddPointLight     (const std::string& name, Render::PointLight* light,                               SceneNode* parent = nullptr);
        SceneSpotLight*       AddSpotLight      (const std::string& name, Render::SpotLight*  light,                               SceneNode* parent = nullptr);
        ScenePrimitive*       AddPrimitive      (const std::string& name, Physics::Primitive* primitive, SceneNode* parent = nullptr);
        Physics::Primitive* AddCollider(SceneNode* node, const Physics::PrimitiveTypes& type, const Core::Maths::Vector3& pos = {0}, const Core::Maths::Vector3& rot = {0}, const Core::Maths::Vector3& scale = {1});
        Physics::Primitive* AddCollider(SceneNode* node, const Physics::PrimitiveTypes& type, const std::vector<Core::Maths::TangentVertex>& vertices);
        Physics::Primitive* AddCollider(SceneNode* node, const Physics::PrimitiveTypes& type, const std::vector<Core::Maths::Vector3>& vertices);

        void StartPlayMode();
        void UpdateAndDrawAll(const Render::Camera& camera, const Render::LightManager& lightManager, const bool& dontUpdateScripts = false);
        void ClearAll();

        SceneNode* FindId(const size_t& searchId);
        SceneNode* Find  (const std::string& searchName);

        void ShowUi();
    };
}