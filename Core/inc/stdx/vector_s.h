#ifndef STDX_VECTOR_S_HPP
#define STDX_VECTOR_S_HPP

#include <stdx/bit.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace stdx
{

namespace detail
{

	template <typename T, std::size_t Size>
	struct small_vector_storage
	{
		using size_type = std::size_t;

		static constexpr size_type capacity = Size;

		std::aligned_storage_t<sizeof( T ) * Size, alignof( T )> buffer;
		size_type size = 0;

		constexpr small_vector_storage() noexcept = default;
		constexpr small_vector_storage( size_type n ) noexcept { dbExpects( n <= Size ); }

		~small_vector_storage()
		{
			std::destroy_n( data(), this->size );
		}

		constexpr void init_reserve( size_type n )
		{
			dbExpects( n <= Size );
			dbExpects( this->size = 0 );
		}

		constexpr T* data() noexcept { return reinterpret_cast<T*>( &this->buffer ); }
		constexpr const T* data() const noexcept { return reinterpret_cast<T*>( &this->buffer ); }
		constexpr size_type max_size() const noexcept { return Size; }
		constexpr void reserve( size_type n ) noexcept { dbExpects( n <= Size ); }
		constexpr void shrink_to_fit() noexcept {}
		constexpr bool empty() const noexcept { return this->size == 0; }

		constexpr void clear()
		{
			std::destroy_n( data(), this->size );
			this->size = 0;
		}

		constexpr void move( small_vector_storage& other )
		{
			dbExpects( this->size == 0 );
			std::uninitialized_move_n( other.data(), other.size, data() );
			std::destroy_n( other.data(), other.size );
			this->size = std::exchange( other.size, 0 );
		}
	};

	template <typename T, std::size_t Size>
	struct sbo_vector_storage : public small_vector_storage<T, Size>
	{
		using parent = small_vector_storage<T, Size>;
		using size_type = typename parent::size_type;

		char* first = reinterpret_cast<char*>( &buffer );
		size_type capacity = Size;

		constexpr sbo_vector_storage() noexcept = default;
		constexpr sbo_vector_storage( size_type n )
		{
			if ( n > Size )
			{
				this->first = new char[ sizeof( T ) * n ];
				this->capacity = n;
			}
		}

		~sbo_vector_storage()
		{
			if ( !is_local() )
			{
				std::destroy_n( data(), this->size );
				delete[] this->first;
				this->size = 0;
			}
		}

		constexpr void init_reserve( size_type n )
		{
			dbExpects( n <= Size );
			dbExpects( this->size = 0 );
			dbExpects( this->first == local_data() );
			this->first = new char[ sizeof( T ) * n ];
			this->capacity = n;
		}

		constexpr T* data() noexcept { return reinterpret_cast<T*>( this->first ); }
		constexpr const T* data() const noexcept { return reinterpret_cast<const T*>( this->first ); }
		constexpr size_type max_size() const noexcept { return std::numeric_limits<size_type>::max(); }

		constexpr void reserve( size_type n )
		{
			if ( n > capacity )
				alloc_buffer_and_move( n );
		}

		constexpr void shrink_to_fit()
		{
			if ( is_local() )
				return;

			if ( this->size <= Size )
			{
				std::uninitialized_move_n( data(), this->size, local_data());
				std::destroy_n( data(), this->size );
				delete[] this->first;
				this->first = reinterpret_cast<char*>( local_data() );
				this->capacity = Size;
			}
			else
			{
				alloc_buffer_and_move( this->size );
			}
		}

		constexpr void move( sbo_vector_storage& other )
		{
			dbExpects( this->size = 0 );
			dbExpects( this->first == local_data() );
			if ( other.is_local() )
			{
				parent::move( other );
			}
			else
			{
				this->first = std::exchange( other.first, other.local_buffer() );
				this->size = std::exchange( other.size, 0 );
				this->capacity = std::exchange( other.capacity, Size );
			}
		}

	private:
		constexpr void alloc_buffer_and_move( size_type n )
		{
			dbExpects( n >= this->size );
			char* newData = new char[ sizeof( T ) * n ];

			std::uninitialized_move_n( data(), this->size, reinterpret_cast<T*>( newData ) );
			std::destroy_n( data(), this->size );

			if ( !is_local() )
				delete[] this->first;

			this->first = newData;
			this->capacity = n;
		}

	private:
		constexpr T* local_data() noexcept { return reinterpret_cast<T*>( &this->buffer ); }
		constexpr const T* local_data() const noexcept { return reinterpret_cast<T*>( &this->buffer ); }

		constexpr bool is_local() const noexcept { return this->first == reinterpret_cast<const T*>( &this->buffer ); }
	};
}

template <typename T, std::size_t Size, bool Resizable = false>
class vector_s
{
private:
	using storage_type = std::conditional_t<Resizable, detail::sbo_vector_storage<T, Size>, detail::small_vector_storage<T, Size>>;

public:
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	using iterator = T*;
	using const_iterator = const T*;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

public:

	// construction/assignment

	constexpr vector_s() noexcept = default;

	constexpr vector_s( size_type count, const T& value ) : m_storage{ count }
	{
		std::uninitialized_fill_n( m_storage.data(), count, value );
		m_storage.m_size = count;
	}

	constexpr vector_s( size_type count ) : m_storage{ count }
	{
		std::uninitialized_fill_n( m_storage.data(), count, T() );
		m_storage.m_size = count;
	}

	template <typename InputIt,
		std::enable_if_t<!std::is_integral_v<InputIt>, int> = 0>
	constexpr vector_s( InputIt first, InputIt last )
	{
		const auto count = stdx::narrow_cast<size_type>( std::distance( first, last ) );
		m_storage.init_reserve( count );
		std::uninitialized_copy( first, last, m_storage.data() );
		m_storage.size = count;
	}

	constexpr vector_s( const vector_s& other ) : vector_s{ other.begin(), other.end() } {}

	constexpr vector_s( vector_s&& other ) noexcept
	{
		m_storage.move( other.m_storage );
	}

	constexpr vector_s( std::initializer_list<T> init ) : vector_s{ init.begin(), init.end() } {}

	~vector_s() = default;

	constexpr vector_s& operator=( const vector_s& other )
	{
		assign( other.begin(), other.end() );
		return *this;
	}

	constexpr vector_s& operator=( vector_s&& other ) noexcept
	{
		clear();
		m_storage.move( other.m_storage );
		return *this;
	}

	constexpr vector_s& operator=( std::initializer_list<T> init )
	{
		assign( init.begin(), init.end() );
		return *this;
	}

	constexpr void assign( size_type count, const T& value )
	{
		m_storage.clear();
		m_storage.reserve( count );
		std::uninitialized_fill_n( m_storage.data(), count, value );
		m_storage.size = count;
	}

	template <typename InputIt,
		std::enable_if_t<!std::is_integral_v<InputIt>, int> = 0>
	constexpr void assign( InputIt first, InputIt last )
	{
		m_storage.clear();
		const auto count = stdx::narrow_cast<size_type>( std::distance( first, last ) );
		m_storage.reserve( count );
		std::uninitialized_copy( first, last, m_storage.data() );
		m_storage.size = count;
	}

	constexpr void assign( std::initializer_list<T> init )
	{
		assign( init.begin(), init.end() );
	}

	// element access

	constexpr T& at( size_type i )
	{
		if ( i >= m_storage.size )
			throw std::out_of_bounds();

		return m_storage.data()[ i ];
	}

	constexpr const T& at( size_type i ) const
	{
		if ( i >= m_storage.size )
			throw std::out_of_bounds();

		return m_storage.data()[ i ];
	}

	constexpr T& operator[]( size_type i ) noexcept
	{
		dbExpects( i < m_storage.size );
		return m_storage.data()[ i ];
	}

	constexpr const T& operator[]( size_type i ) const noexcept
	{
		dbExpects( i < m_storage.size );
		return m_storage.data()[ i ];
	}

	constexpr T& front() noexcept
	{
		dbExpects( !m_storage.empty() );
		return m_storage.data()[ 0 ];
	}

	constexpr const T& front() const noexcept
	{
		dbExpects( !m_storage.empty() );
		return m_storage.data()[ 0 ];
	}

	constexpr T& back() noexcept
	{
		dbExpects( !m_storage.empty() );
		return m_storage.data()[ m_storage.size - 1 ];
	}

	constexpr const T& back() const noexcept
	{
		dbExpects( !m_storage.empty() );
		return m_storage.data()[ m_storage.size - 1 ];
	}

	constexpr T* data() noexcept
	{
		return m_storage.data();
	}

	constexpr const T* data() const noexcept
	{
		return m_storage.data();
	}

	// iterators

	constexpr iterator begin() noexcept { return m_storage.data(); }
	constexpr iterator end() noexcept { return m_storage.data() + m_storage.size; }

	constexpr const_iterator begin() const noexcept { return m_storage.data(); }
	constexpr const_iterator end() const noexcept { return m_storage.data() + m_storage.size; }

	constexpr const_iterator cbegin() const noexcept { return m_storage.data(); }
	constexpr const_iterator cend() const noexcept { return m_storage.data() + m_storage.size; }

	constexpr reverse_iterator rbegin() noexcept { return end(); }
	constexpr reverse_iterator rend() noexcept { return begin(); }

	constexpr const_reverse_iterator rbegin() const noexcept { return end(); }
	constexpr const_reverse_iterator rend() const noexcept { return begin(); }

	constexpr const_reverse_iterator crbegin() const noexcept { return end(); }
	constexpr const_reverse_iterator crend() const noexcept { return begin(); }

	// capacity

	constexpr bool empty() const noexcept
	{
		return m_storage.empty();
	}

	constexpr size_type size() const noexcept
	{
		return m_storage.size;
	}

	constexpr difference_type ssize() const noexcept
	{
		return static_cast<difference_type>( m_storage.size );
	}

	constexpr size_type max_size() const noexcept
	{
		return m_storage.max_size();
	}

	constexpr void reserve( size_type n )
	{
		m_storage.reserve( n );
	}

	constexpr size_type capacity() const noexcept
	{
		return m_storage.capacity;
	}

	constexpr void shrink_to_fit()
	{
		m_storage.shrink_to_fit();
	}

	// modifiers

	constexpr void clear()
	{
		m_storage.clear();
	}

	constexpr iterator insert( const_iterator pos, const T& value )
	{
		if ( pos == end() )
			push_back( value );
		else
		{
			auto[ assign_start, mid, construct_end ] = shift_right( pos - begin(), 1 );
			*assign_start = value;
		}
	}

	constexpr iterator insert( const_iterator pos, T&& value )
	{
		if ( pos == end() )
			push_back( std::move( value ) );
		else
		{
			auto[ assign_start, mid, construct_end ] = shift_right( pos - begin(), 1 );
			*assign_start = std::move( value );
		}
	}

	constexpr iterator insert( const_iterator pos, size_type count, const T& value )
	{
		if ( count == 0 )
			return const_cast<iterator>( pos );

		auto[ assign_start, mid, construct_end ] = shift_right( pos - begin(), count );

		for ( auto it = assign_start; it != mid; ++it )
			*it = value;

		for ( auto it = mid; it != construct_end; ++it )
			new( it ) T( value );
	}

	template <typename InputIt,
		std::enable_if_t<!std::is_integral_v<InputIt>, int> = 0>
	constexpr iterator insert( const_iterator pos, InputIt first, const InputIt last )
	{
		dbExpects( first <= last );
		if ( first == last )
			return const_cast<iterator>( pos );

		auto[ assign_start, mid, construct_end ] = shift_right( pos - begin(), std::distance( first, last ) );
		for ( auto it = assign_start; it != mid; ++it, ++first )
			*it = *first;

		for ( auto it = mid; it != construct_end; ++it, ++first )
			new( it ) T( *first );
	}

	constexpr iterator insert( const_iterator pos, std::initializer_list<T> init )
	{
		return insert( init.begin(), init.end() );
	}

	template <typename... Args>
	constexpr iterator emplace( const_iterator pos, Args&&... args )
	{
		if ( pos == end() )
			emplace_back( std::forward<Args>( args )... );
		else
		{
			auto[ assign_start, mid, construct_end ] = shift_right( pos - begin(), 1 );
			*assign_start = T( std::forward<Args>( args )... );
		}
	}

	constexpr iterator erase( const_iterator pos )
	{
		dbExpects( begin() <= pos );
		dbExpects( pos < end() );

		T* p = m_storage.data() + ( pos - cbegin() );
		return shift_left( p, 1 );
	}

	constexpr iterator erase( const_iterator first, const_iterator last )
	{
		dbExpects( begin() <= first );
		dbExpects( first <= last );
		dbExpects( last < end() );

		T* p = m_storage.data() + ( first - cbegin() );
		return shift_left( p, static_cast<size_type>( last - first ) );
	}

	constexpr void push_back( const T& value )
	{
		reserve_extra( 1 );
		new( m_storage.data() + m_storage.size ) T( value );
		m_storage.size++;
	}

	constexpr void push_back( T&& value )
	{
		reserve_extra( 1 );
		new( m_storage.data() + m_storage.size ) T( std::move( value ) );
		m_storage.size++;
	}

	template <typename... Args>
	constexpr reference emplace_back( Args&&... args )
	{
		reserve_extra( 1 );
		new( m_storage.data() + m_storage.size ) T( std::forward<Args>( args )... );
		m_storage.size++;
	}

	constexpr void pop_back()
	{
		dbExpects( !m_storage.empty() );
		back().~T();
		m_storage.size--;
	}

	constexpr void resize( size_type count )
	{
		if ( count > m_storage.size )
		{
			m_storage.reserve( count );
			const auto first = m_storage.data() + m_storage.size;
			detail::construct( first, first + count );
			m_storage.size += count;
		}
		else if ( count < m_storage.size )
		{
			const auto last = m_storage.data() + m_storage.size;
			detail::destroy( last - count, last );
			m_storage.size -= count;
		}
	}

	constexpr void resize( size_type count, const T& value )
	{
		if ( count > m_storage.size )
		{
			m_storage.reserve( count );
			const auto first = m_storage.data() + m_storage.size;
			detail::construct( first, first + count, value );
			m_storage.size += count;
		}
		else if ( count < m_storage.size )
		{
			const auto last = m_storage.data() + m_storage.size;
			detail::destroy( last - count, last );
			m_storage.size -= count;
		}
	}

	constexpr void swap( vector_s& other ) noexcept
	{
		std::swap( *this, other );
	}

	friend constexpr bool operator==( const vector_s& lhs, const vector_s& rhs )
	{
		return lhs.size() == rhs.size() && std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() ) == 0;
	}

	friend constexpr bool operator!=( const vector_s& lhs, const vector_s& rhs )
	{
		return !( lhs == rhs );
	}

	friend constexpr bool operator<( const vector_s& lhs, const vector_s& rhs )
	{
		return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() ) < 0;
	}

	friend constexpr bool operator>( const vector_s& lhs, const vector_s& rhs )
	{
		return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() ) > 0;
	}

	friend constexpr bool operator<=( const vector_s& lhs, const vector_s& rhs )
	{
		return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() ) <= 0;
	}

	friend constexpr bool operator>=( const vector_s& lhs, const vector_s& rhs )
	{
		return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() ) >= 0;
	}

private:
	constexpr void reserve_extra( size_type count )
	{
		if ( m_storage.size + count > m_storage.capacity )
		{
			reserve( std::max( m_storage.size + count, m_storage.capacity * 2 ) );
		}
	}

	// leaves [pos, end) moved from, and allocates a new region [end, (pos+distance)) to be constructed later
	// returns two ranges, 1st to be assigned, second to be constructed
	constexpr std::tuple<T*, T*, T*> shift_right( const size_type pos, const size_type distance )
	{
		dbExpects( data() <= pos );
		dbExpects( pos <= data() + size() );

		reserve_extra( distance );

		T* p = m_storage.data() + pos;
		T* prev_end = end();

		if ( p < end() )
		{
			T* p = m_storage.data() + pos;
			T* move_construct_start = std::min( p, prev_end - distance );
			detail::move_construct( move_construct_start, prev_end, move_construct_start + distance );
			std::move( p, move_construct_start, p + distance );
			m_storage.size += distance;
			return { p, std::min( p + distance, prev_end ), p + distance };
		}
		else
		{
			m_storage.size += distance;
			return { prev_end, prev_end, prev_end + distance };
		}
	}

	// removes region [(pos-distance), pos)
	// return pointer to first erased element
	constexpr T* shift_left( T* pos, size_type distance )
	{
		dbExpects( data() <= pos );
		dbExpects( pos <= data() + size() );
		dbExpects( distance < size() );

		std::move( pos, end(), pos - distance );
		detail::destroy( end() - distance, end() );
		m_storage.size -= distance;
		return pos;
	}

private:
	storage_type m_storage;
};

template <typename T, std::size_t Size>
using small_vector = vector_s<T, Size, false>;

template <typename T, std::size_t Size>
using sbo_vector = vector_s<T, Size, true>;

}

#endif