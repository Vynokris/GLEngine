#pragma once

#include <string>
#include <vector>
#include "Maths.h"
#include "Physics.h"

namespace Resources
{
    class Mesh;
    class Cubemap;
    class Material;
    class ShaderProgram;
}

namespace Render
{
    class Camera;
    class DirLight;
    class PointLight;
    class SpotLight;
    class LightManager;
}

namespace Core
{
    class App;
}

namespace Scenes
{
    class ObjectScript;
    class PyScript;

    enum class SceneNodeTypes
    {
        Empty,
        Model,
        InstancedModel,
        Skybox,
        Camera,
        DirLight,
        PointLight,
        SpotLight,
        Primitive,
    };

    class SceneNode
    {
    protected:
        bool isInCollision = false;
        static const Resources::ShaderProgram* defaultShaderProgram;
        static const Resources::Material*      defaultMaterial;
        static const Resources::Material*      colliderMaterial;
        static const Resources::Material*      boundingBoxMaterial;
        void HandleScriptCollision();

    public:
        // Node data.
        size_t         id     = 0;
        size_t         depth  = 0;
        std::string    name   = "";
        SceneNodeTypes type   = SceneNodeTypes::Empty;
        std::vector<ObjectScript*>       scripts;
        std::vector<Physics::Primitive*> colliders;
        Physics::Rigidbody*              rigidbody = nullptr;
        Maths::Transform                 transform;

        // Graph data.
        SceneNode*              parent   = nullptr;
        std::vector<SceneNode*> children = {};

        SceneNode(const size_t& _id, const std::string& _name, SceneNode* _parent = nullptr, const SceneNodeTypes& _type = SceneNodeTypes::Empty);
        ~SceneNode();
        static void SetDefaultRenderValues(const Resources::ShaderProgram* shaderProgram, Resources::Material* defaultMat, Resources::Material* colliderMat, Resources::Material* boundingBoxMat);

        void StartPlayMode();
        void HandlePhysics(std::vector<Physics::Primitive*>& sceneColliders, const bool& dontUpdateScripts = false);
        void UpdateAndDrawChildren(const Render::Camera& camera, const Render::LightManager& lightManager, std::vector<Physics::Primitive*> sceneColliders, const bool& dontUpdateScripts = false, const bool& doPhysics = true);
        void DrawColliders(const Render::Camera& camera);
        void DrawBoundingSpheres(const Render::Camera& camera);

        void       RemoveChild     (const size_t& childId);
        bool       MoveNode        (SceneNode* newParent);
        SceneNode* FindInChildrenId(const size_t&       searchId,  const bool& isRootOfSearch = true);
        SceneNode* FindInChildren  (const std::string& searchName, const bool& isRootOfSearch = true);

        void AddRigidbody(const bool& _isKinematic = false);
        void UpdateDepth (const bool& updateChildrenDepth = true);
        void AddScript   (ObjectScript* script, Core::App* app);
        void AddPyScript (const std::string& filename, Core::App* app);
        void ReloadPyScripts();
        ObjectScript* GetCppScript(const size_t& id);
        PyScript*     GetPyScript (const size_t& id);

        void ShowGraphUi(SceneNode*& selectedNode, const bool& showChildrenUi = false);
        virtual void ShowInspectorUi();
    };

    class SceneModel : public SceneNode
    {
    public:
        Resources::Mesh* meshGroup  = nullptr;

        SceneModel(const size_t& _id, const std::string& _name, Resources::Mesh* _meshGroup, SceneNode* _parent = nullptr);
        void Draw(const Render::Camera& camera, const Render::LightManager& lightManager);
        void ShowInspectorUi() override;
    };

    class SceneInstancedModel : public SceneNode
    {
    private:
        unsigned int matrixBufferId = 0;

    public:
        size_t instanceCount;
        std::vector<Transform> instanceTransforms;
        Resources::Mesh* meshGroup = nullptr;

        bool wasLoaded = false;

        SceneInstancedModel(const size_t& _id, const std::string& _name, Resources::Mesh* _meshGroup, const int& _instanceCount, SceneNode* _parent = nullptr);
        void Setup();
        void UpdateMatrixBuffer();
        void Draw(const Render::Camera& camera, const Render::LightManager& lightManager);
        void ShowInspectorUi() override;
        void Shuffle();
    };

    class SceneSkybox : public SceneNode
    {
    private:
        Resources::Mesh* skyboxMesh = nullptr;

    public:
        Resources::Cubemap* cubemap = nullptr;

        SceneSkybox(const size_t& _id, const std::string& _name, Resources::Cubemap* _cubemap, SceneNode* _parent = nullptr);
        void Draw(const Render::Camera& camera);
        void ShowInspectorUi() override;
    };

    class SceneCamera : public SceneNode
    {
    public:
        Render::Camera* camera = nullptr;
        SceneCamera(const size_t& _id, const std::string& _name, Render::Camera* _camera, SceneNode* _parent = nullptr);
        void ShowInspectorUi() override;
    };

    class SceneDirLight : public SceneNode
    {
    public:
        Render::DirLight* light = nullptr;
        SceneDirLight(const size_t& _id, const std::string& _name, Render::DirLight* _light, SceneNode* _parent = nullptr);
        void ShowInspectorUi() override;
    };

    class ScenePointLight : public SceneNode
    {
    public:
        Render::PointLight* light = nullptr;
        ScenePointLight(const size_t& _id, const std::string& _name, Render::PointLight* _light, SceneNode* _parent = nullptr);
        void ShowInspectorUi() override;
    };

    class SceneSpotLight : public SceneNode
    {
    public:
        Render::SpotLight* light = nullptr;
        SceneSpotLight(const size_t& _id, const std::string& _name, Render::SpotLight* _light, SceneNode* _parent = nullptr);
        void ShowInspectorUi() override;
    };

    class ScenePrimitive : public SceneNode
    {
    public :
        Physics::Primitive* primitive = nullptr;
        ScenePrimitive(const size_t& _id, const std::string& _name, Physics::Primitive* _primitive, SceneNode* _parent = nullptr);
        ~ScenePrimitive();
        
        void Draw(const Render::Camera& camera, const Render::LightManager& lightManager);
        void ShowInspectorUi() override;
    };
}