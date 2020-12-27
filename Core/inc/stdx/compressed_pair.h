#pragma once

#include <type_traits>

namespace stdx
{

namespace detail
{

	template <typename T>
	inline constexpr bool is_empty_class_v = std::is_empty_v<T> && std::is_class_v<T>;

	template <int Index, typename T, bool IsEmpty = is_empty_class_v<T>, bool IsReference = std::is_reference_v<T>>
	class compressed_element;

	template <int Index, typename T>
	class compressed_element<Index, T, false, false>
	{
		static_assert( !std::is_reference_v<T> );

	public:
		constexpr compressed_element() = default;
		constexpr compressed_element( const compressed_element& ) = default;
		constexpr compressed_element( compressed_element&& ) = default;

		template <typename U>
		constexpr compressed_element( U&& v ) : m_value( std::forward<U>( v ) ) {}

		constexpr compressed_element& operator=( const compressed_element& ) = default;
		constexpr compressed_element& operator=( compressed_element&& ) = default;

		template <typename U>
		constexpr compressed_element& operator=( U&& v )
		{
			m_value = std::forward<U>( v );
			return *this;
		}

		constexpr T& get() & noexcept { return m_value; }
		constexpr const T& get() const & noexcept { return m_value; }
		constexpr T&& get() && noexcept { return m_value; }
		constexpr const T&& get() const && noexcept { return m_value; }

	private:
		T m_value;
	};

	template <int Index, typename T>
	class compressed_element<Index, T, false, true>
	{
		static_assert( std::is_reference_v<T> );

	public:
		constexpr compressed_element( const compressed_element& ) = default;
		constexpr compressed_element( compressed_element&& ) = default;

		template <typename U>
		constexpr compressed_element( U&& v ) : m_value( std::forward<U>( v ) ) {}

		constexpr T& get() & noexcept { return m_value; }
		constexpr const T& get() const & noexcept { return m_value; }
		constexpr T&& get() && noexcept { return m_value; }
		constexpr const T&& get() const && noexcept { return m_value; }

	private:
		T m_value;
	};

	template <int Index, typename T>
	class compressed_element<Index, T, true, false> : private T
	{
		static_assert( !std::is_reference_v<T> );

	public:
		constexpr compressed_element() = default;
		constexpr compressed_element( const compressed_element& ) = default;
		constexpr compressed_element( compressed_element&& ) = default;

		template <typename U>
		constexpr compressed_element( U&& v ) : T( std::forward<U>( v ) ) {}

		constexpr compressed_element& operator=( const compressed_element& ) = default;
		constexpr compressed_element& operator=( compressed_element&& ) = default;

		template <typename U>
		constexpr compressed_element& operator=( U&& v )
		{
			return *this = compressed_element( std::forward<U>( v ) );
		}

		constexpr T& get() & noexcept { return static_cast<T&>( *this ); }
		constexpr const T& get() const & noexcept { return static_cast<const T&>( *this ); }
		constexpr T&& get() && noexcept { return static_cast<T&&>( *this ); }
		constexpr const T&& get() const && noexcept { return static_cast<const T&&>( *this ); }
	};

} // namespace detail

template <typename T1, typename T2>
class compressed_pair : private detail::compressed_element<0, T1>, private detail::compressed_element<1, T2>
{
public:
	using first_element = detail::compressed_element<0, T1>;
	using second_element = detail::compressed_element<1, T2>;

	using first_type = T1;
	using second_type = T2;

	constexpr compressed_pair() = default;

	constexpr compressed_pair( const compressed_pair& ) = default;

	constexpr compressed_pair( compressed_pair&& ) = default;

	constexpr compressed_pair( const T1& x, const T2& y )
		: first_element( x ), second_element( y )
	{}

	template <typename U1, typename U2>
	constexpr compressed_pair( U1&& x, U2&& y )
		: first_element( std::forward<U1>( x ) ), second_element( std::forward<U2>( y ) )
	{}

	template <typename U1, typename U2>
	constexpr compressed_pair( const compressed_pair<U1, U2>& other )
		: compressed_pair( other.first(), other.second() )
	{}

	template <typename U1, typename U2>
	constexpr compressed_pair( compressed_pair<U1, U2>&& other )
		: compressed_pair( std::move( other ).first(), std::move( other ).second() )
	{}

	constexpr compressed_pair& operator=( const compressed_pair& ) = default;

	constexpr compressed_pair& operator=( compressed_pair&& ) = default;

	template <typename U1, typename U2>
	constexpr compressed_pair& operator=( const compressed_pair<U1, U2>& other )
	{
		first() = other.first();
		second() = other.second();
		return *this;
	}

	template <typename U1, typename U2>
	constexpr compressed_pair& operator=( compressed_pair<U1, U2>&& other )
	{
		first() = std::move( other ).first();
		second() = std::move( other ).second();
		return *this;
	}

	constexpr T1& first() & noexcept
	{
		return first_box::get();
	}

	constexpr const T1& first() const & noexcept
	{
		return first_box::get();
	}

	constexpr T1&& first() && noexcept
	{
		return first_box::get();
	}

	constexpr const T1&& first() const && noexcept
	{
		return first_box::get();
	}

	constexpr T2& second() & noexcept
	{
		return second_box::get();
	}

	constexpr const T2& second() const & noexcept
	{
		return second_box::get();
	}

	constexpr T2&& second() && noexcept
	{
		return second_box::get();
	}

	constexpr const T2&& second() const && noexcept
	{
		return second_box::get();
	}

	constexpr void swap( compressed_pair& other )
	{
		std::swap( first(), other.first() );
		std::swap( second(), other.second() );
	}
};

template <typename T1, typename T2>
compressed_pair( T1&&, T2&& ) -> compressed_pair<std::decay_t<T1>, std::decay_t<T2>>;

template <typename T1, typename T2>
constexpr compressed_pair<std::decay_t<T1>, std::decay_t<T2>> make_compressed_pair( T1&& x, T2&& y )
{
	return compressed_pair{ std::forward<T1>( x ), std::forward<T2>( y ) };
}

template <std::size_t I, typename T1, typename T2>
constexpr auto& get( compressed_pair<T1, T2>& p ) noexcept
{
	if constexpr ( I == 0 )
		return p.first();
	else if constexpr ( I == 1 )
		return p.second();
	else
		static_assert( false, "I is out of bounds" );
}

template <std::size_t I, typename T1, typename T2>
constexpr auto& get( const compressed_pair<T1, T2>& p ) noexcept
{
	if constexpr ( I == 0 )
		return p.first();
	else if constexpr ( I == 1 )
		return p.second();
	else
		static_assert( false, "I is out of bounds" );
}

template <std::size_t I, typename T1, typename T2>
constexpr auto&& get( compressed_pair<T1, T2>&& p ) noexcept
{
	if constexpr ( I == 0 )
		return std::move( p ).first();
	else if constexpr ( I == 1 )
		return std::move( p ).second();
	else
		static_assert( false, "I is out of bounds" );
}

template <std::size_t I, typename T1, typename T2>
constexpr const auto&& get( const compressed_pair<T1, T2>&& p ) noexcept
{
	if constexpr ( I == 0 )
		return std::move( p ).first();
	else if constexpr ( I == 1 )
		return std::move( p ).second();
	else
		static_assert( false, "I is out of bounds" );
}

namespace test
{
struct empty {};
static_assert( sizeof( empty ) == 1 );
static_assert( sizeof( detail::compressed_element<0, empty> ) == 1 );
static_assert( sizeof( compressed_pair<int, empty> ) == sizeof( int ) );
static_assert( sizeof( compressed_pair<empty, int> ) == sizeof( int ) );
// static_assert( sizeof( compressed_pair<empty, empty> ) == 1 );
static_assert( sizeof( compressed_pair<int, int> ) == sizeof( int ) * 2 );
}

} // namespace stdx