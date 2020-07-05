#pragma once

#include <stdx/assert.h>

#include <cstring>
#include <type_traits>

namespace stdx
{

template <typename T, size_t LocalSize>
class polymorphic
{
	static_assert( std::is_class_v<T>, "T must be a struct or class" );

	template <typename U>
	static bool can_inline_v = ( sizeof( U ) <= LocalSize ) && std::is_trivially_copyable_v<U>;

public:
	using element_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	polymorphic() = default;

	polymorphic( const polymorphic& ) = delete;

	polymorphic( polymorphic&& other )
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
	polymorphic( const U& other )
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
	polymorphic( U&& other )
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

	~polymorphic()
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

	polymorphic& operator=( const polymorphic& ) = delete;

	polymorphic& operator=( polymorphic&& other ) noexcept
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

	void swap( polymorphic& other ) noexcept
	{
		polymorphic temp{ std::move( *this ) }
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
	BaseType* m_object = nullptr;
	std::aligned_storage_t<LocalSize> m_buffer;
};



}