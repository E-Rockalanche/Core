#ifndef STDX_BASIC_ITERATOR_HPP
#define STDX_BASIC_ITERATOR_HPP

#include "stdx/type_traits.h"
#include <utility>

namespace stdx {

/*
member functions of the basic_iterator are enabled if certain members of the cursor are available
member functions that can be defined are:

T& read()
T* arrow()
void next()
void prev()
void advance( std::ptrdiff_t n )
std::ptrdiff_t distance_to( const Cursor& )

Cursors must have member function "bool equals( const Cursor& )" defined
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

	template <typename C>
	using cursor_advance_t = decltype( std::declval<C>().advance() );

	template <typename C>
	using cursor_distance_to_t = decltype( std::declval<C>().distance_to( std::declval<C>() ) );

	template <typename C>
	using cursor_equal_t = decltype( std::declval<C>().equal( std::declval<C>() ) );
}

template <typename Cursor>
class basic_iterator
{
public:
	using reference = detected_t<detail::cursor_read_t, Cursor>;
	using pointer = detected_t<detail::cursor_arrow_t, Cursor>;
	using difference_type = detected_or_t<std::ptrdiff_t, detail::cursor_distance_to_t, Cursor>;
	using size_type = std::make_unsigned_t<difference_type>;

	static_assert( stdx::is_detected_v<detail::cursor_equal_t, Cursor>, "basic_iterator cursor must provide method \"bool equal( const Cursor& )\"" );

	constexpr basic_iterator() noexcept = default;
	constexpr basic_iterator( const basic_iterator& ) = default;
	constexpr basic_iterator( basic_iterator&& ) noexcept = default;
	constexpr basic_iterator( const Cursor& c ) : m_cursor{ c } {}
	constexpr basic_iterator( Cursor&& c ) noexcept : m_cursor{ std::move( c ) } {}

	template <typename... Args>
	constexpr explicit basic_iterator( Args&&... args ) : m_cursor{ std::forward<Args>( args )... } {}

	constexpr basic_iterator& operator=( const basic_iterator& ) noexcept = default;

	template <std::enable_if_t<is_detected_v<detail::cursor_read_t, Cursor>, int> = 0>
	constexpr reference operator*() const noexcept { return m_cursor.read(); }

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_arrow_t, C>, int> = 0>
	constexpr pointer operator->() const noexcept { return m_cursor.arrow(); }

	template <typename C = Cursor, std::enable_if_t<
		is_detected_v<detail::cursor_advance_t, C> &&
		is_detected_v<detail::cursor_read_t, C>, int> = 0>
	constexpr reference operator[]( size_type index ) const
	{
		auto copy = basic_iterator( *this );
		copy.advance( static_cast<difference_type>( index ) );
		return copy.read();
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_next_t, C>, int> = 0>
	constexpr basic_iterator& operator++() noexcept
	{
		m_cursor.next();
		return *this;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_next_t, C>, int> = 0>
	constexpr basic_iterator operator++( int ) noexcept
	{
		basic_iterator copy{ *this };
		m_cursor.next();
		return copy;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_prev_t, C>, int> = 0>
	constexpr basic_iterator& operator--() noexcept
	{
		m_cursor.prev();
		return *this;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_prev_t, C>, int> = 0>
	constexpr basic_iterator operator--( int ) noexcept
	{
		basic_iterator copy{ *this };
		m_cursor.prev();
		return copy;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
	constexpr basic_iterator& operator+=( difference_type n ) noexcept
	{
		m_cursor.advance( n );
		return *this;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
	constexpr basic_iterator& operator+=( size_type n ) noexcept
	{
		m_cursor.advance( static_cast<difference_type>( n ) );
		return *this;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
	constexpr basic_iterator& operator-=( difference_type n ) noexcept
	{
		m_cursor.advance( -n );
		return *this;
	}

	template <typename C = Cursor, std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
	constexpr basic_iterator& operator-=( size_type n ) noexcept
	{
		m_cursor.advance( -static_cast<difference_type>( n ) );
		return *this;
	}

	constexpr const Cursor& cursor() const noexcept { return m_cursor; }

private:
	Cursor m_cursor;
};

template <typename C,
	std::enable_if_t<is_detected_v<detail::cursor_distance_to_t, C>, int> = 0>
constexpr typename basic_iterator<C>::difference_type operator-( const basic_iterator<C>& lhs, const basic_iterator<C>& rhs ) noexcept
{
	return lhs.cursor().distance_to( rhs.cursor() );
}

template <typename C,
	std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
constexpr basic_iterator<C> operator+( basic_iterator<C> it, typename basic_iterator<C>::difference_type n ) noexcept
{
	return it += n;
}

template <typename C,
	std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
constexpr basic_iterator<C> operator+( basic_iterator<C> it, typename basic_iterator<C>::size_type n ) noexcept
{
	return it += n;
}

template <typename C,
	std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
constexpr basic_iterator<C> operator-( basic_iterator<C> it, typename basic_iterator<C>::difference_type n ) noexcept
{
	return it -= n;
}

template <typename C,
	std::enable_if_t<is_detected_v<detail::cursor_advance_t, C>, int> = 0>
constexpr basic_iterator<C> operator-( basic_iterator<C> it, typename basic_iterator<C>::size_type n ) noexcept
{
	return it -= n;
}

template <typename C>
constexpr bool operator==( const basic_iterator<C>& lhs, const basic_iterator<C>& rhs ) noexcept
{
	return lhs.cursor().equal( rhs.cursor() );
}

template <typename C>
constexpr bool operator!=( const basic_iterator<C>& lhs, const basic_iterator<C>& rhs ) noexcept
{
	return !lhs.cursor().equal( rhs.cursor() );
}

} // namespace stdx

#endif // STDX_BASIC_ITERATOR_HPP