#include <imgui/imgui.h>
#include <pybind11/pybind11.h>

#include "Ui.h"
#include "Mesh.h"
#include "SubMesh.h"
#include "Cubemap.h"
#include "Material.h"
#include "Camera.h"
#include "LightManager.h"
#include "SceneNode.h"
#include "PyScript.h"
#include "ResourceManager.h"
#include "App.h"
#include <iostream>
using namespace Scenes;
using namespace Render;
using namespace Resources;
using namespace Core::Physics;


void DrawMesh(const ShaderProgram* shaderProgram, const GLuint& vao, const int& vertexCount, const Mat4& worldMat, const Camera& camera, const Material* material, const LightManager* lightManager = nullptr)
{
    const unsigned int shaderProgramId = shaderProgram->GetId();
    if (shaderProgramId == 0 || vao == 0)
        return;

    glUseProgram(shaderProgramId);

    // Send matrices to shader.
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "mvpMatrix"), 1, GL_FALSE, (worldMat * camera.GetViewMat() * camera.GetProjectionMat()).ptr);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "modelMat" ), 1, GL_FALSE,  worldMat.ptr);

    // Tell the shader to deal with the mesh as a single element
    glUniform1i(glGetUniformLocation(shaderProgramId, "instanced"), false);

    // Send the camera position to shader.
    glUniform3f(glGetUniformLocation(shaderProgramId, "ViewPos"), -camera.transform->GetPosition().x, camera.transform->GetPosition().y, camera.transform->GetPosition().z);

    // Send material to shader.
    if (material != nullptr)
        material->SendDataToShader(shaderProgramId, ResourceManager::GetSampler()->GetId());

    // Send lights to shader.
    if (lightManager != nullptr)
        lightManager->SendLightsToShader(shaderProgramId);

    // Draw the mesh.
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void DrawInstancedMesh(const ShaderProgram* shaderProgram, const GLuint& vao, const int& vertexCount, const int& instanceCount, const Mat4& worldMat, const Camera& camera, const Material* material, const LightManager* lightManager = nullptr)
{
    const unsigned int shaderProgramId = shaderProgram->GetId();
    if (shaderProgramId == 0 || vao == 0)
        return;

    glUseProgram(shaderProgramId);

    // Send matrices to shader.
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "mvpMatrix"), 1, GL_FALSE, (worldMat * camera.GetViewMat() * camera.GetProjectionMat()).ptr);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "modelMat"), 1, GL_FALSE, worldMat.ptr);

    // Send the camera position to shader.
    glUniform3f(glGetUniformLocation(shaderProgramId, "ViewPos"), -camera.transform->GetPosition().x, camera.transform->GetPosition().y, camera.transform->GetPosition().z);

    // Send material to shader.
    if (material != nullptr)
        material->SendDataToShader(shaderProgramId, ResourceManager::GetSampler()->GetId());

    // Send lights to shader.
    if (lightManager != nullptr)
        lightManager->SendLightsToShader(shaderProgramId);

    // Draw the mesh.
    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0, instanceCount);
    glBindVertexArray(0);
}

// ----- Scene node ----- //

SceneNode::SceneNode(const size_t& _id, const std::string& _name, SceneNode* _parent, const SceneNodeTypes& _type)
{
    id      = _id;
    name    = _name;
    parent  = _parent;
    type    = _type;
    if (parent) {
        depth = parent->depth + 1;
        transform.parentMat =  _parent->transform.GetModelMat() * _parent->transform.parentMat;
    }
    else {
        depth = 0;
    }
}

SceneNode::~SceneNode()
{
    for (ObjectScript* script : scripts)
        delete script;
    scripts.clear();
    for (SceneNode* child : children)
        delete child;
    children.clear();
}

void SceneNode::SetDefaultRenderValues(const ShaderProgram* shaderProgram, Resources::Material* defaultMat, Resources::Material* colliderMat, Resources::Material* boundingBoxMat)
{
    defaultShaderProgram = shaderProgram;
    defaultMaterial      = defaultMat;
    colliderMaterial     = colliderMat;
    boundingBoxMaterial  = boundingBoxMat;
}

void SceneNode::StartPlayMode()
{
    for (ObjectScript* script : scripts) {
        script->Start();
    }
    for (SceneNode* child : children) {
        child->StartPlayMode();
    }
    for (ObjectScript* script : scripts) {
        script->LateStart();
        script->SetStarted();
    }
}

void SceneNode::HandleScriptCollision()
{
    if (rigidbody->HasCollided() && isInCollision)
    {
        for (ObjectScript* script : scripts) {
            script->OnCollisionStay();
        } 
    }
    else if (rigidbody->HasCollided() && !isInCollision)
    {       
        for (ObjectScript* script : scripts) {
            script->OnCollisionEnter();
        }       
        isInCollision = true;
    }
    else if (!rigidbody->HasCollided() && isInCollision)
    {        
        for (ObjectScript* script : scripts) {
            script->OnCollisionExit();
        }       
        isInCollision = false;
    }
}

void SceneNode::HandlePhysics(std::vector<Physics::Primitive*>& sceneColliders, const bool& dontUpdateScripts)
{
    // Update Rigidbody.
    if (rigidbody)
        rigidbody->Update();

    // Update all of the colliders' parent transforms and bounding spheres.
    Mat4 childrenParentMat = transform.GetModelMat() * transform.parentMat;
    for (Primitive* collider : colliders) {
        collider->transform->parentMat = childrenParentMat;
        collider->boundingSphere.Update(collider, this);
    }

    // Resolve collisions.
    if (rigidbody)
    {
        rigidbody->ResolveCollisions(colliders, sceneColliders);
        HandleScriptCollision();
    }
}



void SceneNode::UpdateAndDrawChildren(const Camera& camera, const LightManager& lightManager, std::vector<Primitive*> sceneColliders, const bool& dontUpdateScripts, const bool& doPhysics)
{
    if (!dontUpdateScripts)
    {
        // Call every script's update function.
        for (ObjectScript* script : scripts) {
            if (!script->HasStarted())
                script->Start();
            script->Update();
        }

        if (doPhysics)
            HandlePhysics(sceneColliders, dontUpdateScripts);
    }

    // Update and draw all children.
    Mat4 childrenParentMat = transform.GetModelMat() * transform.parentMat;
    for (SceneNode* child : children) {
        child->transform.parentMat = childrenParentMat;
        // TODO: TEMP START
        if (child->type == SceneNodeTypes::InstancedModel) {
            SceneInstancedModel* instancedModel = (SceneInstancedModel*)child;
            if (instancedModel->meshGroup->WasSentToOpenGL() && !instancedModel->wasLoaded) 
            {
                instancedModel->Setup();
            }
        }
        // TEMP END
        child->UpdateAndDrawChildren(camera, lightManager, sceneColliders, dontUpdateScripts);
    }

    // Draw scene models.
    if (type == SceneNodeTypes::Model)
        ((SceneModel*)this)->Draw(camera, lightManager);
    else if (type == SceneNodeTypes::Skybox)
        ((SceneSkybox*)this)->Draw(camera);
    else if (type == SceneNodeTypes::InstancedModel)
        ((SceneInstancedModel*)this)->Draw(camera, lightManager);
    else if (type == SceneNodeTypes::Primitive)
        ((ScenePrimitive*)this)->Draw(camera, lightManager);

    // Draw colliders and bounding spheres.
    if (Ui::showColliders)
        DrawColliders(camera);

    // Call every script's late update.
    if (!dontUpdateScripts) {
        for (ObjectScript* script : scripts) {
            if (!script->HasStarted()) {
                script->Start();
                script->SetStarted();
            }
            script->LateUpdate();
        }
    }
}

void SceneNode::DrawColliders(const Render::Camera& camera)
{
    for (Primitive* collider : colliders)
    {
        if (!Ui::wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        collider->transform->parentMat = transform.GetModelMat() * transform.parentMat;
        DrawMesh(defaultShaderProgram, *PrimitiveBuffers::GetVAO(collider->type), PrimitiveBuffers::GetVerticeCount(collider->type), 
                 collider->transform->GetModelMat() * collider->transform->parentMat, camera, colliderMaterial);

        if (!Ui::wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void SceneNode::DrawBoundingSpheres(const Render::Camera& camera)
{
    for (Primitive* collider : colliders)
    {
        if (!Ui::wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if (!Ui::showColliders)
            collider->transform->parentMat = transform.GetModelMat() * transform.parentMat;

        Mat4 worldMatrix = GetScaleMatrix({ collider->boundingSphere.radius }) 
                         * GetTranslationMatrix(transform.GetPosition());
        DrawMesh(defaultShaderProgram, *PrimitiveBuffers::GetVAO(PrimitiveTypes::Sphere), PrimitiveBuffers::GetVerticeCount(PrimitiveTypes::Sphere), 
                 worldMatrix, camera, boundingBoxMaterial);

        if (!Ui::wireframeMode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void SceneNode::RemoveChild(const size_t& childId)
{
    for (int i = 0; i < children.size(); i++) {
        if (children[i]->id == childId) {
            children.erase(children.begin() + i);
            break;
        }
    }
}

bool SceneNode::MoveNode(SceneNode* newParent)
{
    if (depth >= newParent->depth)
    {
        parent->RemoveChild(id);
        parent = newParent;
        transform.parentMat = newParent->transform.GetModelMat() * newParent->transform.parentMat;
        UpdateDepth();
        newParent->children.push_back(this);
        return true;
    }
    return false;
}

SceneNode* SceneNode::FindInChildrenId(const size_t& searchId, const bool& isRootOfSearch)
{
    // Find in direct children.
    for (SceneNode* child : children)
        if (child->id == searchId)
            return child;

    // Find in children's children.
    for (SceneNode* child : children)  {
        SceneNode* foundChild = child->FindInChildrenId(searchId, false);
        if (foundChild != nullptr)
            return foundChild;
    }

    if (isRootOfSearch)
        DebugLogWarning("Unable to find node with id " + std::to_string(searchId) + " in children of " + name + ".");
    return nullptr;
}

SceneNode* SceneNode::FindInChildren(const std::string& searchName, const bool& isRootOfSearch)
{
    // Find in direct children.
    for (SceneNode* child : children)
        if (child->name == searchName)
            return child;

    // Find in children's children.
    for (SceneNode* child : children)  {
        SceneNode* foundChild = child->FindInChildren(searchName, false);
        if (foundChild != nullptr)
            return foundChild;
    }

    if (isRootOfSearch)
        DebugLogWarning("Unable to find a node called " + searchName + " in children of " + name + ".");
    return nullptr;
}

void SceneNode::AddRigidbody(const bool& isKinematic)
{
    if (rigidbody == nullptr)
    {
        Rigidbody* rbody = new Rigidbody(&transform , isKinematic);
        rigidbody = rbody;
    }
}

void SceneNode::UpdateDepth(const bool& updateChildrenDepth)
{
    depth = parent->depth + 1;
    for (SceneNode* child : children) {
        child->UpdateDepth();
    }
}

void SceneNode::AddScript(ObjectScript* script, App* app)
{
    script->object    = this;
    script->transform = &transform;
    script->rigidbody = rigidbody;
    script->inputs    = &app->inputs;
    script->time      = &app->time;
    script->app       = app;
    scripts.push_back(script);
}

void SceneNode::AddPyScript(const std::string& filename, Core::App* app)
{
    AddScript((ObjectScript*)new PyScript(filename), app);
}

void SceneNode::ReloadPyScripts()
{
    for (ObjectScript* script : scripts) {
        if (PyScript* pyScript = script->GetAsPyScript()) {
            pyScript->Load();
        }
    }
    for (SceneNode* child : children) {
        child->ReloadPyScripts();
    }
}

ObjectScript* SceneNode::GetCppScript(const size_t& id)
{
    if (id >= scripts.size()) 
        return nullptr; 
    return scripts[id];
}

PyScript* SceneNode::GetPyScript(const size_t& id)
{
    ObjectScript* cppScript = GetCppScript(id);
    if (!cppScript || !cppScript->IsPyScript())
        return nullptr;
    return cppScript->GetAsPyScript();
}

void SceneNode::ShowGraphUi(SceneNode*& selectedNode, const bool& showChildrenUi)
{
    // Check if the current selected node is this one.
    bool isSelected = selectedNode != nullptr && selectedNode->id == id;

    // Set the tree node's flags.
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (isSelected)
        nodeFlags |= ImGuiTreeNodeFlags_Selected;
    if (children.size() <= 0)
        nodeFlags |= ImGuiTreeNodeFlags_Leaf;

    // Show the tree node ui.
    bool treeNodeOpen = ImGui::TreeNodeEx((name + "##" + std::to_string(id)).c_str(), nodeFlags);
    bool breakProcess = false;

    // Enable dragging tree nodes.
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        void** addressOfThis = new void*(this);
        ImGui::SetDragDropPayload("SceneNode", addressOfThis, sizeof(addressOfThis));
        ImGui::Text("Move %s", name.c_str());
        ImGui::EndDragDropSource();
        delete addressOfThis;
    }
    
    // Check if the tree node was clicked.
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen() && id != 0)
    {
        selectedNode = (isSelected ? nullptr : this);
    }

    // Enable dropping tree nodes.
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneNode"))
        {
            Assert(payload->DataSize == sizeof(this), "Scene node drag/drop payload of wrong size.");
            SceneNode* droppedNode = *(SceneNode**)payload->Data;
            if (droppedNode->MoveNode(this))
                breakProcess = true;
        }
        ImGui::EndDragDropTarget();
    }

    // Draw children Ui.
    if (treeNodeOpen)
    {
        if (!breakProcess)
        {
            ImGui::Unindent(5);
            for (SceneNode* child : children)
                child->ShowGraphUi(selectedNode, showChildrenUi);
            ImGui::Indent(5);
        }
        ImGui::TreePop();
    }
}

void SceneNode::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);
    Ui::ShowTransformUi(transform);
    Ui::ShowPhysicsUi(this);

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}


// ----- Scene model ----- //

const ShaderProgram* SceneNode::defaultShaderProgram = 0;
const Material*      SceneNode::defaultMaterial      = nullptr;
const Material*      SceneNode::colliderMaterial     = nullptr;
const Material*      SceneNode::boundingBoxMaterial  = nullptr;

SceneModel::SceneModel(const size_t& _id, const std::string& _name, Mesh* _meshGroup, SceneNode* _parent)
           : SceneNode(_id, _name, _parent, SceneNodeTypes::Model)
{
    meshGroup = _meshGroup;
}

void SceneModel::Draw(const Camera& camera, const LightManager& lightManager)
{
    if (meshGroup != nullptr)
    {
        Mat4 worldMat = transform.GetModelMat() * transform.parentMat;
        for (size_t i = 0; i < meshGroup->subMeshes.size(); i++)
        {
            if (meshGroup->subMeshes[i]->WasSentToOpenGL())
            {
                const ShaderProgram* shaderProgram = meshGroup->subMeshes[i]->GetShaderProgram();
                const Material*      material      = meshGroup->subMeshes[i]->GetMaterial();
                if (!shaderProgram)  shaderProgram = defaultShaderProgram;
                if (!material)       material      = defaultMaterial;
                DrawMesh(shaderProgram, meshGroup->subMeshes[i]->VAO, meshGroup->subMeshes[i]->GetVertexCount(),
                         worldMat, camera, material, &lightManager);
            }
        }
    }
}

// Returns false if the scene node gets deleted.
void SceneModel::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);
    Ui::ShowTransformUi(transform);

    // Information on meshes.
    if (ImGui::TreeNode("Object sub-meshes"))
    {
        ImGui::Unindent(5);
        for (SubMesh* subMesh : meshGroup->subMeshes) 
        {
            ImGui::Bullet();
            ImGui::TextWrapped((subMesh->GetName() + " (" + std::to_string(subMesh->GetVertexCount()) + " vertices)").c_str());
        }
        ImGui::Indent(5);
        ImGui::TreePop();
    }

    // Information on materials.
    if (ImGui::TreeNode("Object materials"))
    {
        for (SubMesh* subMesh : meshGroup->subMeshes) 
        {
            Material* material = subMesh->GetMaterial();
            if (material != nullptr && ImGui::TreeNode(material->GetName().c_str()))
            {
                ImGui::Unindent(5);
                Core::Ui::ShowMaterialUi(material);
                ImGui::Indent(5);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    Ui::ShowPhysicsUi(this);

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}

// ----- Scene instancied model ----- //

SceneInstancedModel::SceneInstancedModel(const size_t& _id, const std::string& _name, Mesh* _meshGroup, const int& _instanceCount, SceneNode* _parent)
    : SceneNode(_id, _name, _parent, SceneNodeTypes::InstancedModel)
{
    instanceCount = _instanceCount;
    meshGroup = _meshGroup;

    instanceTransforms.resize(instanceCount, Transform());
    Shuffle();
}

void SceneInstancedModel::Setup()
{
    // Create matrix buffer.
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    matrixBufferId = buffer;
    UpdateMatrixBuffer();

    // Get the instanced mesh shader.
    ShaderProgram* instanceShader = Ui::app->resourceManager.Get<ShaderProgram>("MeshInstancedShaderProgram");

    // Update attribute array pointers.
    for (unsigned int i = 0; i < meshGroup->subMeshes.size(); i++)
    {
        meshGroup->subMeshes[i]->SetShaderProgram(instanceShader);

        DebugLog("Filling vertex shader");

        glBindVertexArray(meshGroup->subMeshes[i]->VAO);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4), (void*)(0*sizeof(Vector4)));
        glVertexAttribDivisor(5, 1);

        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4), (void*)(1*sizeof(Vector4)));
        glVertexAttribDivisor(6, 1);

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4), (void*)(2*sizeof(Vector4)));
        glVertexAttribDivisor(7, 1);

        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4), (void*)(3*sizeof(Vector4)));
        glVertexAttribDivisor(8, 1);

        glBindVertexArray(0);

        wasLoaded = true;
    }
}

void SceneInstancedModel::UpdateMatrixBuffer()
{
    if (matrixBufferId == 0)
        return;

    std::vector<Mat4> instanceMatrices; instanceMatrices.reserve(instanceCount);
    for (Transform& t : instanceTransforms)
        instanceMatrices.push_back(t.GetModelMat());
    glBindBuffer(GL_ARRAY_BUFFER, matrixBufferId);
    glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(Mat4), instanceMatrices.data(), GL_STATIC_DRAW);
}

void SceneInstancedModel::Draw(const Camera& camera, const LightManager& lightManager)
{
    if (meshGroup != nullptr)
    {
        Mat4 worldMat = transform.GetModelMat() * transform.parentMat;
        for (size_t i = 0; i < meshGroup->subMeshes.size(); i++)
        {
            if (meshGroup->subMeshes[i]->WasSentToOpenGL())
            {
                const ShaderProgram* shaderProgram = meshGroup->subMeshes[i]->GetShaderProgram();
                const Material* material = meshGroup->subMeshes[i]->GetMaterial();
                if (!shaderProgram)  shaderProgram = defaultShaderProgram;
                if (!material)       material = defaultMaterial;

                DrawInstancedMesh(shaderProgram, meshGroup->subMeshes[i]->VAO, meshGroup->subMeshes[i]->GetVertexCount(), (int)instanceTransforms.size(), worldMat, camera, material, &lightManager);
            }
        }
    }
}

// Returns false if the scene node gets deleted.
void SceneInstancedModel::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);
    Ui::ShowTransformUi(transform);

    // Information on meshes.
    if (ImGui::TreeNode("Object sub-meshes"))
    {
        ImGui::Unindent(5);
        for (SubMesh* subMesh : meshGroup->subMeshes)
        {
            ImGui::Bullet();
            ImGui::TextWrapped((subMesh->GetName() + " (" + std::to_string(subMesh->GetVertexCount()) + " vertices)").c_str());
        }
        ImGui::Indent(5);
        ImGui::TreePop();
    }

    // Information on instance transforms.
    if (ImGui::TreeNode("Instance transforms"))
    {
        ImGui::Unindent(5);
        if (ImGui::TreeNode("All instances"))
        {
            // Inputs for position.
            static Vector3 globalPos;
            ImGui::PushItemWidth(150);
            ImGui::DragFloat3("Position", &globalPos.x, 0.1f);
            ImGui::SameLine();
            if (ImGui::Button("Set for all##GlobalPos")) {
                for (Transform& t : instanceTransforms)
                    t.SetPosition(globalPos);
                UpdateMatrixBuffer();
            }

            // Inputs for rotation.
            static Vector3 globalRot;
            globalRot.x = radToDeg(globalRot.x); globalRot.y = radToDeg(globalRot.y); globalRot.z = radToDeg(globalRot.z);
            ImGui::PushItemWidth(150);
            ImGui::DragFloat3("Rotation", &globalRot.x, 5);
            globalRot.x = degToRad(globalRot.x); globalRot.y = degToRad(globalRot.y); globalRot.z = degToRad(globalRot.z);
            ImGui::SameLine();
            if (ImGui::Button("Set for all##GlobalRot")) {
                for (Transform& t : instanceTransforms)
                    t.SetRotation(globalRot);
                UpdateMatrixBuffer();
            }

            // Inputs for scale.
            static Vector3 globalScale = {1};
            ImGui::PushItemWidth(150);
            ImGui::DragFloat3("Scale   ", &globalScale.x, 0.05f);
            ImGui::SameLine();
            if (ImGui::Button("Set for all##GlobalScale")) {
                for (Transform& t : instanceTransforms)
                    t.SetScale(globalScale);
                UpdateMatrixBuffer();
            }

            ImGui::TreePop();
        }
        for (int i = 0; i < instanceCount; i++)
        {
            if (ImGui::TreeNode(("Instance " + std::to_string(i)).c_str()))
            {
                if (Ui::ShowTransformUi(instanceTransforms[i]))
                    UpdateMatrixBuffer();
                ImGui::TreePop();
            }
        }
        ImGui::Indent(5);
        ImGui::TreePop();
    }

    // Information on materials.
    if (ImGui::TreeNode("Object materials"))
    {
        for (SubMesh* subMesh : meshGroup->subMeshes)
        {
            Material* material = subMesh->GetMaterial();
            if (material != nullptr && ImGui::TreeNode(material->GetName().c_str()))
            {
                ImGui::Unindent(5);
                Core::Ui::ShowMaterialUi(material);
                ImGui::Indent(5);
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    Ui::ShowPhysicsUi(this);

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* ptrToThis = this;
    Ui::ShowRemoveNodeUi(ptrToThis);
}

void Scenes::SceneInstancedModel::Shuffle()
{
    for (Transform& t : instanceTransforms) {
        t.SetPosition(Vector3(0, (float)(rand() % 4 - 2), 0) + Vector3(Vector3(0, degToRad((float)(rand() % 360)), 0), (float)(rand() % 20 + 25)));
        t.SetRotation(Vector3(((float)(rand() % 5)) / 100.f));
        t.SetScale((Vector3((float)(rand() % 300), (float)(rand() % 300), (float)(rand() % 300)) + (float)(rand() % 500) + 200) / 10000.f);
    }
}

// ----- Scene Skybox ----- //

SceneSkybox::SceneSkybox(const size_t& _id, const std::string& _name, Resources::Cubemap* _cubemap, SceneNode* _parent)
            : SceneNode(_id, _name, _parent, SceneNodeTypes::Skybox)
{
    cubemap = _cubemap;
    transform.SetScale(500);
    skyboxMesh = Ui::app->resourceManager.Get<Mesh>("Cubemap"); // TODO: using static... oops
}

void SceneSkybox::Draw(const Render::Camera& camera)
{
    if (!skyboxMesh || skyboxMesh->subMeshes.size() <= 0)
        return;

    const SubMesh* skyboxSubMesh = skyboxMesh->subMeshes[0];
    const unsigned int shaderProgramId = skyboxSubMesh->GetShaderProgram()->GetId();
    if (shaderProgramId == 0 || skyboxSubMesh->VAO == 0)
        return;

    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glUseProgram(shaderProgramId);

    // Send matrices to shader.
    glUniformMatrix4fv(glGetUniformLocation(shaderProgramId, "viewProjMat"), 1, GL_FALSE, (transform.GetModelMat() * GetRotationMatrix(camera.transform->GetRotation(), true) * camera.GetProjectionMat()).ptr);

    // Draw the mesh.
    glBindVertexArray(skyboxSubMesh->VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->GetId());
    glDrawElements(GL_TRIANGLES, skyboxSubMesh->GetVertexCount(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glCullFace(GL_BACK);
}

// Returns false if the scene node gets deleted.
void SceneSkybox::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);
    Ui::ShowTransformUi(transform, true);

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}


// ----- Scene camera ----- //

SceneCamera::SceneCamera(const size_t& _id, const std::string& _name, Render::Camera* _camera, SceneNode* _parent)
           : SceneNode(_id, _name, _parent, SceneNodeTypes::Camera)
{
    camera = _camera;
    camera->transform = &transform;
    transform.isCamera = true;
}

// Returns false if the scene node gets deleted.
void SceneCamera::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);
    Ui::ShowTransformUi(transform);

    // KeyBindings floats for camera parameters.
    CameraParams params = camera->GetParameters();
    float fov = params.fov, near = params.near, far = params.far;
    ImGui::DragFloat("Field of view", &fov,  0.2f);
    ImGui::DragFloat("Near",          &near, 0.1f);
    ImGui::DragFloat("Far",           &far,  0.1f);
    if (params.fov != fov || params.near != near || params.far != far)
    {
        params.fov = fov; params.near = near; params.far = far;
        camera->ChangeParameters(params);
    }

    // Set as scene camera button.
    if (Ui::app->cameraManager.sceneCamera != camera)
    {
        if (ImGui::Button("Use this camera to render the scene"))
        {
            Ui::app->cameraManager.sceneCamera = camera;
        }
    }
    else
    {
        ImGui::Text("This camera renders the scene");
    }

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}


// ----- Scene directional light ----- //

SceneDirLight::SceneDirLight(const size_t& _id, const std::string& _name, Render::DirLight* _light, SceneNode* _parent)
             : SceneNode(_id, _name, _parent, SceneNodeTypes::DirLight)
{
    light = _light;
}

// Returns false if the scene node gets deleted.
void SceneDirLight::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);

    // KeyBindings floats for light parameters.
    ImGui::DragFloat3("Direction", &light->dir.x, 0.02f);
    ImGui::NewLine();
    ImGui::Text("Colors");
    ImGui::ColorEdit3("Ambient",   light->ambient.ptr());
    ImGui::ColorEdit3("Diffuse",   light->diffuse.ptr());
    ImGui::ColorEdit3("Specular",  light->specular.ptr());

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}


// ----- Scene point light ----- //

ScenePointLight::ScenePointLight(const size_t& _id, const std::string& _name, Render::PointLight* _light, SceneNode* _parent)
             : SceneNode(_id, _name, _parent, SceneNodeTypes::PointLight)
{
    light = _light;
}

// Returns false if the scene node gets deleted.
void ScenePointLight::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);

    // KeyBindings floats for light parameters.
    ImGui::DragFloat3("Position",  &light->pos.x, 0.1f);
    ImGui::NewLine();
    ImGui::Text("Colors");
    ImGui::ColorEdit3("Ambient",    light->ambient.ptr());
    ImGui::ColorEdit3("Diffuse",    light->diffuse.ptr());
    ImGui::ColorEdit3("Specular",   light->specular.ptr());
    ImGui::NewLine();
    ImGui::Text("Attenuation");
    ImGui::DragFloat ("Constant",  &light->constant, 0.05f);
    ImGui::DragFloat ("Linear",    &light->linear,   0.05f);
    ImGui::DragFloat ("Quadratic", &light->quadratic, 0.05f);

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}


// ----- Scene spot light ----- //

SceneSpotLight::SceneSpotLight(const size_t& _id, const std::string& _name, Render::SpotLight* _light, SceneNode* _parent)
             : SceneNode(_id, _name, _parent, SceneNodeTypes::SpotLight)
{
    light = _light;
}

// Returns false if the scene node gets deleted.
void SceneSpotLight::ShowInspectorUi()
{
    Ui::ShowNodeNameUi(name, id, type);

    // KeyBindings floats for light parameters.
    ImGui::DragFloat3 ("Position",  &light->pos.x, 0.1f);
    ImGui::DragFloat3 ("Direction", &light->dir.x, 0.02f);
    ImGui::SliderAngle("Outer Cone", &light->outerCone, 0, 180);
    ImGui::SliderAngle("Inner Cone", &light->innerCone, 0, 180);
    ImGui::NewLine();
    ImGui::Text("Colors");
    ImGui::ColorEdit3("Ambient",    light->ambient.ptr());
    ImGui::ColorEdit3("Diffuse",    light->diffuse.ptr());
    ImGui::ColorEdit3("Specular",   light->specular.ptr());
    ImGui::NewLine();

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}

// ----- Scene primitive ----- //

ScenePrimitive::ScenePrimitive(const size_t& _id, const std::string& _name, Primitive* _primitive, SceneNode* _parent)
              : SceneNode(_id, _name, _parent, SceneNodeTypes::Primitive)
{
    primitive = _primitive;
    primitive->transform = &transform;
    primitive->boundingSphere.Update(primitive, this);
}

ScenePrimitive::~ScenePrimitive()
{
    delete primitive;
}

void ScenePrimitive::Draw(const Camera& camera, const LightManager& lightManager)
{
    if (primitive != nullptr)
    {
        const ShaderProgram*  shaderProgram = primitive->GetShaderProgram();
        const Material*       material      = primitive->GetMaterial();
        if (!shaderProgram)   shaderProgram = defaultShaderProgram;
        if (!material)        material      = defaultMaterial;
        DrawMesh(shaderProgram, *PrimitiveBuffers::GetVAO(primitive->type), PrimitiveBuffers::GetVerticeCount(primitive->type),
                 transform.GetModelMat() * transform.parentMat, camera, material, &lightManager);
    }
}

void ScenePrimitive::ShowInspectorUi()
{
    bool uniformTransform = primitive->type == PrimitiveTypes::Sphere ||
                            primitive->type == PrimitiveTypes::Capsule;

    Ui::ShowNodeNameUi(name, id, type);
    Ui::ShowTransformUi(transform, uniformTransform);

    // Information on materials.
    if (ImGui::TreeNode("Object material"))
    {

        Material* material = primitive->GetMaterial();
        if (material != nullptr && ImGui::TreeNode(material->GetName().c_str()))
        {
            ImGui::Unindent(5);
            Core::Ui::ShowMaterialUi(material);
            ImGui::Indent(5);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }

    // Show scripts Ui.
    for (ObjectScript* script : scripts) {
        script->ShowInspectorUi();
    }

    // Remove button.
    SceneNode* refToThis = this;
    Ui::ShowRemoveNodeUi(refToThis);
}
