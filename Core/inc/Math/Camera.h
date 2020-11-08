#pragma once

#include "Vector3.h"

namespace Math
{

template <typename T>
struct Camera
{
	Math::Vector3<T> position{ 0, 0, 0 };
	T yaw = 0; // left/right
	T pitch = 0; // up/down
};

using Cameraf = Camera<float>;

}