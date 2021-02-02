#ifndef STDX_SPAN_HPP
#define STDX_SPAN_HPP

#include <stdx/assert.h>
#include <stdx/compiler.h>

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

	constexpr span( T* first, size_type count ) noexcept : m_imp{ first, count } {}

	template <typename It, STDX_requires( !std::is_integral_v<It> )
	constexpr span( It first, size_type count ) : m_imp{ stdx::to_address( first ), count } {}
	
	template <typename It, STDX_requires( !std::is_integral_v<It> )
	constexpr span( It first, It last ) : m_imp{ stdx::to_address( first ), static_cast<size_type>( last - first ) } {}
	
	template <std::size_t N, STDX_requires( extent == dynamic_extent || N == extent )
	constexpr span( element_type( &arr )[ N ] ) noexcept : m_imp{ arr, N } {}
	
	template <typename U, std::size_t N, STDX_requires( extent == dynamic_extent || N == extent )
	constexpr span( std::array<U, N>& arr ) noexcept : m_imp{ arr.data(), arr.size() } {}
	
	template <typename U, std::size_t N, STDX_requires( extent == dynamic_extent || N == extent )
	constexpr span( const std::array<U, N>& arr ) noexcept : m_imp{ arr.data(), arr.size() } {}
	
	template <typename R, STDX_requires( extent == dynamic_extent )
	constexpr span( R&& r ) noexcept : m_imp{ r.data(), r.size() } {}
	
	template <typename U, std::size_t N, STDX_requires( extent == dynamic_extent || extent == N )
	constexpr span( const span<U, N>& s ) noexcept : m_imp{ s.data(), s.size() } {}
	
	constexpr span( const span& other ) noexcept = default;

	constexpr span& operator=( const span& other ) noexcept = default;

public: // iterators

	constexpr iterator begin() const noexcept { return m_imp.data; }
	constexpr iterator end() const noexcept { return m_imp.data + m_imp.size; }

	constexpr const_iterator cbegin() const noexcept { return m_imp.data; }
	constexpr const_iterator cend() const noexcept { return m_imp.data + m_imp.size; }

	constexpr reverse_iterator rbegin() const noexcept { return end(); }
	constexpr reverse_iterator rend() const noexcept { return begin(); }

	constexpr const_reverse_iterator crbegin() noexcept { return cend(); }
	constexpr const_reverse_iterator crend() const noexcept { return cbegin(); }

public: // element access

	constexpr reference front() const noexcept
	{
		dbExpects( !empty() );
		return m_imp[ 0 ];
	}

	constexpr reference back() const noexcept
	{
		dbExpects( !empty() );
		return m_imp.data[ m_imp.size - 1 ];
	}

	constexpr reference operator[]( size_type i ) const noexcept
	{
		dbExpects( i < m_imp.size );
		return m_imp.data[ i ];
	}

	constexpr pointer data() const noexcept { return m_imp.data; }

public: // observers

	constexpr size_type size() const noexcept { return m_imp.size; }
	constexpr difference_type ssize() const noexcept { return static_cast<difference_type>( m_imp.m_imp.size ); }

	constexpr size_type size_bytes() const noexcept { return m_imp.size * sizeof( element_type ); }

	[[nodiscard]] constexpr bool empty() const noexcept { return m_imp.size == 0; }

public: // subviews

	template <size_t Count>
	constexpr span<element_type, Count> first() const noexcept
	{
		dbExpects( Count <= m_imp.size );
		return span{ m_imp.data, m_imp.data + Count };
	}

	constexpr span<element_type, dynamic_extent> first( size_t count ) const noexcept
	{
		dbExpects( count <= m_imp.size );
		return span{ m_imp.data, m_imp.data + count };
	}

	template <size_t Count>
	constexpr span<element_type, Count> last() const noexcept
	{
		dbExpects( Count <= m_imp.size );
		return span{ ( m_imp.data + m_imp.size ) - Count, ( m_imp.data + m_imp.size ) };
	}

	constexpr span<element_type, dynamic_extent> last( size_t count ) const noexcept
	{
		dbExpects( count <= m_imp.size );
		return span{ ( m_imp.data + m_imp.size ) - count, ( m_imp.data + m_imp.size ) };
	}

	template <size_t Offset, size_t Count = dynamic_extent>
	constexpr span<element_type, Count> subspan() const noexcept
	{
		dbExpects( Offset <= m_imp.size );
		dbExpects( Count == dynamic_extent || Offset + Count <= m_imp.size );
		return span{ m_imp.data + Offset, m_imp.data + std::min<size_t>( Offset + Count, m_imp.size ) };
	}

	constexpr span<element_type, dynamic_extent> subspan( size_t offset, size_t count = dynamic_extent ) const noexcept
	{
		dbExpects( offset <= m_imp.size );
		dbExpects( count == dynamic_extent || offset + count <= m_imp.size );
		return span{ m_imp.data + offset, m_imp.data + std::min<size_t>( offset + count, m_imp.size ) };
	}

private:

	data_type m_imp;
};

// deduction guides

template<class T, std::size_t N>
span( T( &)[ N ] ) -> span<T, N>;

template<class T, std::size_t N>
span( std::array<T, N>& ) -> span<T, N>;

template<class T, std::size_t N>
span( const std::array<T, N>& ) -> span<const T, N>;

template<class Container>
span( Container& ) -> span<typename Container::value_type>;

template<class Container>
span( const Container& ) -> span<const typename Container::value_type>;

} // namespace stdx

#endif