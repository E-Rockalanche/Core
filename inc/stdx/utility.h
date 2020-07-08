#pragma once

#include <cstdint>
#include <utility>

namespace stdx
{

namespace detail
{

	template <typename T>
	struct fnv_constants {};

	template <>
	struct fnv_constants<uint32_t>
	{
		static constexpr uint32_t prime = 0x01000193;
		static constexpr uint32_t basis = 0x811c9dc5;
	};

	template <>
	struct fnv_constants<uint64_t>
	{
		static constexpr uint64_t prime = 0x00000100000001B3;
		static constexpr uint64_t basis = 0xcbf29ce484222325;
	};

}

template <typename T>
constexpr T hash_fnv1a( stdx::span<const uint8_t> data ) noexcept
{
	T hash = detail::fnv_constants<T>::basis;

	for ( uint8_t byte : data )
	{
		hash = ( hash ^ byte ) * detail::fnv_constants<T>::prime;
	}

	return hash;
}

template <typename T>
inline void hash_combine( std::size_t& seed, const T& value ) noexcept
{
	seed ^= std::hash{}( value ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

}

namespace std
{

template <typename T1, typename T2>
struct hash<std::pair<T1, T2>>
{
	operator()( const pair<T1, T2>& p ) const noexcept
	{
		size_t result = std::hash{}( p.first );
		stdx::hash_combine( result, p.second );
		return result;
	}
};

template <typename T1, typename T2>
constexpr bool operator<( const pair<T1, T2>& lhs, const pair<T1, T2>& rhs )
{
	if ( lhs.first != rhs.first )
		return lhs.first < rhs.first;
	else
		return lhs.second < rhs.second;
}


template <typename T1, typename T2>
constexpr bool operator==( const pair<T1, T2>& lhs, const pair<T1, T2>& rhs )
{
	return lhs.first == rhs.first && lhs.second == rhs.second;
}

}