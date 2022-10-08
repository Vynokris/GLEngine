#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdarg>

#include "Debug.h"
#include "Shader.h"
#include "ResourceManager.h"
using namespace Core::Debug;
using namespace Resources;



static std::string LoadShader(const std::string& filename)
{
    // Load shader source.
    std::string fileStr;
    {
        std::stringstream fileContents;
        std::fstream f(filename, std::ios_base::in | std::ios_base::app);
        fileContents << f.rdbuf();
        fileStr = fileContents.str();
        f.close();
    }
    return fileStr;
}

static void CompileShader(const unsigned int& id, const ResourceTypes& type, const char* curFile, const char* curFunction, const long& curLine)
{
    glCompileShader(id);

    // Check for shader compile errors.
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::string shaderName = "";
        switch (type)
        {
            case ResourceTypes::VertexShader:   shaderName = "Vertex";   break;
            case ResourceTypes::FragmentShader: shaderName = "Fragment"; break;
            case ResourceTypes::ComputeShader:  shaderName = "Compute";  break;
            default: break;
        }
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        Assertion::DoAssertion(success, curFile, curFunction, curLine, (shaderName + std::string(" shader compilation failed:\n") + infoLog).c_str());
    }
}

static void SendShaderToOpenGL(const ResourceTypes& type, const std::string& sourceCode, unsigned int& id)
{
    // Get the shader's type.
    unsigned int shaderType;
    switch (type)
    {
        case ResourceTypes::VertexShader:   shaderType = GL_VERTEX_SHADER;   break;
        case ResourceTypes::FragmentShader: shaderType = GL_FRAGMENT_SHADER; break;
        case ResourceTypes::ComputeShader:  shaderType = GL_COMPUTE_SHADER;  break;
        default: break;
    }

    // Create and compile shader.
    const char* shaderSource = sourceCode.c_str();
    id = glCreateShader(shaderType);
    glShaderSource(id, 1, &shaderSource, 0);
    CompileShader(id, type, __FILENAME__, __FUNCTION__, __LINE__);
}


// ----- Vertex Shader ----- //

VertexShader::VertexShader(const std::string& filename)
{
    // Set resource parameters.
    name = filename;
    type = ResourceTypes::VertexShader;
}

VertexShader::~VertexShader()
{
    if (WasSentToOpenGL())
        glDeleteShader(id);
}

void VertexShader::Load()
{
    sourceCode = LoadShader(name);
    SetLoadingDone();
}

void VertexShader::SendToOpenGL()
{
    if (IsLoaded() && !WasSentToOpenGL()) {
        SendShaderToOpenGL(type, sourceCode, id);
        SetOpenGLTransferDone();
    }
}


// ----- Fragment Shader ----- //

FragmentShader::FragmentShader(const std::string& filename)
{
    // Set resource parameters.
    name = filename;
    type = ResourceTypes::FragmentShader;
}

FragmentShader::~FragmentShader()
{
    if (WasSentToOpenGL())
        glDeleteShader(id);
}

void FragmentShader::Load()
{
    sourceCode = LoadShader(name);
    SetLoadingDone();
}

void FragmentShader::SendToOpenGL()
{
    if (IsLoaded()&& !WasSentToOpenGL()) {
        SendShaderToOpenGL(type, sourceCode, id);
        SetOpenGLTransferDone();
    }
}


// ----- Compute Shader ----- //

ComputeShader::ComputeShader(const std::string& filename)
{
    // Set resource parameters.
    name = filename;
    type = ResourceTypes::ComputeShader;
}

ComputeShader::~ComputeShader()
{
    if (WasSentToOpenGL())
        glDeleteShader(id);
}

void ComputeShader::Load()
{
    sourceCode = LoadShader(name);
    SetLoadingDone();
}

void ComputeShader::SendToOpenGL()
{
    if (IsLoaded() && !WasSentToOpenGL()) {
        SendShaderToOpenGL(type, sourceCode, id);
        SetOpenGLTransferDone();
    }
}


// ----- Shader Program ----- //

ShaderProgram::ShaderProgram(const std::string& _name)
{
    name = _name;
    type = ResourceTypes::ShaderProgram;
}

void ShaderProgram::AttachShader(IResource* shader)
{
    if (shader->GetType() == ResourceTypes::VertexShader || shader->GetType() == ResourceTypes::FragmentShader || shader->GetType() == ResourceTypes::ComputeShader)
        attachedShaders.push_back(shader);
}

void ShaderProgram::AttachShaders(const int n, ...)
{
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++)
        AttachShader(va_arg(args, IResource*));
    va_end(args);
}

void ShaderProgram::Load()
{
    SetLoadingDone();
}

void ShaderProgram::SendToOpenGL()
{
    // Check that all attached shaders were sent to openGL.
    for (IResource* shader : attachedShaders)
        if (!shader->WasSentToOpenGL())
            return;

    // Create the program, attach all shaders and link.
    id = glCreateProgram();
    for (IResource* shader : attachedShaders) 
    {
        switch (shader->GetType())
        {
        case ResourceTypes::VertexShader:
            glAttachShader(id, ((VertexShader*)shader)->GetId());
            break;
        case ResourceTypes::FragmentShader:
            glAttachShader(id, ((FragmentShader*)shader)->GetId());
            break;
        case ResourceTypes::ComputeShader:
            glAttachShader(id, ((ComputeShader*)shader)->GetId());
            break;
        default:
            break;
        }
    }
    glLinkProgram(id);

    // Check for linking errors.
    int success;
    char infoLog[512];
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        Assert(success, (std::string("Shader program linking failed:\n") + infoLog).c_str());
    }
    SetOpenGLTransferDone();
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}