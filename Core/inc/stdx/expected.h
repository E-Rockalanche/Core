#pragma once

#include <optional>
#include <type_traits>
#include <variant>

namespace stdx
{

template <typename E>
class unexpected
{
	static_assert( !std::is_same_v<E, void> );

public:
	unexpected() = delete;

	constexpr unexpected( const E& val ) : m_value( val ) {}
	constexpr unexpected( E&& val ) : m_value( std::move( val ) ) {}

	constexpr E& value() & { return m_value; }
	constexpr const E& value() const & { return m_value; }

	constexpr E&& value() && { return std::move( m_value ); }
	constexpr const E&& value() const && { return std::move( m_value ); }

private:
	E m_value;
};

template <typename E>
constexpr bool operator==( const unexpected<E>& lhs, const unexpected<E>& rhs )
{
	return lhs.value() == rhs.value();
}

template <typename E>
constexpr bool operator!=( const unexpected<E>& lhs, const unexpected<E>& rhs )
{
	return lhs.value() != rhs.value();
}

struct unexpect_t
{
	unexpect_t() = default;
};

inline constexpr unexpect_t unexpect{};

template <typename T, typename E>
class expected
{
public:
	using value_type = T;
	using error_type = E;
	using unexpected_type = unexpected<E>;

	template<class U>
	using rebind = expected<U, error_type>;

	constexpr expected() = default;

	constexpr expected( const expected& ) = default;

	constexpr expected( expected&& other ) noexcept
	{
		if ( other.has_value() )
			m_value = std::move( *other );
		else
			m_value = unexpected( std::move( other.error() ) );
	}

	template <typename U, typename G>
	explicit constexpr expected( const expected<U, G>& other )
	{
		if ( other.has_value() )
			m_value.emplace<T>( *other );
		else
			m_value.emplace<unexpected_type>( other.error() );
	}

	template <typename U, typename G>
	explicit constexpr expected( expected<U, G>&& other )
	{
		if ( other.has_value() )
			m_value.emplace<T>( std::move( *other ) );
		else
			m_value.emplace<unexpected_type>( std::move( other ).error() );
	}

	template <typename U = T>
	explicit constexpr expected( U&& v )
		: m_value( std::in_place_type<U>, std::forward<U>( v ) )
	{}

	template <typename... Args>
	constexpr explicit expected( std::in_place_t, Args&&... args )
		: m_value( std::in_place_type<T>, std::forward<Args>( args )... )
	{}

	template <typename U, typename... Args>
	constexpr explicit expected( std::in_place_t, std::initializer_list<U> init, Args&&... args )
		: m_value( std::in_place_type<T>, init, std::forward<Args>( args )... )
	{}

	template <typename G = E>
	constexpr explicit expected( const unexpected<G>& u )
		: m_value( std::in_place_type<unexpected_type>, u.value() )
	{}

	template <typename G = E>
	constexpr explicit expected( unexpected<G>&& u )
		: m_value( std::in_place_type<unexpected_type>, std::move( u ).value() )
	{}

	template <typename... Args>
	constexpr explicit expected( unexpect_t, Args&&... args )
		: m_value( std::in_place_type<unexpected_type>, E( std::forward<Args>( args )... ) )
	{}

	template <typename U, typename... Args>
	constexpr explicit expected( unexpect_t, std::initializer_list<U> init, Args&&... args )
		: m_value( std::in_place_type<unexpected_type>, E( init, std::forward<Args>( args )... ) )
	{}

	~expected() = default;

	// assignment

	expected& operator=( const expected& ) = default;
	expected& operator=( expected&& ) noexcept = default;

	template <typename U = T>
	expected& operator=( U&& v )
	{
		m_value.emplace<T>( std::forward<U>( v ) );
		return *this;
	}

	template <typename G = E>
	expected& operator=( const unexpected<G>& u )
	{
		m_value.emplace<unexpected_type>( u.value() );
		return *this;
	}

	template <typename G = E>
	expected& operator=( unexpected<G>&& u ) noexcept
	{
		m_value.emplace<unexpected_type>( std::move( u ).value() );
		return *this;
	}

	template <typename... Args>
	void emplace( Args&&... args )
	{
		m_value.emplace<T>( std::forward<Args>( args )... );
		return *this;
	}

	template <typename U, typename... Args>
	void emplace( std::initializer_list<U> init, Args&&... args )
	{
		m_value.emplace<T>( init, std::forward<Args>( args )... );
		return *this;
	}

	// modifiers

	void swap( expected& other ) noexcept
	{
		m_value.swap( other.m_value );
	}

	// observers

	constexpr T* operator->()
	{
		return std::addressof( std::get<T>( m_value ) );
	}

	constexpr const T* operator->() const
	{
		return std::addressof( std::get<T>( m_value ) );
	}

	constexpr T& operator*() &
	{
		return std::get<T>( m_value );
	}

	constexpr const T& operator*() const &
	{
		return std::get<T>( m_value );
	}

	constexpr T&& operator*() &&
	{
		return std::move( std::get<T>( m_value ) );
	}

	constexpr const T&& operator*() const &&
	{
		return std::move( std::get<T>( m_value ) );
	}

	constexpr bool has_value() const noexcept
	{
		return m_value.index() == 0;
	}

	constexpr explicit operator bool() const noexcept
	{
		return has_value();
	}

	constexpr T& value() &
	{
		return std::get<T>( m_value );
	}

	constexpr const T& value() const &
	{
		return std::get<T>( m_value );
	}

	constexpr T&& value() &&
	{
		return std::move( std::get<T>( m_value ) );
	}

	constexpr const T&& value() const &&
	{
		return std::move( std::get<T>( m_value ) );
	}

	constexpr E& error() &
	{
		return std::get<unexpected_type>( m_value ).value();
	}

	constexpr const E& error() const &
	{
		return std::get<unexpected_type>( m_value ).value();
	}

	constexpr E&& error() &&
	{
		return std::get<unexpected_type>( std::move( m_value ) ).value();
	}

	constexpr const E&& error() const &&
	{
		return std::get<unexpected_type>( std::move( m_value ) ).value();
	}

	template <typename U>
	constexpr T value_or( U&& default_value ) const&
	{
		return has_value() ? std::get<T>( m_value ) : std::forward<U>( default_value );
	}

	template <typename U>
	constexpr T value_or( U&& default_value ) &&
	{
		return has_value() ? std::get<T>( std::move( m_value ) ) : std::forward<U>( default_value );
	}

private:
	std::variant<T, unexpected_type> m_value;
};

template <typename E>
class expected<void, E>
{
public:
	using error_type = E;
	using unexpected_type = unexpected<E>;

	template<class U>
	using rebind = expected<U, error_type>;

	constexpr expected() = default;

	constexpr expected( const expected& ) = default;

	constexpr expected( expected&& other ) noexcept
	{
		if ( !other.has_value() )
			m_value = unexpected( std::move( other.error() ) );
	}

	template <typename G>
	explicit constexpr expected( const expected<void, G>& other )
	{
		if ( !other.has_value() )
			m_value.emplace<unexpected_type>( other.error() );
	}

	template <typename G>
	explicit constexpr expected( expected<void, G>&& other )
	{
		if ( !other.has_value() )
			m_value.emplace<unexpected_type>( std::move( other ).error() );
	}

	template <typename G = E>
	constexpr explicit expected( const unexpected<G>& u )
		: m_value( std::in_place, u.value() )
	{}

	template <typename G = E>
	constexpr explicit expected( unexpected<G>&& u )
		: m_value( std::in_place, std::move( u ).value() )
	{}

	template <typename... Args>
	constexpr explicit expected( unexpect_t, Args&&... args )
		: m_value( std::in_place, E( std::forward<Args>( args )... ) )
	{}

	template <typename U, typename... Args>
	constexpr explicit expected( unexpect_t, std::initializer_list<U> init, Args&&... args )
		: m_value( std::in_place, E( init, std::forward<Args>( args )... ) )
	{}

	~expected() = default;

	// assignment

	expected& operator=( const expected& ) = default;
	expected& operator=( expected&& ) noexcept = default;

	template <typename G = E>
	expected& operator=( const unexpected<G>& u )
	{
		m_value.emplace<unexpected_type>( u.value() );
		return *this;
	}

	template <typename G = E>
	expected& operator=( unexpected<G>&& u ) noexcept
	{
		m_value.emplace<unexpected_type>( std::move( u ).value() );
		return *this;
	}

	// modifiers

	void swap( expected& other ) noexcept
	{
		m_value.swap( other.m_value );
	}

	// observers

	constexpr explicit operator bool() const noexcept
	{
		return !m_value.has_value();
	}

	constexpr bool has_value() const noexcept
	{
		return !m_value.has_value();
	}

	constexpr E& error() &
	{
		return m_value.value().value();
	}

	constexpr const E& error() const &
	{
		return m_value.value().value();
	}

	constexpr E&& error() &&
	{
		return std::move( m_value ).value().value();
	}

	constexpr const E&& error() const &&
	{
		return std::move( m_value ).value().value();
	}

private:
	std::optional<unexpected_type> m_value;
};

// expected relational operators

template <typename T, typename E>
constexpr bool operator==( const expected<T, E>& lhs, const expected<T, E>& rhs )
{
	if ( lhs.has_value() && rhs.has_value() )
		return *lhs == *rhs;
	else
		return lhs.error() == rhs.error();
}

template <typename E>
constexpr bool operator==( const expected<void, E>& lhs, const expected<void, E>& rhs )
{
	if ( lhs.has_value() && rhs.has_value() )
		return true;
	else
		return lhs.error() == rhs.error();
}

template <typename T, typename E>
constexpr bool operator!=( const expected<T, E>& lhs, const expected<T, E>& rhs )
{
	return !( lhs == rhs );
}

// comparison with T

template <typename T, typename E>
constexpr bool operator==( const expected<T, E>& lhs, const T& rhs )
{
	return lhs.has_value() && *lhs == rhs;
}

template <typename T, typename E>
constexpr bool operator!=( const expected<T, E>& lhs, const T& rhs )
{
	return !( lhs == rhs );
}

template <typename T, typename E>
constexpr bool operator==( const T& lhs, const expected<T, E>& rhs )
{
	return rhs.has_value() && lhs == *rhs;
}

template <typename T, typename E>
constexpr bool operator!=( const T& lhs, const expected<T, E>& rhs )
{
	return !( lhs == rhs );
}

// comparison with unexpected

template <typename T, typename E>
constexpr bool operator==( const expected<T, E>& lhs, const unexpected<E>& rhs )
{
	return !lhs.has_value() && lhs.error() == rhs.value();
}

template <typename T, typename E>
constexpr bool operator!=( const expected<T, E>& lhs, const unexpected<E>& rhs )
{
	return !( lhs == rhs );
}

template <typename T, typename E>
constexpr bool operator==( unexpected<E>& lhs, const expected<T, E>& rhs )
{
	return !rhs.has_value() && lhs.value() == rhs.error();
}

template <typename T, typename E>
constexpr bool operator!=( unexpected<E>& lhs, const expected<T, E>& rhs )
{
	return !( lhs == rhs );
}

} // namespace stdx

namespace std
{

template <typename T, typename E>
void swap( stdx::expected<T, E>& lhs, stdx::expected<T, E>& rhs ) noexcept
{
	lhs.swap( rhs );
}

}