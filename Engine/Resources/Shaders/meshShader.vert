#version 450 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 tbnMatrix;

uniform mat4 mvpMatrix;
uniform mat4 modelMat;

void main()
{
	gl_Position    = mvpMatrix * vec4(aPos, 1.0);
	FragPos        = (modelMat * vec4(aPos, 1.0)).xyz;
	TexCoords      = aTexCoord;
	Normal         = normalize((modelMat * vec4(aNormal, 0.0))).xyz;
	vec3 Tangent   = normalize(vec3(modelMat * vec4(aTangent,   0.0)));
	vec3 Bitangent = normalize(vec3(modelMat * vec4(aBitangent, 0.0)));
	tbnMatrix      = mat3(Tangent, Bitangent, Normal);
}
