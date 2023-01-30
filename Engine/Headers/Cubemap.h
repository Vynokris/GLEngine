#pragma once

#include "IResource.h"

namespace Resources
{
	class Texture;

	enum class CubeSides
	{
		Right,
		Left,
		Top,
		Bottom,
		Front,
		Back,
	};

	class Cubemap : public IResource
	{
	private:
        unsigned int id = 0;
		std::string sideTextureNames[6] = {};

	public:
		Cubemap(const std::string& _name);
        ~Cubemap();

        void Load() override;
        void SendToOpenGL() override;

		void SetTexture(const CubeSides& side, std::string name);

        unsigned int GetId() { return id; }
        static ResourceTypes GetResourceType() { return ResourceTypes::Cubemap; }
	};
}