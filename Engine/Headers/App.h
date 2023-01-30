#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include "Maths.h"
#include "TimeManager.h"
#include "PostProcessor.h"
#include "ResourceManager.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "SceneGraph.h"
#include "TextureSampler.h"

namespace Render
{
    class Camera;
}

namespace Core
{
    class App;

    struct AppInitializer
    {
        unsigned int width, height, major, minor;
        const char* windowName;
        bool maximized = false, vsync = false;
        int fps = 60;
    };

    struct AppInputs
    {
        bool exitKeyPressed  = false;
        bool reloadPyScripts = false;
        bool mouseRightClick = false;
        bool mouseLeftClick  = false;
        float mouseScroll    = 0;
        Core::Maths::Vector2 mousePos;
        Core::Maths::Vector2 mouseDelta;
        Core::Maths::Vector3 movement;
    };

    enum class AppStates {
        StartMenu,
        OptionsMenu,
        Game,
        Editor,
    };

    class App
    {
    private:
        GLFWwindow* window;
        unsigned int windowWidth = 1920, windowHeight = 1080;

        // Play mode.
        bool inSceneView       = false;
        bool inPlayMode        = false;
        bool playModePaused    = false;

        // Variables for chrono async and and synchronous loading.
        std::chrono::steady_clock::time_point begin;
        std::chrono::steady_clock::time_point end;
        bool chronoHasEnded = false;

        // Benchmark resources.
        void LoadBenchmark();
        int  cptLoad            = 0;
        bool chronoStarted      = false;

    public:
        int  maxLoad           = 10;
        bool shouldReloadScene = false;
        bool isInBenchmark     = false;

        AppStates                  state = AppStates::StartMenu;
        AppInputs                  inputs;
        Time                       time;
        Resources::ResourceManager resourceManager;
        Render::LightManager       lightManager;
        Render::CameraManager      cameraManager;
        Render::PostProcessor      postProcessor;
        Scenes::SceneGraph         sceneGraph;

        // Constructor/Destructor.
        App(const AppInitializer& init);
        void InitPyInterpreter();
        ~App();

		// Delete all the copy constructors.
		App(const App&)			   = delete;
		App(App&&)				   = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&)      = delete;

        // The app's render loop.
        void Run();

        // Inputs and rendering.
        void ProcessInputs();
        void BeginRender();
        void EndRender();
        void Render();
        void RenderEditor();
        void RenderGame();

        // Play mode.
        bool InSceneView();
        void ToggleView();
        bool InPlayMode();
        void StartPlayMode();
        void StopPlayMode();
        bool PlayModePaused();
        void PausePlayMode();
        void ResumePlayMode();

        // Resource and scene loading.
        void LoadDefaultResources();
        void LoadResources();
        void SendLoadedResources();
        bool UnloadResources();
        void LoadExampleScene();
        void Benchmark();
        void UnloadScene();
        bool ReloadAll();

        // Getters.
        GLFWwindow*  GetWindow () { return window;       }
        unsigned int GetWindowW() { return windowWidth;  }
        unsigned int GetWindowH() { return windowHeight; }
        void SetWindowSize(const int& width, const int& height, const bool& resizeUi = true);
    };
}