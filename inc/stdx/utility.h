#pragma once

#include <utility>

namespace stdx
{

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