#pragma once

#include <stdx/assert.h>
#include <stdx/algorithm.h>

namespace stdx
{

template <typename Iterator, typename Compare>
class sorted_map_range
{
public:
	using iterator = Iterator;
	using reverse_iterator = std::reverse_iterator<iterator>;

	using value_type = typename std::iterator_traits<Iterator>::value_type;
	using reference = typename std::iterator_traits<Iterator>::reference;
	using pointer = typename std::iterator_traits<Iterator>::pointer;
	using difference_type = typename std::iterator_traits<Iterator>::difference_type;

	using key_type = std::remove_const_t<typename value_type::first_type>;
	using mapped_type = typename value_type::second_type;
	using size_type = std::make_unsigned_t<difference_type>;
	using key_compare = Compare;

	constexpr sorted_map_range( iterator first, iterator last ) : m_first{ first }, m_last{ last }
	{
		dbVerify( stdx::is_sorted( first, last, [ cmp = key_compare{} ]( auto& lhs, auto& rhs ) { return cmp( lhs.first, rhs.first ); } ) );
	}

	// element access

	constexpr mapped_type& at( const key_type& key )
	{
		auto it = find( key );
		if ( it == m_last )
			throw std::out_of_range( "stdx::sorted_map_range out of range" );

		return it->second;
	}

	constexpr const mapped_type& at( const key_type& key ) const
	{
		auto it = find( key );
		if ( it == m_last )
			throw std::out_of_range( "stdx::sorted_map_range out of range" );

		return it->second;
	}

	// iterators

	constexpr iterator begin() const noexcept { return m_first; }
	constexpr iterator end() const noexcept { return m_last; }

	constexpr reverse_iterator rbegin() const noexcept { return m_last; }
	constexpr reverse_iterator rend() const noexcept { return m_first; }

	// capacity

	constexpr bool empty() const noexcept { return size() == 0; }
	constexpr size_type size() const noexcept { return std::distance( m_first, m_last ); }

	// lookup

	constexpr size_type count( const key_type& key ) const
	{
		return static_cast<size_type>( std::count_if( m_first, m_last, [&key]( auto& entry ) { return entry.first == key; } ) );
	}

	template <typename K>
	constexpr size_type count( const K& key ) const
	{
		return static_cast<size_type>( std::count_if( m_first, m_last, [&key]( auto& entry ) { return entry.first == key; } ) );
	}

	constexpr iterator find( const key_type& key ) const
	{
		auto it = lower_bound( key );
		return ( it != m_last && it->first == key ) ? it : m_last;
	}

	template <typename K>
	constexpr iterator find( const K& key ) const
	{
		auto it = lower_bound( key );
		return ( it != m_last && it->first == key ) ? it : m_last;
	}

	constexpr bool contains( const key_type& key ) const
	{
		return find( key ) != m_last;
	}

	template <typename K>
	constexpr bool contains( const K& key ) const
	{
		return find( key ) != m_last;
	}

	constexpr std::pair<iterator, iterator> equal_range( const key_type& key ) const
	{
		return std::equal_range( m_first, m_last, key, [ cmp = key_compare{} ]( auto& entry, const key_type& key ) { return cmp( entry.first, key ); } );
	}

	template <typename K>
	constexpr std::pair<iterator, iterator> equal_range( const K& key ) const
	{
		return std::equal_range( m_first, m_last, key, [ cmp = key_compare{} ]( auto& entry, const K& key ) { return cmp( entry.first, key ); } );
	}

	constexpr iterator lower_bound( const key_type& key ) const
	{
		return std::lower_bound( m_first, m_last, key, [ cmp = key_compare{} ]( auto& entry, const key_type& key ) { return cmp( entry.first, key ); } );
	}

	template <typename K>
	constexpr iterator lower_bound( const K& key ) const
	{
		return std::lower_bound( m_first, m_last, key, [ cmp = key_compare{} ]( auto& entry, const K& key ) { return cmp( entry.first, key ); } );
	}

	constexpr iterator upper_bound( const key_type& key ) const
	{
		return std::lower_bound( m_first, m_last, key, [ cmp = key_compare{} ]( auto& entry, const key_type& key ) { return cmp( entry.first, key ); } );
	}

	template <typename K>
	constexpr iterator upper_bound( const K& key ) const
	{
		return std::lower_bound( m_first, m_last, key, [ cmp = key_compare{} ]( auto& entry, const K& key ) { return cmp( entry.first, key ); } );
	}

private:
	iterator m_first;
	iterator m_last;
};

} // namespace stdx