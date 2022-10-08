#pragma once
#include "Maths.h"
#include "Physics.h"

namespace Core
{
    struct AppInputs;
    class  Time;
    class  App;
}

namespace Scenes
{
    class SceneNode;
    class PyScript;

    class ObjectScript
    {
    private:
        bool isPyScript = false;
        bool started    = false;

    public:
        SceneNode*                object    = nullptr;
        Core::Maths::Transform*   transform = nullptr;
        Core::Physics::Rigidbody* rigidbody = nullptr;
        Core::AppInputs*          inputs    = nullptr;
        Core::Time*               time      = nullptr;
        Core::App*                app       = nullptr;

        ObjectScript(const bool& _isPyScript = false) : isPyScript(_isPyScript) {}
        virtual ~ObjectScript() {}
        virtual void ShowInspectorUi() {}

        bool         HasStarted() { return started; }
        void         SetStarted() { started = true; }
        virtual void Start     () {}
        virtual void LateStart () {}
        virtual void Update    () {}
        virtual void LateUpdate() {}

        virtual void OnCollisionEnter() {}
        virtual void OnCollisionStay () {}
        virtual void OnCollisionExit () {}

        bool      IsPyScript()    { return isPyScript; }
        PyScript* GetAsPyScript() { if (IsPyScript()) return (PyScript*)this; return nullptr; }
    };
    
}