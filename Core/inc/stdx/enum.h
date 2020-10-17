#pragma once

#include <stdx/algorithm.h>
#include <stdx/bit.h>
#include <stdx/basic_iterator.h>
#include <stdx/int.h>
#include <stdx/reflection.h>
#include <stdx/type_traits.h>

#include <array>
#include <cstdint>
#include <optional>

namespace stdx {

namespace detail {
	template <typename E>
	using bitset_enum_t = decltype( E::is_bitset_enum );
}

template <typename E>
struct is_bitset_enum
{
	static constexpr bool value = std::is_enum_v<E> && is_detected_v<detail::bitset_enum_t, E>;
};

template <typename E>
constexpr bool is_bitset_enum_v = is_bitset_enum<E>::value;

template <typename E>
struct enum_range_traits
{
	static_assert( std::is_enum_v<E> );

	static constexpr int64_t min = -8;
	static constexpr int64_t max = 127;
};

namespace detail {

	template <typename E>
	constexpr int64_t reflected_min() noexcept
	{
		return static_cast<int64_t>( enum_range_traits<E>::min );
	}

	template <typename E>
	constexpr int64_t reflected_max() noexcept
	{
		return static_cast<int64_t>( enum_range_traits<E>::max );
	}

	template <typename E>
	constexpr std::size_t reflected_size() noexcept
	{
		static_assert( reflected_min<E>() <= reflected_max<E>() );
		return static_cast<std::size_t>( reflected_max<E>() - reflected_min<E>() + 1 );
	}

#pragma warning( push )
#pragma warning( disable : 4293 )
	// allow undefined behaviour of left shifting 1 to obtain value of 0

	template <typename E, int64_t... Is>
	constexpr auto make_values( std::integer_sequence<int64_t, Is...> ) noexcept
	{
		if constexpr ( !is_bitset_enum_v<E> )
		{
			constexpr std::array<bool, sizeof...( Is )> test_results
			{
				{ ( reflection::value_name_v<E, static_cast<E>( Is + reflected_min<E>() )>.size() != 0 )... }
			};

			constexpr std::size_t valid_count = ( static_cast<std::size_t>( test_results[ Is ] ) + ... );
			std::array<E, valid_count> enum_values{};

			for ( std::size_t test_index = 0, i = 0; i < valid_count; ++test_index )
			{
				if ( test_results[ test_index ] )
				{
					enum_values[ i++ ] = static_cast<E>( static_cast<int64_t>( test_index ) + reflected_min<E>() );
				}
			}

			return enum_values;
		}
		else
		{
			using unsigned_type = std::make_unsigned_t<std::underlying_type_t<E>>;

			constexpr std::array<bool, sizeof...( Is )> test_results
			{
				{ ( reflection::value_name_v<E, static_cast<E>( unsigned_type( 1 ) << Is )>.size() != 0 )... }
			};

			constexpr std::size_t valid_count = ( static_cast<int>( test_results[ Is ] ) + ... );
			std::array<E, valid_count> enum_values{};

			for ( std::size_t test_index = 0, i = 0; i < valid_count; ++test_index )
			{
				if ( test_results[ test_index ] )
				{
					enum_values[ i++ ] = static_cast<E>( unsigned_type( 1 ) << test_index );
				}
			}

			return enum_values;
		}
	}

#pragma warning( pop )
}

template <typename E>
struct enum_values
{
	static constexpr auto value = detail::make_values<E>( std::make_integer_sequence<int64_t, is_bitset_enum_v<E> ? ( 1 + bit_sizeof<E>() ) : detail::reflected_size<E>()>{} );
};

template <typename E>
constexpr auto enum_values_v = enum_values<E>::value;

constexpr std::size_t enum_index_npos = std::numeric_limits<std::size_t>::max();

template <typename E>
struct enum_count
{
	static constexpr std::size_t value = enum_values_v<E>.size();
};

template <typename E>
constexpr std::size_t enum_count_v = enum_count<E>::value;

template <typename E>
struct enum_min
{
	static constexpr std::underlying_type_t<E> value = ( enum_count_v<E> > 0 ) ? static_cast<std::underlying_type_t<E>>( enum_values_v<E>.front() ) : 0;
};

template <typename E>
constexpr auto enum_min_v = enum_min<E>::value;

template <typename E>
struct enum_max
{
	static constexpr std::underlying_type_t<E> value = ( enum_count_v<E> > 0 ) ? static_cast<std::underlying_type_t<E>>( enum_values_v<E>.back() ) : 0;
};

template <typename E>
constexpr auto enum_max_v = enum_max<E>::value;

template <typename E>
struct enum_range
{
	static constexpr std::size_t value = static_cast<std::size_t>( enum_max_v<E> - enum_min_v<E> + 1 );
};

template <typename E>
constexpr std::size_t enum_range_v = enum_range<E>::value;

template <typename E>
struct is_dense_enum
{
	static constexpr bool value = ( enum_count_v<E> == enum_range_v<E> );
};

template <typename E>
constexpr bool is_dense_enum_v = is_dense_enum<E>::value;

template <typename E>
struct is_sparse_enum
{
	static constexpr bool value = enum_count_v<E> != enum_range_v<E>;
};

template <typename E>
constexpr bool is_sparse_enum_v = is_sparse_enum<E>::value;

template <typename E>
struct enum_mask
{
	static_assert( is_bitset_enum_v<E>, "E must be a bitset to have a mask" );

	static constexpr E value = static_cast<E>( ( enum_max_v<E> << 1 ) - 1 );
};

template <typename E>
constexpr E enum_mask_v = enum_mask<E>::value;

namespace detail
{

	template <typename E, int... Is>
	constexpr auto make_names( std::integer_sequence<int64_t, Is...> ) noexcept
	{
		std::array<std::string_view, sizeof...( Is )> names
		{
			{ reflection::value_name_v<E, enum_values_v<E>[ Is ]>... }
		};
		return names;
	}

	template <typename E, int... Is>
	constexpr auto make_pairs( std::integer_sequence<int64_t, Is...> ) noexcept
	{
		std::array<std::pair<E, std::string_view>, sizeof...( Is )> pairs
		{
			{ { enum_values_v[ Is ], reflection::value_name_v<E, enum_values_v<E>[ Is ]> }... }
		};
		return pairs;
	}

}

template <typename E>
struct enum_names
{
	static constexpr auto value = detail::make_names<E>( std::make_integer_sequence<int64_t, enum_count_v<E>>{} );
};

template <typename E>
constexpr auto enum_names_v = enum_names<E>::value;

template <typename E>
struct enum_pairs
{
	static constexpr auto value = detail::make_pairs<E>( std::make_integer_sequence<int64_t, enum_count_v<E>>{} );
};

template <typename E>
constexpr auto enum_pairs_v = enum_pairs<E>::value;

template <typename E>
struct is_scoped_enum
{
	static constexpr bool value = std::is_enum_v<E> && !std::is_convertible_v<E, std::underlying_type<E>>;
};

template <typename E>
constexpr bool is_scoped_enum_v = is_scoped_enum<E>::value;

template <typename E>
struct is_unscoped_enum
{
	static constexpr bool value = std::is_enum_v<E> && std::is_convertible_v<E, std::underlying_type<E>>;
};

template <typename E>
constexpr bool is_unscoped_enum_v = is_unscoped_enum<E>::value;

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
	return enum_index_npos;
}

template <typename E>
constexpr bool enum_contains( E value ) noexcept
{
	return enum_index<E>( value ) != enum_index_npos;
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

	return {};
}

// 
template <typename E>
constexpr std::optional<E> enum_cast( std::underlying_type_t<E> value ) noexcept
{
	const E result = static_cast<E>( value );
	if ( enum_contains<E>( result ) )
		return result;

	return {};
}

template <typename E>
std::underlying_type_t<E> enum_underlying_cast( E value ) noexcept
{
	return static_cast<std::underlying_type_t<E>>( value );
}

// does safe narrowing conversion of enum underlying type value to T
template <typename T, typename E>
constexpr T enum_int_cast( E value ) noexcept
{
	return stdx::narrow_cast<T>( enum_underlying_cast( value ) );
}

template <typename E>
constexpr std::string_view enum_name( E value ) noexcept
{
	const std::size_t index = enum_index( value );
	if ( index < enum_count_v<E> )
		return enum_names_v<E>[ index ];

	return {};
}



// bitfield ops

namespace enum_bitset_ops
{

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E operator~( E value ) noexcept
{
	return static_cast<E>( ~static_cast<std::underlying_type_t<E>>( value ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E operator|( E lhs, E rhs ) noexcept
{
	return static_cast<E>( static_cast<std::underlying_type_t<E>>( lhs ) | static_cast<std::underlying_type_t<E>>( rhs ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E operator&( E lhs, E rhs ) noexcept
{
	return static_cast<E>( static_cast<std::underlying_type_t<E>>( lhs ) & static_cast<std::underlying_type_t<E>>( rhs ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E operator^( E lhs, E rhs ) noexcept
{
	return static_cast<E>( static_cast<std::underlying_type_t<E>>( lhs ) ^ static_cast<std::underlying_type_t<E>>( rhs ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E& operator|=( E& lhs, E rhs ) noexcept
{
	return lhs = lhs | rhs;
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E& operator&=( E& lhs, E rhs ) noexcept
{
	return lhs = lhs & rhs;
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr E& operator^=( E& lhs, E rhs ) noexcept
{
	return lhs = lhs ^ rhs;
}

} // namespace enum_bitset_ops

using namespace enum_bitset_ops;

// bitset testing

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr bool any_of( E flags, E mask ) noexcept
{
	return static_cast<bool>( ( flags & mask ) != static_cast<E>( 0 ) );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr bool all_of( E flags, E mask ) noexcept
{
	return static_cast<bool>( ( flags & mask ) == mask );
}

template <typename E, std::enable_if_t<is_bitset_enum_v<E>, int> = 0>
constexpr bool none_of( E flags, E mask ) noexcept
{
	return static_cast<bool>( ( flags & mask ) == static_cast<E>( 0 ) );
}



template <typename E>
class enum_bitset
{
public:
	static_assert( is_bitset_enum_v<E>, "enum must be a bitset" );

	constexpr enum_bitset() noexcept = default;
	constexpr enum_bitset( E flags ) noexcept : m_value{ flags }
	{
		dbExpects( ( flags & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
	}

	constexpr enum_bitset( const enum_bitset& other ) noexcept = default;

	constexpr size_t count() const noexcept { return stdx::popcount( m_value ); }

	constexpr size_t size() const noexcept { return stdx::bit_width( static_cast<std::underlying_type_t<E>>( enum_mask_v<E> ) ); }

	constexpr bool any_of( E flags ) const noexcept { return stdx::any_of( m_value, flags ); }

	constexpr bool none_of( E flags ) const noexcept { return stdx::none_of( m_value, flags ); }

	constexpr bool all_of( E flags ) const noexcept { return stdx::all_of( m_value, flags ); }

	constexpr bool any() const noexcept { return m_value != static_cast<E>( 0 ); }

	constexpr bool none() const noexcept { return m_value == static_cast<E>( 0 ); }

	constexpr bool all() const noexcept { return stdx::all_of( m_value, enum_mask_v<E> ); }

	constexpr enum_bitset& set() noexcept
	{
		m_value = enum_mask_v<E>;
		return *this;
	}

	constexpr enum_bitset& set( E flags ) noexcept
	{
		m_value |= flags;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset& set( E flags, bool value ) noexcept
	{
		if ( value )
			m_value |= flags;
		else
			m_value &= ~flags;

		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset& reset() noexcept
	{
		m_value = static_cast<E>( 0 );
		return *this;
	}

	constexpr enum_bitset& reset( E flags ) noexcept
	{
		m_value &= ~flags;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset& flip() noexcept
	{
		m_value = ~m_value & enum_mask_v<E>;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset& flip( E flags ) noexcept
	{
		m_value ^= flags;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr std::string to_string() const
	{
		dbExpects( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		std::string str;
		str.reserve( size() );
		for ( E b = 1 << ( size() - 1 ); b != 0; b >>= 1 )
		{
			str += ( m_value & b ) ? '1' : '0';
		}
		return str;
	}

	constexpr unsigned long to_ulong() const noexcept
	{
		static_assert( sizeof( unsigned long ) >= sizeof( E ) );
		dbExpects( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return static_cast<unsigned long>( m_value );
	}

	constexpr unsigned long long to_ullong() const noexcept
	{
		static_assert( sizeof( unsigned long long ) >= sizeof( E ) );
		dbExpects( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return static_cast<unsigned long long>( m_value );
	}

	// operators

	constexpr enum_bitset& operator&=( enum_bitset rhs ) noexcept
	{
		m_value &= rhs.m_value;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset& operator|=( enum_bitset rhs ) noexcept
	{
		m_value |= rhs.m_value;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset& operator^=( enum_bitset rhs ) noexcept
	{
		m_value ^= rhs.m_value;
		dbEnsures( ( m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return *this;
	}

	constexpr enum_bitset operator~() noexcept
	{
		return enum_bitset{ ~m_value & enum_mask_v<E> };
	}

	friend constexpr bool operator==( enum_bitset lhs, enum_bitset rhs ) noexcept
	{
		return lhs.m_value == rhs.m_value;
	}

	friend constexpr bool operator!=( enum_bitset lhs, enum_bitset rhs ) noexcept
	{
		return lhs.m_value != rhs.m_value;
	}

	friend constexpr enum_bitset operator&( enum_bitset lhs, enum_bitset rhs ) noexcept
	{
		return enum_bitset{ lhs.m_value & rhs.m_value };
	}

	friend constexpr enum_bitset operator|( enum_bitset lhs, enum_bitset rhs ) noexcept
	{
		return enum_bitset{ lhs.m_value | rhs.m_value };
	}

	friend constexpr enum_bitset operator^( enum_bitset lhs, enum_bitset rhs ) noexcept
	{
		return enum_bitset{ lhs.m_value ^ rhs.m_value };
	}

	template <typename CharT, typename Traits>
	friend std::basic_istream<CharT, Traits>& operator>>( std::basic_istream<CharT, Traits>& is, enum_bitset& x )
	{
		x = enum_bitset{};
		for ( size_t n = 0; n < size() && !is.eof(); ++n )
		{
			auto c = is.peek();
			if ( c == '0' )
			{
				is.get();
				x.m_value <<= 1;
			}
			else if ( c == '1' )
			{
				is.get();
				x.m_value = ( x.m_value << 1 ) | static_cast<E>( 1 );
			}
			else
			{
				break;
			}
		}
		dbEnsures( ( x.m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		return is;
	}

	template <typename CharT, typename Traits>
	friend std::basic_ostream<CharT, Traits>& operator<<( std::basic_ostream<CharT, Traits>& os, const enum_bitset& x )
	{
		dbExpects( ( x.m_value & ~enum_mask_v<E> ) == static_cast<E>( 0 ) );
		for ( E b = 1 << ( size() - 1 ); b != 0; b >>= 1 )
		{
			if ( x.m_value & b )
				os << '1';
			else
				os << '0';
		}
		return os;
	}

private:
	E m_value = static_cast<E>( 0 );
};



// enum_map

namespace detail
{

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

} // namespace detail

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

	constexpr enum_map() = default;

	constexpr explicit enum_map( const T& value )
		: enum_map( value, std::make_integer_sequence<int64_t, enum_count_v<E>>{} )
	{}

	constexpr enum_map( const enum_map& ) = default;
	constexpr enum_map( enum_map&& ) noexcept = default;

	constexpr enum_map( const std::initializer_list<value_type> init )
	{
#ifdef _DEBUG
		std::array<bool, enum_count_v<E>> checks{};
		std::size_t initCount = 0;
#endif

		for ( const auto& entry : init )
		{
			const auto index = enum_index( entry.first );
			dbExpects( index < size() );
			m_data[ index ] = entry.second;

#ifdef _DEBUG
			dbExpects( !checks[ index ] );
			checks[ index ] = true;
			initCount++;
#endif
		}
#ifdef _DEBUG
		dbEnsures( initCount == enum_count_v<E> );
#endif
	}

	constexpr enum_map& operator=( const enum_map& ) = default;
	constexpr enum_map& operator=( enum_map&& ) noexcept = default;

	constexpr enum_map& operator=( const std::initializer_list<value_type> init )
	{
		return *this = enum_map{ init };
	}

	// iterators
	constexpr iterator begin() noexcept { return iterator( { *this, 0 } ); }
	constexpr iterator end() noexcept { return iterator( { *this, ssize() } ); }
	constexpr const_iterator begin() const noexcept { return const_iterator( { *this, 0 } ); }
	constexpr const_iterator end() const noexcept { return const_iterator( { *this, ssize() } ); }
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

	template<std::size_t>
	static constexpr const mapped_type& value_for_index( const mapped_type& value ) noexcept { return value; }

	template <std::size_t... Is>
	constexpr enum_map( const T& value, std::index_sequence<Is...> )
		: m_data{ value_for_index<Is>( value )... }
	{}

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

using namespace stdx::enum_bitset_ops;

namespace std
{
	template<typename E>
	struct hash<stdx::enum_bitset<E>>
	{
		size_t operator()( stdx::enum_bitset<E> x ) const noexcept
		{
			return std::hash<E>{}( x.value() );
		}
	};
}