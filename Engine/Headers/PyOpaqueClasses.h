#pragma once
#include "Debug.h"
#include "PyScript.h"
#include "App.h"

PYBIND11_MAKE_OPAQUE(Core::App);
PYBIND11_MAKE_OPAQUE(Core::Time);
PYBIND11_MAKE_OPAQUE(Core::Debug::Assertion);
PYBIND11_MAKE_OPAQUE(Core::Debug::Log);
PYBIND11_MAKE_OPAQUE(Core::Debug::LogFileCache);
PYBIND11_MAKE_OPAQUE(Core::ThreadManager);

PYBIND11_MAKE_OPAQUE(Resources::ResourceManager);
PYBIND11_MAKE_OPAQUE(Resources::IResource);
PYBIND11_MAKE_OPAQUE(Resources::TextureSampler);
PYBIND11_MAKE_OPAQUE(Resources::Texture);
PYBIND11_MAKE_OPAQUE(Resources::DynamicTexture);
PYBIND11_MAKE_OPAQUE(Resources::Material);
PYBIND11_MAKE_OPAQUE(Resources::Mesh);
PYBIND11_MAKE_OPAQUE(Resources::SubMesh);
PYBIND11_MAKE_OPAQUE(Resources::VertexShader);
PYBIND11_MAKE_OPAQUE(Resources::FragmentShader);
PYBIND11_MAKE_OPAQUE(Resources::ComputeShader);
PYBIND11_MAKE_OPAQUE(Resources::ShaderProgram);
PYBIND11_MAKE_OPAQUE(Resources::ObjFile);
PYBIND11_MAKE_OPAQUE(Resources::MtlFile);

PYBIND11_MAKE_OPAQUE(Render::CameraManager);
PYBIND11_MAKE_OPAQUE(Render::Camera);
PYBIND11_MAKE_OPAQUE(Render::EngineCamera);
PYBIND11_MAKE_OPAQUE(Render::LightManager)
PYBIND11_MAKE_OPAQUE(Render::ILight);
PYBIND11_MAKE_OPAQUE(Render::DirLight);
PYBIND11_MAKE_OPAQUE(Render::PointLight);
PYBIND11_MAKE_OPAQUE(Render::SpotLight);

PYBIND11_MAKE_OPAQUE(Scenes::SceneGraph);
PYBIND11_MAKE_OPAQUE(Scenes::SceneNode);
PYBIND11_MAKE_OPAQUE(Scenes::SceneCamera);
PYBIND11_MAKE_OPAQUE(Scenes::SceneDirLight);
PYBIND11_MAKE_OPAQUE(Scenes::ScenePointLight);
PYBIND11_MAKE_OPAQUE(Scenes::SceneSpotLight);
PYBIND11_MAKE_OPAQUE(Scenes::SceneModel);
PYBIND11_MAKE_OPAQUE(Scenes::ScenePrimitive);
PYBIND11_MAKE_OPAQUE(Scenes::ObjectScript);
PYBIND11_MAKE_OPAQUE(Scenes::PyScript);
