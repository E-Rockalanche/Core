#ifndef STDX_CONTAINER_HPP
#define STDX_CONTAINER_HPP

#include <stdx/assert.h>

#include <algorithm>

namespace stdx {
	
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
template <typename Container>
constexpr typename Container::iterator erase( Container& c, const typename Container::value_type& value )
{
	return c.erase( std::remove( c.begin(), c.end(), value ), c.end() );
}

// erase-remove idiom in one function
template <typename Container, typename UnaryPredicate>
constexpr typename Container::iterator erase_if( Container& c, UnaryPredicate p )
{
	return c.erase( std::remove_if( c.begin(), c.end(), p ), c.end() );
}

} // namespace stdx

#endif // STDX_CONTAINER_HPP