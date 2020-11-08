#pragma once

#include <stdx/iterator/basic_iterator.h>
#include <stdx/utility.h>

namespace stdx
{

namespace detail
{

template <typename Iterator>
using zip_decrement_t = decltype( --std::declval<Iterator>() );

template <typename Iterator>
using zip_equal_t = decltype( std::declval<Iterator>() == std::declval<Iterator>() );

template <typename Iterator>
using zip_reference_t = decltype( *std::declval<Iterator>() );

template <typename... Iterators>
class zip_iterator_cursor
{
	static constexpr auto index_seq = std::make_index_sequence<sizeof...( Iterators )>{};

public:
	using reference = std::tuple<zip_reference_t<Iterators>...>;

	constexpr zip_iterator_cursor( const zip_iterator_cursor& ) = default;

	constexpr explicit zip_iterator_cursor( Iterators... iterators ) : m_iterators{ std::move( iterators )... } {}

	constexpr reference read() const
	{
		return read_imp( index_seq );
	}

	constexpr void next()
	{
		stdx::for_each_in_tuple( m_iterators, []( auto& it ) { ++it; } );
	}

	template<bool RequiresTag = true, std::enable_if_t<( stdx::is_detected_v<zip_decrement_t, Iterators> && ... ), int> = 0>
	constexpr void prev()
	{
		stdx::for_each_in_tuple( m_iterators, []( auto& it ) { --it; } );
	}

	template<bool RequiresTag = true, std::enable_if_t<( stdx::is_detected_v<zip_equal_t, Iterators> && ... ), int> = 0>
	constexpr bool equal( const zip_iterator_cursor& other ) const
	{
		return equal_imp( other, index_seq );
	}

private:

	template <std::size_t... Is>
	constexpr reference read_imp( std::index_sequence<Is...> ) const
	{
		return std::forward_as_tuple( *std::get<Is>( m_iterators )... );
	}

	template <std::size_t... Is,
		bool RequiresTag = true, std::enable_if_t<( stdx::is_detected_v<zip_equal_t, Iterators> && ... ), int> = 0>
	constexpr bool equal_imp( const zip_iterator_cursor& other, std::index_sequence<Is...> ) const
	{
		// zip iterators are equal if ANY sub-iterators are equal
		// this way we can zip ranges of different sizes
		return ( ( std::get<Is>( m_iterators ) == std::get<Is>( other.m_iterators ) ) || ... );
	}

private:
	std::tuple<Iterators...> m_iterators;
};

} // namespace detail

template <typename... Iterators>
class zip_iterator : public stdx::basic_iterator<detail::zip_iterator_cursor<Iterators...>>
{
public:
	using stdx::basic_iterator<detail::zip_iterator_cursor<Iterators...>>::basic_iterator;
};

template <typename... Iterators>
class zip_range
{
public:
	using iterator = zip_iterator<Iterators...>;

	constexpr zip_range( iterator first, iterator last ) : m_first{ std::move( first ) }, m_last{ std::move( last ) } {}

	constexpr iterator begin() const { return m_first; }
	constexpr iterator end() const { return m_first; }

private:
	iterator m_first;
	iterator m_last;
};

namespace detail
{

template <typename Container>
using zip_iterator_t = decltype( std::begin( std::declval<Container>() ) );

}

template <typename... C>
constexpr zip_range<detail::zip_iterator_t<C>...> zip( C&&... c )
{
	return zip_range
	{
		zip_iterator<detail::zip_iterator_t<C>...>{ std::begin( c )... },
		zip_iterator<detail::zip_iterator_t<C>...>{ std::end( c )... }
	};
}

}