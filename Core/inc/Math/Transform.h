#pragma once

#include "Camera.h"
#include "Matrix.h"
#include "Vector3.h"

#include <cmath>

// transformations assume transposed layout, so matrix multiplication should be in the order of left to right

namespace Math
{

/*
// column vector multiplication
template <typename T>
[[nodiscard]] constexpr Vector3<T> operator*( const Matrix<3, 3, T>& m, const Vector3<T>& v ) noexcept
{
	Vector3<T> result( 0 );
	for ( size_t j = 0; j < 3; ++j )
	{
		auto& sum = result[ j ];
		for ( size_t k = 0; k < 3; ++k )
		{
			sum += m[ j ][ k ] * v[ k ];
		}
	}
	return result;
}

// column vector multiplication
template <typename T>
[[nodiscard]] constexpr Vector3<T> operator*( const Matrix<4, 4, T>& m, const Vector3<T>& v ) noexcept
{
	Vector3<T> result( 0 );
	for ( size_t j = 0; j < 3; ++j )
	{
		auto& sum = result[ j ];
		for ( size_t k = 0; k < 3; ++k )
		{
			sum += m[ j ][ k ] * v[ k ];
		}
		sum += m[ j ][ 3 ]; // * w=1
	}
	return result;
}
*/

// row vector multiplication
template <typename T>
[[nodiscard]] constexpr Vector3<T> operator*( const Vector3<T>& v, const Matrix<3, 3, T>& m ) noexcept
{
	Vector3<T> result( 0 );
	for ( size_t i = 0; i < 3; ++i )
	{
		auto& sum = result[ i ];
		for ( size_t k = 0; k < 3; ++k )
		{
			sum += v[ k ] * m[ k ][ i ];
		}
	}
	return result;
}

// row vector multiplication
template <typename T>
[[nodiscard]] constexpr Vector3<T> operator*( const Vector3<T>& v, const Matrix<4, 4, T>& m ) noexcept
{
	Vector3<T> result( 0 );
	for ( size_t i = 0; i < 3; ++i )
	{
		auto& sum = result[ i ];
		for ( size_t k = 0; k < 3; ++k )
		{
			sum += v[ k ] * m[ k ][ i ];
		}
		sum += m[ 3 ][ i ]; // * w=1
	}
	return result;
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Translate( const Vector3<T>& v ) noexcept
{
	return
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		v.x, v.y, v.z, 1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Translate( const Matrix<4, 4, T>& m, const Vector3<T>& v ) noexcept
{
	Matrix<4, 4, T> result = m;
	for ( size_t i = 0; i < 3; ++i )
		result[ 3 ][ i ] += v[ i ];

	return result;
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Perspective( T fovRadians, T near, T far, T screenWidth, T screenHeight ) noexcept
{
	// world coordinate system is righ handed. Object's in view will have negative z coordinate with respect to camera.
	// must convert negative z value to positive z value for normalized device coordinates
	constexpr T zSign = -1;

	const T aspectRatio = screenWidth / screenHeight;

	const T mapX = 1 / std::tan( fovRadians / 2 ); // = near / right
	// x * mapX / w=z -> [-1, 1]

	const T mapY = mapX * aspectRatio;
	// y * mapY / w=z -> [-1, 1]

	const T depth = far - near;

	const T scaleZ = zSign * ( far + near ) / depth;
	const T cropZ = -2 * far * near / depth;
	// ( z * scaleZ + w=1 * cropZ ) / w=z -> [-1, 1]

	return
	{
		mapX,	0,		0,		0,
		0,		mapY,	0,		0,
		0,		0,		scaleZ,	zSign,
		0,		0,		cropZ,	0
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Scale( T x, T y, T z ) noexcept
{
	return
	{
		x, 0, 0, 0,
		0, y, 0, 0,
		0, 0, z, 0,
		0, 0, 0, 1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Scale( T scale ) noexcept
{
	return Scale( scale, scale, scale );
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Scale( const Vector3<T>& scales ) noexcept
{
	return Scale( scales.x, scales.y, scales.z );
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> RotateX( T angle ) noexcept
{
	const auto c = std::cos( angle );
	const auto s = std::sin( angle );

	return
	{
		1, 0, 0, 0,
		0, c, s, 0,
		0, -s, c, 0,
		0, 0, 0, 1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> RotateY( T angle ) noexcept
{
	const auto c = std::cos( angle );
	const auto s = std::sin( angle );

	return
	{
		c, 0, -s, 0,
		0, 1, 0, 0,
		s, 0, c, 0,
		0, 0, 0, 1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> RotateZ( T angle ) noexcept
{
	const auto c = std::cos( angle );
	const auto s = std::sin( angle );

	return
	{
		c, s, 0, 0,
		-s, c, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> Rotate( T angle, const Math::Normal3<float> axis ) noexcept
{
	const auto x = axis.x;
	const auto y = axis.y;
	const auto z = axis.z;

	const auto xx = x * x;
	const auto yy = y * y;
	const auto zz = z * z;

	const auto xy = x * y;
	const auto xz = x * z;
	const auto yz = y * z;

	const auto c = std::cos( angle );
	const auto s = std::sin( angle );

	return
	{
		xx + ( 1 - xx ) * c,		xy * ( 1 - c ) + z * s,		xz * ( 1 - c ) - y * s,		0,
		xy * ( 1 - c ) - z * s,		yy + ( 1 - yy ) * c,		yz * ( 1 - c ) + x * s,		0,
		xz * ( 1 - c ) + y * s,		yz * ( 1 - c ) - x * s,		zz + ( 1 - zz ) * c,		0,
		0,							0,							0,							1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> LookAt( const Math::Vector3<T>& cameraPosition, const Math::Vector3<T>& targetPosition, const Math::Vector3<T>& cameraUp = { 0, 1, 0 } ) noexcept
{
	const auto forward = Normalize( targetPosition - cameraPosition );
	const auto right = Normalize( CrossProduct( forward, cameraUp ) );
	const auto up = Normalize( CrossProduct( right, forward ) );

	return
	{
		right.x(),								up.x(),								-forward.x(),								0,
		right.y(),								up.y(),								-forward.y(),								0,
		right.z(),								up.z(),								-forward.z(),								0,
		-DotProduct( right, cameraPosition ),	-DotProduct( up, cameraPosition ),	-DotProduct( -forward, cameraPosition ),	1
	};
}

template <typename T>
[[nodiscard]] constexpr Matrix<4, 4, T> CameraView( const Camera<T>& camera ) noexcept
{
	// matrices are transposed, so multiply left to right
	return Translate( -camera.position ) * RotateY( -camera.yaw ) * RotateX( -camera.pitch );
}

}