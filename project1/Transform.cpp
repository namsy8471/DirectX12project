#include "Transform.h"

Transform::Transform() : mPosition(Vector3(0,0,0)), mRotation(Vector3(0, 0, 0)), mScale(Vector3(0, 0, 0))
{}

Transform::Transform(Vector3 v3) : Transform()
{
	mPosition = v3;
}

Transform::Transform(float x, float y, float z) : Transform(Vector3(x, y, z)) {}

Transform::~Transform()
{
}

Vector3 Transform::GetPosition()
{
	return mPosition;
}

Vector3 Transform::GetRotation()
{
	return mRotation;
}

Vector3 Transform::GetScale()
{
	return mScale;
}

void Transform::SetPosition(Vector3 v3)
{
	mPosition = v3;
}

void Transform::SetRotation(Vector3 v3)
{
	mRotation = v3;
}

void Transform::SetScale(Vector3 v3)
{
	mScale = v3;
}
