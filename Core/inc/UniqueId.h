#pragma once

#include <stdx/assert.h>
#include <stdx/bit.h>
#include <stdx/int.h>

template <typename Tag, size_t IndexBits = 20, size_t GenerationBits = 12>
class UniqueId
{
public:
	using base_type = stdx::uint_exact_t<IndexBits + GenerationBits>;
	using tag_type = Tag;

	static constexpr size_t index_bits = IndexBits;
	static constexpr size_t generation_bits = GenerationBits;

	constexpr UniqueId() : m_ordinal{ static_cast<base_type>( -1 ) } {}

	constexpr UniqueId( const UniqueId& ) = default;

	constexpr UniqueId( base_type index, base_type generation ) : m_index{ index }, m_generation{ generation }
	{
		dbExpects( index <= stdx::unsigned_bits_max( IndexBits ) );
		dbExpects( generation <= stdx::unsigned_bits_max( GenerationBits ) );
	}

	constexpr base_type GetIndex() const noexcept { return m_index; }

	constexpr base_type GetGeneration() const noexcept { return m_generation; }

	constexpr base_type GetOrdinal() const noexcept { return m_ordinal; }

	friend constexpr bool operator==( UniqueId lhs, UniqueId rhs ) noexcept
	{
		return lhs.m_ordinal == rhs.m_ordinal;
	}

	friend constexpr bool operator!=( UniqueId lhs, UniqueId rhs ) noexcept
	{
		return lhs.m_ordinal != rhs.m_ordinal;
	}

	friend constexpr bool operator<( UniqueId lhs, UniqueId rhs ) noexcept
	{
		return lhs.m_ordinal < rhs.m_ordinal;
	}

	friend constexpr bool operator>( UniqueId lhs, UniqueId rhs ) noexcept
	{
		return lhs.m_ordinal > rhs.m_ordinal;
	}

	friend constexpr bool operator<=( UniqueId lhs, UniqueId rhs ) noexcept
	{
		return lhs.m_ordinal <= rhs.m_ordinal;
	}

	friend constexpr bool operator>=( UniqueId lhs, UniqueId rhs ) noexcept
	{
		return lhs.m_ordinal >= rhs.m_ordinal;
	}

private:
	union
	{
		struct
		{
			base_type m_index : IndexBits;
			base_type m_generation : GenerationBits;
		};

		base_type m_ordinal;
	};

	// static_assert( sizeof( UniqueId<Tag, IndexBits, GenerationBits> ) == sizeof( base_type ) );
};

template <typename To, typename Tag, size_t I, size_t G, typename Base>
constexpr To UniqueIdCast( UniqueId<Tag, I, G> id ) noexcept
{
	return To
	{
		stdx::narrow_cast<typename To::base_type>( id.GetIndex() ),
		stdx::narrow_cast<typename To::base_type>( id.GetGeneration() )
	};
}