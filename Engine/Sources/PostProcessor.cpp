#include "PostProcessor.h"
#include "Textures.h"
#include "Shader.h"
#include "Debug.h"
#include "Vector2.h"
#include <glad/glad.h>
using namespace Core::Maths;
using namespace Resources;
using namespace Render;


PostProcessor::~PostProcessor()
{
	if (framebufferProgram) delete framebufferProgram;
	if (renderTexture)      delete renderTexture;
	if (rectVAO != 0)       glDeleteVertexArrays(1, &rectVAO);
	if (rectVBO != 0)       glDeleteBuffers(1, &rectVBO);
}

void PostProcessor::Setup(const int& width, const int& height)
{
	// Load the framebuffer shaders.
    VertexShader*   framebufferVert = new VertexShader  ("Resources/Shaders/postProcessShader.vert"); framebufferVert->Load(); framebufferVert->SendToOpenGL();
    FragmentShader* framebufferFrag = new FragmentShader("Resources/Shaders/postProcessShader.frag"); framebufferFrag->Load(); framebufferFrag->SendToOpenGL();
	framebufferProgram = new ShaderProgram("FramebufferShaderProgram");
    framebufferProgram->Load();
    framebufferProgram->AttachShaders(2, framebufferVert, framebufferFrag);
    framebufferProgram->SendToOpenGL();
	delete framebufferVert; delete framebufferFrag;
	glUseProgram(framebufferProgram->GetId());
	glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "screenTexture"), 0);

    // Create the framebuffer rectangle's VBO and VAO.
	float rectangleVertices[] = 
	{
		// Coords    // texCoords
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,

		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Create the rendertexture.
	renderTexture = new RenderTexture("PostProcessRenderTexture");
    renderTexture->SetSize(width, height);
    renderTexture->Load();
    renderTexture->SendToOpenGL();
}

void PostProcessor::BeginRender()
{
	if (renderTexture)
		renderTexture->BeginRender();
	else
		DebugLogWarning("Unable to render using post processor: it is not loaded");
}

void PostProcessor::EndRender()
{
	if (renderTexture && framebufferProgram && rectVAO != 0)
	{
		renderTexture->EndRender();
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the post process shaders.
		glUseProgram(framebufferProgram->GetId());
		glUniform2f(glGetUniformLocation(framebufferProgram->GetId(), "screenSize"       ), (float)renderTexture->GetWidth(), (float)renderTexture->GetHeight());
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "grayscale"        ), grayscale        );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "negative"         ), negative         );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "vignette"         ), vignette         );
		glUniform1f(glGetUniformLocation(framebufferProgram->GetId(), "vignetteIntensity"), vignetteIntensity);
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "bloom"            ), bloom            );
		glUniform1f(glGetUniformLocation(framebufferProgram->GetId(), "bloomIntensity"   ), bloomIntensity   );
		glUniform1f(glGetUniformLocation(framebufferProgram->GetId(), "bloomThreshold"   ), bloomThreshold   );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "bloomSpread"      ), bloomSpread      );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "bloom"            ), bloom            );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "blur"             ), blur             );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "blurRadius"       ), blurRadius       );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "toonShading"      ), toonShading      );
		glUniform1i(glGetUniformLocation(framebufferProgram->GetId(), "toonLevels"       ), toonLevels       );

		// Draw the framebuffer rectangle.
		glBindVertexArray(rectVAO);
		glBindTexture(GL_TEXTURE_2D, renderTexture->GetId());
		glDisable(GL_DEPTH_TEST); glDisable(GL_CULL_FACE); // Prevents framebuffer rectangle from being discarded.
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST); glEnable(GL_CULL_FACE);
	}
}

void PostProcessor::SetFramebufferSize(const int& width, const int& height)
{
	if (renderTexture)
		renderTexture->SetSize(width, height);
	else
		Setup(width, height);
}

void Render::PostProcessor::SetClearColor(const Core::Maths::RGBA& color)
{
	if (renderTexture)
		renderTexture->clearColor = color;
}
