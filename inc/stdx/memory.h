#ifndef STDX_MEMORY_HPP
#define STDX_MEMORY_HPP

#include <memory>

namespace stdx
{

template <typename T>
constexpr T* to_address( T* p ) noexcept
{
	static_assert( !std::is_function_v<T> );
	return p;
}

template <typename T>
constexpr auto to_address( const T& p ) noexcept
{
	return to_address( p.operator->() );
}

template <typename T,
	std::enable_if_t<!std::is_array_v<T>, int> = 0>
std::unique_ptr<T> make_unique_for_overwrite()
{
	return std::unique_ptr<T>( new T );
}

template <typename T,
	std::enable_if_t<std::is_array_v<T>, int> = 0>
std::unique_ptr<T> make_unique_for_overwrite( std::size_t size )
{
	return std::unique_ptr<T>( new typename std::remove_extent<T>::type[ size ] );
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_for_overwrite( Args&&... args ) = delete;

// pointer type with small buffer optimization for POD types
template <typename T, size_t LocalSize>
class polymorphic_ptr
{
	static_assert( std::is_class_v<T>, "T must be a struct or class" );
	static_assert( !std::is_union_v<T>, "T cannot be a union" );

	template <typename U>
	static constexpr bool can_inline_v = ( sizeof( U ) <= LocalSize ) && std::is_trivially_copyable_v<U>;

public:
	using element_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	polymorphic_ptr() = default;

	polymorphic_ptr( const polymorphic_ptr& ) = delete;

	polymorphic_ptr( polymorphic_ptr&& other )
	{
		dbAssert( other.m_object );
		if ( other.is_local() )
		{
			m_object = &m_buffer.data;
			std::memcpy( m_buffer.data, other.m_buffer.data, LocalSize );
		}
		else
		{
			m_object = std::exchange( other.m_object, nullptr );
		}
	}

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	polymorphic_ptr( const U& other )
	{
		if ( can_inline_v<U> )
		{
			m_object = &m_buffer.data;
			new ( m_object ) U( other );
		}
		else
		{
			m_object = new U( other );
		}
	}

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	polymorphic_ptr( U&& other )
	{
		if ( can_inline_v<U> )
		{
			m_object = &m_buffer.data;
			new ( m_object ) U( other );
		}
		else
		{
			m_object = new U( std::move( other ) );
		}
	}

	~polymorphic_ptr()
	{
		if ( is_local() )
		{
			m_object->~T();
		}
		else if ( m_object )
		{
			delete m_object;
		}
	}

	polymorphic_ptr& operator=( const polymorphic_ptr& ) = delete;

	polymorphic_ptr& operator=( polymorphic_ptr&& other ) noexcept
	{
		if ( !is_local() && m_object )
			delete m_object;

		if ( other.is_local() )
		{
			m_object = &m_buffer.data;
			std::memcpy( m_buffer.data, other.m_buffer.data, LocalSize );
		}
		else
		{
			m_object = std::exchange( other.m_object, nullptr );
		}
	}

	T* operator->() noexcept
	{
		dbAssert( m_object );
		return m_object;
	}

	const T* operator->() const noexcept
	{
		dbAssert( m_object );
		return m_object;
	}

	T& operator*() noexcept
	{
		dbAssert( m_object );
		return *m_object;
	}

	const T& operator*() const noexcept
	{
		dbAssert( m_object );
		return *m_object;
	}

	template <typename U = T,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
		U* get() noexcept
	{
		return static_cast<U>( m_object );
	}

	template <typename U = T,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
		const U* get() const noexcept
	{
		return static_cast<U>( m_object );
	}

	void swap( polymorphic_ptr& other ) noexcept
	{
		polymorphic_ptr temp{ std::move( *this ) };
		*this = std::move( other );
		other = std::move( temp );
	}

	operator bool() const noexcept
	{
		return m_object != nullptr;
	}

private:
	bool is_local() const noexcept
	{
		return static_cast<void*>( m_object ) == static_cast<void*>( m_buffer.data );
	}

private:
	T* m_object = nullptr;
	std::aligned_storage_t<LocalSize> m_buffer;
};

} // namespace stdx

#endif