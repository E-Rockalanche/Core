#pragma once

#include <stdx/assert.h>
#include <stdx/algorithm.h>

#include <array>
#include <memory>

namespace stdx
{

template <typename K, typename T, std::size_t N>
class static_map
{
	using value_type_imp = std::pair<K, T>;

public:
	using key_type = K;
	using mapped_type = T;
	using value_type = std::pair<const K, T>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	constexpr static_map( const value_type (&init)[ N ] ) noexcept
	{
		dbExpects( init.size() == m_data.size() );

		stdx::copy( std::begin( init ), std::end( init ), m_data.begin() );
		stdx::sort( m_data.begin(), m_data.end(), []( auto& lhs, auto& rhs ) { return lhs.first < rhs.first; } );

#ifdef DEBUG
		for ( std::size_t i = 1; i < m_data.size(); ++i )
		{
			dbEnsures( m_data[ i - 1 ].first < m_data[ 0 ].first );
		}
#endif
	}

	constexpr iterator begin() noexcept { return reinterpret_cast<iterator>( stdx::to_address( m_data.begin() ) ); }
	constexpr iterator end() noexcept { return reinterpret_cast<iterator>( stdx::to_address( m_data.end() ) ); }

	constexpr const_iterator begin() const noexcept { return reinterpret_cast<const_iterator>( stdx::to_address( m_data.begin() ) ); }
	constexpr const_iterator end() const noexcept { return reinterpret_cast<const_iterator>( stdx::to_address( m_data.end() ) ); }

	constexpr bool empty() const noexcept { return N == 0; }
	constexpr size_type size() const noexcept { return N; }
	constexpr size_type max_size() const noexcept { return N; }

	constexpr iterator = find( const K& key ) noexcept
	{
		return stdx::find_if( begin(), end(), [&key]( auto& kv ) { return kv.first == key; } );
	}

	constexpr const_iterator = find( const K& key ) const noexcept
	{
		return stdx::find_if( begin(), end(), [&key]( auto& kv ) { return kv.first == key; } );
	}

private:
	std::array<value_type_imp, N> m_data;
};

}