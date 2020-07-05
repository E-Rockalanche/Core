#ifndef STDX_SIMPLE_MAP_HPP
#define STDX_SIMPLE_MAP_HPP

#include <stdx/container.h>

#include <algorithm>
#include <utility>
#include <vector>

namespace stdx {

// unordered contiguous set of of key-value pairs with O(N) lookup time
// faster lookup than an ordered map with a small data set ( less than 64 )
template <typename Key, typename T>
class simple_map
{
public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<Key, T>;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using size_type = typename std::vector<value_type>::size_type;
	using difference_type = typename std::vector<value_type>::difference_type;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using reverse_iterator = typename std::vector<value_type>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<value_type>::const_reverse_iterator;

	simple_map() = default;
	simple_map( const simple_map& ) = default;
	simple_map( simple_map&& ) = default;

	simple_map( std::initializer_list<value_type> init )
	{
		insert( init );
	}

	simple_map& operator=( const simple_map& other ) = default;
	simple_map& operator=( simple_map&& other ) = default;

	simple_map& operator=( std::initializer_list<value_type> init )
	{
		clear();
		insert( init );
		return *this;
	}

	T& operator[]( const key_type& key )
	{
		auto it = find( key );
		if ( it == end() )
		{
			m_data.emplace_back( key, mapped_type{} );
			return m_data.back();
		}
		return *it;
	}

	T& operator[]( key_type&& key )
	{
		auto it = find( key );
		if ( it == end() )
		{
			m_data.emplace_back( std::move( key ), mapped_type{} );
			return m_data.back().second;
		}
		return it->second;
	}

	iterator begin() noexcept { return m_data.begin(); }
	iterator end() noexcept { return m_data.end(); }

	const_iterator begin() const noexcept { return m_data.begin(); }
	const_iterator end() const noexcept { return m_data.end(); }

	const_iterator cbegin() const noexcept { return m_data.begin(); }
	const_iterator cend() const noexcept { return m_data.end(); }

	reverse_iterator rbegin() noexcept { return m_data.rbegin(); }
	reverse_iterator rend() noexcept { return m_data.rend(); }

	const_reverse_iterator rbegin() const noexcept { return m_data.rbegin(); }
	const_reverse_iterator rend() const noexcept { return m_data.rend(); }

	const_reverse_iterator crbegin() const noexcept { return m_data.crbegin(); }
	const_reverse_iterator crend() const noexcept { return m_data.crend(); }

	[[nodiscard]] bool empty() const noexcept { return m_data.empty(); }
	size_type size() const noexcept { return m_data.size(); }
	difference_type ssize() const noexcept { return static_cast<difference_type>( size() ); }
	size_type max_size() const noexcept { return m_data.max_size(); }

	void clear() noexcept { m_data.clear(); }

	std::pair<iterator, bool> insert( const value_type& value )
	{
		auto it = find( value.first );
		if ( it != end() )
			return { it, false };

		m_data.push_back( value );
		return { end() - 1, true };
	}

	std::pair<iterator, bool> insert( value_type&& value )
	{
		auto it = find( value.first );
		if ( it != end() )
			return { it, false };

		m_data.push_back( std::move( value ) );
		return { end() - 1, true };
	}

	template <typename InputIt>
	void insert( InputIt first, InputIt last )
	{
		m_data.reserve( size() + static_cast<size_type>( last - first ) );
		for( ; first != last; ++first )
		{
			insert( *first );
		}
	}

	void insert( std::initializer_list<value_type> init )
	{
		m_data.reserve( size() + init.size() );
		insert( init.begin(), init.end() );
	}

	template <typename M>
	std::pair<iterator, bool> insert_or_assign( const key_type& k, M&& obj )
	{
		auto it = find( k );
		if ( it == end() )
		{
			m_data.push_back( { k, std::forward<M>( obj ) } );
			return { end() - 1, true };
		}
		else
		{
			it->second = std::forward<M>( obj );
			return { it, false };
		}
	}

	template <typename M>
	std::pair<iterator, bool> insert_or_assign( key_type&& k, M&& obj )
	{
		auto it = find( k );
		if ( it == end() )
		{
			m_data.push_back( { std::move( k ), std::forward<M>( obj ) } );
			return { end() - 1, true };
		}
		else
		{
			it->second = std::forward<M>( obj );
			return { it, false };
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace( Args&&... args )
	{
		return insert( value_type( std::forward<Args>( args )... ) );
	}

	template <typename... Args>
	std::pair<iterator, bool> try_emplace( const key_type& key, Args&&... args )
	{
		if ( !contains( key ) )
		{
			m_data.emplace_back( key, mapped_type( std::forward<Args>( args )... ) );
		}
	}

	template <typename... Args>
	std::pair<iterator, bool> try_emplace( key_type&& key, Args&&... args )
	{
		if ( !contains( key ) )
		{
			m_data.emplace_back( std::move( key ), mapped_type( std::forward<Args>( args )... ) );
		}
	}

	iterator erase( const_iterator pos ) noexcept
	{
		return stdx::backswap_erase( m_data, pos );
	}

	iterator erase( const_iterator first, const_iterator last ) noexcept
	{
		return stdx::backswap_erase( m_data, first, last );
	}

	size_type erase( const key_type& key ) noexcept
	{
		for( auto it = begin(), last = end(); it != last; ++it )
		{
			if ( it->first == key )
			{
				std::swap( *it, m_data.back() );
				m_data.pop_back();
				return 1;
			}
		}
		return 0;
	}

	void swap( simple_map& other ) noexcept
	{
		m_data.swap( other.m_data );
	}

	// lookup

	size_type count( const key_type& key ) const noexcept
	{
		return ( find( key ) == end() ) ? 0 : 1;
	}

	template <typename K>
	size_type count( const K& x ) const noexcept
	{
		return ( find( x ) == end() ) ? 0 : 1;
	}

	iterator find( const key_type& key ) noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [&]( auto& entry ){ return entry.first == key; } );
	}

	const_iterator find( const key_type& key ) const noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [&]( auto& entry ){ return entry.first == key; } );
	}

	template <typename K>
	iterator find( const K& x ) noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [&]( auto& entry ){ return entry.first == x; } );
	}

	template <typename K>
	const_iterator find( const K& x ) const noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [&]( auto& entry ){ return entry.first == x; } );
	}

	bool contains( const key_type& key ) const noexcept
	{
		return find( key ) != end();
	}

	template <typename K>
	bool contains( const K& x ) const noexcept
	{
		return find( x ) != end();
	}

	// sort underlying data structure to optimize linear lookups
	template <typename KeyComparator>
	void sort( KeyComparator c ) noexcept
	{
		std::sort( m_data.begin(), m_data.end(), [ c ]( auto& lhs, auto& rhs ) { return c( lhs.first, rhs.first ); } );
	}

private:
	std::vector<value_type> m_data;
};

} // namespace stdx

#endif // STDX_SIMPLE_MAP