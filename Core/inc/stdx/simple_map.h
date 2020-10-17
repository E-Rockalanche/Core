#ifndef STDX_SIMPLE_MAP_HPP
#define STDX_SIMPLE_MAP_HPP

#include <stdx/container.h>

#include <algorithm>
#include <utility>
#include <vector>

namespace stdx {

// unordered contiguous set of of key-value pairs with O(N) lookup time
// faster lookup than an ordered map with a small data set ( less than 64 )
template <typename Key, typename T, typename KeyEqual = std::equal_to<Key>>
class simple_map
{
public:
	using key_type = Key;
	using mapped_type = T;
	using value_type = std::pair<Key, T>;
	using size_type = typename std::vector<value_type>::size_type;
	using difference_type = typename std::vector<value_type>::difference_type;
	using key_equal = KeyEqual;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using iterator = typename std::vector<value_type>::iterator;
	using const_iterator = typename std::vector<value_type>::const_iterator;
	using reverse_iterator = typename std::vector<value_type>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<value_type>::const_reverse_iterator;

	// construction

	simple_map() = default;
	simple_map( const simple_map& ) = default;
	simple_map( simple_map&& ) = default;

	simple_map( std::initializer_list<value_type> init )
	{
		insert( init );
	}

	// assignment

	simple_map& operator=( const simple_map& other ) = default;
	simple_map& operator=( simple_map&& other ) = default;

	simple_map& operator=( std::initializer_list<value_type> init )
	{
		clear();
		insert( init );
		return *this;
	}

	// access

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

	// iterators

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

	// query

	[[nodiscard]] bool empty() const noexcept { return m_data.empty(); }
	size_type size() const noexcept { return m_data.size(); }
	difference_type ssize() const noexcept { return static_cast<difference_type>( size() ); }
	size_type max_size() const noexcept { return m_data.max_size(); }

	// capacity

	void clear() noexcept { m_data.clear(); }
	void reserve( size_type capacity ) { m_data.reserve( capacity ); }
	size_type capacity() const noexcept { return m_data.capacity(); }
	void shrink_to_fit() { m_data.shrink_to_fit(); }

	// modifiers

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
		m_data.erase( pos );
	}

	iterator erase( const_iterator first, const_iterator last ) noexcept
	{
		m_data.erase( first, last );
	}

	size_type erase( const key_type& key ) noexcept
	{
		size_type prevSize = size();
		stdx::erase_remove_if( m_data, [ &key, eq = key_equal{} ]( auto& entry ) { return eq( entry.first, key ); } );
		return prevSize - size();
	}

	void swap( simple_map& other ) noexcept
	{
		m_data.swap( other.m_data );
	}

	template <typename KeyCompare>
	void sort( KeyCompare comp ) noexcept
	{
		std::sort( m_data.begin(), m_data.end(), [comp]( auto& lhs, auto& rhs ) { return comp( lhs.first, rhs.first ); } );
	}

	// lookup

	size_type count( const key_type& key ) const noexcept
	{
		return static_cast<size_type>( std::count_if( m_data.begin(), m_data.end(), [ &key, eq = key_equal{} ]( auto& entry ) { return eq( entry.first, key ); } ) );
	}

	template <typename K>
	size_type count( const K& x ) const noexcept
	{
		return static_cast<size_type>( std::count_if( m_data.begin(), m_data.end(), [ &x, eq = key_equal{} ]( auto& entry ) { return eq( entry.first, x ); } ) );
	}

	iterator find( const key_type& key ) noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [ &key, eq = key_equal{} ]( auto& entry ){ return eq( entry.first, key ); } );
	}

	const_iterator find( const key_type& key ) const noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [ &key, eq = key_equal{} ]( auto& entry ){ return eq( entry.first, key ); } );
	}

	template <typename K>
	iterator find( const K& x ) noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [ &x, eq = key_equal{} ]( auto& entry ){ return eq( entry.first, x ); } );
	}

	template <typename K>
	const_iterator find( const K& x ) const noexcept
	{
		return std::find_if( m_data.begin(), m_data.end(), [ &x, eq = key_equal{} ]( auto& entry ){ return eq( entry.first, x ); } );
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

	key_equal key_eq() const { return key_equal{}; }

	friend bool operator==( const simple_map& lhs, const simple_map& rhs ) noexcept { return lhs.m_data == rhs.m_data; }
	friend bool operator!=( const simple_map& lhs, const simple_map& rhs ) noexcept { return lhs.m_data != rhs.m_data; }
	friend bool operator<( const simple_map& lhs, const simple_map& rhs ) noexcept { return lhs.m_data < rhs.m_data; }
	friend bool operator>( const simple_map& lhs, const simple_map& rhs ) noexcept { return lhs.m_data > rhs.m_data; }
	friend bool operator<=( const simple_map& lhs, const simple_map& rhs ) noexcept { return lhs.m_data <= rhs.m_data; }
	friend bool operator>=( const simple_map& lhs, const simple_map& rhs ) noexcept { return lhs.m_data >= rhs.m_data; }

private:
	std::vector<value_type> m_data;
};

} // namespace stdx

#endif // STDX_SIMPLE_MAP