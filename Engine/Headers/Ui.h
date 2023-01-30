#pragma once

#include <imgui/imgui.h>
#include <string>
#include "Maths.h"
#include "Physics.h"

namespace Resources
{
    class ResourceManager;
    class Texture;
    class Material;
}

namespace Render
{
    class LightManager;
}

namespace Scenes
{
    enum class SceneNodeTypes;
    class SceneGraph;
    class SceneNode;
    class Player;
}

namespace Core
{
    class App;

    struct UiWindows {
        enum
        {
            StartMenu,
            SceneGraph,
            Inspector,
            Resources,
            PostProcess,
            Logs,
            Stats,
            Play,
        };
    };

    class Ui
    {
    private:
        static ImVec2 windowPositions [8];
        static ImVec2 windowSizes     [8];
        static bool   windowsCollapsed[8];
        static int    windowWidth, windowHeight;

    public:
        static App* app;
        static bool wireframeMode;
        static bool showColliders;

        static void SetAppPtr                (App* _app);
        static void FramebufferResizeCallback(const int& width, const int& height);
        static void CheckWindowSize          (const int& windowIndex);

        static void ShowNodeNameUi  (std::string&         name, const size_t& id, const Scenes::SceneNodeTypes& type);
        static bool ShowTransformUi (Maths::Transform&    transform, bool isScaleUniform = false);
        static void ShowTextureUi   (Resources::Texture*  texture, float size);
        static void ShowMaterialUi  (Resources::Material* material);
        static void ShowRemoveNodeUi(Scenes::SceneNode*&  node);
        static void ShowPhysicsUi   (Scenes::SceneNode*   node);

		static void ShowStartMenu        ();
		static void ShowOptionsMenu      ();
        static void ShowSceneGraphWindow ();
        static void ShowInspectorWindow  ();
        static void ShowResourcesWindow  ();
        static void ShowPostProcessWindow();
        static void ShowLogsWindow       ();
        static void ShowStatsWindow      ();
        static void ShowPlayWindow       ();
    };
}
