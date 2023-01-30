#version 450 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 viewProjMat;

void main()
{
    TexCoords = aPos;
    gl_Position = viewProjMat * vec4(aPos, 1.0);
}
