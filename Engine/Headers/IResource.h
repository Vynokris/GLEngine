#pragma once
#include <atomic>
#include <string>

namespace Resources
{
    enum class ResourceTypes
    {
        Texture,
        DynamicTexture,
        RenderTexture,
        Cubemap,
        Material,
        Mesh,
        InstancedMesh,
        VertexShader,
        FragmentShader,
        ComputeShader,
        ShaderProgram,
        ObjFile,
        MtlFile,
    };

    class IResource
    {
    protected:
        std::atomic_bool loaded       = false;
        std::atomic_bool sentToOpenGL = false;
        std::string      name = "";
        ResourceTypes    type;

    public:
        virtual ~IResource() {}
        std::string   GetName() const { return name; }
        ResourceTypes GetType() const { return type; }

        virtual void  Load()         = 0;
        virtual void  SendToOpenGL() = 0;
        // virtual void* CreateCopy()   = 0;
        bool          IsLoaded()        const { return loaded.load();       }
        void          SetLoadingDone()        { loaded.store(true);         }
        bool          WasSentToOpenGL() const { return sentToOpenGL.load(); }
        void          SetOpenGLTransferDone() { sentToOpenGL.store(true);   }
    };
}