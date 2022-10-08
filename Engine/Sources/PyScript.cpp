#include <pybind11/embed.h>
#include <pybind11/pytypes.h>
#include "PyScript.h"
#include "PyOpaqueClasses.h"
#include "Debug.h"
#include <sstream>
#include <fstream>
#include <algorithm>
using namespace Scenes;


void PyScript::Load()
{
    // Translate the name from filename format to python import format.
    std::string pyImportName = name.c_str();
    if (pyImportName.substr(pyImportName.size() - 3, 3).compare(".py") == 0)
        pyImportName = pyImportName.substr(0, pyImportName.size() - 3);
    std::replace(pyImportName.begin(), pyImportName.end(), '/',  '.');
    std::replace(pyImportName.begin(), pyImportName.end(), '\\', '.');
    size_t lastDotIndex = pyImportName.find_last_of('.') + 1;

    // Import the file as a module with pybind.
    try {
        pybind11::module_ scriptModule = pybind11::module::import(pyImportName.c_str());
        pybind11::object  scriptClass  = scriptModule.attr(pyImportName.substr(lastDotIndex, pyImportName.size() - lastDotIndex).c_str());
        scriptObject = scriptClass();
    }
    catch (pybind11::error_already_set error) {
        Core::Debug::Log::ToFile(Core::Debug::LogTypes::Error, name.c_str(), "", -1, error.what(), true);
    }
}


static void RunPyObjectFunc(const char* scriptName, pybind11::object& object, const char* funcName)
{
    if (!object.ptr() || object.attr("app").is_none()) return;

    try {
        object.attr(funcName)();
    }
    catch (pybind11::error_already_set error) {
        Core::Debug::Log::ToFile(Core::Debug::LogTypes::Error, scriptName, "", -1, error.what(), true);
    }
}

void PyScript::Start() 
{
    scriptObject.attr("object")    = object;
    scriptObject.attr("transform") = transform;
    scriptObject.attr("rigidbody") = rigidbody;
    scriptObject.attr("inputs")    = inputs;
    scriptObject.attr("time")      = time;
    scriptObject.attr("app")       = app;

    RunPyObjectFunc(name.c_str(), scriptObject, "Start");
}

void PyScript::LateStart       () { RunPyObjectFunc(name.c_str(), scriptObject, "LateStart");        }
void PyScript::Update          () { RunPyObjectFunc(name.c_str(), scriptObject, "Update");           }
void PyScript::LateUpdate      () { RunPyObjectFunc(name.c_str(), scriptObject, "LateUpdate");       }
void PyScript::OnCollisionEnter() { RunPyObjectFunc(name.c_str(), scriptObject, "OnCollisionEnter"); }
void PyScript::OnCollisionStay () { RunPyObjectFunc(name.c_str(), scriptObject, "OnCollisionStay");  }
void PyScript::OnCollisionExit () { RunPyObjectFunc(name.c_str(), scriptObject, "OnCollisionExit");  }
