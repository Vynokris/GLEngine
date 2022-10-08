#pragma once
#include "ObjectScript.h"
#include <pybind11/pybind11.h>
#include <filesystem>
#include <string>

namespace Scenes
{
    class PyScript : public ObjectScript
    {
    private:
        std::string name;

    public:
        pybind11::object scriptObject;

        PyScript(const std::string& _name) : ObjectScript(true), name(_name) { Load(); };
        ~PyScript() {}

        void Load();

        void Start     () override;
        void LateStart () override;
        void Update    () override;
        void LateUpdate() override;

        void OnCollisionEnter() override;
        void OnCollisionStay () override;
        void OnCollisionExit () override;
    };
}