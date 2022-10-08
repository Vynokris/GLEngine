#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#pragma warning(disable : 4996)

#include "Maths.h"
#include "Debug.h"
#include "App.h"
#include "Ui.h"
#include "KeyBindings.h"
using namespace Core;
using namespace Core::Maths;
using namespace Resources;
using namespace Scenes;

ImVec2 Ui::windowPositions [7] = { { 460,  90  }, { 0,   0   }, { 1550, 0    }, { 1180, 0    }, { 0,    850 }, { 215, 0   }, { 915, 0  } };
ImVec2 Ui::windowSizes     [7] = { { 1000, 900 }, { 215, 850 }, { 370,  1080 }, { 370,  1080 }, { 1180, 230 }, { 175, 300 }, { 101, 58 } };
bool   Ui::windowsCollapsed[7] = { false, false, false, false, false, false };
int    Ui::windowWidth         = 1920;
int    Ui::windowHeight        = 1080;
App*   Ui::app                 = nullptr;
bool   Ui::wireframeMode       = false;
bool   Ui::showColliders       = false;

void Ui::SetAppPtr(App* _app)
{
    app = _app;
}

void Ui::FramebufferResizeCallback(const int& width, const int& height)
{
    if (width <= 0 && height <= 0)
        return;
    float widthF  = (float)width;
    float heightF = (float)height;

    // Set start menu pos and size.
    windowPositions[UiWindows::StartMenu] = ImVec2(widthF, heightF) * windowPositions[UiWindows::StartMenu] / ImVec2((float)windowWidth, (float)windowHeight);
    windowSizes    [UiWindows::StartMenu] = ImVec2(widthF, heightF) * windowSizes    [UiWindows::StartMenu] / ImVec2((float)windowWidth, (float)windowHeight);

    // Set scene graph size.
    windowSizes[UiWindows::SceneGraph] = { windowSizes[UiWindows::SceneGraph].x, heightF - windowSizes[UiWindows::Logs].y };
    if (windowsCollapsed[UiWindows::Logs])
        windowSizes[UiWindows::SceneGraph].y += windowSizes[UiWindows::Logs].y - (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2);

    // Set inspector pos and size.
    windowPositions[UiWindows::Inspector] = { widthF - windowSizes[UiWindows::Inspector].x, 0 };
    windowSizes    [UiWindows::Inspector] = { windowSizes[UiWindows::Inspector].x, heightF };

    // Set resources pos and size.
    windowPositions[UiWindows::Resources] = { widthF - windowSizes[UiWindows::Inspector].x - windowSizes[UiWindows::Resources].x, 0 };
    windowSizes    [UiWindows::Resources] = { windowSizes[UiWindows::Resources].x, heightF };

    // Set logs pos and size.
    windowPositions[UiWindows::Logs] = { 0, heightF - windowSizes[UiWindows::Logs].y };
    windowSizes    [UiWindows::Logs] = { widthF - windowSizes[UiWindows::Inspector].x - windowSizes[UiWindows::Resources].x, windowSizes[UiWindows::Logs].y };
    if (windowsCollapsed[UiWindows::Logs]) {
        windowPositions[UiWindows::Logs].y += windowSizes[UiWindows::Logs].y;
    }
    if (windowsCollapsed[UiWindows::Resources]) {
        windowSizes[UiWindows::Logs].x += windowSizes[UiWindows::Resources].x;
        if (windowsCollapsed[UiWindows::Inspector]) {
            windowSizes[UiWindows::Logs].x += windowSizes[UiWindows::Inspector].x;
        }
    }

    // Set stats pos and size.
    windowPositions[UiWindows::Stats] = { windowSizes[UiWindows::SceneGraph].x, 0 };
    windowSizes    [UiWindows::Stats] = { windowSizes[UiWindows::Stats].x, windowSizes[UiWindows::Stats].y };

    // Set play pos.
    windowPositions[UiWindows::Play] = { width / 2 - windowSizes[UiWindows::Play].x / 2, 0 };

    // Update app window size variables.
    windowWidth  = width;
    windowHeight = height;
    if (app != nullptr)
        app->SetWindowSize(width, height, false);
}

void Ui::CheckWindowSize(const int& windowIndex)
{
    ImVec2 windowSize    = ImGui::GetWindowSize();
    ImVec2 windowPos     = ImGui::GetWindowPos();
    bool windowCollapsed = ImGui::IsWindowCollapsed();

    if (windowCollapsed != windowsCollapsed[windowIndex])
    {
        windowsCollapsed[windowIndex] = windowCollapsed;
        FramebufferResizeCallback(windowWidth, windowHeight);
    }

    if (windowSize.x != windowSizes    [windowIndex].x || windowSize.y != windowSizes    [windowIndex].y ||
        windowPos .x != windowPositions[windowIndex].x || windowPos .y != windowPositions[windowIndex].y)
    {
        ImGui::SetWindowSize(windowSizes[windowIndex]);
        ImGui::SetWindowPos (windowPositions[windowIndex]);
    }

    if (app->isInBenchmark && windowIndex == UiWindows::Stats)
    {
        ImGui::SetWindowSize({ windowSizes[windowIndex].x, windowSizes[windowIndex].y - 60 });
    }
}

void Ui::ShowNodeNameUi(std::string& name, const SceneNodeTypes& type)
{
    ImGui::AlignTextToFramePadding();
    switch (type)
    {
    case SceneNodeTypes::Model:
        ImGui::Text("Selected model: ");
        break;
    case SceneNodeTypes::Camera:
        ImGui::Text("Selected camera: ");
        break;
    case SceneNodeTypes::DirLight:
        ImGui::Text("Selected directional light: ");
        break;
    case SceneNodeTypes::PointLight:
        ImGui::Text("Selected point light: ");
        break;
    case SceneNodeTypes::SpotLight:
        ImGui::Text("Selected spot light: ");
        break;
    default:
        ImGui::Text("Selected object: ");
        break;
    }
    ImGui::SameLine();
        
    // Allow text input to modify the object's name.
    static char inputBuffer[100];
    std::strcpy(inputBuffer, name.c_str());
    ImGui::InputText("##objectName", inputBuffer, 100 * sizeof(char));
    name = inputBuffer;
}

void Ui::ShowTransformUi(Maths::Transform& transform, bool isScaleUniform)
{
    // Inputs for position.
    Vector3 objectPos = transform.GetPosition();
    if (ImGui::DragFloat3("Position", &objectPos.x, 0.1f))
        transform.SetPosition(objectPos);

    // Inputs for rotation.
    Vector3 objectRot = transform.GetRotation();
    objectRot.x = radToDeg(objectRot.x); objectRot.y = radToDeg(objectRot.y); objectRot.z = radToDeg(objectRot.z);
    ImGui::DragFloat3("Rotation", &objectRot.x, 5);
    objectRot.x = degToRad(objectRot.x); objectRot.y = degToRad(objectRot.y); objectRot.z = degToRad(objectRot.z);
    if (objectRot != transform.GetRotation())
        transform.SetRotation(objectRot);

    // Inputs for scale.
    if (!transform.isCamera)
    {
        if (!isScaleUniform)
        {
            Vector3 objectScale = transform.GetScale();
            if (ImGui::DragFloat3("Scale", &objectScale.x, 0.05f))
                transform.SetScale(objectScale);
        }
        else
        {
            Vector3 objectScale = transform.GetScale();
            if (ImGui::DragFloat("Uniform scale", &objectScale.x, 0.05f))
            {
                objectScale.y = objectScale.x;
                objectScale.z = objectScale.x;
                transform.SetScale(objectScale);
            }
        }
    }
}

void Ui::ShowTextureUi(Texture*  texture, float size)
{
    Vector2 normalizedSize = Vector2((float)texture->GetWidth(), (float)texture->GetHeight()) / std::max(texture->GetWidth(), texture->GetHeight());
    ImGui::Image((ImTextureID)texture->GetId(), 
                 { normalizedSize.x * size, normalizedSize.y * size }, 
                 { 0, 1 }, { 1, 0 });
}

void Ui::ShowMaterialUi(Material* material)
{
    ImGui::PushItemWidth(225);
    ImGui::ColorEdit3("Ambient",  material->ambient.ptr());
    ImGui::ColorEdit3("Diffuse",  material->diffuse.ptr());
    ImGui::ColorEdit3("Specular", material->specular.ptr());
    ImGui::ColorEdit3("Emission", material->emission.ptr());
    ImGui::DragFloat("Shininess", &material->shininess, 1, 0, 100);
    ImGui::DragFloat("Transparency", &material->transparency, 0.01f, 0, 1);
    ImGui::PopItemWidth();

    Texture**    materialTextures[]    = { &material->ambientTexture, &material->diffuseTexture, &material->specularTexture, &material->emissionTexture, &material->shininessMap, &material->alphaMap, &material->normalMap };
    const char* materialTextureNames[] = { "Ambient texture", "Diffuse texture", "Specular texture", "Emission texture", "Shininess map", "Alpha map", "Normal map" };
    for (int i = 0; i < 7; i++)
    {
        if (*materialTextures[i] == nullptr) continue;
        if (ImGui::TreeNode(materialTextureNames[i]))
        {
            ImGui::Unindent(5);
            ImGui::TextWrapped((*materialTextures[i])->GetName().c_str());
            ShowTextureUi(*materialTextures[i], ImGui::GetWindowWidth() - 70);
            if (ImGui::BeginDragDropTarget())
            {
                // Enable dropping textures onto materials.
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ResourceTexture"))
                {
                    Assert(payload->DataSize == sizeof(materialTextures[i]), "Texture drag/drop payload of wrong size.");
                    Texture* droppedNode = *(Texture**)payload->Data;
                    *materialTextures[i] = droppedNode;
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::Indent(5);
            ImGui::TreePop();
        }
    }
}

void Ui::ShowRemoveNodeUi(SceneNode*& node)
{
    if (ImGui::Button("Remove from scene"))
    {
        // Lambda function that recursively deletes children of the node.
        static std::function<void(SceneNode*)> deleteChildren = [](SceneNode* sceneNode) -> 
        void {
            switch (sceneNode->type)
            {
            case SceneNodeTypes::DirLight:
                app->lightManager.Delete<DirLight>(((SceneDirLight*)sceneNode)->light->GetId());
                break;
            case SceneNodeTypes::PointLight:
                app->lightManager.Delete<PointLight>(((ScenePointLight*)sceneNode)->light->GetId());
                break;
            case SceneNodeTypes::SpotLight:
                app->lightManager.Delete<SpotLight>(((SceneSpotLight*)sceneNode)->light->GetId());
                break;
            case SceneNodeTypes::Model:
                for (SubMesh* subMesh : ((SceneModel*)sceneNode)->meshGroup->subMeshes)
                    app->sceneGraph.totalVertexCount -= subMesh->GetVertexCount();
                break;
            default:
                break;
            }
            for (SceneNode* child : sceneNode->children)
                deleteChildren(child);
        };

        // Remove node from scene graph.
        deleteChildren(node);
        node->parent->RemoveChild(node->id);
        app->sceneGraph.selectedNode = nullptr;
        for (Physics::Primitive* p : node->colliders)
        {
            std::vector<Physics::Primitive*>* sceneColliders = &app->sceneGraph.sceneColliders;
            sceneColliders->erase(std::find(sceneColliders->begin(), sceneColliders->end(), p));
            delete p;
        }
        delete node;
    }
}

void Ui::ShowStartMenu()
{
    bool windowOpen = ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    CheckWindowSize(UiWindows::StartMenu);
    ImVec2 buttonSize = { windowSizes[UiWindows::StartMenu].x / 2, 
                          windowSizes[UiWindows::StartMenu].y / 9.5f };

    if (windowOpen)
    {
        ImGui::Indent(buttonSize.x / 2);
        ImGui::Dummy({ 0, buttonSize.y });

        // Editor button.
        if (ImGui::Button("Launch Editor", buttonSize))
        {
            app->ReloadAll();
            app->state = AppStates::Editor;
        }
        ImGui::Dummy({ 0, buttonSize.y });

        // Game button.
        if (ImGui::Button("Launch Game", buttonSize))
        {
            wireframeMode      = false;
            showColliders      = false;
            app->ReloadAll();
            app->state = AppStates::Game;
            app->StartPlayMode();
        }
        ImGui::Dummy({ 0, buttonSize.y });

        // Options button.
        if (ImGui::Button("Options", buttonSize))
        {
            app->state = AppStates::OptionsMenu;
        }
        ImGui::Dummy({ 0, buttonSize.y });

        // Exit button.
        if (ImGui::Button("Exit", buttonSize))
        {
            glfwSetWindowShouldClose(app->GetWindow(), true);
        }
        ImGui::Unindent(buttonSize.x / 2);
    }
    ImGui::End();
}

void Ui::ShowOptionsMenu()
{
    bool windowOpen = ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    CheckWindowSize(UiWindows::StartMenu);
    ImVec2 buttonSize = { windowSizes[UiWindows::StartMenu].x / 5, 
                          windowSizes[UiWindows::StartMenu].y / 9.5f };

    if (windowOpen)
    {
        ImGui::Indent(windowSizes[UiWindows::StartMenu].x / 2 - buttonSize.x / 2);

        bool  showErrorMessage = false;
        float addedHeight = 0;

        // Forwards keybinding.
        ImGui::Dummy({ 0, buttonSize.y });
        showErrorMessage = !KeyBindings::ChangeBinding(app->GetWindow(), buttonSize, InputDir::Forwards)  || showErrorMessage;
        
        // Backwards keybinding.
        ImGui::Dummy({ 0, buttonSize.y / 2 });
        showErrorMessage = !KeyBindings::ChangeBinding(app->GetWindow(), buttonSize, InputDir::Backwards) || showErrorMessage;
        
        // Left keybinding.
        ImGui::SameLine();
        ImGui::Unindent(buttonSize.x * 1.25f);
        showErrorMessage = !KeyBindings::ChangeBinding(app->GetWindow(), buttonSize, InputDir::Left)      || showErrorMessage;
        // Right keybinding.
        ImGui::SameLine();
        ImGui::Indent(buttonSize.x * 2.5f);
        showErrorMessage = !KeyBindings::ChangeBinding(app->GetWindow(), buttonSize, InputDir::Right)     || showErrorMessage;
        ImGui::Unindent(buttonSize.x * 1.25f);
        
        // Sneak keybinding.
        ImGui::Dummy({ 0, buttonSize.y / 2 });
        ImGui::Unindent(buttonSize.x * 0.625f);
        showErrorMessage = !KeyBindings::ChangeBinding(app->GetWindow(), buttonSize, InputDir::Sneak)     || showErrorMessage;
        // Jump keybinding.
        ImGui::SameLine();
        ImGui::Indent(buttonSize.x * 1.25f);
        showErrorMessage = !KeyBindings::ChangeBinding(app->GetWindow(), buttonSize, InputDir::Jump)      || showErrorMessage;
        ImGui::Unindent(buttonSize.x * 0.125f);

        // Press any key text.
        static float changeKeyDuration = 0;
        {
            static int   prevSelectedButton    = -1;
            static std::string prevKeyName     = "";
            ImVec4 textColor = { 1, 1, 1, 0 };

            if (KeyBindings::selectedButton != -1) {
                textColor.w = 1;
                prevKeyName = KeyBindings::InputDirToStr((InputDir)KeyBindings::selectedButton);
            }
            else if (prevSelectedButton != -1) {
                changeKeyDuration = 0.01f;
                textColor.w = 1;
            }
            else if (changeKeyDuration > 0) {
                changeKeyDuration += app->time.DeltaTime();
                textColor.w = clampUnder(cos(changeKeyDuration*PI)*2, 1);
            }
            if (changeKeyDuration > 1) {
                changeKeyDuration = 0;
            }

            std::string changeKeybindText = "Press any key to set the " + prevKeyName + " keybinding.";
            ImVec2      textSize = ImGui::CalcTextSize(changeKeybindText.c_str());
            
            ImGui::Dummy({ 0, buttonSize.y / 2 });
            ImGui::Unindent(textSize.x / 2);
            ImGui::TextColored(textColor, changeKeybindText.c_str());
            ImGui::Indent(textSize.x / 2);
            addedHeight += buttonSize.y / 2 + textSize.y + 8;
            prevSelectedButton = KeyBindings::selectedButton;
        }

        // Key already in use text.
        static float errorDuration = 0;
        if (showErrorMessage || errorDuration > 0)
        {
            if (showErrorMessage)
                errorDuration = 0.01f;
            else
                errorDuration += app->time.DeltaTime();

            ImVec2 textSize = ImGui::CalcTextSize("This key is already in use");
            ImGui::NewLine();
            ImGui::Unindent(textSize.x / 2);
            ImGui::TextColored(ImVec4(1, 1, 1, clampUnder(cos(errorDuration*PI)*2, 1)), "This key is already in use.");
            ImGui::Indent(textSize.x / 2);

            if (errorDuration > 1)
                errorDuration = 0;
            addedHeight += (textSize.y + 4) * 2;
        }
        
        // Back button.
        ImGui::Dummy({ 0, buttonSize.y * 2 - addedHeight });
        ImGui::Unindent(buttonSize.x / 2);
        if (ImGui::Button("Back", buttonSize)) {
            app->state = AppStates::StartMenu;
        }
        ImGui::Indent(buttonSize.x / 2);

        ImGui::Unindent(windowSizes[UiWindows::StartMenu].x / 2 - buttonSize.x / 2);
    }
    ImGui::End();
}

void Ui::ShowSceneGraphWindow()
{
ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
bool windowOpen = ImGui::Begin("Scene Graph", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
CheckWindowSize(UiWindows::SceneGraph);
if (windowOpen)
app->sceneGraph.ShowUi();
ImGui::End();
}

void Ui::ShowInspectorWindow()
{
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    static SceneNode* prevSelectedNode = app->sceneGraph.selectedNode;
    if (app->sceneGraph.selectedNode != prevSelectedNode) {
        ImGui::SetNextWindowCollapsed(app->sceneGraph.selectedNode == nullptr);
        prevSelectedNode = app->sceneGraph.selectedNode;
    }
    bool windowOpen = ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    CheckWindowSize(UiWindows::Inspector);

    if (app->sceneGraph.selectedNode == nullptr)
    {
        if (windowOpen)
            ImGui::Text("Selected object: none");
        ImGui::End();
        return;
    }
    if (windowOpen && app->sceneGraph.selectedNode != nullptr)
    {
        app->sceneGraph.selectedNode->ShowInspectorUi();
        if (app->sceneGraph.selectedNode->rigidbody != nullptr)
            ShowRigidbodyUi(app->sceneGraph.selectedNode->rigidbody);
        else  if (ImGui::Button("Add Rigidbody"))
            app->sceneGraph.selectedNode->AddRigidbody();

        if (ImGui::TreeNode("Add Collider"))
        {
            for (int i = 0; i < 3; i++)
            {
                if (ImGui::Button(("Add " + Physics::Primitive::GetPrimitiveName((Physics::PrimitiveTypes)i)).c_str()))
                {
                    Physics::Primitive* sph = app->sceneGraph.AddCollider(app->sceneGraph.selectedNode, (Physics::PrimitiveTypes)i);
                }
            }
            ImGui::TreePop();
        }

        ShowCollidersUi(app->sceneGraph.selectedNode);
    }
    ImGui::End();
}

void Ui::ShowCollidersUi(Scenes::SceneNode* node)
{
    if (ImGui::TreeNode("Colliders"))
    {
        int i = 0;
        for (Physics::Primitive* collider : node->colliders)
        {
            if (ImGui::TreeNode(("Collider " + std::to_string(i)).c_str()))
            {
                ShowTransformUi(*collider->transform, collider->type != Physics::PrimitiveTypes::Cube);
                ImGui::TreePop();
            }
            i++;
        }

        ImGui::TreePop();
    }
}

void Ui::ShowResourcesWindow()
{
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    bool windowOpen = ImGui::Begin("Resources", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    CheckWindowSize(UiWindows::Resources);

    if (windowOpen)
    {
        std::unordered_map<std::string, IResource*>& resources = app->resourceManager.GetResources();
        std::unordered_map<std::string, IResource*>::iterator resource;

        // Empty object.
        if (ImGui::Button("+##Empty")) {
            app->sceneGraph.AddEmpty("Empty");
        }
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextWrapped("Empty object");

        // Primitive.
        if (ImGui::CollapsingHeader("Primitives"))
        {
            Material* defaultMat = app->resourceManager.Get<Material>("DefaultMat");                 // TODO: Temporary (should be done automatically).
            ShaderProgram* shaderProgram = app->resourceManager.Get<ShaderProgram>("ShaderProgram"); // TODO: Temporary (should be done automatically).

            ImGui::Indent(3);

            for (int i = 0; i < 3; i++)
            {
                if (ImGui::Button(("+##" + Physics::Primitive::GetPrimitiveName((Physics::PrimitiveTypes)i) ).c_str()))
                {
                    SceneNode* p = app->sceneGraph.AddPrimitive(Physics::Primitive::GetPrimitiveName((Physics::PrimitiveTypes)i), new Physics::Primitive((Physics::PrimitiveTypes)i));
                    ((ScenePrimitive*)p)->primitive->SetShaderProgram(shaderProgram);
                    ((ScenePrimitive*)p)->primitive->SetMaterial(defaultMat);

                }
                ImGui::SameLine();
                ImGui::Text((Physics::Primitive::GetPrimitiveName((Physics::PrimitiveTypes)i)).c_str());
            }

            ImGui::Unindent(3);
        }

        // Meshes.
        if (ImGui::CollapsingHeader("Meshes"))
        {
            for (resource = resources.begin(); resource != resources.end(); resource++)
            {
                // Skip resources that are not mesh groups.
                if (resource->second->GetType() != ResourceTypes::Mesh)
                    continue;
                ImGui::Indent(3);
                if (ImGui::Button(("+##" + resource->first).c_str()))
                    app->sceneGraph.AddModel(resource->first, (Mesh*)resource->second);
                ImGui::Unindent(3);

                // Show every mesh in the group under the tree node.
                ImGui::SameLine();
                ImGui::AlignTextToFramePadding();
                if (ImGui::TreeNode(resource->first.c_str()))
                {
                    ImGui::Indent(5);
                    for (SubMesh* subMesh : ((Mesh*)resource->second)->subMeshes)
                    {
                        ImGui::Bullet();
                        ImGui::TextWrapped((subMesh->GetName() + " (" + std::to_string(subMesh->GetVertexCount()) + " vertices)").c_str());
                    }
                    ImGui::Unindent(5);
                    ImGui::TreePop();
                }
            }
        }

        // Lights.
        if (ImGui::CollapsingHeader("Lights"))
        {
            static const char* lightNames[3] = { "Directional Light", "Point Light", "Spot Light" };
            for (int i = 0; i < 3; i++)
            {
                ImGui::Indent(3);
                if (ImGui::Button(("+##" + std::string(lightNames[i])).c_str())) 
                {
                    switch (i)
                    {
                    case 0:
                        if (DirLight* dirLight = app->lightManager.Create<DirLight>())
                            app->sceneGraph.AddDirLight(lightNames[i], dirLight);
                        break;
                    case 1:
                        if (PointLight* pointLight = app->lightManager.Create<PointLight>())
                            app->sceneGraph.AddPointLight(lightNames[i], pointLight);
                        break;
                    case 2:
                        if (SpotLight* spotLight = app->lightManager.Create<SpotLight>())
                            app->sceneGraph.AddSpotLight(lightNames[i], spotLight);
                        break;
                    default:
                        break;
                    }
                }
                ImGui::Unindent(3);
                ImGui::SameLine();
                ImGui::AlignTextToFramePadding();
                ImGui::TextWrapped(lightNames[i]);
            }
        }

        // Materials.
        if (ImGui::CollapsingHeader("Materials"))
        {
            for (resource = resources.begin(); resource != resources.end(); resource++)
            {
                // Skip resources that are not materials.
                if (resource->second->GetType() != ResourceTypes::Material)
                    continue;

                // Show the current material under a tree node.
                if (ImGui::TreeNode(resource->first.c_str()))
                {
                    ImGui::Unindent(5);
                    ShowMaterialUi((Material*)resource->second);
                    ImGui::Indent(5);
                    ImGui::TreePop();
                }
            }
        }

        // Textures.
        if (ImGui::CollapsingHeader("Textures"))
        {
            int i = 0;
            for (resource = resources.begin(); resource != resources.end(); resource++)
            {
                // Skip resources that are not textures.
                if (resource->second->GetType() != ResourceTypes::Texture &&
                    resource->second->GetType() != ResourceTypes::DynamicTexture)
                    continue;

                // Put 3 textures per line.
                if (i % 3 > 0)
                    ImGui::SameLine();
                ShowTextureUi((Texture*)resource->second, (ImGui::GetWindowWidth() - 50) / 3);

                // Texture dragging packets.
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                {
                    void** addressOfThis = new void*(resource->second);
                    ImGui::SetDragDropPayload("ResourceTexture", addressOfThis, sizeof(addressOfThis));
                    ImGui::Text("%s", resource->first.c_str());
                    ImGui::EndDragDropSource();
                    delete addressOfThis;
                }

                // Show texture name in tooltip.
                else if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text(resource->first.c_str());
                    ImGui::EndTooltip();
                }
                i++;
            }
        }
    }
    ImGui::End();
}

void Ui::ShowLogsWindow()
{
    ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
    bool windowOpen = ImGui::Begin("Logs", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    CheckWindowSize(UiWindows::Logs);

    if (windowOpen)
    {
        static unsigned int prevLineCount = 0;
        unsigned int        lineCount     = 0;

        std::string line;
        std::getline(Debug::Log::logFileCache->fileCache, line);
        for (lineCount; std::getline(Debug::Log::logFileCache->fileCache, line); lineCount++)
        {
            // Show infos in green.
            if (line.compare(0, 5, "INFO:") == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "INFO:");
                ImGui::PopStyleColor();
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1));
                ImGui::TextWrapped(line.substr(6, line.size()-6).c_str());
                ImGui::PopStyleColor();
            }

            // Show warnings in yellow.
            else if (line.compare(0, 8, "WARNING:") == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "WARNING:");
                ImGui::PopStyleColor();
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1));
                ImGui::TextWrapped(line.substr(9, line.size()-9).c_str());
                ImGui::PopStyleColor();
            }
            
            // Show errors in red.
            else if (line.compare(0, 6, "ERROR:") == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                ImGui::TextWrapped("ERROR:");
                ImGui::PopStyleColor();
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1));
                ImGui::TextWrapped(line.substr(7, line.size()-7).c_str());
                ImGui::PopStyleColor();
            }

            // Show the rest in white.
            else
            {
                ImGui::TextWrapped(line.c_str());
            }
        }
        Debug::Log::logFileCache->fileCache.clear();
        Debug::Log::logFileCache->fileCache.seekg(0);

        // Scroll to the bottom when new logs arrive.
        if (lineCount != prevLineCount)
        {
            ImGui::SetScrollHereY(1.0);
            prevLineCount = lineCount;
        }
    }
    ImGui::End();
}

void Ui::ShowStatsWindow()
{
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    bool windowOpen = ImGui::Begin("Stats", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    CheckWindowSize(UiWindows::Stats);

    if (windowOpen)
    {
        // Target FPS.
        ImGui::AlignTextToFramePadding(); ImGui::Text("Target FPS:"); ImGui::SameLine();
        int targetFps = app->time.GetTargetFPS();
        ImGui::SetNextItemWidth(55);
        if (ImGui::DragInt("##targetFPS", &targetFps, 0.1f, 15) && targetFps >= 15)
            app->time.SetTargetFPS(targetFps);

        // FPS and delta time.
        ImGui::Text(("FPS: " + std::to_string(app->time.FPS())).c_str());
        ImGui::Text(("Delta Time: " + std::to_string(app->time.DeltaTime())).c_str());

        // Vertex count.
        ImGui::TextWrapped(("Vertex count: " + std::to_string(app->sceneGraph.totalVertexCount)).c_str());

        // Engine camera speed.
        std::string cameraSpeed = std::to_string((int)(app->cameraManager.engineCamera->moveSpeed * 20));
        ImGui::TextWrapped(("Camera speed: " + cameraSpeed).c_str());

        // VSync toggle.
        bool vsync = app->time.IsVsyncOn();
        if (ImGui::Checkbox("VSync", &vsync))
            app->time.SetVsyncOn(vsync);

        // Wireframe toggle.
        if (ImGui::Checkbox("Wireframe", &wireframeMode))
        {
            if (wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else               glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Show colliders toggle.
        ImGui::Checkbox("Colliders", &showColliders);

        // Async resource loading toggle.
        static bool asyncLoading = true;
        if (ImGui::Checkbox("Async loading", &asyncLoading))
            app->resourceManager.SetAsyncLoading(asyncLoading);

        // Reload Resources
        ImGui::AlignTextToFramePadding();
        if (ImGui::Button("Reload Resources")) {
            app->StopPlayMode();
            app->shouldReloadScene = true;
        }


        // Benchmark iterations.
        if (!app->isInBenchmark)
        {
            // Start benchmark.
            if(ImGui::Button("Launch Benchmark"))
                app->isInBenchmark = true;

            ImGui::Text("Reload count:");
            ImGui::SliderInt("##benchmartIterations", &app->maxLoad, 2, 20);
        }

        ImGui::AlignTextToFramePadding();
    }
    ImGui::End();
}

void Ui::ShowPlayWindow()
{
    bool windowOpen = ImGui::Begin("Play", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
    CheckWindowSize(UiWindows::Play);

    if (windowOpen)
    {
        bool inSceneView    = app->InSceneView();
        bool inPlayMode     = app->InPlayMode();
        bool playModePaused = app->PlayModePaused();

        bool disablePlayButton  =  inPlayMode && !playModePaused;
        bool disablePauseButton = !inPlayMode || playModePaused;
        bool disableStopButton  = !inPlayMode;

        ImGui::Indent(6);

        // Show play button (disabled in play mode).
        if (disablePlayButton) ImGui::BeginDisabled();
        if (ImGui::ArrowButton("##right", ImGuiDir_Right))
        {
            if (!inPlayMode) app->StartPlayMode();
            else             app->ResumePlayMode();
        }
        if (disablePlayButton) ImGui::EndDisabled();
        ImGui::SameLine();

        // Show pause button (disabled out of play mode and when paused).
        if (disablePauseButton) ImGui::BeginDisabled();
        if (ImGui::Button("||")) app->PausePlayMode();
        if (disablePauseButton) ImGui::EndDisabled();
        ImGui::SameLine();

        // Show stop button (disabled out of play mode).
        if (disableStopButton) ImGui::BeginDisabled();
        if (ImGui::Button("X" )) app->StopPlayMode();
        if (disableStopButton) ImGui::EndDisabled();

        ImGui::Unindent(6);

        // Show switch view button.
        if (inSceneView) ImGui::Indent(4);
        if (ImGui::Button(inSceneView ? "Scene View##switchView" : "Engine View##switchView"))
            app->ToggleView();
        if (inSceneView) ImGui::Unindent(4);
    }
    ImGui::End();
}

void Ui::ShowRigidbodyUi(Physics::Rigidbody* rigidbody)
{
    ImGui::Indent(6);

    if (ImGui::TreeNode("Rigidbody"))
    {
        Vector3 velocity = rigidbody->GetVelocity();
        if (ImGui::DragFloat3("Velocity", &velocity.x, 0.1f))
            rigidbody->SetVelocity(velocity);

        Vector3 acceleration = rigidbody->GetAcceleration();
        if (ImGui::DragFloat3("Acceleration", &acceleration.x, 0.1f))
            rigidbody->SetAcceleration(acceleration);

        ImGui::TreePop();
    }

    ImGui::Unindent(6);
}
