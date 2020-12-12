#pragma once

#if ( __cplusplus <= 201703L )
	// c++17

#include <type_traits>

#define STDX_enable_if( condition ) bool RequiresTag = true, std::enable_if_t<( condition ), int> = 0>

#define STDX_requires( condition ) template <bool RequiresTag = true, std::enable_if_t<( condition ), int> = 0>

#define STDX_concept inline constexpr bool

#else
	// c++20

#define STDX_enable_if( condition ) bool RequiresTag = true> requires( condition )

#define STDX_requires( condition ) requires( condition )

#define STDX_concept concept

#endif