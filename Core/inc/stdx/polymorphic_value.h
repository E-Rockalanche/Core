#pragma once

#include <memory>
#include <type_traits>

namespace stdx
{

template <typename Base, std::size_t BufferSize = sizeof( Base )>
class polymorphic_value
{
	struct metadata
	{
		using destroy_fn = void( * )( const void* );
		using copy_fn = void( * )( void*, const void* );
		using move_fn = void( * )( void*, void* );

		template <typename T>
		static void destroy_imp( const void* obj ) { std::destroy_at( static_cast<const T*>( obj ) ); }

		template <typename T>
		static void copy_imp( void* dest, const void* other ) { new( dest ) T( *static_cast<const T*>( other ) ); }

		template <typename T>
		static void move_imp( void* dest, void* src ) { new( dest ) T( std::move( *static_cast<T*>( src ) ) ); }

		destroy_fn destroy;
		copy_fn copy;
		move_fn move;
		std::size_t size;
	};

	template <typename T>
	static inline metadata s_metadata
	{
		&metadata::template destroy_imp<T>,
		&metadata::template copy_imp<T>,
		&metadata::template move_imp<T>,
		sizeof( T )
	};

	enum class rep : uintptr_t
	{
		null,
		small,
		big
	};

	static constexpr uintptr_t rep_mask = 0x3;

	using small_storage = std::aligned_storage<BufferSize, sizeof( void* )>;
	
	struct big_storage
	{
		std::aligned_storage<BufferSize - sizeof( void* ), sizeof( void* )> padding;
		void* data;
	};

public:
	using pointer = Base*;
	using element_type = Base;

	polymorphic_value() noexcept : m_metadata{ 0 } {}

	polymorphic_value( const polymorphic_value& other )
	{
		copy_imp( other );
	}

	polymorphic_value( polymorphic_value&& other ) noexcept
	{
		move_imp( std::move( other ) );
	}

	template <typename T>
	polymorphic_value( T&& value )
	{
		emplace_imp<T>( std::forward<T>( value ) );
	}

	template <typename T, typename... Args>
	polymorphic_value( std::in_place_t, Args&&... args )
	{
		emplace_imp<T>( std::forward<Args>( args )... );
	}

	~polymorphic_value()
	{
		reset();
	}

	polymorphic_value& operator=( const polymorphic_value& other )
	{
		reset();
		copy_imp( other );
		return *this;
	}

	polymorphic_value& operator=( polymorphic_value&& other )
	{
		reset();
		move_imp( std::move( other ) );
		return *this;
	}

	template <typename T>
	polymorphic_value& operator=( T&& value )
	{
		reset();
		emplace_imp<T>( std::forward<T>( value ) );
		return *this;
	}

	Base* get() noexcept
	{
		switch ( get_rep() )
		{
			default:
			case rep::null:
				return nullptr;

			case rep::small:
				return reinterpret_cast<Base*>( &m_small );

			case rep::big:
				return static_cast<Base*>( m_big.data );
		}
	}

	const Base* get() const noexcept
	{
		switch ( get_rep() )
		{
			default:
			case rep::null:
				return nullptr;

			case rep::small:
				return reinterpret_cast<const Base*>( &m_small );

			case rep::big:
				return static_cast<const Base*>( m_big.data );
		}
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
			case rep::null:
				break;

			case rep::small:
				std::invoke( get_meta()->destroy, &m_small );
				m_metadata = 0;
				break;

			case rep::big:
				std::invoke( get_meta()->destroy, &m_big.data );
				m_metadata = 0;
				break;
		}
	}

	void swap( polymorphic_value& other ) noexcept
	{
		*this = std::exchange( other, *this );
	}

	bool has_value() const noexcept
	{
		return m_metadata != 0;
	}

private:

	void copy_imp( const polymorphic_value& other )
	{
		m_metadata = other.m_metadata;

		switch ( get_rep() )
		{
			case rep::null:
				break;

			case rep::small:
				std::invoke( get_meta()->copy, &m_small, &other.m_small );
				break;

			case rep::big:
				m_big.data = new char[ get_meta()->size ];
				std::invoke( get_meta()->copy, m_big.data, other.m_big.data );
				break;
		}
	}

	void move_imp( polymorphic_value&& other ) noexcept
	{
		m_metadata = other.m_metadata;

		switch ( get_rep() )
		{
			case rep::null:
				break;

			case rep::small:
				std::invoke( get_meta()->move, &m_small, &other.m_small );

				// destroying other value now so it is left with a null value
				std::invoke( get_meta()->destroy, &other.m_small );
				other.m_metadata = 0;
				break;

			case rep::big:
				m_big.data = std::exchange( other.m_big.data, nullptr );
				other.m_metadata = 0;
				break;
		}
	}

	template <typename T, typename... Args>
	T& emplace_imp( Args&&... args )
	{
		static_assert( !std::is_const_v<std::decay_t<T>> );
		static_assert( std::is_base_of_v<Base, T> );

		set_meta( &s_metadata<T> );

		if constexpr ( sizeof( T ) <= BufferSize )
		{
			m_metadata &= static_cast<uintptr_t>( rep::small );
			new ( static_cast<void*>( &m_small ) ) T( std::forward<Args>( args )... );
			return *reinterpret_cast<T*>( &m_small );
		}
		else
		{
			m_metadata &= static_cast<uintptr_t>( rep::big );
			m_big.data = new char[ sizeof( T ) ];
			new ( m_big.data ) T( std::forward<Args>( args )... );
			return *static_cast<T*>( m_big.data );
		}
	}

	rep get_rep() const noexcept
	{
		return static_cast<rep>( m_metadata & rep_mask );
	}

	const metadata* get_meta() const noexcept
	{
		return reinterpret_cast<const metadata*>( m_metadata & ~rep_mask );
	}

	void set_meta( const metadata* metadata ) noexcept
	{
		m_metadata = reinterpret_cast<uintptr_t>( metadata );
	}

private:
	union
	{
		struct
		{
			union
			{
				small_storage m_small;
				big_storage m_big;
			};
			uintptr_t m_metadata;
		};
		max_align_t m_aligner;
	};
};



}