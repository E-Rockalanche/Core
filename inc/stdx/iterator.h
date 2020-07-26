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
			stdx::for_each( m_iterators, []( auto& it ) { ++it; } );
		}

		constexpr void prev() noexcept
		{
			stdx::for_each( m_iterators, []( auto& it ) { --it; } );
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

	template <typename T>
	class enumerator_cursor
	{
	public:
		static_assert( std::is_integral_v<T> &&!std::is_same_v<T, bool>, "can only enumerate over non-boolean integral values" );

		using difference_type = std::ptrdiff_t;

		constexpr enumerator_cursor() noexcept = default;
		constexpr enumerator_cursor( const enumerator_cursor& ) noexcept = default;
		constexpr explicit enumerator_cursor( T index ) noexcept : m_index{ index } {}
		constexpr enumerator_cursor& operator=( const enumerator_cursor& ) noexcept = default;

		constexpr T read() const noexcept { return m_index; }

		constexpr void next() noexcept
		{
			dbExpects( m_index != std::numeric_limits<T>::max() );
			++m_index;
		}

		constexpr void prev() noexcept
		{
			dbExpects( m_index != 0 );
			--m_index;
		}

		constexpr void advance( difference_type n ) noexcept
		{
			dbExpects( ( n >= 0 ) ? ( m_index + n >= m_index ) : ( m_index + n < m_index ) );
			m_index += n;
		}

		constexpr difference_type distance_to( const enumerator_cursor& other ) const noexcept
		{
			dbExpects( other.m_index >= m_index );
			return static_cast<difference_type>( other.m_index ) - static_cast<difference_type>( m_index );
		}

		constexpr bool equal( const enumerator_cursor& other ) const noexcept
		{
			return m_index == other.m_index;
		}

	private:
		T m_index = 0;
	};

} // namespace detail

template <typename... Its>
using zip_iterator = stdx::basic_iterator<detail::zip_iterator_cursor<Its...>>;

template <typename... C>
constexpr auto zip( C&&... c ) noexcept -> range<zip_iterator<typename stdx::container_traits<C>::iterator...>>
{
	return range
	{
		zip_iterator<typename stdx::container_traits<C>::iterator...>{ std::begin( c )... },
		zip_iterator<typename stdx::container_traits<C>::iterator...>{ std::end( c )... }
	};
}

template <typename T>
using enumerator = stdx::basic_iterator<detail::enumerator_cursor<T>>;

template <typename... C>
constexpr auto enumerate( C&&... c ) noexcept -> range<zip_iterator<enumerator<std::size_t>, typename stdx::container_traits<C>::iterator...>>
{
	const std::size_t maxCount = std::max( { std::distance( std::begin( c ) ,std::end( c ) )... } );
	const auto enum_first = stdx::basic_iterator<detail::enumerator_cursor<std::size_t>()>{};
	const auto enum_last = enum_first + maxCount;
	return range
	{
		zip_iterator<enumerator<std::size_t>, typename stdx::container_traits<C>::iterator...>{ enum_first, std::begin( c )... },
		zip_iterator<enumerator<std::size_t>, typename stdx::container_traits<C>::iterator...>{ enum_last, std::end( c )...}
	};
}

} // namespace stdx