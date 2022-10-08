#pragma once

#include "IResource.h"
#include <vector>
#include <string>

namespace Resources
{
    class VertexShader : public IResource
    {
    private:
        unsigned int id = 0;
        std::string sourceCode;

    public:
        VertexShader(const std::string& filename);
        ~VertexShader();

        void Load()         override;
        void SendToOpenGL() override;

        unsigned int GetId() const { return id; }
        static ResourceTypes GetResourceType() { return ResourceTypes::VertexShader; }
    };

    class FragmentShader : public IResource
    {
    private:
        unsigned int id = 0;
        std::string sourceCode;

    public:
        FragmentShader(const std::string& filename);
        ~FragmentShader();

        void Load()         override;
        void SendToOpenGL() override;

        unsigned int GetId() const { return id; }
        static ResourceTypes GetResourceType() { return ResourceTypes::FragmentShader; }
    };

    class ComputeShader : public IResource
    {
    private:
        unsigned int id = 0;
        std::string sourceCode;

    public:
        ComputeShader(const std::string& filename);
        ~ComputeShader();

        void Load()         override;
        void SendToOpenGL() override;

        unsigned int GetId() const { return id; }
        static ResourceTypes GetResourceType() { return ResourceTypes::ComputeShader; }
    };

    class ShaderProgram : public IResource
    {
    private:
        unsigned int id = 0;
        std::vector<IResource*> attachedShaders;

    public :
        ShaderProgram(const std::string& _name);
        ~ShaderProgram();

        void AttachShader(IResource* shader);
        void AttachShaders(const int n, ...);

        void Load()         override;
        void SendToOpenGL() override;

        unsigned int GetId() const { return id; }
        static ResourceTypes GetResourceType() { return ResourceTypes::ShaderProgram; }
    };
}