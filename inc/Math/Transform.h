#pragma once

#include <Math/Matrix3.h>

namespace Math
{

template <typename T>
struct Transform
{
	Matrix3<T> rotation;
	Position3<T> position;
};

}