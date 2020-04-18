#ifndef STDX_SPAN_HPP
#define STDX_SPAN_HPP

#include "assert.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <type_traits>

namespace stdx
{

constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();
	
template <class T, std::size_t Extent = dynamic_extent>
class span
{
public:
	using element_type = T;
	using value_type = std::remove_cv_t<T>;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static constexpr size_type extent = Extent;

public: // construction and assignment

	template <std::enable_if_t<extent == 0 || extent == dynamic_extent, int> = 0>
	constexpr span() noexcept : m_first{ nullptr }, m_last{ nullptr } {};

	template <typename It>
	constexpr span( It first, size_type count ) : m_first{ &*first }, m_last{ m_first + count }
	{
		dbAssert( extent == dynamic_extent || extent == size() );
	}
	
	template <typename It, typename End>
	constexpr span( It first, End last ) : m_first{ &*first }, m_last{ &*last }
	{
		dbAssert( extent == dynamic_extent || extent == size() );
	}
	
	template <std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( element_type ( &arr )[ N ] ) noexcept : m_first{ arr }, m_last{ arr + N } {}
	
	template <typename U, std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( std::array<U, N>& arr ) noexcept : m_first{ arr.data() }, m_last{ arr.data() + arr.size() } {}
	
	template <typename U, std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( const std::array<U, N>& arr ) noexcept : m_first{ arr.data() }, m_last{ arr.data() + arr.size() } {}
	
	template <typename R,
		std::enable_if_t<extent == dynamic_extent, int> = 0>
	constexpr span( R&& r ) : m_first{ r.data() }, m_last{ r.data() + r.size() } {}
	
	template <typename U, std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( const span<U, N>& s ) noexcept : m_first{ s.m_first }, m_last{ s.m_last } {}
	
	constexpr span( const span& other ) noexcept = default;

	constexpr span& operator=( const span& other ) noexcept = default;

public: // iterators

	constexpr iterator begin() const noexcept { return m_first; }
	constexpr iterator end() const noexcept { return m_last; }

	constexpr const_iterator cbegin() noexcept { return m_first; }
	constexpr const_iterator cend() const noexcept { return m_last; }

	constexpr reverse_iterator rbegin() const noexcept { return m_first; }
	constexpr reverse_iterator rend() const noexcept { return m_last; }

	constexpr const_reverse_iterator crend() const noexcept { return m_last; }
	constexpr const_reverse_iterator crbegin() noexcept { return m_first; }

public: // element access

	constexpr reference front() const noexcept
	{
		dbAssert( !empty() );
		return *m_first;
	}

	constexpr reference back() const noexcept
	{
		dbAssert( !empty() );
		return *( m_last - 1 );
	}

	constexpr reference operator[]( size_type i ) const noexcept
	{
		dbAssert( i < size() );
		return m_first[ i ];
	}

	constexpr pointer data() const noexcept { return m_first; }

public: // observers

	constexpr size_type size() const noexcept { return static_cast<size_type>( m_last - m_first ); }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( m_last - m_first ); }

	constexpr size_type size_bytes() const noexcept { return size() * sizeof( element_type ); }

	[[nodiscard]] constexpr bool empty() const noexcept { return m_first == m_last; }

public: // subviews

	template <size_t Count>
	constexpr span<element_type, Count> first() const noexcept
	{
		dbAssert( Count <= size() );
		return span{ m_first, m_first + Count };
	}

	constexpr span<element_type, dynamic_extent> first( size_t Count ) const noexcept
	{
		dbAssert( Count <= size() );
		return span{ m_first, m_first + Count };
	}

	template <size_t Count>
	constexpr span<element_type, Count> last() const noexcept
	{
		dbAssert( Count <= size() );
		return span{ m_last - Count, m_last };
	}

	constexpr span<element_type, dynamic_extent> last( size_t Count ) const noexcept
	{
		dbAssert( Count <= size() );
		return span{ m_last - Count, m_last };
	}

	template <size_t Offset, size_t Count = dynamic_extent>
	constexpr span<element_type, Count> subspan() const noexcept
	{
		dbAssert( Offset <= size() );
		dbAssert( Count == dynamic_extent || Offset + Count <= size() );
		return span{ m_first + Offset, m_first + std::min<size_t>( Offset + Count, size() ) };
	}

	constexpr span<element_type, dynamic_extent> subspan( size_t Offset, size_t Count = dynamic_extent ) const noexcept
	{
		dbAssert( Offset <= size() );
		dbAssert( Count == dynamic_extent || Offset + Count <= size() );
		return span{ m_first + Offset, m_first + std::min<size_t>( Offset + Count, size() ) };
	}

private:

	pointer m_first = nullptr;
	pointer m_last = nullptr;
};

}

#endif