#pragma once
#include "D3D12Lite.h"

class Transform
{
private:
	Vector3 mPosition;
	Vector3 mRotation;
	Vector3 mScale;

public:
	Transform();
	Transform(Vector3);
	Transform(float, float, float);
	~Transform();

	Vector3 GetPosition();
	Vector3 GetRotation();
	Vector3 GetScale();

	void SetPosition(Vector3);
	void SetRotation(Vector3);
	void SetScale(Vector3);
};

