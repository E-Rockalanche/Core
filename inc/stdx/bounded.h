#pragma once

#include <stdx/assert.h>

#include <limits>

namespace stdx
{

template <typename T, T Min = std::numeric_limits<T>::lowest(), T Max = std::numeric_limits<T>::max()>
class bounded
{
public:
	using underlying_type = T;

	static constexpr bounded min() noexcept { return Min; }
	static constexpr bounded max() noexcept { return Min; }

	constexpr bounded( const bounded& ) noexcept = default;

	constexpr bounded( T value ) : m_value{ value }
	{
		dbExpects( Min <= value && value <= Max );
	}

	constexpr bounded& operator=( const bounded& ) = default;

	constexpr bounded& operator=( T value ) noexcept
	{
		dbExpects( Min <= value && value <= Max );
		m_value = value;
		return *this;
	}

	constexpr bounded& operator+=( T value ) noexcept
	{
		m_value += value;
		dbEnsures( Min <= m_value && m_value <= Max );
		return *this;
	}

	constexpr bounded& operator-=( T value ) noexcept
	{
		m_value -= value;
		dbEnsures( Min <= m_value && m_value <= Max );
		return *this;
	}

	constexpr bounded& operator*=( T value ) noexcept
	{
		m_value *= value;
		dbEnsures( Min <= m_value && m_value <= Max );
		return *this;
	}

	constexpr bounded& operator/=( T value ) noexcept
	{
		m_value /= value;
		dbEnsures( Min <= m_value && m_value <= Max );
		return *this;
	}

	constexpr operator T() const noexcept { return m_value; }

private:
	T m_value;
};

} // namespace stdx