#pragma once
#include <iostream>
#include <vector>
#include "D3D12Lite.h"
#include "Transform.h"

class GameObject
{
private:

	Transform mTransform;
	std::vector<GameObject*> mChildren;

public:
	GameObject();
	GameObject(Transform);
	~GameObject();


};

