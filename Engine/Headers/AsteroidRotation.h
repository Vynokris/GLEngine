#pragma once
#include "ObjectScript.h"
#include "Vector3.h"

namespace Scenes
{
	class SceneInstancedModel;

	class AsteroidRotation : public ObjectScript
	{
	private:
		SceneInstancedModel* instancedModel;
		std::vector<float> selfRotationSpeed;
		std::vector<float> originRotationSpeed;
		
	public:
		void Start () override;
		void Update() override;
	};
}
