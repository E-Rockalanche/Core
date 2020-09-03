#ifndef STDX_CONTAINER_HPP
#define STDX_CONTAINER_HPP

#include <stdx/assert.h>
#include <stdx/type_traits.h>

#include <algorithm>

namespace stdx {

namespace detail
{
	template <typename C>
	using container_pointer_t = decltype( std::data( std::declval<C>() ) );

	template <typename C>
	using container_const_pointer_t = decltype( std::data( std::declval<const C>() ) );

	template <typename C>
	using key_type_t = typename C::key_type;

	template <typename C>
	using mapped_type_t = typename C::mapped_type;
}

template <typename Container>
class container_traits
{
public:
	using iterator = decltype( std::begin( std::declval<Container>() ) );
	using const_iterator = decltype( std::begin( std::declval<const Container>() ) );

	using reference = decltype( *std::declval<iterator>() );
	using const_reference = decltype( *std::declval<const_iterator>() );

	using value_type = std::remove_reference_t<reference>;

	using size_type = decltype( std::size( std::declval<Container>() ) );

	using pointer = stdx::detected_t<detail::container_pointer_t, Container>;
	using const_pointer = stdx::detected_t<detail::container_const_pointer_t, Container>;

	using key_type = stdx::detected_t<detail::key_type_t, Container>;
	using mapped_type = stdx::detected_t<detail::mapped_type_t, Container>;
};
	
// quick erasure of elements that does not preserve order
template <typename Container>
constexpr void backswap_erase( Container& c, const typename Container::value_type& value )
{
	for( auto it = c.begin(); it != c.end(); )
	{
		if ( *it == value )
		{
			std::swap( *it, c.back() );
			c.pop_back();
		}
		else
		{
			++it;
		}
	}
}
	
// quick erasure of elements that does not preserve order
template <typename Container, typename UnaryPredicate>
constexpr void backswap_erase_if( Container& c, UnaryPredicate p )
{
	for( auto it = c.begin(); it != c.end(); )
	{
		if ( p( *it ) )
		{
			std::swap( *it, c.back() );
			c.pop_back();
		}
		else
		{
			++it;
		}
	}
}

// quick erase of single element that does not preserve order
// returns iterator following last remove element
template <typename Container>
constexpr typename Container::iterator backswap_erase( Container& c, typename Container::const_iterator pos )
{
	dbExpects( c.begin() <= pos && pos < c.end() );

	typename Container::iterator it = c.begin() + std::distance( c.cbegin(), pos );
	std::swap( *it, c.back() );
	c.pop_back();
	return it;
}

// quick erase of elements that does not preserve order
// returns iterator to new sequence end
template <typename Container>
constexpr typename Container::iterator backswap_erase( Container& c, typename Container::const_iterator first, typename Container::const_iterator last )
{
	dbExpects( c.begin() <= first && first <= last && last < c.end() );

	typename Container::iterator it = c.begin() + std::distance( c.begin(), first );
	last = std::min( last, c.end() - std::distance( first, last ) );
	for( ; first != last; ++first )
	{
		backswap_erase( c, first );
	}
	return it;
}

// erase-remove idiom in one function
template <typename Container, typename T>
constexpr typename stdx::container_traits<Container>::size_type erase( Container& c, const T& value )
{
	const auto sizeBefore = c.size();
	c.erase( std::remove( c.begin(), c.end(), value ), c.end() );
	return sizeBefore - c.size();
}

// erase-remove idiom in one function
template <typename Container, typename UnaryPredicate>
constexpr typename stdx::container_traits<Container>::size_type erase_if( Container& c, UnaryPredicate p )
{
	const auto sizeBefore = c.size();
	c.erase( std::remove_if( c.begin(), c.end(), p ), c.end() );
	return sizeBefore - c.size();
}

template <typename Container, typename T>
constexpr typename stdx::container_traits<Container>::size_type erase_first_of( Container& c, const T& value )
{
	auto it = std::find( c.begin(), c.end(), value );
	if ( it == c.end() )
		return 0;
	
	c.erase( it );
	return 1;
}

template <typename Container, typename UnaryPredicate>
constexpr typename stdx::container_traits<Container>::size_type erase_first_of_if( Container& c, UnaryPredicate p )
{
	auto it = std::find_if( c.begin(), c.end(), p );
	if ( it == c.end() )
		return 0;

	c.erase( it );
	return 1;
}

template <typename Container, typename T>
constexpr void push_back_unique( Container& c, T&& value )
{
	auto it = std::find( c.begin(), c.end(), value );
	if ( it == c.end() )
		c.push_back( std::forward<T>( value ) );
}

} // namespace stdx

#endif // STDX_CONTAINER_HPP