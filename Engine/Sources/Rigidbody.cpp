#include "Rigidbody.h"
using namespace Core::Physics;
using namespace Core::Maths;

#include "Ui.h"
#include "App.h"


Rigidbody::Rigidbody(Transform* _transform, bool _isKinematic)
{
	isGrounded  = false;
	transform   = _transform;
	isKinematic = _isKinematic;
}

void Rigidbody::ApplyGravity()
{
	AddAcceleration(gravity);
}

void Rigidbody::Update()
{
	if (!isKinematic)
	{
		ApplyGravity();
		velocity += acceleration * Ui::app->time.DeltaTime();
		transform->Move(velocity * Ui::app->time.DeltaTime());
		velocity *= 0.97f;
	}
}

bool Rigidbody::ResolveCollisions(const std::vector<Primitive*>& colliders, const std::vector<Primitive*>& sceneColliders)
{
	bool isColliding = false;
	bool isCollidingBottom = false;
	for (int i = 0; i < colliders.size(); i++)
	{
		for (int j = 0; j < sceneColliders.size(); j++)
		{
			if (std::find(colliders.begin(), colliders.end(), sceneColliders[j]) != colliders.end())
				continue;
			
			Vector3 collision = Collisions::PrimitivesResolution(colliders[i], sceneColliders[j]);
			if (collision != Vector3())
			{
				isColliding = true;
				isCollidingBottom = (collision.y > 0.00001f);
				if (!isKinematic) {
					transform->Move(collision);
					acceleration = { 0, 0, 0 };
					velocity     = { velocity.x, 0, velocity.z };
				}
			}
		}
	}
	hasCollided = isColliding;
	isGrounded  = isCollidingBottom;
	return hasCollided;
}
