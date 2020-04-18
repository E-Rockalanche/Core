#ifndef STDX_FLAT_SET_HPP
#define STDX_FLAT_SET_HPP

#include <algorithm>
#include <functional>
#include <vector>

namespace stdx
{

template <class Key>
class flat_set
{
public:
	using key_type 					= Key;
	using value_type 				= Key;
	using size_type 				= typename std::vector<Key>::size_type;
	using difference_type 			= typename std::vector<Key>::difference_type;
	using key_compare 				= std::less<Key>;
	using value_compare 			= std::less<Key>;
	using reference 				= value_type&;
	using const_reference 			= const value_type&;
	using pointer 					= value_type*;
	using const_pointer 			= const value_type*;
	using iterator 					= typename std::vector<Key>::iterator;
	using reverse_iterator 			= typename std::vector<Key>::reverse_iterator;
	using const_iterator 			= typename std::vector<Key>::const_iterator;
	using const_reverse_iterator	= typename std::vector<Key>::const_reverse_iterator;

	flat_set() noexcept = default;

	template <class InputIt>
	flat_set( InputIt first, InputIt last )
	{
		m_values.reserve( static_cast<size_type>( last - first ) );
		for( auto it = first; it != last; ++it )
			insert( *it );
	}

	flat_set( const flat_set& other ) : m_values( other.m_values ) {}
	flat_set( flat_set&& other ) noexcept : m_values( std::move( other.m_values ) ) {}
	flat_set( std::initializer_list<value_type> init ) : flat_set( init.begin(), init.end() ) {}
	
	~flat_set() = default;

	void reserve( size_type s ) { m_values.reserve( s ); }

	flat_set& operator=( const flat_set& ) = default;
	flat_set& operator=( flat_set&& ) noexcept = default;
	flat_set& operator=( std::initializer_list<value_type> iList )
	{
		clear();
		reserve( iList.size() );
		for( auto& value : iList )
			insert( value );
	}

	iterator begin() noexcept { return m_values.begin(); }
	iterator end() noexcept { return m_values.end(); }

	const_iterator cbegin() const noexcept { return m_values.cbegin(); }
	const_iterator cend() const noexcept { return m_values.cend(); }

	reverse_iterator rbegin() noexcept { return m_values.rbegin(); }
	reverse_iterator rend() noexcept { return m_values.rend(); }

	const_reverse_iterator crbegin() const noexcept { return m_values.crbegin(); }
	const_reverse_iterator crend() const noexcept { return m_values.crend(); }

	bool empty() const noexcept { return m_values.empty(); }
	size_type size() const noexcept { return m_values.size(); }
	size_type max_size() const noexcept { return m_values.max_size(); }
	size_type capacity() const noexcept { return m_values.capacity(); }

	void shrink_to_fit() { return m_values.shrink_to_fit(); }

	void clear() noexcept { m_values.clear(); }

	std::pair<iterator, bool> insert( const value_type& value )
	{
		return insert( begin(), end(), value );
	}

	std::pair<iterator, bool> insert( value_type&& value )
	{
		return insert( begin(), end(), std::move( value ) );
	}

	iterator insert( const_iterator hint, const value_type& value )
	{
		if ( hint == end() || value < *hint )
		{
			if ( hint > begin() && *(hint-1) < value )
				return m_values.insert( hint, value );

			return insert( begin(), hint, value ).first;
		}

		return insert( hint+1, end(), value ).first;
	}

	iterator insert( const_iterator hint, value_type&& value )
	{
		if ( hint == end() || value < *hint )
		{
			if ( hint > begin() && *(hint-1) < value )
				return m_values.insert( hint, std::move( value ) );

			return insert( begin(), hint, std::move( value ) ).first;
		}

		return insert( hint+1, end(), std::move( value ) ).first;
	}

	template< class InputIt >
	void insert( InputIt first, InputIt last )
	{
		for( auto it = first; it != last; ++it )
			insert( *it );
	}

	void insert( std::initializer_list<value_type> iList )
	{
		insert( iList.begin(), iList.end() );
	}

	template < class... Args >
	std::pair<iterator, bool> emplace( Args&&... args )
	{
		return insert( value_type( std::forward<Args>( args )... ) );
	}

	template < class... Args >
	iterator emplace_hint( const_iterator hint, Args&&... args )
	{
		return insert( hint, value_type( std::forward<Args>( args )... ) );
	}

	iterator erase( const_iterator pos ) { return m_values.erase( pos ); }
	iterator erase( const_iterator first, const_iterator last ) { return m_values.erase( first, last ); }

	size_type erase( const key_type& key )
	{
		auto it = find( key );
		if ( it == end() )
			return 0;

		m_values.erase( it );
		return 1;
	}

	void swap( flat_set& other ) noexcept
	{
		std::vector<value_type> temp = std::move( m_values );
		m_values = std::move( other.m_values );
		other.m_values = std::move( temp );
	}

	size_type count( const Key& key ) const
	{
		return ( find( key ) == cend() ) ? 0 : 1;
	}

	template <class K>
	size_type count( const K& x ) const
	{
		return ( find( x ) == cend() ) ? 0 : 1;
	}

	iterator find( const Key& key )
	{
		auto it = lower_bound( key );
		return isEntry( it, key ) ? it : end();
	}

	const_iterator find( const Key& key ) const
	{
		auto it = lower_bound( key );
		return isEntry( it, key ) ? it : cend();
	}

	template <class K>
	iterator find( const K& x )
	{
		auto it = lower_bound( x );
		return isEntry( it, key ) ? it : end();
	}

	template <class K>
	const_iterator find( const K& x ) const
	{
		auto it = lower_bound( x );
		return isEntry( it, key ) ? it : cend();
	}

	bool contains( const Key& key ) const
	{
		return find( key ) != cend();
	}

	template <class K>
	bool contains( const K& x ) const
	{
		return find( x ) != cend();
	}

	iterator lower_bound( const Key& key )
	{
		return std::lower_bound( begin(), end(), key );
	}

	const_iterator lower_bound( const Key& key ) const
	{
		return std::lower_bound( cbegin(), cend(), key );
	}

	template <class K>
	iterator lower_bound( const K& x )
	{
		return std::lower_bound( begin(), end(), x );
	}

	template <class K>
	const_iterator lower_bound( const K& x ) const
	{
		return std::lower_bound( cbegin(), cend(), x );
	}

	iterator upper_bound( const Key& key )
	{
		return std::upper_bound( begin(), end(), key );
	}

	const_iterator upper_bound( const Key& key ) const
	{
		return std::upper_bound( cbegin(), cend(), key );
	}

	template <class K>
	iterator upper_bound( const K& x )
	{
		return std::upper_bound( begin(), end(), x );
	}

	template <class K>
	const_iterator upper_bound( const K& x ) const
	{
		return std::upper_bound( cbegin(), cend(), x );
	}

	key_compare key_comp() const { return key_compare(); }
	value_compare value_comp() const { return value_compare(); }

	friend bool operator==( const flat_set& lhs, const flat_set& rhs )
	{
		return lhs.m_values == rhs.m_values;
	}

	friend bool operator!=( const flat_set& lhs, const flat_set& rhs )
	{
		return lhs.m_values != rhs.m_values;
	}

	friend bool operator<( const flat_set& lhs, const flat_set& rhs )
	{
		return lhs.m_values < rhs.m_values;
	}

	friend bool operator>( const flat_set& lhs, const flat_set& rhs )
	{
		return lhs.m_values > rhs.m_values;
	}

	friend bool operator<=( const flat_set& lhs, const flat_set& rhs )
	{
		return lhs.m_values <= rhs.m_values;
	}

	friend bool operator>=( const flat_set& lhs, const flat_set& rhs )
	{
		return lhs.m_values >= rhs.m_values;
	}

private:
	std::pair<iterator, bool> insert( const_iterator first, const_iterator last, const value_type& value )
	{
		auto it = std::lower_bound( first, last, value );
		if ( it != end() && *it == value )
			return { it, false };

		m_values.insert( it, value );
		return { it, true };
	}

	std::pair<iterator, bool> insert( const_iterator first, const_iterator last, value_type&& value )
	{
		auto it = std::lower_bound( first, last, value );
		if ( it != end() && *it == value )
			return { it, false };

		m_values.insert( it, std::move( value ) );
		return { it, true };
	}

	bool isEntry( const_iterator it, const value_type& value ) const
	{
		return ( it != cend() ) && ( *it == value );
	}

private:
	std::vector<value_type> m_values;
};

}

namespace std
{
	template <class T>
	void swap( stdx::flat_set<T>& lhs, stdx::flat_set<T>& rhs ) noexcept
	{
		lhs.swap( rhs );
	}

	template <class T, class Pred>
	void erase_if( stdx::flat_set<T>& set, Pred pred )
	{
		for( auto it = set.begin(), last = set.end(); it != last; )
		{
			if ( pred( *it ) )
				it = set.erase( it );
			else
				++it;
		}
	}
}

#endif