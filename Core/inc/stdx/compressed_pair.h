#pragma once
/*
#include <type_traits>

namespace stdx
{

template <typename T1, typename T2, std::enable_if_t<!std::is_empty_v<T1> && !std::is_empty_v<T2>, int> = 0>
class compressed_pair
{
public:
	using first_type = T1;
	using second_type = T2;

	constexpr compressed_pair() = default;

	constexpr compressed_pair( const T1& x, const T2& y ) : m_first( x ), m_second( y ) {}

	template <typename U1, typename U2>
	constexpr compressed_pair( U1&& x, U2&& y ) : m_first( std::forward<U1>( x ) ), m_second( std::forward<U2>( y ) ) {}

	T1& first() noexcept { return m_first; }
	const T1& first() const noexcept { return m_first; }

	T1& second() noexcept { return m_second; }
	const T1& second() const noexcept { return m_second; }

	void swap( compressed_pair& other )
	{
		std::swap( first(), other.first() );
		std::swap( second(), other.second() );
	}

private:
	T1 m_first;
	T2 m_second;
};

template <typename T1, typename T2, std::enable_if_t<!std::is_empty_v<T1> && std::is_empty_v<T2>, int> = 0>
class compressed_pair : private T2
{
public:
	using first_type = T1;
	using second_type = T2;

	constexpr compressed_pair() = default;

	constexpr compressed_pair( const T1& x, const T2& y ) : T2( y ), m_first( x ) {}

	template <typename U1, typename U2>
	constexpr compressed_pair( U1&& x, U2&& y ) : T2( std::forward<U2>( y ) ), m_first( std::forward<U1>( x ) ) {}

	T1& first() noexcept { return m_first; }
	const T1& first() const noexcept { return m_first; }

	T1& second() noexcept { return *this; }
	const T1& second() const noexcept { return *this; }

	void swap( compressed_pair& other )
	{
		std::swap( first(), other.first() );
		std::swap( second(), other.second() );
	}

private:
	T1 m_first;
};

template <typename T1, typename T2, std::enable_if_t<std::is_empty_v<T1>, int> = 0>
class compressed_pair : private T1
{
public:
	using first_type = T1;
	using second_type = T2;

	constexpr compressed_pair() = default;

	constexpr compressed_pair( const T1& x, const T2& y ) : T1( x ), m_second( y ) {}

	template <typename U1, typename U2>
	constexpr compressed_pair( U1&& x, U2&& y ) : T1( std::forward<U1>( x ) ), m_second( std::forward<U2>( y ) ) {}

	T1& first() noexcept { return *this; }
	const T1& first() const noexcept { return *this; }

	T1& second() noexcept { return m_second; }
	const T1& second() const noexcept { return m_second; }

	void swap( compressed_pair& other )
	{
		std::swap( first(), other.first() );
		std::swap( second(), other.second() );
	}

private:
	T2 m_second;
};

} // namespace stdx
*/