#include "Primitive.h"
#include "Maths.h"
#include "SceneNode.h"
using namespace Core::Maths;
using namespace Core::Physics;
using namespace Scenes;


// ----- Bounding spheres ----- //

BoundingSphere::BoundingSphere()
{
    radius = 0;
}

BoundingSphere::BoundingSphere(const Primitive* primitive, const SceneNode* node)
{
	radius = 0;
    Update(primitive, node);
}

void BoundingSphere::Update(const Primitive* primitive, const SceneNode* node)
{
	if (primitive->transform == nullptr)
		return;

	Vector3 scale = primitive->transform->GetScale() * node->transform.GetScale();

	switch (primitive->type)
	{
	case PrimitiveTypes::Sphere:
		radius = std::max(scale.x, std::max(scale.y, scale.z)) / 2;
		break;
	case PrimitiveTypes::Cube:
		radius = scale.getLength();
		break;
	case PrimitiveTypes::Capsule:
		radius = scale.y / 2;
		break;
	default:
		break;
	}
}


// ----- Primitives ----- //

Primitive::Primitive(PrimitiveTypes _type, bool allocTransform)
{
	type = _type;
	transformAllocated = allocTransform;
	if (transformAllocated)
		transform = new Transform;
}

Primitive::~Primitive()
{
	if (transformAllocated)
		delete transform;
}


// ----- Primitive buffers ----- //

unsigned int PrimitiveBuffers::buffers[3][3];
int PrimitiveBuffers::vertexCount[3];
std::vector<Core::Maths::Vector3> PrimitiveBuffers::vertices[3];


void PrimitiveBuffers::CreateBuffers()
{
	CreateSphereBuffers();
	CreateCubeBuffers();
	CreateCapsuleBuffers();
}

unsigned int* PrimitiveBuffers::GetVAO(PrimitiveTypes type)
{
	return &buffers[(int)type][0];
}

unsigned int* PrimitiveBuffers::GetVBO(PrimitiveTypes type)
{
	return &buffers[(int)type][1];
}

unsigned int* PrimitiveBuffers::GetEBO(PrimitiveTypes type)
{
	return &buffers[(int)type][2];
}

int& PrimitiveBuffers::GetVerticeCount(PrimitiveTypes type)
{
	return vertexCount[(int)type];
}

void PrimitiveBuffers::SendVerticesToOpenGL(PrimitiveTypes type, const std::vector<Vertex>& vertexBuffer, const std::vector<unsigned int>& indexBuffer)
{
	// Store the number of vertices in the model.
	vertexCount[(int)type] = (unsigned int)vertexBuffer.size();

	unsigned int* vao = nullptr; vao = GetVAO(type);
	unsigned int* vbo = nullptr; vbo = GetVBO(type);
	unsigned int* ebo = nullptr; ebo = GetEBO(type);

	// Create and bind the Vertex Array Object.
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	// Create, bind and set the vertex 
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount[(int)type] * sizeof(Vertex), vertexBuffer.data(), GL_STATIC_DRAW);

	// Create, bind and set the index 
	glGenBuffers(1, ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int), indexBuffer.data(), GL_STATIC_DRAW);

	// Set the position attribute pointer.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Set the uv attribute pointer.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set the normal attribute pointer.
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	for (Vertex vertex : vertexBuffer)
	{
		Vector3 pos = { round(vertex.pos.x * 100) / 100, round(vertex.pos.y * 100) / 100, round(vertex.pos.z * 100) / 100 };
		if (std::find(vertices[(int)type].begin(), vertices[(int)type].end(), pos) == vertices[(int)type].end())
			vertices[(int)type].push_back(pos);
	}
}

void PrimitiveBuffers::CreateSphereBuffers()
{
	static const float lat = 10, lon = 4;
	const float r = 0.5f;

	std::vector<Vertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	for (float j = 0; j < lat; ++j)
	{
		float theta0 = ( j      / lat) * 2 * PI;
		float theta1 = ((j + 1) / lat) * 2 * PI;

		for (float i = 0; i < lon; ++i)
		{
			float phi0 = ( i      / lon) * PI;
			float phi1 = ((i + 1) / lon) * PI;

			// Get coordinates.
			Vector3 p0 = getSphericalCoords(r, theta0, phi0);
			Vector3 p1 = getSphericalCoords(r, theta0, phi1);
			Vector3 p2 = getSphericalCoords(r, theta1, phi1);
			Vector3 p3 = getSphericalCoords(r, theta1, phi0);

			// Get UVs.
			Vector2 u0 = {  j    / lat,  i    / lon };
			Vector2 u1 = {  j    / lat, (i+1) / lon };
			Vector2 u2 = { (j+1) / lat, (i+1) / lon };
			Vector2 u3 = { (j+1) / lat,  i    / lon };

			// Get normals.
			Vector3 n0 = p0.getNormalized();
			Vector3 n1 = p1.getNormalized();
			Vector3 n2 = p2.getNormalized();
			Vector3 n3 = p3.getNormalized();

			// Create the first triangle.
			if (!(j == 0 || j == lat/2))
			{
				vertexBuffer.push_back({ p0, u0, n0 });
				if (j >= lat/2) {
					vertexBuffer.push_back({ p1, u1, n1 });
					vertexBuffer.push_back({ p2, u2, n2 });
				}
				else {
					vertexBuffer.push_back({ p2, u2, n2 });
					vertexBuffer.push_back({ p1, u1, n1 });
				}
			}

			// Create the second triangle.
			if (!(j == lat - 1 || j == lat/2-1))
			{
				vertexBuffer.push_back({ p0, u0, n0 });
				if (j >= lat/2) {
					vertexBuffer.push_back({ p2, u2, n2 });
					vertexBuffer.push_back({ p3, u3, n3 });
				}
				else {
					vertexBuffer.push_back({ p3, u3, n3 });
					vertexBuffer.push_back({ p2, u2, n2 });
				}
			}
		}
	}

	// Fill index buffer.
	for (int i = 0; i < vertexBuffer.size(); i++)
		indexBuffer.push_back(i);

	SendVerticesToOpenGL(PrimitiveTypes::Sphere, vertexBuffer, indexBuffer);
}

void PrimitiveBuffers::CreateCubeBuffers()
{
	Mat4 m = Mat4(true);

	std::vector<Vertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	// Lambda that creates a quad with the given matrix.
	auto createQuad = [&vertexBuffer](Mat4& m) -> void
	{
		Vector3 pos[4];
		pos[0] = (Vector4(-0.5f, -0.5f, -0.5f, 1) * m).toVector3();
		pos[1] = (Vector4( 0.5f, -0.5f, -0.5f, 1) * m).toVector3();
		pos[2] = (Vector4( 0.5f,  0.5f, -0.5f, 1) * m).toVector3();
		pos[3] = (Vector4(-0.5f,  0.5f, -0.5f, 1) * m).toVector3();

		Vector2 uv[4];
		uv[0] = { 0, 0 };
		uv[1] = { 1, 0 };
		uv[2] = { 1, 1 };
		uv[3] = { 0, 1 };

		Vector3 normal = (Vector4(0, 0, -1, 1) * m).toVector3().getNormalized();

		vertexBuffer.push_back({ pos[0], uv[0], normal });
		vertexBuffer.push_back({ pos[2], uv[2], normal });
		vertexBuffer.push_back({ pos[1], uv[1], normal });
		vertexBuffer.push_back({ pos[2], uv[2], normal });
		vertexBuffer.push_back({ pos[0], uv[0], normal });
		vertexBuffer.push_back({ pos[3], uv[3], normal });
	};

	// Create the 4 vertical faces.
	for (int i = 0; i < 4; i++)
	{
		createQuad(m);
		m *= GetRotationMatrix({ 0,PI / 2,0 });
	}

	// Create the top and bottom faces.
	for (int i = 0; i < 2; i++)
	{
		m *= GetRotationMatrix({ PI / 2, 0, 0 });
		createQuad(m);
		m *= GetRotationMatrix({ PI/2 , 0,0 });
	}

	// Fill index buffer.
	for (int i = 0; i < vertexBuffer.size(); i++)
		indexBuffer.push_back(i);

	SendVerticesToOpenGL(PrimitiveTypes::Cube, vertexBuffer, indexBuffer);
}

void PrimitiveBuffers::CreateCapsuleBuffers()
{
	std::vector<Vertex> vertexBuffer;
	std::vector<unsigned int> indexBuffer;

	const Vector3 capStart  = { 0, -0.5, 0 };
	const Vector3 capEnd    = { 0, 0.5f, 0 };
	const float   capRadius = 0.5f;
	const Vector3 axis   = capEnd - capStart;
	const float   length = axis.getLength();
	const Vector3 localZ = axis / length;
	const Vector3 localX = ((localZ.y != 0.0f || localZ.z != 0.0f) ? Vector3(1, 0, 0) : Vector3(0, 1, 0));
	const Vector3 localY = localZ ^ localX;

	const Vector3 start      = { 0 };
	const Vector3 end        = { 1 };
	const float   lat = 6, lon = 2;
	const Vector3 stepLat = (end - start) / lat;
	const Vector3 stepLon = (end - start) / lon;

	auto cylinder = [localX, localY, localZ, length, capStart, capRadius](const float u, const float v) -> Vector3
	{
			return capStart + localX * cos(2.0f * PI * u) * capRadius
				            + localY * sin(2.0f * PI * u) * capRadius
				            + localZ * v * length;

	};

	auto sphereStart = [localX, localY, localZ, capStart, capRadius](const float u, const float v) -> Vector3 
	{
			const float latitude = (PI / 2.0f) * (v - 1);

			return capStart + localX * cos(2.0f * PI * u) * cos(latitude) * capRadius
				            + localY * sin(2.0f * PI * u) * cos(latitude) * capRadius
				            + localZ * sin(latitude) * capRadius;
	};

	auto sphereEnd = [localX, localY, localZ, capEnd, capRadius](const float u, const float v) -> Vector3 
	{
		const float latitude = (PI / 2.0f) * v;
		return capEnd + localX * cos(2.0f * PI * u) * cos(latitude) * capRadius
			          + localY * sin(2.0f * PI * u) * cos(latitude) * capRadius
			          + localZ * sin(latitude) * capRadius;
	};

	for (float i = 0; i < lat; i++) 
	{
		const float u  = i * stepLat.x + start.x;
		const float un = (i + 1 == lat) ? end.x : (i + 1) * stepLat.x + start.x;

		for (float j = 0; j < lon; j++) 
		{
			const float v  = j * stepLon.y + start.y;
			const float vn = (j + 1 == lon) ? end.y : (j + 1) * stepLon.y + start.y;

			// Create cylinder.
			{
				const Vector3 p0 = cylinder(u, v);
				const Vector3 p1 = cylinder(u, vn);
				const Vector3 p2 = cylinder(un, v);
				const Vector3 p3 = cylinder(un, vn);

				const Vector2 u0 = {  i    / lat,  j    / lon };
				const Vector2 u1 = {  i    / lat, (j+1) / lon };
				const Vector2 u2 = { (i+1) / lat,  j    / lon };
				const Vector2 u3 = { (i+1) / lat, (j+1) / lon };

				const Vector3 n0 = p0.getNormalized();
				const Vector3 n1 = p1.getNormalized();
				const Vector3 n2 = p2.getNormalized();
				const Vector3 n3 = p3.getNormalized();

				vertexBuffer.push_back({ p0, u0, n0 });
				vertexBuffer.push_back({ p2, u2, n2 });
				vertexBuffer.push_back({ p1, u1, n1 });
				vertexBuffer.push_back({ p3, u3, n3 });
				vertexBuffer.push_back({ p1, u1, n1 });
				vertexBuffer.push_back({ p2, u2, n2 });
			}

			// Create sphere start.
			{
				const Vector3 p0 = sphereStart(u, v);
				const Vector3 p1 = sphereStart(u, vn);
				const Vector3 p2 = sphereStart(un, v);
				const Vector3 p3 = sphereStart(un, vn);

				const Vector2 u0 = {  i    / lat,  j    / lon };
				const Vector2 u1 = {  i    / lat, (j+1) / lon };
				const Vector2 u2 = { (i+1) / lat,  j    / lon };
				const Vector2 u3 = { (i+1) / lat, (j+1) / lon };

				const Vector3 n0 = p0.getNormalized();
				const Vector3 n1 = p1.getNormalized();
				const Vector3 n2 = p2.getNormalized();
				const Vector3 n3 = p3.getNormalized();

				vertexBuffer.push_back({ p0, u0, n0 });
				vertexBuffer.push_back({ p2, u2, n2 });
				vertexBuffer.push_back({ p1, u1, n1 });
				vertexBuffer.push_back({ p3, u3, n3 });
				vertexBuffer.push_back({ p1, u1, n1 });
				vertexBuffer.push_back({ p2, u2, n2 });
			}

			// Create sphere end.
			{
				const Vector3 p0 = sphereEnd(u, v);
				const Vector3 p1 = sphereEnd(u, vn);
				const Vector3 p2 = sphereEnd(un, v);
				const Vector3 p3 = sphereEnd(un, vn);

				const Vector2 u0 = {  i    / lat,  j    / lon };
				const Vector2 u1 = {  i    / lat, (j+1) / lon };
				const Vector2 u2 = { (i+1) / lat,  j    / lon };
				const Vector2 u3 = { (i+1) / lat, (j+1) / lon };

				const Vector3 n0 = p0.getNormalized();
				const Vector3 n1 = p1.getNormalized();
				const Vector3 n2 = p2.getNormalized();
				const Vector3 n3 = p3.getNormalized();

				vertexBuffer.push_back({ p0, u0, n0 });
				vertexBuffer.push_back({ p2, u2, n2 });
				vertexBuffer.push_back({ p1, u1, n1 });
				vertexBuffer.push_back({ p3, u3, n3 });
				vertexBuffer.push_back({ p1, u1, n1 });
				vertexBuffer.push_back({ p2, u2, n2 });
			}
		}
	}

	// Fill index buffer.
	for (int i = 0; i < vertexBuffer.size(); i++)
		indexBuffer.push_back(i);

	SendVerticesToOpenGL(PrimitiveTypes::Capsule, vertexBuffer, indexBuffer);

}


std::string Primitive::GetPrimitiveName(PrimitiveTypes type)
{
	switch (type)
	{
	case Core::Physics::PrimitiveTypes::Sphere:
		return "Sphere";
	case Core::Physics::PrimitiveTypes::Cube:
		return "Cube";
	case Core::Physics::PrimitiveTypes::Capsule:
		return "Capsule";
	default:
		return "";
	}
}