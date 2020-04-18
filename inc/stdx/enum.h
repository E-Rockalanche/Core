#pragma once

#include <stdx/algorithm.h>
#include <stdx/basic_iterator.h>
#include <stdx/reflection.h>
#include <stdx/type_traits.h>

#include <array>
#include <optional>

namespace stdx {

template <typename E>
struct enum_range_traits
{
	static_assert( std::is_enum_v<E> );

	static constexpr int min = -8;
	static constexpr int max = 127;
};

namespace detail {
	template <typename E>
	using bitset_enum_t = decltype( E::is_bitset_enum );
}

template <typename E>
constexpr bool is_bitset_enum_v = std::is_enum_v<E> && is_detected_v<detail::bitset_enum_t, E>;

namespace detail {

	template <typename E>
	constexpr int reflected_min() noexcept
	{
		return static_cast<int>( enum_range_traits<E>::min );
	}

	template <typename E>
	constexpr int reflected_max() noexcept
	{
		return static_cast<int>( enum_range_traits<E>::max );
	}

	template <typename E>
	constexpr std::size_t reflected_size() noexcept
	{
		static_assert( reflected_min<E>() <= reflected_max<E>() );
		return static_cast<std::size_t>( reflected_max<E>() - reflected_min<E>() + 1 );
	}

	template <typename E, int... Is>
	constexpr auto make_values( std::integer_sequence<int, Is...> ) noexcept
	{
		constexpr std::array<bool, sizeof...( Is )> test_results
		{
			{ ( reflection::value_name_v<E, static_cast<E>( Is + reflected_min<E>() )>.size() != 0 )... }
		};

		constexpr int valid_count = ( test_results[ Is ] + ... );
		std::array<E, valid_count> enum_values{};

		for ( int test_index = 0, i = 0; i < valid_count; ++test_index )
		{
			if ( test_results[ test_index ] )
			{
				enum_values[ i++ ] = static_cast<E>( test_index + reflected_min<E>() );
			}
		}

		return enum_values;
	}
}

template <typename E>
struct enum_values
{
	static constexpr auto value = detail::make_values<E>( std::make_integer_sequence<int, detail::reflected_size<E>()>{} );
};

template <typename E>
constexpr auto enum_values_v = enum_values<E>::value;

template <typename E>
constexpr std::size_t enum_count_v = enum_values_v<E>.size();

template <typename E>
constexpr int enum_min_v = enum_values_v<E>.empty() ? 0 : static_cast<int>( enum_values_v<E>.front() );

template <typename E>
constexpr int enum_max_v = enum_values_v<E>.empty() ? 0 : static_cast<int>( enum_values_v<E>.back() );

template <typename E>
constexpr std::size_t enum_range_v = static_cast<std::size_t>( enum_max_v<E> - enum_min_v<E> + 1 );

template <typename E>
constexpr bool is_dense_enum_v = enum_count_v<E> == enum_range_v<E>;

template <typename E>
constexpr bool is_sparse_enum_v = enum_count_v<E> != enum_range_v<E>;

namespace detail
{

	template <typename E, int... Is>
	constexpr auto make_names( std::integer_sequence<int, Is...> ) noexcept
	{
		std::array<std::string_view, sizeof...( Is )> names
		{
			{ reflection::value_name_v<E, enum_values_v<E>[ Is ]>... }
		};
		return names;
	}

	template <typename E, int... Is>
	constexpr auto make_pairs( std::integer_sequence<int, Is...> ) noexcept
	{
		std::array<std::pair<E, std::string_view>, sizeof...( Is )> pairs
		{
			{ { enum_values_v[ Is ], reflection::value_name_v<E, enum_values_v<E>[ Is ]> }... }
		};
		return pairs;
	}

}

template <typename E>
constexpr auto enum_names_v = detail::make_names<E>( std::make_integer_sequence<int, enum_count_v<E>>{} );

template <typename E>
constexpr auto enum_pairs_v = detail::make_pairs<E>( std::make_integer_sequence<int, enum_count_v<E>>{} );

template <typename E>
constexpr bool is_scoped_enum_v = std::is_enum_v<E> && !std::is_convertible_v<E, std::underlying_type<E>>;

template <typename E>
constexpr bool is_unscoped_enum_v = std::is_enum_v<E> && std::is_convertible_v<E, std::underlying_type<E>>;

template <typename E>
constexpr std::size_t enum_index( E value )
{
	if constexpr ( is_dense_enum_v<E> )
	{
		if ( enum_min_v<E> <= (int)value && (int)value <= enum_max_v<E> )
			return static_cast<std::size_t>( static_cast<int>( value ) - enum_min_v<E> );
	}
	else
	{
		const auto it = stdx::lower_bound( enum_values_v<E>.begin(), enum_values_v<E>.end(), value );
		if ( *it == value )
			return static_cast<std::size_t>( it - enum_values_v<E>.begin() );
	}
	return (std::size_t)-1;
}

template <typename E>
constexpr bool enum_contains( E value ) noexcept
{
	return enum_index<E>( value ) != ( std::size_t ) - 1;
}

template <typename E>
constexpr E enum_value( std::size_t index )
{
	dbExpects( index < enum_count_v<E> );
	if constexpr ( is_dense_enum_v<E> )
	{
		return static_cast<E>( enum_min_v<E> + static_cast<int>( index ) );
	}
	else
	{
		return enum_values_v<E>[ index ];
	}
}

template <typename E>
constexpr std::optional<E> enum_cast( std::string_view str ) noexcept
{
	for ( auto& pair : enum_pairs_v<E> )
	{
		if ( pair.second == str )
			return pair.first;
	}
	return std::nullopt;
}

template <typename E>
constexpr std::optional<E> enum_cast( std::underlying_type_t<E> value ) noexcept
{
	if ( enum_contains<E>( value ) )
		return static_cast<E>( value );

	return std::nullopt;
}

template <typename E>
constexpr std::string_view enum_name( E value ) noexcept
{
	const auto index = enum_index( value );
	if ( index != (std::size_t)-1 )
		return enum_names_v<E>[ index ];

	return {};
}

// bitfield ops

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E operator~( E value ) noexcept
{
	return static_cast<E>( ~static_cast<std::underlying_type_t<E>>( value ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr auto operator|( E lhs, E rhs ) noexcept
{
	return static_cast<E>( static_cast<std::underlying_type_t<E>>( lhs ) | static_cast<std::underlying_type_t<E>>( rhs ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr auto operator&( E lhs, E rhs ) noexcept
{
	return static_cast<E>( static_cast<std::underlying_type_t<E>>( lhs ) & static_cast<std::underlying_type_t<E>>( rhs ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr auto operator^( E lhs, E rhs ) noexcept
{
	return static_cast<E>( static_cast<std::underlying_type_t<E>>( lhs ) ^ static_cast<std::underlying_type_t<E>>( rhs ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr auto operator|=( E& lhs, E rhs ) noexcept
{
	return lhs = lhs | rhs;
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr auto operator&=( E& lhs, E rhs ) noexcept
{
	return lhs = lhs & rhs;
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr auto operator^=( E& lhs, E rhs ) noexcept
{
	return lhs = lhs ^ rhs;
}

// enum_map

namespace detail {

	template <class Map, typename T>
	class enum_map_cursor
	{
	public:
		using key_type = typename Map::key_type;
		using mapped_type = T;
		using reference = std::pair<const key_type, mapped_type&>;

		constexpr enum_map_cursor() noexcept = default;
		constexpr enum_map_cursor( const enum_map_cursor& ) = default;

		constexpr enum_map_cursor( Map& map, const std::ptrdiff_t index ) noexcept : m_map{ &map }, m_index{ index }
		{
			dbEnsures( 0 <= index && index <= m_map->ssize() );
		}

		template <typename Map2, typename T2>
		constexpr enum_map_cursor( const enum_map_cursor<Map2, T2>& other ) noexcept : m_map{ other.m_map }, m_index{ other.m_index } {}

		constexpr enum_map_cursor& operator=( const enum_map_cursor& ) = default;

		template <typename Map2, typename T2>
		constexpr enum_map_cursor& operator=( const enum_map_cursor<Map2, T2>& other )
		{
			m_map = other.m_map;
			m_index = other.m_index;
		}

		constexpr reference read() const noexcept
		{
			dbExpects( m_map );
			return { m_map->keys()[ m_index ], m_map->values()[ m_index ] };
		}

		constexpr void next() noexcept
		{
			dbExpects( m_map );
			++m_index;
			dbEnsures( m_index <= m_map->ssize() );
		}

		constexpr void prev() noexcept
		{
			dbExpects( m_map );
			--m_index;
			dbEnsures( m_index >= 0 );
		}

		constexpr void advance( const std::ptrdiff_t n ) noexcept
		{
			dbExpects( m_map );
			m_index += n;
			dbEnsures( 0 <= m_index && m_index <= m_map->ssize() );
		}

		constexpr bool equal( const enum_map_cursor& other ) const noexcept
		{
			dbExpects( m_map && m_map == other.m_map );
			return m_index == other.m_index;
		}

		constexpr std::ptrdiff_t distance_to( const enum_map_cursor& other ) const noexcept
		{
			dbExpects( m_map && m_map == other.m_map );
			return other.m_index - m_index;
		}

	private:
		Map* m_map = nullptr;
		std::ptrdiff_t m_index = 0;
	};

}

template <typename E, typename T>
class enum_map
{
public:
	using key_type = E;
	using mapped_type = T;
	using value_type = std::pair<const key_type, mapped_type>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = std::pair<const key_type, mapped_type&>;
	using const_reference = std::pair<const key_type, const mapped_type&>;

	using iterator = basic_iterator<detail::enum_map_cursor<enum_map<E, T>, T>>;
	using const_iterator = basic_iterator<detail::enum_map_cursor<const enum_map<E, T>, const T>>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	constexpr enum_map() noexcept = default;
	constexpr enum_map( const std::initializer_list<value_type> init ) noexcept
	{
		dbExpects( init.size() == enum_count_v<E> );

#ifndef SHIPPING
		std::size_t initialized_count = 0;
		std::array<bool, enum_count_v<E>> checks{};
#endif

		for ( auto& entry : init )
		{
			auto index = enum_index( entry.first );
			dbExpects( index < size() );
			m_data[ index ] = entry.second;

#ifndef SHIPPING
			dbExpects( checks[ index ] == false );
			checks[ index ] = true;
			++initialized_count;
#endif
		}

#ifndef SHIPPING
		dbExpects( initialized_count == enum_count_v<E> );
#endif
	}
	constexpr enum_map( const T& value ) noexcept : enum_map( value, std::make_integer_sequence<int, enum_count_v<E>>{} ) {}
	constexpr enum_map( const enum_map& ) noexcept = default;
	constexpr enum_map( enum_map&& ) noexcept = default;

	enum_map& operator=( const enum_map& ) = default;
	enum_map& operator=( enum_map&& ) = default;

	// iterators
	constexpr iterator begin() noexcept { return iterator( *this, 0 ); }
	constexpr iterator end() noexcept { return iterator( *this, ssize() ); }
	constexpr const_iterator begin() const noexcept { return const_iterator( *this, 0 ); }
	constexpr const_iterator end() const noexcept { return const_iterator( *this, ssize() ); }
	constexpr const_iterator cbegin() const noexcept { return begin(); }
	constexpr const_iterator cend() const noexcept { return end(); }
	constexpr reverse_iterator rbegin() noexcept { return reverse_iterator( end() ); }
	constexpr reverse_iterator rend() noexcept { return reverse_iterator( begin() ); }
	constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator( end() ); }
	constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator( begin() ); }
	constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator( end() ); }
	constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator( begin() ); }

	// capacity
	[[nodiscard]] constexpr bool empty() const noexcept { return enum_count_v<E> == 0; }
	constexpr size_type size() const noexcept { return enum_count_v<E>; }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( size() ); }
	constexpr size_type max_size() const noexcept { return size(); }

	// element access
	constexpr mapped_type& operator[]( const key_type key ) noexcept
	{
		const auto index = enum_index( key );
		dbExpects( index < size() );
		return m_data[ index ];
	}

	constexpr const mapped_type& operator[]( const key_type key ) const noexcept
	{
		const auto index = enum_index( key );
		dbExpects( index < size() );
		return m_data[ index ];
	}

	constexpr const auto& keys() const noexcept { return enum_values_v<E>; }
	constexpr auto& values() noexcept { return m_data; }
	constexpr const auto& values() const noexcept { return m_data; }

	// operations
	constexpr void fill( const T& value ) { m_data.fill( value ); }
	constexpr void swap( enum_map& other ) { m_data.swap( other.m_values ); }

private:

	template<int>
	static constexpr const mapped_type& value_for_index( const mapped_type& value ) noexcept { return value; }

	template <int... Is>
	constexpr enum_map( const T& value, std::integer_sequence<int, Is...> ) : m_data{ value_for_index<Is>( value )... } {}

private:
	std::array<T, enum_count_v<E>> m_data{};
};

template <typename E, typename T>
constexpr bool operator==( const enum_map<E, T>& lhs, const enum_map<E, T>& rhs )
{
	return lhs.values() == rhs.values();
}

template <typename E, typename T>
constexpr bool operator!=( const enum_map<E, T>& lhs, const enum_map<E, T>& rhs )
{
	return lhs.values() != rhs.values();
}

template <typename E, typename T>
constexpr bool operator<( const enum_map<E, T>& lhs, const enum_map<E, T>& rhs )
{
	return lhs.values() < rhs.values();
}

template <typename E, typename T>
constexpr bool operator<=( const enum_map<E, T>& lhs, const enum_map<E, T>& rhs )
{
	return lhs.values() <= rhs.values();
}

template <typename E, typename T>
constexpr bool operator>( const enum_map<E, T>& lhs, const enum_map<E, T>& rhs )
{
	return lhs.values() > rhs.values();
}

template <typename E, typename T>
constexpr bool operator>=( const enum_map<E, T>& lhs, const enum_map<E, T>& rhs )
{
	return lhs.values() >= rhs.values();
}

} // namespace stdx