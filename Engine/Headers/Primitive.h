#pragma once
#include "Maths.h"
#include <vector>

namespace Resources
{
	class ShaderProgram;
	class Material;
}

namespace Scenes
{
	class SceneNode;
}

namespace Core::Physics
{
	class Primitive;
	enum class PrimitiveTypes
	{
		Sphere,
		Cube,
		Capsule,
	};


	class BoundingSphere
	{
	public:
		float radius;
		BoundingSphere();
		BoundingSphere(const Primitive* primitive, const Scenes::SceneNode* node);
		void Update   (const Primitive* primitive, const Scenes::SceneNode* node);
	};


	class Primitive
	{
	protected:
		bool transformAllocated = false;
		const Resources::ShaderProgram* shaderProgram = nullptr;
		      Resources::Material*      material      = nullptr;

	public:
		PrimitiveTypes type;
		Core::Maths::Transform* transform = nullptr;
		BoundingSphere boundingSphere;

		Primitive(PrimitiveTypes _type, bool allocTransform = false);
		~Primitive();

		const Resources::ShaderProgram* GetShaderProgram() { return shaderProgram; }
		      Resources::Material*      GetMaterial()      { return material; }

		void SetShaderProgram(const Resources::ShaderProgram* _shaderProgram) { shaderProgram = _shaderProgram; }
		void SetMaterial     (      Resources::Material*      _material)      { material      = _material; }

		static std::string GetPrimitiveName(PrimitiveTypes);
	};


	class PrimitiveBuffers
	{
	private:
		// buffers [0][ ] = sphere, [1][ ] = cube, [2][ ] = capsule
		//		   [ ][0] = VAO   , [ ][1] = VBO , [ ][2] = EBO
		static unsigned int buffers[3][3]; 
		//VertexCount 0 = vertex Sphere , 1 = vertex Cube , 2 = vertex Capsule
		static int vertexCount[3];

		static void CreateSphereBuffers ();
		static void CreateCubeBuffers   ();
		static void CreateCapsuleBuffers();

		static void SendVerticesToOpenGL(PrimitiveTypes type, const std::vector<Vertex>& vertexBuffer, const std::vector<unsigned int>& indexBuffer);

	public:
		// [0] = sphere, [1] = cube, [2] = capsule.
		static std::vector<Core::Maths::Vector3> vertices[3];

		static void CreateBuffers();
		static unsigned int* GetVAO(PrimitiveTypes type);
		static unsigned int* GetVBO(PrimitiveTypes type);
		static unsigned int* GetEBO(PrimitiveTypes type);
		static int& GetVerticeCount(PrimitiveTypes type);
	};
};