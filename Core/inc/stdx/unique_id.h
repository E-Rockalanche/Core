#pragma once

#include <stdx/assert.h>

#include <cstdint>
#include <limits>
#include <type_traits>

namespace stdx
{

namespace detail
{

template <typename T>
struct unique_id_default_index_bits {};

template <>
struct unique_id_default_index_bits<uint32_t>
{
	static constexpr size_t value = 20;
};

template <>
struct unique_id_default_index_bits<uint64_t>
{
	static constexpr size_t value = 32;
};

}

template <typename Tag = void, typename Base = uint32_t, size_t IndexBits = detail::unique_id_default_index_bits<Base>::value>
class unique_id
{
	static_assert( std::is_integral_v<Base> && std::is_unsigned_v<Base> ); // base type must be an unsigned integer
	static_assert( IndexBits > 0 ); // must have at least one index bit
	static_assert( IndexBits < sizeof( Base ) * 8 ); // must have at least 1 generation bit

public:
	using tag_type = Tag;
	using base_type = Base;

	static constexpr size_t index_bits = IndexBits;
	static constexpr size_t generation_bits = sizeof( Base ) * 8 - IndexBits;

	static constexpr base_type index_mask = ( static_cast<base_type>( 1 ) << index_bits ) - 1;
	static constexpr base_type generation_mask = ~index_mask;

	static constexpr base_type invalid_index = index_mask;

	static constexpr base_type index_max = invalid_index - 1;
	static constexpr base_type generation_max = generation_mask >> index_bits;

	// creates an invalid id
	constexpr unique_id() noexcept = default;

	// creates an invalid id
	constexpr unique_id( std::nullptr_t ) noexcept {}

	explicit constexpr unique_id( base_type value ) : m_value{ value } {}

	// index and generation must both be valid
	constexpr unique_id( base_type i, base_type gen ) : unique_id{ i | ( gen << index_bits ) }
	{
		dbExpects( i <= index_max );
		dbExpects( gen <= generation_max );
	}

	constexpr base_type value() const noexcept
	{
		return m_value;
	}

	constexpr base_type index() const noexcept
	{
		return m_value & index_mask;
	}

	constexpr base_type generation() const noexcept
	{
		dbExpects( valid() );
		return ( m_value & generation_mask ) >> index_bits;
	}

	constexpr bool valid() const noexcept
	{
		return m_value != std::numeric_limits<base_type>::max();
	}

	explicit constexpr operator bool() const noexcept
	{
		return valid();
	}

	// return id with same index and next generation (generation can wrap to 0)
	constexpr unique_id next() const noexcept
	{
		dbExpects( valid() );
		return unique_id{ index(), ( generation() + 1 ) & generation_mask };
	}

private:
	base_type m_value = std::numeric_limits<base_type>::max();
};

template <typename T, typename B, size_t I>
constexpr bool operator==( unique_id<T, B, I> lhs, unique_id<T, B, I> rhs ) noexcept
{
	return lhs.value() == rhs.value();
}

template <typename T, typename B, size_t I>
constexpr bool operator!=( unique_id<T, B, I> lhs, unique_id<T, B, I> rhs ) noexcept
{
	return lhs.value() != rhs.value();
}

template <typename T, typename B, size_t I>
constexpr bool operator<( unique_id<T, B, I> lhs, unique_id<T, B, I> rhs ) noexcept
{
	return lhs.value() < rhs.value();
}

template <typename T, typename B, size_t I>
constexpr bool operator>( unique_id<T, B, I> lhs, unique_id<T, B, I> rhs ) noexcept
{
	return lhs.value() > rhs.value();
}

template <typename T, typename B, size_t I>
constexpr bool operator<=( unique_id<T, B, I> lhs, unique_id<T, B, I> rhs ) noexcept
{
	return lhs.value() <= rhs.value();
}

template <typename T, typename B, size_t I>
constexpr bool operator>=( unique_id<T, B, I> lhs, unique_id<T, B, I> rhs ) noexcept
{
	return lhs.value() >= rhs.value();
}

} // namespace stdx