#pragma once

#include "stdx/type_traits.h"
#include <utility>

namespace stdx {

/*
required:

	T& read() const
	void next()

optional:

	T* arrow() const
	void prev()
	void advance( difference_type n )
	void advance( size_type n )
	difference_type distance_to( const Cursor& ) const
	bool equal( const Cursor& ) const
*/

namespace detail {
	template <typename C>
	using cursor_read_t = decltype( std::declval<C>().read() );

	template <typename C>
	using cursor_arrow_t = decltype( std::declval<C>().arrow() );

	template <typename C>
	using cursor_next_t = decltype( std::declval<C>().next() );

	template <typename C>
	using cursor_prev_t = decltype( std::declval<C>().prev() );

	template <typename C, typename T>
	using cursor_advance_t = decltype( std::declval<C>().advance( std::declval<T>() ) );

	template <typename C>
	using cursor_distance_to_t = decltype( std::declval<C>().distance_to( std::declval<C>() ) );

	template <typename C>
	using cursor_equal_t = decltype( std::declval<C>().equal( std::declval<C>() ) );
}

template <typename Cursor>
class basic_iterator : public Cursor
{
public:
	static_assert( is_detected_v<detail::cursor_read_t, Cursor>, "cursor must have read() function to dereference iterator" );
	static_assert( is_detected_v<detail::cursor_next_t, Cursor>, "cursor must have next() function to increment iterator" );

	using reference = detected_t<detail::cursor_read_t, Cursor>;
	using value_type = std::remove_reference_t<reference>;
	using pointer = detected_or_t<std::add_pointer_t<value_type>, detail::cursor_arrow_t, Cursor>;

	using difference_type = detected_or_t<std::ptrdiff_t, detail::cursor_distance_to_t, Cursor>;
	using size_type = std::make_unsigned_t<difference_type>;

public:
	// construction/assignment

	using Cursor::Cursor;

	constexpr basic_iterator() = default;
	constexpr basic_iterator( const basic_iterator& ) = default;
	constexpr basic_iterator( basic_iterator&& ) = default;

	constexpr basic_iterator& operator=( const basic_iterator& ) = default;
	constexpr basic_iterator& operator=( basic_iterator&& ) = default;

	// access

	constexpr reference operator*() const
	{
		return Cursor::read();
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_arrow_t, C>, int> = 0>
	constexpr pointer operator->() const
	{
		return Cursor::arrow();
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, size_type> || stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	constexpr reference operator[]( size_type index ) const
	{
		basic_iterator copy{ *this };

		if constexpr ( stdx::is_detected_v<detail::cursor_advance_t, C, size_type> )
			copy.advance( index );
		else
			copy.advance( static_cast<difference_type>( index ) );

		return copy.read();
	}

	// increment/decrement

	constexpr basic_iterator& operator++()
	{
		Cursor::next();
		return *this;
	}

	constexpr basic_iterator operator++( int )
	{
		basic_iterator copy{ *this };
		Cursor::next();
		return copy;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_prev_t, C>, int> = 0>
	constexpr basic_iterator& operator--()
	{
		Cursor::prev();
		return *this;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_prev_t, C>, int> = 0>
	constexpr basic_iterator operator--( int )
	{
		basic_iterator copy{ *this };
		Cursor::prev();
		return copy;
	}

	// advancement

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	constexpr basic_iterator& operator+=( difference_type n )
	{
		Cursor::advance( n );
		return *this;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type> ||
		stdx::is_detected_v<detail::cursor_advance_t, C, size_type>, int> = 0>
	constexpr basic_iterator& operator+=( size_type n )
	{
		if constexpr ( stdx::is_detected_v<detail::cursor_advance_t, C, size_type> )
			Cursor::advance( n );
		else
			Cursor::advance( static_cast<difference_type>( n ) );

		return *this;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	constexpr basic_iterator& operator-=( difference_type n )
	{
		Cursor::advance( -n );
		return *this;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	constexpr basic_iterator& operator-=( size_type n )
	{
		Cursor::advance( -static_cast<difference_type>( n ) );
		return *this;
	}

	// difference

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr difference_type operator-( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		return rhs.distance_to( lhs );
	}

	// advancement

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	friend constexpr basic_iterator operator+( basic_iterator it, difference_type n )
	{
		return it += n;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	friend constexpr basic_iterator operator+( difference_type n, basic_iterator it )
	{
		return it += n;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type> ||
		stdx::is_detected_v<detail::cursor_advance_t, C, size_type>, int> = 0>
	friend constexpr basic_iterator operator+( basic_iterator it, size_type n )
	{
		return it += n;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type> ||
		stdx::is_detected_v<detail::cursor_advance_t, C, size_type>, int> = 0>
	friend constexpr basic_iterator operator+( size_type n, basic_iterator it )
	{
		return it += n;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	friend constexpr basic_iterator operator-( basic_iterator it, difference_type n )
	{
		return it -= n;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_advance_t, C, difference_type>, int> = 0>
	friend constexpr basic_iterator operator-( basic_iterator it, size_type n )
	{
		return it -= n;
	}

	// comparison

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_equal_t, C> ||
		stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr bool operator==( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		if constexpr ( stdx::is_detected_v<detail::cursor_equal_t, C> )
			return lhs.equal( rhs );
		else
			return lhs.distance_to( rhs ) == 0;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_equal_t, C> ||
		stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr bool operator!=( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		if constexpr ( stdx::is_detected_v<detail::cursor_equal_t, C> )
			return !lhs.equal( rhs );
		else
			return lhs.distance_to( rhs ) != 0;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr bool operator<( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		return lhs.distance_to( rhs ) > 0;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr bool operator>( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		return lhs.distance_to( rhs ) < 0;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr bool operator<=( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		return lhs.distance_to( rhs ) >= 0;
	}

	template <typename C = Cursor,
		std::enable_if_t<stdx::is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
	friend constexpr bool operator>=( const basic_iterator& lhs, const basic_iterator& rhs )
	{
		return lhs.distance_to( rhs ) <= 0;
	}
};

} // namespace stdx
