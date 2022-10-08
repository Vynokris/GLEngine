#pragma once

#include "Maths.h"
#include "Primitive.h"
#include "Collisions.h"
#include <vector>

namespace Core::Physics
{
	class Rigidbody
	{
	private:
		Core::Maths::Vector3 velocity , acceleration;
		Transform* transform;
		bool isGrounded  = false;
		bool hasCollided = false;
		bool isKinematic = false;

		void ApplyGravity();
		
	public:
		Core::Maths::Vector3 gravity = Core::Maths::Vector3(0, -9.81f, 0);

		Rigidbody(Transform* _transform , bool _isKinematic = false);

		void Update();
		bool ResolveCollisions(const std::vector<Primitive*>& colliders , const std::vector<Primitive*>& sceneColliders);

		Core::Maths::Vector3 GetVelocity()                       { return velocity;               }
		void SetVelocity(Core::Maths::Vector3 _velocity)         { velocity  = _velocity;         }
		void AddVelocity(Core::Maths::Vector3 _velocity)         { velocity += _velocity;         }
		Core::Maths::Vector3 GetAcceleration()                   { return acceleration;           }
		void SetAcceleration(Core::Maths::Vector3 _acceleration) { acceleration  = _acceleration; }
		void AddAcceleration(Core::Maths::Vector3 _acceleration) { acceleration += _acceleration; }

		bool IsGrounded()                    { return isGrounded; }
		bool HasCollided()                   { return hasCollided; }
		bool IsKinematic()                   { return isKinematic; }
		void SetKinematic(bool _isKinematic) { isKinematic = _isKinematic; }
	};
};
