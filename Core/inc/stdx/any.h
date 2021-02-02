#pragma once

#include <stdx/assert.h>
#include <stdx/type_traits.h>

namespace stdx
{

namespace detail
{

	struct any_small_vtable
	{
		using destroy_fn = void( * )( const void* );
		using copy_fn = void( * )( void*, const void* );
		using move_fn = void( * )( void*, void* );

		template <typename T>
		static void destroy_imp( const void* obj )
		{
			std::destroy_at( static_cast<const T*>( obj ) );
		}

		template <typename T>
		static void copy_imp( void* dest, const void* src )
		{
			new ( dest ) T( *static_cast<const T*>( src ) );
		}

		template <typename T>
		static void move_imp( void* dest, void* src )
		{
			new ( dest ) T( std::move( *static_cast<T*>( src ) ) );
		}

		destroy_fn destroy;
		copy_fn copy;
		move_fn move;
	};

	template <typename T>
	inline constexpr any_small_vtable any_small_vtable_obj
	{
		&any_vtable::template destroy_imp<T>,
		&any_vtable::template copy_imp<T>,
		&any_vtable::template move_imp<T>
	};

	struct any_big_vtable
	{
		using destroy_fn = void( * )( const void* );
		using copy_fn = void( * )( void*, const void* );

		template <typename T>
		static void destroy_imp( const void* obj )
		{
			std::destroy_at( static_cast<const T*>( obj ) );
		}

		template <typename T>
		static void copy_imp( void* dest, const void* other )
		{
			new ( dest ) T( *static_cast<const T*>( other ) );
		}

		destroy_fn destroy;
		copy_fn copy;
	};

	template <typename T>
	inline constexpr any_big_vtable any_big_vtable_obj
	{
		&any_vtable::template destroy_imp<T>,
		&any_vtable::template copy_imp<T>
	};

	template <typename T>
	struct in_place_type {};

} // namespace detail

// std::any type with customizable small size buffer

template <std::size_t N>
class basic_any
{
	static constexpr std::size_t small_size_v = N;
	static constexpr std::size_t trivial_size_v = N + sizeof( void* );

	template <typename T>
	static constexpr bool any_is_trivial_v = alignof( T ) <= alignof( max_align_t ) &&
		std::is_trivially_copyable_v<T> &&
		sizeof( T ) <= trivial_size_v;

	template <typename T>
	static constexpr bool any_is_small_v = alignof( T ) <= alignof( max_align_t ) &&
		std::is_nothrow_move_constructible_v<T> &&
		sizeof( T ) <= small_size_v;

	enum class rep : uintptr_t
	{
		empty = 0,
		trivial = 1,
		small = 2,
		big = 3
	};

	static constexpr uintptr_t rep_mask = 0x3;

public:
	basic_any() noexcept : m_type { 0 } {}

	basic_any( const basic_any& other )
	{
		copy( other );
	}

	basic_any( basic_any&& other ) noexcept
	{
		move( std::move( other ) );
	}

	template <typename T>
	basic_any( T&& t )
	{
		emplace_imp<std::decay_t<T>>( std::forward<T>( t ) );
	}

	template <typename T, typename... Args>
	explicit basic_any( detail::in_place_type<T>, Args&&... args )
	{
		emplace_imp<T>( std::forward<Args>( args )... );
	}

	template <typename T, typename U, typename... Args>
	explicit basic_any( detail::in_place_type<T>, std::initializer_list<U> init, Args&&... args )
	{
		emplace_imp<T>( init, std::forward<Args>( args )... );
	}

	~basic_any() noexcept
	{
		reset();
	}

	basic_any& operator=( const basic_any& other )
	{
		reset();
		copy( other );
		return *this;
	}

	basic_any& operator=( basic_any&& other )
	{
		reset();
		move( other );
		return *this;
	}

	template <typename T>
	basic_any& operator=( T&& t )
	{
		reset();
		emplace_imp<std::decay_t<T>>( std::forward<T>( t ) );
		return *this;
	}

	template <typename T, typename... Args>
	T& emplace( Args&&... args )
	{
		reset();
		return emplace_imp<T>( std::forward<Args>( args )... );
	}

	void reset() noexcept
	{
		switch ( get_rep() )
		{
			case rep::empty:
			case rep::trivial:
				break;

			case rep::small:
				m_small.vtable->destroy( &m_small.data );
				break;

			case rep::big:
				m_big.vtable->destroy( &m_big.data );
				break;
		}
		m_type = 0;
	}

	void swap( basic_any& other ) noexcept
	{
		other = std::exchange( *this, std::move( other ) );
	}

	[[nodiscard]] bool has_value() const noexcept
	{
		return m_type != 0;
	}

	[[nodiscard]] const std::type_info& type() const noexcept
	{
		auto* info = get_type();
		return info ? *info : typeid( void );
	}

	template <typename T>
	T* cast()
	{
		static_assert( !std::is_void_v<T> );
		static_assert( stdx::is_decayed_v<T> );

		auto* info = get_type();
		return ( info && *info == typeid( T ) )
			? reinterpret_cast<T*>( get_data() )
			: nullptr;
	}

	template <typename T>
	const T* cast() const
	{
		static_assert( !std::is_void_v<T> );
		static_assert( stdx::is_decayed_v<T> );

		auto* info = get_type();
		return ( info && *info == typeid( T ) )
			? reinterpret_cast<const T*>( get_data() )
			: nullptr;
	}

private:
	void copy( const basic_any& other )
	{
		m_type = other.m_type;
		switch ( get_rep() )
		{
			case rep::empty:
				break;

			case rep::trivial:
				m_trivial = other.m_trivial;
				break;

			case rep::small:
				m_small.vtable = other.m_small.vtable;
				m_small.vtable->copy( &m_small.data, &other.m_small.data );
				break;

			case rep::big:
				m_big.vtable = other.m_big.vtable;
				m_big.vtable->copy( &m_big.data, &other.m_big.data );
				break;
		}
	}

	void move( basic_any&& other )
	{
		m_type = std::exchange( other.m_type, 0 );
		switch ( get_rep() )
		{
			case rep::empty:
				break;

			case rep::trivial:
				m_trivial = other.m_trivial;
				break;

			case rep::small:
				m_small.vtable = other.m_small.vtable;
				m_small.vtable->move( &m_small.data, &other.m_small.data );
				break;

			case rep::big:
				m_big.vtable = other.m_big.vtable;
				m_big.vtable->copy( &m_big.data, &other.m_big.data );
				break;
		}
	}

	template <typename T, typename... Args>
	T& emplace_imp( Args&&... args )
	{
		dbExpects( !has_value() );

		static_assert( !std::is_void_v<T> );
		static_assert( stdx::is_decayed_v<T> );

		if constexpr ( any_is_trivial_v<T> )
		{
			T& dest = reinterpret_cast<T&>( m_trivial.data );
			dest = std::forward<T>( std::forward<Args>( args )... );
			m_type = reinterpret_cast<uintptr_t>( &typeid( T ) ) | static_cast<uintptr_t>( rep::trivial );
			return dest;
		}
		else if constexpr ( any_is_small_v<T> )
		{
			T& dest = reinterpret_cast<T&>( m_small.data );
			detail::construct_in_place( dest, std::forward<Args>( args )... );
			m_small.vtable = &detail::any_small_vtable_obj<T>;
			m_type = reinterpret_cast<uintptr_t>( &typeid( T ) ) | static_cast<uintptr_t>( rep::small );
			return dest;
		}
		else
		{
			T*& dest = reinterpret_cast<T*&>( m_big.data );
			dest = new T( std::forward<Args>( args )... );
			m_big.vtable = &detail::any_big_vtable_obj<T>;
			m_type = reinterpret_cast<uintptr_t>( &typeid( T ) ) | static_cast<uintptr_t>( rep::big );
			return *dest;
		}
	}

	const rep get_rep() const noexcept
	{
		return static_cast<rep>( m_type & rep_mask );
	}

	const std::type_info* get_type() const noexcept
	{
		return reinterpret_cast<const std::type_info*>( m_type & ~rep_mask );
	}

	void* get_data() noexcept
	{
		switch ( get_rep() )
		{
			case rep::empty: return nullptr;
			case rep::trivial: return &m_trivial.data;
			case rep::small: return &m_small.data;
			case rep::big: return &m_big.data;
		}

		dbBreak();
		return nullptr;
	}

	const void* get_data() const noexcept
	{
		switch ( get_rep() )
		{
			case rep::empty: return nullptr;
			case rep::trivial: return &m_trivial.data;
			case rep::small: return &m_small.data;
			case rep::big: return &m_big.data;
		}

		dbBreak();
		return nullptr;
	}

private:
	// align storage types to void* for optimal packing. We use m_dummy to align to max_align_t

	using trivial_storage = std::aligned_storage_t<trivial_size_v, sizeof( void* )>;

	struct small_storage
	{
		std::aligned_storage_t<small_size_v, sizeof( void* )> data;
		detail::any_small_vtable* vtable;
	};

	struct big_storage
	{
		std::aligned_storage_t<small_size_v - sizeof( void* ), sizeof( void* )> padding; // push active members closer to m_type
		void* data;
		detail::any_big_vtable* vtable;
	};

	static_assert( sizeof( trivial_storage ) == sizeof( small_storage ) );
	static_assert( sizeof( trivial_storage ) == sizeof( big_storage ) );

	union
	{
		struct
		{
			union
			{
				trivial_storage m_trivial;
				small_storage m_small;
				big_storage m_big;
			};
			uintptr_t m_type;
		};
		max_align_t m_dummy;
	};
};

template <std::size_t N, typename T, typename... Args>
basic_any<N> basic_make_any( Args&&... args )
{
	return basic_any<N>( detail::in_place_type<T>{}, std::forward<Args>( args )... );
}

template <std::size_t N, typename T, typename U, typename... Args>
basic_any<N> basic_make_any( std::initializer_list<U> init, Args&&... args )
{
	return basic_any<N>( detail::in_place_type<T>{}, init, std::forward<Args>( args )... );
}

template <typename T, std::size_t N>
T* any_cast( basic_any<N>* a )
{
	return a ? a->cast<T>() : nullptr;
}

template <typename T, std::size_t N>
const T* any_cast( const basic_any<N>* a )
{
	return a ? a->cast<T>() : nullptr;
}

template <typename T, std::size_t N>
T any_cast( basic_any<N>& a )
{
	auto* value = any_cast<T>( &a );
	dbAssert( data );
	return *value;
}

template <typename T, std::size_t N>
T any_cast( const basic_any<N>& a )
{
	auto* value = any_cast<T>( &a );
	dbAssert( data );
	return *value;
}

template <typename T, std::size_t N>
T any_cast( basic_any<N>&& a )
{
	auto* value = any_cast<T>( &a );
	dbAssert( data );
	return std::move( *value );
}

using any = basic_any<sizeof( void* ) * 6>;

static_assert( sizeof( any ) == sizeof( void* ) * 8 );

template <typename T, typename... Args>
any make_any( Args&&... args )
{
	return any( detail::in_place_type<T>{}, std::forward<Args>( args )... );
}

template <typename T, typename U, typename... Args>
any make_any( std::initializer_list<U> init, Args&&... args )
{
	return any( detail::in_place_type<T>{}, init, std::forward<Args>( args )... );
}

}