#include "AsteroidRotation.h"
#include "SceneNode.h"
#include "TimeManager.h"
using namespace Maths;

void Scenes::AsteroidRotation::Start()
{
	instancedModel = (SceneInstancedModel*)object;
	selfRotationSpeed  .reserve(instancedModel->instanceCount);
	originRotationSpeed.reserve(instancedModel->instanceCount);
	for (int i = 0; i < instancedModel->instanceCount; i++)
	{
		selfRotationSpeed  .push_back((rand() % 6 + 4) / 100.f);
		originRotationSpeed.push_back((rand() % 6 + 2) / 100.f);
	}
}

void Scenes::AsteroidRotation::Update()
{
	for (int i = 0; i < instancedModel->instanceCount; i++)
	{
		instancedModel->instanceTransforms[i].Rotate(Vector3(0, 1, 0) * selfRotationSpeed[i] * time->DeltaTime());

		Vector3 originToPos = Vector3(transform->GetPosition(), instancedModel->instanceTransforms[i].GetPosition()); originToPos.y = 0;
		originToPos.rotate(Vector3(0, 1, 0) * originRotationSpeed[i] * time->DeltaTime());
		Vector3 newPos = transform->GetPosition() + originToPos; newPos.y = instancedModel->instanceTransforms[i].GetPosition().y;
		instancedModel->instanceTransforms[i].SetPosition(newPos);
	}
	instancedModel->UpdateMatrixBuffer();
}
