#pragma once

#include "Vector3.h"
#include "Matrix.h"

namespace Core::Maths
{
    class Transform
    {
	private:
		Core::Maths::Vector3 pos, rot, scale;
		Core::Maths::Mat4 modelMat;

	public:
		Core::Maths::Mat4 parentMat = Core::Maths::Mat4(true);
		bool isCamera = false;
		Transform(bool _isCamera = false);

        // Position.
		Core::Maths::Vector3 GetPosition() const;
		void SetPosition(const Core::Maths::Vector3& position);
		void Move(const Core::Maths::Vector3& movement);

		// Forward.
		Core::Maths::Vector3 Forward() const;
		Core::Maths::Vector3 Up()      const;
		Core::Maths::Vector3 Right()   const;
		void SetForward(const Core::Maths::Vector3& forward);

		// Rotation.
		Core::Maths::Vector3 GetRotation() const;
		void SetRotation(const Core::Maths::Vector3& rotation);
		void Rotate(const Core::Maths::Vector3& rotation);

		// Scale.
		Core::Maths::Vector3 GetScale() const;
		Core::Maths::Vector3 GetUniformScale() const;
		void SetScale(const Core::Maths::Vector3& _scale);

		// Matrices.
		Core::Maths::Mat4 GetModelMat() const;

	private:
		void UpdateModelMat();
    };
}