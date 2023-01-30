#include <glad/glad.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include "PyOpaqueClasses.h"

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <filesystem>

#include "Debug.h"
#include "App.h"
#include "Ui.h"
#include "KeyBindings.h"
#include "Camera.h"
#include "PyScript.h"
#include "AsteroidRotation.h"
#include "Cubemap.h"

using namespace Core;
using namespace Core::Physics;
using namespace Scenes;


// TEMP START
unsigned int /*FBO, RBO, framebufferTexture, */rectVAO, rectVBO;
ShaderProgram* framebufferProgram = nullptr;
RenderTexture* postProcessRenderTexture = nullptr;
float rectangleVertices[] =
{
	// Coords    // texCoords
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};
// TEMP END

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Make sure the viewport matches the new window dimensions.
    glViewport(0, 0, width, height);

    // Resize all screen-size cameras.
    for (int i = 0; i < CameraManager::screenScaleCameras.size(); i++)
    {
        Render::CameraParams params = CameraManager::screenScaleCameras[i]->GetParameters();
        params.width  = width;
        params.height = height;
        CameraManager::screenScaleCameras[i]->ChangeParameters(params);
    }

    Ui::FramebufferResizeCallback(width, height);
}


App::App(const AppInitializer& init)
    : time(init.vsync, init.fps)
{
    windowWidth  = init.width;
    windowHeight = init.height;

    // Initialize and configure GLFW.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, init.major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, init.minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // Create a GLFW window.
    window = glfwCreateWindow(init.width, init.height, init.windowName, NULL, NULL);
    Assert(window != NULL, "Failed to create GLFW window.");
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // Load all OpenGL function pointers from GLAD.
    Assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize GLAD.");

    // Setup OpenGL flags.
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glFrontFace(GL_CCW);

    // Initialize ImGui.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Set the app's log file.
    DebugSetAndClearLogFile("Logs/app.log");
    Ui::SetAppPtr(this);
    Ui::FramebufferResizeCallback(init.width, init.height);

    // Create the engine's camera.
    cameraManager.engineCamera = new EngineCamera({ GetWindowW(), GetWindowH(), 0.1f, 1000.f, 80.f });
    cameraManager.screenScaleCameras.push_back(cameraManager.engineCamera);

    // Create the resource manager's texture sampler.
    resourceManager.CreateSampler();

    // Maximize the window.
    if (init.maximized)
        glfwMaximizeWindow(window);

    // Set the window's user pointer to the app.
    glfwSetWindowUserPointer(window, this);

    // Update the engine camera's speed when the scroll wheel is used.
    glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) -> 
    void { 
        ImGui_ImplGlfw_ScrollCallback(window, x, y);
        App* app = (App*)glfwGetWindowUserPointer(window);
        app->inputs.mouseScroll = (float)y;
        EngineCamera* cam = app->cameraManager.engineCamera;
        if (cam && app->inputs.mouseRightClick && !app->InSceneView())
            cam->moveSpeed = clampAbove(cam->moveSpeed + (float)y * 10, 0.05f);
    });

    InitPyInterpreter();

    // Load the post processor.
    postProcessor.Setup(windowWidth, windowHeight);
    postProcessor.SetClearColor({ 0.2f, 0.3f, 0.3f, 1.f });
}

void App::InitPyInterpreter()
{
    // Initialize the python interpreter and add the working directory to the python path.
    pybind11::initialize_interpreter();
    pybind11::module_ sys = pybind11::module::import("sys");
    sys.attr("path").attr("insert")(1,  std::filesystem::current_path().string().c_str());
    sys.attr("path").attr("insert")(1, (std::filesystem::current_path().string() + "\\Libs").c_str());
    pybind11::print(sys.attr("path"));
}

App::~App()
{
    delete cameraManager.engineCamera;
    DebugSaveLogFile();
    sceneGraph.~SceneGraph(); // Destroy all objects before stopping the Python interpreter.
    pybind11::finalize_interpreter();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void App::Run()
{
    while (!glfwWindowShouldClose(window)) 
    {
        if (time.CanStartNextFrame()) 
        {
            SendLoadedResources();
            if (!chronoHasEnded)
            {
                if (resourceManager.AreAllResourcesInOpenGL() && !isInBenchmark)
                {
                    chronoHasEnded = true;
                    end = std::chrono::steady_clock::now();
                    std::string strTime = "Time for loading : ";
                    strTime += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());
                    strTime += " ms \n";
                    DebugLog(strTime);
                }
            }
            Render();
        }
    }

    // Destroy the thread manager first to kill all threads before unallocating any memory.
    resourceManager.threadManager.~ThreadManager();
}


void App::ProcessInputs()
{
    // Clear inputs from the previous frame.
    Vector2 oldMousePos = inputs.mousePos;
    inputs = AppInputs();

    glfwPollEvents();

    // Exit app.
    static bool prevEscapeState = false;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        if (!prevEscapeState) {
            inputs.exitKeyPressed = true;
            prevEscapeState = true;
        }
    }
    else if (prevEscapeState)
    {
        prevEscapeState = false;
    }

    // Reload Python scripts.
    static bool prevF5State = false;
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
    {
        if (!prevF5State) {
            inputs.reloadPyScripts = true;
            prevF5State = true;
        }
    }
    else if (prevF5State)
    {
        prevF5State = false;
    }

    // Z-axis movement.
    if (glfwGetKey(window, KeyBindings::directionKeybinds[0]) == GLFW_PRESS)
        inputs.movement.z++;
    if (glfwGetKey(window, KeyBindings::directionKeybinds[1]) == GLFW_PRESS)
        inputs.movement.z--;
    
    // X-axis movement.
    if (glfwGetKey(window, KeyBindings::directionKeybinds[2]) == GLFW_PRESS)
        inputs.movement.x++;
    if (glfwGetKey(window, KeyBindings::directionKeybinds[3]) == GLFW_PRESS)
        inputs.movement.x--;
    
    //Diagonal movement.
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) 
    {
        inputs.movement.x++;
        inputs.movement.z++;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        inputs.movement.x--;
        inputs.movement.z++;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        inputs.movement.x++;
        inputs.movement.z--;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        inputs.movement.x--;
        inputs.movement.z--;
    }

    // Normalize movement.
    if (inputs.movement.getLength() > 1)
        inputs.movement.normalize();
    
    // Y-axis movement.
    if (glfwGetKey(window, KeyBindings::directionKeybinds[4]) == GLFW_PRESS)
        inputs.movement.y++;
    if (glfwGetKey(window, KeyBindings::directionKeybinds[5]) == GLFW_PRESS)
        inputs.movement.y--;

    // Mouse right click.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
        inputs.mouseRightClick = true;

    // Mouse right click.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
        inputs.mouseLeftClick = true;

    // Mouse position.
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    inputs.mousePos = Vector2((float)mouseX, (float)mouseY);

    // Mouse delta.
    inputs.mouseDelta = inputs.mousePos - oldMousePos;
}

void App::BeginRender()
{
    time.NewFrame();
    ProcessInputs();
    postProcessor.BeginRender();

    // Start a new ImGui frame.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void App::EndRender()
{
    postProcessor.EndRender();

    // Render ImGui.
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap window buffers.
    glfwSwapBuffers(window);
}

void App::Render()
{
    BeginRender();
    {
        if (inputs.exitKeyPressed)
        {
            if (state != AppStates::StartMenu) {
                StopPlayMode();
                state = AppStates::StartMenu;
            }
            else {
                glfwSetWindowShouldClose(window, true);
            }
        }
        else
        {
            switch (state)
            {
            case AppStates::StartMenu:
                Ui::ShowStartMenu();
                break;
            case AppStates::OptionsMenu:
                Ui::ShowOptionsMenu();
                break;
            case AppStates::Editor:
                RenderEditor();
                break;
            case AppStates::Game:
                RenderGame();
                break;
            default:
                break;
            }
        }
    }
    EndRender();

    // Reload the scene if necessary.
    if (shouldReloadScene)
    {
        if (isInBenchmark) {
            if (UnloadResources())
                LoadBenchmark();
        }
        UnloadScene();
        LoadExampleScene();
        shouldReloadScene = false;
    }

    // Reload the Python scripts if necessary.
    if (inputs.reloadPyScripts && !inPlayMode) {
        pybind11::finalize_interpreter();
        InitPyInterpreter();
        sceneGraph.root->ReloadPyScripts();
        DebugLog("Reloaded all Python scripts.");
    }
}

void App::RenderEditor()
{
    // Get the camera currently in use.
    Camera* camera = (inSceneView ? cameraManager.sceneCamera : cameraManager.engineCamera);

    // Draw all scene objects.
    bool shouldUpdateScripts = inPlayMode && !playModePaused && resourceManager.AreAllResourcesLoaded();
    sceneGraph.UpdateAndDrawAll(*camera, lightManager, !shouldUpdateScripts);

    // Update engine camera transform.
    if (!inSceneView)
        cameraManager.engineCamera->ProcessInputs(inputs, window);

    if(isInBenchmark)
        Benchmark();    
    
    // Draw ui windows.
    Ui::ShowSceneGraphWindow();
    Ui::ShowInspectorWindow();
    Ui::ShowResourcesWindow();
    Ui::ShowPostProcessWindow();
    Ui::ShowLogsWindow();
    Ui::ShowStatsWindow();
    Ui::ShowPlayWindow();
}

void App::RenderGame()
{
    // Draw all scene objects.
    bool shouldUpdateScripts = resourceManager.AreAllResourcesLoaded();
    sceneGraph.UpdateAndDrawAll(*cameraManager.sceneCamera, lightManager , !shouldUpdateScripts);
}

// ----- Play mode ----- //

bool App::InSceneView()    { return inSceneView; }
void App::ToggleView()     { inSceneView = !inSceneView; }
bool App::InPlayMode()     { return inPlayMode; }
bool App::PlayModePaused() { return playModePaused; }
void App::PausePlayMode()  { playModePaused = true; }
void App::ResumePlayMode() { playModePaused = false; }
void App::StartPlayMode()
{ 
    inSceneView    = true;
    inPlayMode     = true;
    playModePaused = false;
    sceneGraph.StartPlayMode();
}
void App::StopPlayMode()
{
    inSceneView       = false;
    inPlayMode        = false;
    playModePaused    = false;
    shouldReloadScene = true;
}


// ----- Scene loading ----- //
void App::LoadDefaultResources()
{
    // ----- Build and compile shader program ----- //
    
    // Create mesh shaders and shader program.
    VertexShader*   meshShaderVert    = resourceManager.Create<VertexShader  >("Resources/Shaders/meshShader.vert");
    FragmentShader* meshShaderFrag    = resourceManager.Create<FragmentShader>("Resources/Shaders/meshShader.frag");
    ShaderProgram*  meshShaderProgram = resourceManager.Create<ShaderProgram >("MeshShaderProgram");
    meshShaderProgram->AttachShader((IResource*)meshShaderVert);
    meshShaderProgram->AttachShader((IResource*)meshShaderFrag);

    // Create mesh shaders and shader program.
    VertexShader* meshInstancedShaderVert = resourceManager.Create<VertexShader  >("Resources/Shaders/meshInstancedShader.vert");
    ShaderProgram* meshInstanceShaderProgram = resourceManager.Create<ShaderProgram >("MeshInstancedShaderProgram");
    meshInstanceShaderProgram->AttachShader((IResource*)meshInstancedShaderVert);
    meshInstanceShaderProgram->AttachShader((IResource*)meshShaderFrag);

    // Create skybox shaders and shader program.
    VertexShader*   skyboxShaderVert    = resourceManager.Create<VertexShader  >("Resources/Shaders/skyboxShader.vert");
    FragmentShader* skyboxShaderFrag    = resourceManager.Create<FragmentShader>("Resources/Shaders/skyboxShader.frag");
    ShaderProgram*  skyboxShaderProgram = resourceManager.Create<ShaderProgram >("SkyboxShaderProgram");
    skyboxShaderProgram->AttachShader((IResource*)skyboxShaderVert);
    skyboxShaderProgram->AttachShader((IResource*)skyboxShaderFrag);


    // ----- Default resources ----- //

    PrimitiveBuffers::CreateBuffers();
    Texture*  defaultTexture = resourceManager.Create<Texture>("Resources/Textures/defaultTexture.png");
    Material* defaultMat = resourceManager.Create<Material>("DefaultMat");
    Material* colliderMat = resourceManager.Create<Material>("ColliderMat");
    Material* boundingBoxMat = resourceManager.Create<Material>("BoundingBoxMat");
    defaultMat->SetParams({ 0, 0, 0 }, { 1, 1, 1 }, { 1, 1, 1 }, { 0, 0, 0 }, 32);
    colliderMat->SetParams({ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 1, 0 }, 0);
    boundingBoxMat->SetParams({ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1, 0 }, 0);
    SceneNode::SetDefaultRenderValues(meshShaderProgram, defaultMat, colliderMat, boundingBoxMat);

    // Load the cubemap mesh.
    ObjFile* cubemapObj = resourceManager.Create<ObjFile>("Resources/Obj/cubemap.obj");
    if (cubemapObj->createdResources.size() > 0)
        ((Mesh*)cubemapObj->createdResources[0])->subMeshes[0]->SetShaderProgram(skyboxShaderProgram);
}

void App::LoadResources()
{
    LoadDefaultResources();

    std::thread t{ [&]() {
            if(!isInBenchmark)
                begin = std::chrono::steady_clock::now();

            // ----- Load all meshes, materials and textures ----- //

            Texture* steveTexture = resourceManager.Create<Texture>("Resources/Textures/steveCursed.png");
            Material* matSteve = resourceManager.Create<Material>("SteveMaterial");
            matSteve->diffuse = RGB(1, 1, 1);
            matSteve->shininess = 32;
            matSteve->diffuseTexture = steveTexture;
            
            Cubemap* skybox = resourceManager.Create<Cubemap>("Skybox");
            skybox->SetTexture(CubeSides::Right,  "Resources/Skyboxes/Red/bkg1_right1.png");
            skybox->SetTexture(CubeSides::Left,   "Resources/Skyboxes/Red/bkg1_left2.png");
            skybox->SetTexture(CubeSides::Top,    "Resources/Skyboxes/Red/bkg1_top3.png");
            skybox->SetTexture(CubeSides::Bottom, "Resources/Skyboxes/Red/bkg1_bottom4.png");
            skybox->SetTexture(CubeSides::Back,   "Resources/Skyboxes/Red/bkg1_back6.png");
            skybox->SetTexture(CubeSides::Front,  "Resources/Skyboxes/Red/bkg1_front5.png");
            /*
            Cubemap* minecraftBox = resourceManager.Create<Cubemap>("MinecraftBox");
            minecraftBox->SetTexture(CubeSides::Back, "Resources/Skyboxes/Minecraft/panorama_0.png");
            minecraftBox->SetTexture(CubeSides::Left, "Resources/Skyboxes/Minecraft/panorama_1.png");
            minecraftBox->SetTexture(CubeSides::Front, "Resources/Skyboxes/Minecraft/panorama_2.png");
            minecraftBox->SetTexture(CubeSides::Right, "Resources/Skyboxes/Minecraft/panorama_3.png");
            minecraftBox->SetTexture(CubeSides::Top, "Resources/Skyboxes/Minecraft/panorama_4.png");
            minecraftBox->SetTexture(CubeSides::Bottom, "Resources/Skyboxes/Minecraft/panorama_5.png");
            */
            resourceManager.Create<ObjFile>("Resources/Assets/Gun/gun.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Rico/rico.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/TrainingStadium/SuperTrainingStage.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Moon/moon.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Headcrab/headcrab.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Crowbar/crowbar.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/DoomSlayer/doommarine.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/BFG/bfg.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/MasterChief/masterChief.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/EnergySword/energySword.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/ItemBox/box.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Banana/banana.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Palutena/palutena.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Palutena/palutenaStaff.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/Alduin/alduin.obj");

            resourceManager.Create<ObjFile>("Resources/Assets/Asteroid/Asteroid.obj");
            resourceManager.Create<ObjFile>("Resources/Assets/MercuryPlanet/Mercury 1K.obj");
        }
    };
    t.detach();
}

void App::SendLoadedResources()
{
    bool textureSentThisFrame = false;
    resourceManager.CheckForNewPyResources();

    // Find resources that just finished loading and send their data to openGL.
    std::unordered_map<std::string, IResource*>& resources = resourceManager.GetResources();
    for (std::unordered_map<std::string, IResource*>::iterator it = resources.begin(); it != resources.end(); it++)
    {
        if (it->second->WasSentToOpenGL() || (textureSentThisFrame && it->second->GetType() == ResourceTypes::Texture))
            continue;

        // For mesh resources, send each sub-mesh to openGL.
        if (it->second->GetType() == ResourceTypes::Mesh) {
            SubMesh* subMesh = nullptr;
            for (int i = 0; i < ((Mesh*)it->second)->subMeshes.size(); ++i) {
                subMesh = ((Mesh*)it->second)->subMeshes[i];
                if (subMesh->IsLoaded() && !subMesh->WasSentToOpenGL())
                    subMesh->SendVerticesToOpenGL(sceneGraph.totalVertexCount);
            }
        }

        // Send the resource to openGL.
        if (it->second->IsLoaded() && !it->second->WasSentToOpenGL()) {
            it->second->SendToOpenGL();
            if (it->second->GetType() == ResourceTypes::Texture)
                textureSentThisFrame = true;
        }
    }
}

bool App::UnloadResources()
{
    sceneGraph.totalVertexCount = 0;
    chronoHasEnded = false;
    return resourceManager.Reset();
}

void App::LoadExampleScene()
{
    // ----- Create the skybox ----- //

    SceneSkybox* skybox = sceneGraph.AddSkybox("Skybox", resourceManager.Get<Cubemap>("Skybox"));
    // SceneSkybox* skybox = sceneGraph.AddSkybox("Skybox", resourceManager.Get<Cubemap>("MinecraftBox"));


    // ----- Create the camera ----- //

    Camera* camera = cameraManager.Create({ GetWindowW(), GetWindowH(), 0.1f, 1000.f, 80.f }, true);
    cameraManager.sceneCamera = camera;

    // ----- Setup scene lights ----- //

    DirLight*   light0 = lightManager.Create<DirLight>();
    DirLight*   light1 = lightManager.Create<DirLight>();
    PointLight* light2 = lightManager.Create<PointLight>();
    SpotLight*  light3 = lightManager.Create<SpotLight>();
    PointLight* light4 = lightManager.Create<PointLight>();
    PointLight* light5 = lightManager.Create<PointLight>();

    light0->SetParams({0, 0, 0}, {0.7f, 0.7f, 0.7f}, {0, 0, 0}, {-0.5f, -1, 0.5f});
    light1->SetParams({0, 0, 0}, {0.7f, 0.7f, 0.7f}, {0, 0, 0}, {0.5f, 1, -0.5f});
    light2->SetParams({0, 0, 0}, {1, 1, 1}, {1, 1, 1}, 1, 0.025f, 0.01f, {0, 1, -1});
    light3->SetParams({0, 0, 0}, {1, 1, 1}, {1, 1, 1}, {0, 6.8f, 0}, {0, -1, 0}, PI/8, PI/12);
    light4->SetParams({0, 0, 0}, {1, 0, 0}, {1, 0, 0}, 1, 0.025f, 0.05f, {-4.5f, 9.5f, 15.5f});
    light5->SetParams({0, 0, 0}, {0.5f, 0, 0}, {0.5f, 0, 0}, 1, 0.025f, 0.03f, {-15, 14, 13});

    
    // ----- Setup scene graph ----- //

    // Camera and lights.
    SceneNode* sceneCamera = sceneGraph.AddCamera    ("Camera",           camera);
    SceneNode* dirLight    = sceneGraph.AddDirLight  ("DirectionalLight", light0);
    SceneNode* dirLight2   = sceneGraph.AddDirLight  ("DirectionalLight", light1);
    SceneNode* pointLight  = sceneGraph.AddPointLight("PointLight",       light2);
    SceneNode* spotLight   = sceneGraph.AddSpotLight ("SpotLight",        light3);

    // Models.
    SceneNode* player      = sceneGraph.AddModel("Player",       resourceManager.Get<Mesh>("mesh_Rico"));
    SceneNode* gunLeft     = sceneGraph.AddModel("Gun",          resourceManager.Get<Mesh>("mesh_Gun") , player);
    SceneNode* gunRight    = sceneGraph.AddModel("Gun",          resourceManager.Get<Mesh>("mesh_Gun") , player);
    SceneNode* stadium     = sceneGraph.AddModel("Stadium",      resourceManager.Get<Mesh>("stadium"));
    SceneNode* moon        = sceneGraph.AddModel("Moon",         resourceManager.Get<Mesh>("mesh_moon"));
    SceneNode* headcrab    = sceneGraph.AddModel("Headcrab",     resourceManager.Get<Mesh>("mesh_headcrab"));
    SceneNode* crowbar     = sceneGraph.AddModel("Crowbar",      resourceManager.Get<Mesh>("mesh_crowbar"), headcrab);
    SceneNode* doomSlayer  = sceneGraph.AddModel("Doom Slayer",  resourceManager.Get<Mesh>("mesh_doommarine"));
    SceneNode* bfg         = sceneGraph.AddModel("BFG",          resourceManager.Get<Mesh>("mesh_bfg"), doomSlayer);
    SceneNode* masterChief = sceneGraph.AddModel("Master Chief", resourceManager.Get<Mesh>("mesh_MasterChief"));
    SceneNode* energySword = sceneGraph.AddModel("Energy Sword", resourceManager.Get<Mesh>("energySword"), masterChief);
    SceneNode* itemBox     = sceneGraph.AddModel("Item Box",     resourceManager.Get<Mesh>("mesh_box"));
    SceneNode* banana      = sceneGraph.AddModel("Banana",       resourceManager.Get<Mesh>("mesh_banana"), itemBox);
    SceneNode* palutena    = sceneGraph.AddModel("Palutena",     resourceManager.Get<Mesh>("mesh_palutena"));
    SceneNode* staff       = sceneGraph.AddModel("Staff",        resourceManager.Get<Mesh>("mesh_palutenaStaff"), palutena);
    SceneNode* alduin      = sceneGraph.AddModel("Alduin",       resourceManager.Get<Mesh>("Alduin"));
    SceneNode* mercury     = sceneGraph.AddModel("Mercury",      resourceManager.Get<Mesh>("mesh_mercury"));

    SceneNode* asteroidBelt = sceneGraph.AddInstancedModel("Asteroid Belt", resourceManager.Get<Mesh>("mesh_asteroid"), 400);

    SceneNode* pointLight2 = sceneGraph.AddPointLight("PointLight", light4, alduin);
    SceneNode* pointLight3 = sceneGraph.AddPointLight("PointLight", light5, alduin);
    SceneNode* killZone    = sceneGraph.AddEmpty("KillZone");

    // Primitives.
    SceneNode* primCube0 = sceneGraph.AddPrimitive("Cube0",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube1 = sceneGraph.AddPrimitive("Cube2",  new Primitive(PrimitiveTypes::Cube)); 
    SceneNode* primCube2 = sceneGraph.AddPrimitive("Cube3",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube3 = sceneGraph.AddPrimitive("Cube4",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube4 = sceneGraph.AddPrimitive("Cube5",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube5 = sceneGraph.AddPrimitive("Cube6",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube6 = sceneGraph.AddPrimitive("Cube7",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube7 = sceneGraph.AddPrimitive("Cube8",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube8 = sceneGraph.AddPrimitive("Cube9",  new Primitive(PrimitiveTypes::Cube));
    SceneNode* primCube9 = sceneGraph.AddPrimitive("Cube10", new Primitive(PrimitiveTypes::Cube));
                                        
    // Primitive textures.
    ((ScenePrimitive*)primCube0)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube1)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube2)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube3)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube4)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube5)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube6)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube7)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube8)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));
    ((ScenePrimitive*)primCube9)->primitive->SetMaterial(resourceManager.Get<Material>("SteveMaterial"));

    // Colliders.
    Primitive* cubeCollider0    = sceneGraph.AddCollider(primCube0, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider1    = sceneGraph.AddCollider(primCube1, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider2    = sceneGraph.AddCollider(primCube2, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider3    = sceneGraph.AddCollider(primCube3, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider4    = sceneGraph.AddCollider(primCube4, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider5    = sceneGraph.AddCollider(primCube5, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider6    = sceneGraph.AddCollider(primCube6, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider7    = sceneGraph.AddCollider(primCube7, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider8    = sceneGraph.AddCollider(primCube8, Physics::PrimitiveTypes::Cube);
    Primitive* cubeCollider9    = sceneGraph.AddCollider(primCube9, Physics::PrimitiveTypes::Cube);
    Primitive* playerCollider   = sceneGraph.AddCollider(player,    Physics::PrimitiveTypes::Capsule , Vector3(0, 0.25f, 0), Vector3(0, 0, 0), Vector3(0.25, 0.25, 0.25));
    Primitive* bananaCollider   = sceneGraph.AddCollider(banana,    Physics::PrimitiveTypes::Cube);
    Primitive* killZoneCollider = sceneGraph.AddCollider(killZone,  Physics::PrimitiveTypes::Cube, Vector3(0, 0, 0) , Vector3(0, 0, 0), Vector3(200, 1, 200));

    // Rigidbodies.
    player  ->AddRigidbody();
    killZone->AddRigidbody(true);

    // Transform scene objects.
    player      ->transform.SetScale({ 3.f ,3.f     ,3.f   });
    player      ->transform.Move    ({ -2,    0,     0     });
    player      ->transform.SetRotation({ 0, PI / 2 , 0 });
    gunLeft     ->transform.SetPosition({ 0.3f , 0.38f , 0 });
    gunLeft     ->transform.SetScale({ 0.05f, 0.05f, 0.05f });
    gunRight    ->transform.SetPosition({ -0.3f , 0.38f , 0 });
    gunRight    ->transform.SetScale({ 0.05f, 0.05f, 0.05f });
    camera      ->transform->Move   ({ 0, 1, -8 });
    camera      ->transform->Rotate ({ degToRad(15), 0, 0 });
    primCube0   ->transform.Move    ({ 19.7f, 2,     0.5f  });
    primCube0   ->transform.SetScale({ 4.5f,  1,     12.1f });
    primCube1   ->transform.Move    ({ 26.4f, 3.8f,  0     });
    primCube1   ->transform.SetScale({ 3.6f,  0.25f, 4.9f  });
    primCube2   ->transform.Move    ({ 0,    -1,     0     });
    primCube2   ->transform.SetScale({ 23.5f, 1,     7.15f });
    primCube3   ->transform.Move    ({ 14.6f, 0,     0.9f  });
    primCube3   ->transform.SetScale({ 1.85f, 1,     3.7f  });
    primCube4   ->transform.Move    ({ 29.9f, 4.9f,  11.3f });
    primCube4   ->transform.SetScale({ 3.6f,  0.25f, 4.9f  });
    primCube5   ->transform.Move    ({ 33.9f, 2.9f,  3.7f  });
    primCube5   ->transform.SetScale({ 3.6f,  0.25f, 4.9f  });
    primCube6   ->transform.Move    ({ 23.8f, 6.6f,  14.6f });
    primCube6   ->transform.SetScale({ 3.6f,  0.25f, 4.9f  });
    primCube7   ->transform.Move    ({ 17.1f, 7,     24.2f });
    primCube7   ->transform.SetScale({ 3.6f,  0.25f, 4.9f  });
    primCube8   ->transform.Move    ({ 17.2f, 7.1f,  33    });
    primCube8   ->transform.SetScale({ 6.6f,  1.35f, 5.65f });
    primCube9   ->transform.Move    ({ 17.95f, 3.975f, 1 });
    primCube9   ->transform.SetScale({ 1.f, 2.95f, 1.f });
    killZone    ->transform.Move    ({ 0, -3.9f, 0 });
    stadium     ->transform.Move    ({ 0, -3, 0 });
    stadium     ->transform.SetScale({ 4, 4, 4 });
    moon        ->transform.Move    ({ 0, 40, 40 });
    moon        ->transform.Rotate  ({ 0, PI/2, -6*PI/9 });
    moon        ->transform.SetScale({ 0.01f, 0.01f, 0.01f });
    headcrab    ->transform.Move    ({ -4.5f, 1.6f, 5.f });
    headcrab    ->transform.Rotate  ({ 0, PI/2, 0 });
    headcrab    ->transform.SetScale({ 3, 3, 3 });
    crowbar     ->transform.Move    ({ 0, 0.75, 0 });
    crowbar     ->transform.Rotate  ({ -PI/3, PI, -PI/2 });
    crowbar     ->transform.SetScale({ 0.03f, 0.03f, 0.03f });
    itemBox     ->transform.Move    ({ -4.5, 0, 5.f });
    itemBox     ->transform.SetScale({ 0.7f, 0.7f, 0.7f });
    banana      ->transform.Rotate({PI / 2, -PI / 1.2f, 0});
    banana      ->transform.SetScale({ 0.02f, 0.02f, 0.02f });
    doomSlayer  ->transform.Move    ({ -1.5f, 0, 5.f });
    doomSlayer  ->transform.SetScale({ 0.02f, 0.02f, 0.02f });
    bfg         ->transform.Move    ({ 0, 250, 0 });
    bfg         ->transform.Rotate  ({ 0, PI/2, 0 });
    masterChief ->transform.Move    ({ 1.5f, 0, 5.f });
    masterChief ->transform.SetScale({ 0.35f, 0.35f, 0.35f });
    energySword ->transform.Move    ({ 0, 13.85f, 0 });
    energySword ->transform.Rotate  ({ PI/2, PI/2, 0 });
    energySword ->transform.SetScale({ 0.17f, 0.17f, 0.17f });
    palutena    ->transform.Move    ({ 4.5, 0, 5.f });
    palutena    ->transform.SetScale({ 9, 9, 9 });
    staff       ->transform.Move    ({ 0, 0.54f, 0 });
    staff       ->transform.Rotate  ({ 0, 0, PI/2 });
    alduin      ->transform.Move    ({ -20, 1.5f, 24 });
    alduin      ->transform.Rotate  ({ 0, PI/6, -PI/18 });
    alduin      ->transform.SetScale({ 0.05f, 0.05f, 0.05f });
    mercury     ->transform.Move    ({ 0, 100, 0 });
    mercury     ->transform.Rotate  ({ PI/8, 0, PI/8 });
    mercury     ->transform.SetScale({ 75, 75, 75 });
    asteroidBelt->transform.Move    ({ 0, 100, 0 });
    asteroidBelt->transform.Rotate  ({ PI/8, 0, PI/8 });

    // Update bounding boxes.
    cubeCollider0 ->boundingSphere.Update(cubeCollider0,  primCube0);
    cubeCollider1 ->boundingSphere.Update(cubeCollider1,  player   );
    cubeCollider2 ->boundingSphere.Update(cubeCollider2,  primCube2);
    cubeCollider3 ->boundingSphere.Update(cubeCollider3,  primCube3);
    cubeCollider4 ->boundingSphere.Update(cubeCollider4,  primCube4);
    cubeCollider5 ->boundingSphere.Update(cubeCollider5,  primCube5);
    cubeCollider6 ->boundingSphere.Update(cubeCollider6,  primCube6);
    cubeCollider7 ->boundingSphere.Update(cubeCollider7,  primCube7);
    cubeCollider8 ->boundingSphere.Update(cubeCollider8,  primCube8);
    cubeCollider9 ->boundingSphere.Update(cubeCollider9,  primCube9);
    bananaCollider->boundingSphere.Update(bananaCollider, banana);

    // Scripts.
    skybox      ->AddScript((ObjectScript*)new PyScript("Scripts/SkyboxRotation.py"),   this);
    player      ->AddScript((ObjectScript*)new PyScript("Scripts/Player.py"),           this);
    sceneCamera ->AddScript((ObjectScript*)new PyScript("Scripts/CameraScript.py"),     this);
    killZone    ->AddScript((ObjectScript*)new PyScript("Scripts/KillZone.py"),         this);
    mercury     ->AddScript((ObjectScript*)new PyScript("Scripts/RotateObject.py"),   this);
    asteroidBelt->AddScript((ObjectScript*)new AsteroidRotation(), this);
}

void App::Benchmark()
{
    if (!chronoStarted)
    {
        chronoStarted = true;
        shouldReloadScene = true;
        begin = std::chrono::steady_clock::now();
    }

    if (resourceManager.AreAllResourcesLoaded())
    {
        
        if (cptLoad >= maxLoad &&resourceManager.AreAllResourcesInOpenGL())
        {
            end = std::chrono::steady_clock::now();
            std::string strTime = "Average time for loading resources : ";
            strTime += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>((end - begin) / maxLoad).count());
            strTime += " ms \n";
            DebugLog(strTime);
    
            cptLoad = 0;
            isInBenchmark = false;
            chronoStarted = false;
            chronoHasEnded = true;
        }
        else
        {
            shouldReloadScene = true;
        }
    }

}

void App::LoadBenchmark()
{
    LoadResources();
    cptLoad++;
}

void App::UnloadScene()
{
    sceneGraph   .ClearAll();
    cameraManager.ClearCameras();
    lightManager .ClearLights();
}

bool App::ReloadAll()
{
    if (UnloadResources())
    {
        LoadResources();
        UnloadScene();
        LoadExampleScene();
        return true;
    }
    return false;
}

void App::SetWindowSize(const int& width, const int& height, const bool& resizeUi)
{
    windowWidth  = width;
    windowHeight = height;
    postProcessor.SetFramebufferSize(width, height);
    if (resizeUi)
        Ui::FramebufferResizeCallback(width, height);
}
