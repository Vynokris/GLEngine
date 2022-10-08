#include <imgui/imgui.h>
#include <algorithm>

#include "App.h"
#include "Mesh.h"
#include "SceneNode.h"
#include "SceneGraph.h"
using namespace Scenes;
using namespace Core::Physics;
using namespace Core::Maths;


SceneGraph::~SceneGraph()
{
    for (Physics::Primitive* collider : sceneColliders)
        delete collider;
    sceneColliders.clear();

    if (root) 
        delete root;
    root = nullptr;
}

size_t SceneGraph::FindFirstFreeId()
{
    // Find the first free node ID.
    size_t freeId = assignedNodeIds.size();
	for (int i = 1; i < assignedNodeIds.size(); i++) {
		if (assignedNodeIds[i] - 1 != assignedNodeIds[i - 1]) {
			freeId = i - 1;
			break;
		}
	}
    return freeId + 1;
}

SceneNode* SceneGraph::AddEmpty(const std::string& name, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

	// Add the new id to the assigned node ids and sort the vector.
	assignedNodeIds.push_back(freeId);
	std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Create a new node and add it to the parent's children.
    SceneNode* newNode = new SceneNode(freeId, name, parent);
    parent->children.push_back(newNode);
    return newNode;
}

SceneModel* SceneGraph::AddModel(const std::string& name, Resources::Mesh* meshGroup, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

	// Add the new id to the assigned node ids and sort the vector.
	assignedNodeIds.push_back(freeId);
	std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Increment the total vertex count.
    // TODO: Make this work with multithreaded loading.
    // if (meshGroup->IsLoaded())
    //     for (int i = 0; i < meshGroup->subMeshes.size(); i++)
    //         totalVertexCount += meshGroup->subMeshes[i]->GetVertexCount();

    // Create a new node and add it to the parent's children.
    SceneModel* newNode = new SceneModel(freeId, name, meshGroup, parent);
    parent->children.push_back(newNode);
    return newNode;
}

SceneCamera* SceneGraph::AddCamera(const std::string& name, Render::Camera* camera, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

	// Add the new id to the assigned node ids and sort the vector.
	assignedNodeIds.push_back(freeId);
	std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Create a new node and add it to the parent's children.
    SceneCamera* newNode = new SceneCamera(freeId, name, camera, parent);
    parent->children.push_back(newNode);
    return newNode;
}

SceneDirLight* SceneGraph::AddDirLight(const std::string& name, Render::DirLight* light, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

	// Add the new id to the assigned node ids and sort the vector.
	assignedNodeIds.push_back(freeId);
	std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Create a new node and add it to the parent's children.
    SceneDirLight* newNode = new SceneDirLight(freeId, name, light, parent);
    parent->children.push_back(newNode);
    return newNode;
}

ScenePointLight* SceneGraph::AddPointLight(const std::string& name, Render::PointLight* light, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

	// Add the new id to the assigned node ids and sort the vector.
	assignedNodeIds.push_back(freeId);
	std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Create a new node and add it to the parent's children.
    ScenePointLight* newNode = new ScenePointLight(freeId, name, light, parent);
    parent->children.push_back(newNode);
    return newNode;
}

SceneSpotLight* SceneGraph::AddSpotLight(const std::string& name, Render::SpotLight* light, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

	// Add the new id to the assigned node ids and sort the vector.
	assignedNodeIds.push_back(freeId);
	std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Create a new node and add it to the parent's children.
    SceneSpotLight* newNode = new SceneSpotLight(freeId, name, light, parent);
    parent->children.push_back(newNode);
    return newNode;
}

ScenePrimitive* SceneGraph::AddPrimitive(const std::string& name, Physics::Primitive* primitive, SceneNode* parent)
{
    // Set the parent if it is null.
    if (parent == nullptr)
        parent = root;
    size_t freeId = FindFirstFreeId();

    // Add the new id to the assigned node ids and sort the vector.
    assignedNodeIds.push_back(freeId);
    std::sort(assignedNodeIds.begin(), assignedNodeIds.end());

    // Create a new node and add it to the parent's children.
    ScenePrimitive* newNode = new ScenePrimitive(freeId, name, primitive, parent);
    parent->children.push_back(newNode);

    return newNode;
}

Physics::Primitive* SceneGraph::AddCollider(SceneNode* node, const PrimitiveTypes& type, const Vector3& pos, const Vector3& rot, const Vector3& scale)
{
    // Create a new primitive with allocated transform.
    Primitive* collider = new Primitive(type, true);

    // Set the collider's position, rotation and scale, and update its bounding sphere.
    collider->transform->SetPosition(pos);
    collider->transform->SetRotation(rot);
    if (type == PrimitiveTypes::Cube)
        collider->transform->SetScale(scale);
    else
        collider->transform->SetScale({ std::max(scale.x, std::max(scale.y, scale.z)) });
    collider->boundingSphere.Update(collider, node);

    // Set the parent of the collider's transform to the given node's transform.
    collider->transform->parentMat = node->transform.GetModelMat() * node->transform.parentMat;
    
    // Add the colider to the node and the scene's collider lists.
    node->colliders.push_back(collider);
    sceneColliders.push_back(collider);

    return collider;
}

Physics::Primitive* SceneGraph::AddCollider(SceneNode* node, const Physics::PrimitiveTypes& type, const std::vector<Core::Maths::TangentVertex>& vertices)
{
    if (vertices.size() <= 0)
        return nullptr;

    Vector3 min = Vector3(FLT_MAX), max = Vector3(FLT_MIN);
    for (const TangentVertex& vert : vertices)
    {
        if (vert.pos.x < min.x) min.x = vert.pos.x;
        if (vert.pos.y < min.y) min.y = vert.pos.y;
        if (vert.pos.z < min.z) min.z = vert.pos.z;
        if (vert.pos.x > max.x) max.x = vert.pos.x;
        if (vert.pos.y > max.y) max.y = vert.pos.y;
        if (vert.pos.z > max.z) max.z = vert.pos.z;
    }

    Vector3 center = (min + max) / 2;
    Vector3 scale  = Vector3(center, max) * 2;
    center.x *= -1;
    return AddCollider(node, type, center, {}, scale);
}

Physics::Primitive* SceneGraph::AddCollider(SceneNode* node, const Physics::PrimitiveTypes& type, const std::vector<Core::Maths::Vector3>& vertices)
{
    if (vertices.size() <= 0)
        return nullptr;

    Vector3 min = Vector3(FLT_MAX), max = Vector3(FLT_MIN);
    for (const Vector3& vert : vertices)
    {
        if (vert.x < min.x) min.x = vert.x;
        if (vert.y < min.y) min.y = vert.y;
        if (vert.z < min.z) min.z = vert.z;
        if (vert.x > max.x) max.x = vert.x;
        if (vert.y > max.y) max.y = vert.y;
        if (vert.z > max.z) max.z = vert.z;
    }

    Vector3 center = (min + max) / 2;
    Vector3 scale  = max - center;
    return AddCollider(node, type, center, {}, scale);
}

void SceneGraph::StartPlayMode()
{
    root->StartPlayMode();
}

void SceneGraph::UpdateAndDrawAll(const Render::Camera& camera, const Render::LightManager& lightManager, const bool& dontUpdateScripts)
{
    static bool shouldDoPhysics = true;
    root->UpdateAndDrawChildren(camera, lightManager, sceneColliders ,dontUpdateScripts , shouldDoPhysics);
    shouldDoPhysics = !shouldDoPhysics;

}

void SceneGraph::ClearAll()
{
    for (SceneNode* node : root->children)
    {
        for (Physics::Primitive* collider : node->colliders)
            delete collider;
        node->colliders.clear();
        delete node;
    }
    sceneColliders.clear();
    root->children.clear();
    assignedNodeIds.clear();
    totalVertexCount = 0;
    selectedNode = nullptr;
}

SceneNode* SceneGraph::FindId(const size_t& searchId)
{
    SceneNode* node = root->FindInChildrenId(searchId);
    if (node == nullptr)
        DebugLogWarning("No node with id " + std::to_string(searchId) + ".");
    return node;
}

 SceneNode* SceneGraph::Find(const std::string& searchName)
 {
    SceneNode* node = root->FindInChildren(searchName);
    if (node == nullptr)
        DebugLogWarning("No node called " + searchName + ".");
    return node;
 }

void SceneGraph::ShowUi()
{
    ImGui::SetNextItemOpen(1);
    root->ShowGraphUi(selectedNode, true);
}
