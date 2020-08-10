#pragma once

#include <stdx/basic_iterator.h>
#include <stdx/container.h>
#include <stdx/utility.h>

#include <iterator>

namespace stdx
{

template <typename T>
constexpr T* to_address( T* p ) noexcept
{
	static_assert( !std::is_function_v<T> );
	return p;
}

template <typename T>
constexpr auto to_address( const T& p ) noexcept
{
	return to_address( p.operator->() );
}

template <typename Iterator>
class range
{
public:
	constexpr range() noexcept = default;
	constexpr range( const range& ) noexcept = default;

	constexpr explicit range( Iterator first, Iterator last ) noexcept : m_first{ std::move( first ) }, m_last{ std::move( last ) } {}

	constexpr range& operator=( const range& ) noexcept = default;

	constexpr Iterator begin() const noexcept { return m_first; }
	constexpr Iterator end() const noexcept { return m_last; }

	constexpr std::reverse_iterator<Iterator> rbegin() const noexcept { return m_last; }
	constexpr std::reverse_iterator<Iterator> rend() const noexcept { return m_first; }

private:
	Iterator m_first = Iterator{};
	Iterator m_last = Iterator{};
};

namespace detail
{

	template <typename... Iterators>
	class zip_iterator_cursor
	{
		static constexpr auto index_seq = std::make_index_sequence<sizeof...( Iterators )>{};

		template <typename It>
		using it_ref_t = decltype( *std::declval<It>() );

	public:
		using reference = std::tuple<it_ref_t<Iterators>...>;

		constexpr zip_iterator_cursor() noexcept : m_iterators{ std::make_tuple( Iterators{}... ) } {}

		constexpr zip_iterator_cursor( const zip_iterator_cursor& ) noexcept = default;

		constexpr explicit zip_iterator_cursor( Iterators... iterators ) noexcept : m_iterators{ iterators... } {}

		constexpr void next() noexcept
		{
			stdx::for_each_in_tuple( m_iterators, []( auto& it ) { ++it; } );
		}

		constexpr void prev() noexcept
		{
			stdx::for_each_in_tuple( m_iterators, []( auto& it ) { --it; } );
		}

		constexpr reference read() const noexcept
		{
			return read_imp( index_seq );
		}

		constexpr bool equal( const zip_iterator_cursor& other ) const noexcept
		{
			return equal_imp( other, index_seq );
		}

		constexpr std::ptrdiff_t distance_to( const zip_iterator_cursor& other ) const noexcept
		{
			return distance_to_imp( other, index_seq );
		}

	private:

		template <std::size_t... Is>
		constexpr reference read_imp( std::index_sequence<Is...> ) const noexcept
		{
			return std::forward_as_tuple( *std::get<Is>( m_iterators )... );
		}

		template <std::size_t... Is>
		constexpr bool equal_imp( const zip_iterator_cursor& other, std::index_sequence<Is...> ) const noexcept
		{
			// zip iterators are equal if ANY sub-iterators are equal
			// this way we can zip ranges of different sizes
			return ( ( std::get<Is>( m_iterators ) == std::get<Is>( other.m_iterators ) ) || ... );
		}

		template <std::size_t... Is>
		constexpr std::ptrdiff_t distance_to_imp( const zip_iterator_cursor& other, std::index_sequence<Is...> ) const noexcept
		{
			return std::min( { std::distance( std::get<Is>( m_iterators ), std::get<Is>( other.m_iterators ) )... } );
		}

	private:
		std::tuple<Iterators...> m_iterators;
	};

} // namespace detail

template <typename... Its>
class zip_iterator : public stdx::basic_iterator<detail::zip_iterator_cursor<Its...>>
{
public:
	using stdx::basic_iterator<detail::zip_iterator_cursor<Its...>>::basic_iterator;

	constexpr zip_iterator( Its... iterators ) : zip_iterator( detail::zip_iterator_cursor{ iterators... } ) {}
};

template <typename... C>
constexpr auto zip( C&&... c ) noexcept -> range<zip_iterator<typename stdx::container_traits<C>::iterator...>>
{
	return range
	{
		zip_iterator<typename stdx::container_traits<C>::iterator...>{ std::begin( c )... },
		zip_iterator<typename stdx::container_traits<C>::iterator...>{ std::end( c )... }
	};
}

namespace detail
{

	template <typename T>
	class integer_iterator_cursor
	{
	public:
		static_assert( std::is_integral_v<T> && !std::is_same_v<T, bool>, "can only enumerate over non-boolean integral values" );

		constexpr integer_iterator_cursor() noexcept = default;
		constexpr integer_iterator_cursor( const integer_iterator_cursor& ) noexcept = default;
		constexpr explicit integer_iterator_cursor( const T& index ) noexcept : m_index{ index } {}

		constexpr integer_iterator_cursor& operator=( const integer_iterator_cursor& ) noexcept = default;

		constexpr const T& read() const noexcept { return m_index; }

		constexpr void next() noexcept
		{
			++m_index;
		}

		constexpr void prev() noexcept
		{
			--m_index;
		}

		constexpr void advance( std::ptrdiff_t n ) noexcept
		{
			m_index = stdx::narrow_cast<T>( m_index + n );
		}

		constexpr std::ptrdiff_t distance_to( const integer_iterator_cursor& other ) const noexcept
		{
			return static_cast<std::ptrdiff_t>( other.m_index ) - static_cast<std::ptrdiff_t>( m_index );
		}

		constexpr bool equal( const integer_iterator_cursor& other ) const noexcept
		{
			return m_index == other.m_index;
		}

	private:
		T m_index = 0;
	};

} // namespace detail

template <typename T>
class integer_iterator : public stdx::basic_iterator<detail::integer_iterator_cursor<T>>
{
public:
	using stdx::basic_iterator<detail::integer_iterator_cursor<T>>::basic_iterator;

	constexpr integer_iterator( T value ) noexcept : integer_iterator( detail::integer_iterator_cursor{ value } ) {}
};

template <typename T>
integer_iterator( const T& ) -> integer_iterator<T>;

template <typename T = std::size_t, typename... C>
constexpr auto enumerate( C&&... c ) -> range<zip_iterator<integer_iterator<T>, typename stdx::container_traits<C>::iterator...>>
{
	const auto maxCount = std::max( { std::size( c )... } );
	const integer_iterator<T> enum_first;
	const integer_iterator<T> enum_last{ stdx::narrow_cast<T>( maxCount ) };

	return range
	{
		zip_iterator<integer_iterator<T>, typename stdx::container_traits<C>::iterator...>{ enum_first, std::begin( c )... },
		zip_iterator<integer_iterator<T>, typename stdx::container_traits<C>::iterator...>{ enum_last, std::end( c )... }
	};
}

} // namespace stdx