#pragma once

#include <stdx/assert.h>
#include <type_traits>

namespace stdx
{

namespace detail
{
struct make_polymorphic_ptr_tag {};
}

// pointer type with small buffer optimization for POD types
template <typename T, size_t LocalSize>
class polymorphic_ptr
{
	template <typename U>
	static constexpr bool can_inline_v = ( sizeof( U ) <= LocalSize );

	static_assert( std::is_class_v<T>, "T must be a struct or class" );
	static_assert( !std::is_union_v<T>, "T cannot be a union" );
	static_assert( can_inline_v<T>, "T must fit within local buffer and be trivially move constructable" );

public:
	using element_type = T;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;

	// construction/assignment

	polymorphic_ptr() = default;

	polymorphic_ptr( const polymorphic_ptr& ) = delete;

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	polymorphic_ptr( polymorphic_ptr<U, LocalSize>&& other ) noexcept
	{
		move_no_reset( std::move( other ) );
	}

	template<typename... Args>
	explicit polymorphic_ptr( detail::make_polymorphic_ptr_tag, Args&&... args )
	{
		m_object = reinterpret_cast<T*>( &m_buffer );
		new ( m_object ) T( std::forward<Args>( args )... );
	}

	~polymorphic_ptr()
	{
		reset();
	}

	polymorphic_ptr& operator=( const polymorphic_ptr& ) = delete;

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	polymorphic_ptr& operator=( polymorphic_ptr<U, LocalSize>&& other ) noexcept
	{
		reset();
		move_no_reset( std::move( other ) );
		return *this;
	}

	// modifiers

	void reset() noexcept
	{
		if ( m_object )
		{
			if ( is_local() )
				m_object->~T();
			else
				delete m_object;

			m_object = nullptr;
		}
	}

	void swap( polymorphic_ptr& other ) noexcept
	{
		auto temp = std::move( *this );
		*this = std::move( other );
		other = std::move( temp );
	}

	// observers

	T* get() noexcept { return m_object; }

	const T* get() const noexcept { return m_object; }

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	U* get() noexcept
	{
		dbAssert( dynamic_cast<U>( m_object ) );
		return static_cast<U>( m_object );
	}

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	const U* get() const noexcept
	{
		dbAssert( dynamic_cast<U>( m_object ) );
		return static_cast<U>( m_object );
	}

	operator bool() const noexcept
	{
		return m_object != nullptr;
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

	bool is_local() const noexcept
	{
		return m_object == reinterpret_cast<const T*>( &m_buffer );
	}

private:

	template <typename U, std::size_t S> friend class polymorphic_ptr;

	template <typename U,
		std::enable_if_t<std::is_base_of_v<T, U>, int> = 0>
	void move_no_reset( polymorphic_ptr<U, LocalSize>&& other ) noexcept
	{
		dbAssert( static_cast<void*>( &other ) != static_cast<void*>( this ) );
		if ( other )
		{
			if ( other.is_local() )
			{
				m_object = reinterpret_cast<T*>( &m_buffer );
				m_buffer = other.m_buffer;
			}
			else
			{
				m_object = other.m_object;
			}

			other.m_object = nullptr;
		}
	}

private:
	T* m_object = nullptr;
	std::aligned_storage_t<LocalSize> m_buffer;
};

template <typename T, size_t LocalSize, typename... Args>
polymorphic_ptr<T, LocalSize> make_polymorphic_ptr( Args&&... args )
{
	return polymorphic_ptr<T, LocalSize>( detail::make_polymorphic_ptr_tag{}, std::forward<Args>( args )... );
}

} // namespace stdx