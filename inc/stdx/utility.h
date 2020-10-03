#pragma once

#include <string_view>

#include <cstdint>
#include <utility>

namespace stdx
{

// FNV1A hashing

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
constexpr T hash_fnv1a( std::string_view data ) noexcept
{
	static_assert( std::is_unsigned_v<T> );

	T hash = detail::fnv_constants<T>::basis;

	for ( uint8_t byte : data )
	{
		hash = static_cast<T>( ( hash ^ byte ) * static_cast<unsigned long long>( detail::fnv_constants<T>::prime ) );
	}

	return hash;
}

template<>
constexpr uint16_t hash_fnv1a<uint16_t>( std::string_view data ) noexcept
{
	const uint32_t hash = hash_fnv1a<uint32_t>( data );
	return static_cast<uint16_t>( ( hash >> 16 ) ^ ( hash & 0xffff ) );
}

template<>
constexpr uint8_t hash_fnv1a<uint8_t>( std::string_view data ) noexcept
{
	const uint16_t hash = hash_fnv1a<uint16_t>( data );
	return static_cast<uint8_t>( ( hash >> 8 ) ^ ( hash & 0xff ) );
}



template <typename T>
inline void hash_combine( std::size_t& seed, const T& value ) noexcept
{
	seed ^= std::hash<T>{}( value ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

template <auto V>
struct constant
{
	static constexpr auto value = V;
};

template <auto V>
constexpr auto constant_v = constant<V>::value;



namespace detail
{

	template <typename Tuple, typename Func, std::size_t... Is>
	void for_each_in_tuple_imp( Tuple& t, Func f, std::index_sequence<Is...> )
	{
		auto temp = { ( f( std::get<Is>( t ) ), 0 )... };
		(void)temp;
	}

} // namespace detail

template <typename Func, typename... Ts>
void for_each_in_tuple( std::tuple<Ts...>& data, Func f )
{
	detail::for_each_in_tuple_imp( data, f, std::make_index_sequence<sizeof...( Ts )>{} );
}

} // namespace stdx

namespace std
{

template <typename T1, typename T2>
struct hash<std::pair<T1, T2>>
{
	std::size_t operator()( const pair<T1, T2>& p ) const noexcept
	{
		std::size_t result = std::hash{}( p.first );
		stdx::hash_combine( result, p.second );
		return result;
	}
};

} // namespace std