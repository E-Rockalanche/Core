#ifndef STDX_SPAN_HPP
#define STDX_SPAN_HPP

#include "assert.h"

#include <stdx/iterator.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <type_traits>

namespace stdx
{

constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

namespace detail
{
	template <typename T, std::size_t N>
	struct span_data
	{
		constexpr span_data( T* data_, std::size_t count ) noexcept : data{ data_ }
		{
			dbExpects( count == N );
			(void)count;
		}

		T* data;
		static constexpr std::size_t size = N;
	};

	template <typename T>
	struct span_data<T, dynamic_extent>
	{
		constexpr span_data() noexcept = default;
		constexpr span_data( T* data_, std::size_t size_ ) noexcept : data{ data_ }, size{ size_ } {}

		T* data = nullptr;
		std::size_t size = 0;
	};

	template <typename T>
	struct span_data<T, 0>
	{
		constexpr span_data() noexcept = default;
		constexpr span_data( T*, std::size_t count ) noexcept
		{
			dbExpects( count == 0 );
		}

		static constexpr T* data = nullptr;
		static constexpr std::size_t size = 0;
	};

} // namespace detail
	
template <class T, std::size_t Extent = dynamic_extent>
class span
{
	using data_type = typename detail::span_data<T, Extent>;

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

	constexpr span() noexcept = default;

	template <typename It>
	constexpr span( It first, size_type count ) : m_data{ stdx::to_address( first ), count } {}
	
	template <typename It>
	constexpr span( It first, It last ) : m_data{ stdx::to_address( first ), static_cast<size_type>( last - first ) } {}
	
	template <std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( element_type( &arr )[ N ] ) noexcept : m_data{ arr, N } {}
	
	template <typename U, std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( std::array<U, N>& arr ) noexcept : m_data{ arr.data(), arr.size() } {}
	
	template <typename U, std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( const std::array<U, N>& arr ) noexcept : m_data{ arr.data(), arr.size() } {}
	
	template <typename R,
		std::enable_if_t<extent == dynamic_extent, int> = 0>
	constexpr span( R&& r ) noexcept : m_data{ r.data(), r.size() } {}
	
	template <typename U, std::size_t N,
		std::enable_if_t<extent == dynamic_extent || N == extent, int> = 0>
	constexpr span( const span<U, N>& s ) noexcept : m_data{ s.data(), s.size() } {}
	
	constexpr span( const span& other ) noexcept = default;

	constexpr span& operator=( const span& other ) noexcept = default;

public: // iterators

	constexpr iterator begin() const noexcept { return m_data.data; }
	constexpr iterator end() const noexcept { return m_data.data + m_data.size; }

	constexpr const_iterator cbegin() noexcept { return m_data.data; }
	constexpr const_iterator cend() const noexcept { return m_data.data + m_data.size; }

	constexpr reverse_iterator rbegin() const noexcept { return m_data.data + m_data.size; }
	constexpr reverse_iterator rend() const noexcept { return m_data; }

	constexpr const_reverse_iterator crend() const noexcept { return  m_data.data + m_data.size; }
	constexpr const_reverse_iterator crbegin() noexcept { return m_data; }

public: // element access

	constexpr reference front() const noexcept
	{
		dbExpects( !empty() );
		return m_data[ 0 ];
	}

	constexpr reference back() const noexcept
	{
		dbExpects( !empty() );
		return m_data.data[ m_data.size - 1 ];
	}

	constexpr reference operator[]( size_type i ) const noexcept
	{
		dbExpects( i < m_data.size );
		return m_data.data[ i ];
	}

	constexpr pointer data() const noexcept { return m_data.data; }

public: // observers

	constexpr size_type size() const noexcept { return m_data.size; }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( m_data.m_data.size ); }

	constexpr size_type size_bytes() const noexcept { return m_data.size * sizeof( element_type ); }

	[[nodiscard]] constexpr bool empty() const noexcept { return m_data.size == 0; }

public: // subviews

	template <size_t Count>
	constexpr span<element_type, Count> first() const noexcept
	{
		dbExpects( Count <= m_data.size );
		return span{ m_data.data, m_data.data + Count };
	}

	constexpr span<element_type, dynamic_extent> first( size_t count ) const noexcept
	{
		dbExpects( count <= m_data.size );
		return span{ m_data.data, m_data.data + count };
	}

	template <size_t Count>
	constexpr span<element_type, Count> last() const noexcept
	{
		dbExpects( Count <= m_data.size );
		return span{ ( m_data.data + m_data.size ) - Count, ( m_data.data + m_data.size ) };
	}

	constexpr span<element_type, dynamic_extent> last( size_t count ) const noexcept
	{
		dbExpects( count <= m_data.size );
		return span{ ( m_data.data + m_data.size ) - count, ( m_data.data + m_data.size ) };
	}

	template <size_t Offset, size_t Count = dynamic_extent>
	constexpr span<element_type, Count> subspan() const noexcept
	{
		dbExpects( Offset <= m_data.size );
		dbExpects( Count == dynamic_extent || Offset + Count <= m_data.size );
		return span{ m_data.data + Offset, m_data.data + std::min<size_t>( Offset + Count, m_data.size ) };
	}

	constexpr span<element_type, dynamic_extent> subspan( size_t offset, size_t count = dynamic_extent ) const noexcept
	{
		dbExpects( offset <= m_data.size );
		dbExpects( count == dynamic_extent || offset + count <= m_data.size );
		return span{ m_data.data + offset, m_data.data + std::min<size_t>( offset + count, m_data.size ) };
	}

private:

	data_type m_data;
};

// deduction guides

template <typename T, std::size_t N>
span( T( &arr )[ N ] ) -> span<T, N>;

template <typename U, std::size_t N>
span( std::array<U, N>& arr ) -> span<U, N>;

template <typename U, std::size_t N>
span( const std::array<U, N>& arr ) -> span<const U, N>;

} // namespace stdx

#endif