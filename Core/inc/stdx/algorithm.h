#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>

namespace stdx
{

template <typename T, typename U>
constexpr T lerp( T lhs, T rhs, U x )
{
	return static_cast<T>( lhs + ( rhs - lhs ) * x );
}

template <class InputIt, class UnaryPredicate>
constexpr bool all_of( InputIt first, const InputIt last, UnaryPredicate p )
{
	for ( ; first != last; ++first )
	{
		if ( !p( *first ) )
			return false;
	}
	return true;
}

template <class InputIt, class UnaryPredicate>
constexpr bool any_of( InputIt first, const InputIt last, UnaryPredicate p )
{
	for ( ; first != last; ++first )
	{
		if ( p( *first ) )
			return true;
	}
	return false;
}

template <class InputIt, class UnaryPredicate>
constexpr bool none_of( InputIt first, const InputIt last, UnaryPredicate p )
{
	for ( ; first != last; ++first )
	{
		if ( p( *first ) )
			return false;
	}
	return true;
}

template <class InputIt, class UnaryFunction>
constexpr UnaryFunction for_each( InputIt first, const InputIt last, UnaryFunction f )
{
	for ( ; first != last; ++first )
	{
		f( *first );
	}
	return f;
}

template <class InputIt, class Size, class UnaryFunction>
constexpr UnaryFunction for_each_n( const InputIt first, const Size count, UnaryFunction f )
{
	return stdx::for_each( first, first + count, f );
}

template <class InputIt, class T>
constexpr typename std::iterator_traits<InputIt>::difference_type
	count( InputIt first, const InputIt last, const T& value )
{
	typename std::iterator_traits<InputIt>::difference_type result = 0;
	for ( ; first != last; ++first )
	{
		if ( value == *first )
			++result;
	}
	return result;
}

template <class InputIt, class UnaryPredicate>
constexpr typename std::iterator_traits<InputIt>::difference_type
	count_if( InputIt first, const InputIt last, UnaryPredicate p )
{
	typename std::iterator_traits<InputIt>::difference_type result = 0;
	for ( ; first != last; ++first )
	{
		if ( p( *first ) )
			++result;
	}
	return result;
}

template <class InputIt, class T>
constexpr InputIt find( InputIt first, const InputIt last, const T& value )
{
	for ( ; first != last; ++first )
	{
		if ( value == *first )
			return first;
	}
	return last;
}

template <class InputIt, class UnaryPredicate>
constexpr InputIt find_if( InputIt first, const InputIt last, UnaryPredicate p )
{
	for ( ; first != last; ++first )
	{
		if ( p( *first ) )
			return first;
	}
	return last;
}

template <class InputIt, class UnaryPredicate>
constexpr InputIt find_if_not( InputIt first, const InputIt last, UnaryPredicate p )
{
	for ( ; first != last; ++first )
	{
		if ( !p( *first ) )
			return first;
	}
	return last;
}

template <class InputIt, class T>
constexpr bool contains( InputIt first, const InputIt last, const T& value )
{
	for ( ; first != last; ++first )
	{
		if ( value == *first )
			return true;
	}
	return false;
}

template <class InputIt, class OutputIt>
constexpr OutputIt copy( InputIt first, const InputIt last, OutputIt dest )
{
	for ( ; first != last; ++first, ++dest )
	{
		*dest = *first;
	}
	return dest;
}

template <class InputIt, class OutputIt, class UnaryPredicate>
constexpr OutputIt copy_if( InputIt first, const InputIt last, OutputIt dest, UnaryPredicate p )
{
	for ( ; first != last; ++first, ++dest )
	{
		if ( p( *first ) )
			*dest = *first;
	}
	return dest;
}

template <class InputIt, class Size, class OutputIt>
constexpr OutputIt copy_n( const InputIt first, const Size count, OutputIt dest )
{
	return stdx::copy( first, first + count, dest );
}

template <class InputIt, class OutputIt>
constexpr OutputIt move( const InputIt first, const InputIt last, OutputIt dest )
{
	if ( first <= dest && dest < last )
	{
		// move range in reverse
		auto rfirst = std::reverse_iterator( last );
		auto rlast = std::reverse_iterator( first );
		auto rdest = std::reverse_iterator( dest );
		for ( ; rfirst != rlast; ++rfirst, ++rdest )
		{
			*rdest = std::move( *rfirst );
		}
	}
	else
	{
		for ( auto it = first; it != last; ++it, ++dest )
		{
			*dest = std::move( *it );
		}
	}
}

template <class ForwardIt, class T>
constexpr void fill( ForwardIt first, const ForwardIt last, const T& value )
{
	for ( ; first != last; ++first )
	{
		*first = value;
	}
}

template <class ForwardIt, class Size, class T>
constexpr void fill_n( const ForwardIt first, const Size count, const T& value )
{
	stdx::fill( first, first + count, value );
}

template <class InputIt, class OutputIt, class UnaryOperation>
constexpr OutputIt transform( InputIt first, const InputIt last, OutputIt dest, UnaryOperation op )
{
	for ( ; first != last; ++first, ++dest )
	{
		*dest = op( *first );
	}
	return dest;
}

template <class InputIt1, class InputIt2, class OutputIt, class BinaryOperation>
constexpr OutputIt transform( InputIt1 first1, const InputIt1 last1, InputIt2 first2, OutputIt dest, BinaryOperation op )
{
	for ( ; first1 != last1; ++first1, ++first2, ++dest )
	{
		*dest = op( *first1, *first2 );
	}
	return dest;
}

template <class InputIt, class Size, class OutputIt, class UnaryOperation>
constexpr OutputIt transform_n( const InputIt first, const Size count, OutputIt dest, UnaryOperation op )
{
	return transform( first, first + count, dest, op );
}

template <class ForwardIt, class Generator>
constexpr void generate( ForwardIt first, const ForwardIt last, Generator g )
{
	for ( ; first != last; ++first )
	{
		*first = g();
	}
}

template <class ForwardIt, class Size, class Generator>
constexpr void generate_n( const ForwardIt first, const Size count, Generator g )
{
	stdx::generate( first, first + count, g );
}

template <class ForwardIt, class T>
constexpr ForwardIt remove( ForwardIt first, const ForwardIt last, const T& value )
{
	ForwardIt dest = first;
	for ( ; first != last; ++first )
	{
		if ( *first != value )
		{
			if ( first != dest )
			{
				*dest = std::move( *first );
			}
			++dest;
		}
	}
	return dest;
}

template <class ForwardIt, class UnaryPredicate>
constexpr ForwardIt remove_if( ForwardIt first, const ForwardIt last, UnaryPredicate p )
{
	ForwardIt dest = first;
	for ( ; first != last; ++first )
	{
		if ( !p( *first ) )
		{
			if ( first != dest )
			{
				*dest = std::move( *first );
			}
			++dest;
		}
	}
	return dest;
}

template <class InputIt, class OutputIt, class T>
constexpr OutputIt remove_copy( InputIt first, const InputIt last, OutputIt dest, const T& value )
{
	for ( ; first != last; ++first )
	{
		if ( *first != value )
		{
			*( dest++ ) = *first;
		}
	}
	return dest;
}

template <class InputIt, class OutputIt, class UnaryPredicate>
constexpr OutputIt remove_copy_if( InputIt first, const InputIt last, OutputIt dest, UnaryPredicate p )
{
	for ( ; first != last; ++first )
	{
		if ( !p( *first ) )
		{
			*( dest++ ) = *first;
		}
	}
	return dest;
}

template <class ForwardIt, class T>
constexpr void replace( ForwardIt first, const ForwardIt last, const T& old_value, const T& new_value )
{
	for ( ; first != last; ++first )
	{
		if ( *first == old_value )
			*first = new_value;
	}
}

template <class ForwardIt, class UnaryPredicate, class T>
constexpr void replace_if( ForwardIt first, const ForwardIt last, UnaryPredicate p, const T& new_value )
{
	for ( ; first != last; ++first )
	{
		if ( p( *first ) )
			*first = new_value;
	}
}

template <class InputIt, class OutputIt, class T>
constexpr OutputIt replace_copy( InputIt first, const InputIt last, OutputIt dest, const T& old_value, const T& new_value )
{
	for ( ; first != last; ++first, ++dest )
	{
		if ( *first == old_value )
			*dest = new_value;
		else
			*dest = *first;
	}
	return dest;
}

template <class InputIt, class OutputIt, class UnaryPredicate, class T>
constexpr OutputIt replace_copy( InputIt first, const InputIt last, OutputIt dest, UnaryPredicate p, const T& new_value )
{
	for ( ; first != last; ++first, ++dest )
	{
		if ( p( *first ) )
			*dest = new_value;
		else
			*dest = *first;
	}
	return dest;
}

namespace detail
{
	template <class T,
		std::enable_if_t<std::is_move_constructible_v<T> && std::is_move_assignable_v<T>, int> = 0>
	constexpr void swap_no_check( T& a, T& b )
		noexcept( std::is_nothrow_move_constructible<T>::value && std::is_nothrow_move_assignable<T>::value )
	{
		T temp = std::move( a );
		a = std::move( b );
		b = std::move( temp );
	}
}

template <class T,
	std::enable_if_t<std::is_move_constructible_v<T> && std::is_move_assignable_v<T>, int> = 0>
constexpr void swap( T& a, T& b )
{
	if ( std::addressof( a ) != std::addressof( b ) )
	{
		stdx::detail::swap_no_check( a, b );
	}
}

template <class ForwardIt1, class ForwardIt2>
constexpr ForwardIt2 swap_ranges( ForwardIt1 first1, const ForwardIt1 last1, ForwardIt2 first2 )
{
	if ( first1 == first2 )
		return last1;

	for ( ; first1 != last1; ++first1, ++first2 )
	{
		stdx::detail::swap_no_check( *first1, *first2 );
	}
	return first2;
}

template <class T, std::size_t N,
	std::enable_if_t<std::is_swappable_v<T>, int> = 0>
constexpr void swap( T( &a )[ N ], T( &b )[ N ] )
{
	stdx::swap_ranges( &a, &a + N, &b );
}

template<class ForwardIt1, class ForwardIt2>
constexpr void iter_swap( const ForwardIt1 a, const ForwardIt2 b )
{
	stdx::swap( *a, *b );
}

template <class BidirIt>
constexpr void reverse( BidirIt first, BidirIt last )
{
	while ( ( first != last ) && ( first != --last ) )
	{
		stdx::detail::swap_no_check( *first, *last );
		++first;
	}
}

template <class BidirIt, class OutputIt>
constexpr OutputIt reverse_copy( const BidirIt first, BidirIt last, OutputIt dest )
{
	while ( first != last )
	{
		*( dest++ ) = *( --last );
	}
	return dest;
}

template <class ForwardIt, class Compare>
constexpr ForwardIt is_sorted_until( ForwardIt first, const ForwardIt last, Compare comp )
{
	if ( first != last )
	{
		ForwardIt next = first;
		while ( ++next != last )
		{
			if ( comp( *next, *first ) )
				return next;
			first = next;
		}
	}
	return last;
}

template<class ForwardIt>
constexpr ForwardIt is_sorted_until( const ForwardIt first, const ForwardIt last )
{
	return is_sorted_until( first, last, std::less<>() );
}

template<class ForwardIt>
constexpr bool is_sorted( const ForwardIt first, const ForwardIt last )
{
	return is_sorted_until( first, last ) == last;
}

template<class ForwardIt, class Compare>
constexpr bool is_sorted( const ForwardIt first, const ForwardIt last, Compare comp )
{
	return is_sorted_until( first, last, comp ) == last;
}

template<class ForwardIt, class T>
constexpr ForwardIt lower_bound( ForwardIt first, const ForwardIt last, const T& value )
{
	ForwardIt it{};
	typename std::iterator_traits<ForwardIt>::difference_type count = std::distance( first, last );
	typename std::iterator_traits<ForwardIt>::difference_type step = 0;

	while ( count > 0 )
	{
		it = first;
		step = count / 2;
		std::advance( it, step );
		if ( *it < value )
		{
			first = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return first;
}

template<class ForwardIt, class T, class Compare>
constexpr ForwardIt lower_bound( ForwardIt first, const ForwardIt last, const T& value, Compare comp )
{
	ForwardIt it{};
	typename std::iterator_traits<ForwardIt>::difference_type count = std::distance( first, last );
	typename std::iterator_traits<ForwardIt>::difference_type step = 0;

	while ( count > 0 )
	{
		it = first;
		step = count / 2;
		std::advance( it, step );
		if ( comp( *it, value ) )
		{
			first = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return first;
}

template<class ForwardIt, class T>
constexpr ForwardIt upper_bound( ForwardIt first, const ForwardIt last, const T& value )
{
	ForwardIt it{};
	typename std::iterator_traits<ForwardIt>::difference_type count = std::distance( first, last );
	typename std::iterator_traits<ForwardIt>::difference_type step = 0;

	while ( count > 0 )
	{
		it = first;
		step = count / 2;
		std::advance( it, step );
		if ( !( value < *it ) )
		{
			first = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return first;
}

template<class ForwardIt, class T, class Compare>
constexpr ForwardIt upper_bound( ForwardIt first, const ForwardIt last, const T& value, Compare comp )
{
	ForwardIt it{};
	typename std::iterator_traits<ForwardIt>::difference_type count = std::distance( first, last );
	typename std::iterator_traits<ForwardIt>::difference_type step = 0;

	while ( count > 0 )
	{
		it = first;
		step = count / 2;
		std::advance( it, step );
		if ( !comp( value, *it ) )
		{
			first = ++it;
			count -= step + 1;
		}
		else
			count = step;
	}
	return first;
}

template<class ForwardIt, class T>
constexpr bool binary_search( ForwardIt first, const ForwardIt last, const T& value )
{
	first = std::lower_bound( first, last, value );
	return ( !( first == last ) && !( value < *first ) );
}

template<class ForwardIt, class T, class Compare>
constexpr bool binary_search( ForwardIt first, const ForwardIt last, const T& value, Compare comp )
{
	first = std::lower_bound( first, last, value, comp );
	return ( !( first == last ) && !( comp( value, *first ) ) );
}

template<class ForwardIt, class T>
constexpr std::pair<ForwardIt, ForwardIt>
	equal_range( const ForwardIt first, const ForwardIt last, const T& value )
{
	return std::make_pair(
		std::lower_bound( first, last, value ),
		std::upper_bound( first, last, value ) );
}

template<class ForwardIt, class T, class Compare>
constexpr std::pair<ForwardIt, ForwardIt>
	equal_range( const ForwardIt first, const ForwardIt last, const T& value, Compare comp )
{
	return std::make_pair(
		std::lower_bound( first, last, value, comp ),
		std::upper_bound( first, last, value, comp ) );
}

template <typename ForwardIt, typename Generator>
typename ForwardIt
	random_element( const ForwardIt first, const ForwardIt last, Generator&& g )
{
	dbExpects( first != last );
	std::uniform_int_distribution<std::ptrdiff_t> dist{ 0, std::distance( first, last ) - 1 };
	return first + dist( g );
}

template <typename InputIt, typename T>
constexpr T accumulate( InputIt first, const InputIt last, T init )
{
	for ( ; first != last; ++first )
		init = std::move( init ) + *first;

	return init;
}

template <typename InputIt, typename T, typename BinaryOp>
constexpr T accumulate( InputIt first, const InputIt last, T init, BinaryOp op )
{
	for ( ; first != last; ++first )
		init = op( std::move( init ), *first );

	return init;
}

} // namespace stdx