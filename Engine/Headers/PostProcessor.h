#pragma once

namespace Core::Maths
{
	class RGBA;
}

namespace Resources
{
	class RenderTexture;
	class ShaderProgram;
}

namespace Render
{
	class PostProcessor
	{
	private:
		Resources::ShaderProgram* framebufferProgram = nullptr;
		Resources::RenderTexture* renderTexture = nullptr;
		unsigned int rectVAO = 0, rectVBO = 0;

	public:
		bool  grayscale = false, negative = false, vignette = false, bloom = false, blur = false, toonShading = false;
		float vignetteIntensity = .25f, bloomIntensity = .7f, bloomThreshold = .5f;
		int   bloomSpread = 7, blurRadius = 5, toonLevels = 4;

		PostProcessor() {}
		~PostProcessor();

		void Setup(const int& width, const int& height);
		void BeginRender();
		void EndRender();

		void SetFramebufferSize(const int& width, const int& height);
		void SetClearColor(const Core::Maths::RGBA& color);
	};
}