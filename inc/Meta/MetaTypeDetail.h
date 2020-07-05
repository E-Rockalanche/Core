#pragma once

#include "MetaType.h"
#include "MetaIO.h"

#include <Math/Colour.h>

#include <stdx/enum.h>
#include <stdx/flat_map.h>
#include <stdx/format.h>
#include <stdx/simple_map.h>
#include <stdx/type_traits.h>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace detail
{
	template <typename Container>
	using emplace_back_default_t = decltype( std::declval<Container>{}.emplace_back() );

	template <typename Map>
	using insert_value_t = decltype( std::declval<Map>{}.insert( std::declval<typename Map::value_type>{} ) );
}

template <typename T>
class MetaPrimitive : public MetaType
{
public:
	// new meta primitives must be declared in MetaType.cpp

	MetaPrimitive( std::string name ) : MetaType( std::move( name ) ) {}
	MetaPrimitive() : MetaType( stdx::reflection::type_name_v<T>.c_str() ) {}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;
};

template <typename T>
void MetaPrimitive<T>::write( MetaWriter& writer, const void* data ) const
{
	const T& value = *static_cast<const T*>( data );

	static_assert( !std::is_pointer_v<T>, "Cannot meta write pointer type" );

	if constexpr ( std::is_enum_v<T> )
	{
		writer.writeString( stdx::enum_name( value ) );
	}
	else if constexpr ( std::is_integral_v<T> )
	{
		writer.writeInt( narrow_cast<int64_t>( value ) );
	}
	else if constexpr ( std::is_floating_point_v<T> )
	{
		writer.writeFloat( static_cast<double>( value ) );
	}
	else if constexpr ( std::is_convertible_v<T, std::string_view> )
	{
		// assume convertible to string_view
		writer.writeString( value );
	}
	else
	{
		static_assert( "Could not determine meta primitive type to write" );
	}
}

template <typename T>
void MetaPrimitive<T>::read( MetaReader& reader, void* data ) const
{
	T& value = *static_cast<T*>( data );

	static_assert( !std::is_pointer_v<T>, "Cannot meta read pointer type" );

	if constexpr ( std::is_enum_v<T> )
	{
		auto name = reader.readString();
		auto result = stdx::enum_cast( name );
		if ( !result )
			throw MetaIOException( stdx::format( "Value \"{}\" does not exist in enum \"{}\"", name, getName() ) );

		value = *result;
	}
	else if constexpr ( std::is_integral_v<T> )
	{
		const int64_t result = reader.readInt();

		constexpr uint64_t MaxT = std::min<uint64_t>( std::numeric_limits<T>::max(), std::numeric_limits<int64_t>::max() );
		constexpr int64_t MinT = std::max<int64_t>( std::numeric_limits<T>::min(), std::numeric_limits<int64_t>::min() );
		if ( result > narrow_cast<int64_t>( MaxT ) || result < narrow_cast<int64_t>( MinT ) )
			throw MetaIOException( stdx::format( "Integer {} cannot fit in type {}", result, getName() ) );

		value = static_cast<T>( result );
	}
	else if constexpr ( std::is_floating_point_v<T> )
	{
		double result = reader.readFloat();
		if ( result > static_cast<double>( std::numeric_limits<T>::max() ) || result < static_cast<double>( std::numeric_limits<T>::lowest() ) )
			throw MetaIOException( stdx::format( "Float {} cannot fit in type {}", result, getName() ) );

		value = static_cast<T>( result );
	}
	else if constexpr ( std::is_convertible_v<std::string_view, T> )
	{
		// assume assignable from string_view
		value = reader.readString();
	}
	else
	{
		static_assert( "Could not determine meta primitive type to write" );
		(void)value;
	}
}

template <>
inline void MetaPrimitive<bool>::write( MetaWriter& writer, const void* data ) const
{
	writer.writeBool( *static_cast<const bool*>( data ) );
}

template <>
inline void MetaPrimitive<bool>::read( MetaReader& reader, void* data ) const
{
	*static_cast<bool*>( data ) = reader.readBool();
}

template <>
inline void MetaPrimitive<char>::write( MetaWriter& writer, const void* data ) const
{
	writer.writeString( std::string_view( static_cast<const char*>( data ), 1 ) );
}

template <>
inline void MetaPrimitive<char>::read( MetaReader& reader, void* data ) const
{
	auto str = reader.readString();
	if ( str.size() != 1 )
		throw MetaIOException( stdx::format( "Invalid char: {}", str ) );

	*static_cast<char*>( data ) = str.front();
}

#define DECLARE_META_PRIMITIVE( type ) \
template<> inline const MetaType* MetaTypeResolver<type>::get() { \
	static const MetaPrimitive<type> s_metaType; \
	return &s_metaType; \
}

DECLARE_META_PRIMITIVE( int8_t )
DECLARE_META_PRIMITIVE( uint8_t )
DECLARE_META_PRIMITIVE( int16_t )
DECLARE_META_PRIMITIVE( uint16_t )
DECLARE_META_PRIMITIVE( int32_t )
DECLARE_META_PRIMITIVE( uint32_t )
DECLARE_META_PRIMITIVE( int64_t )
DECLARE_META_PRIMITIVE( uint64_t )
DECLARE_META_PRIMITIVE( float )
DECLARE_META_PRIMITIVE( double )
DECLARE_META_PRIMITIVE( bool )
DECLARE_META_PRIMITIVE( std::string )

template <typename List>
class MetaList : public MetaType
{
public:
	MetaList( const MetaType* elementType )
		: MetaType( stdx::reflection::type_name_v<List> )
		, m_elementType{ elementType }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const MetaType* getElementType() const { return m_elementType; }

private:
	const MetaType* m_elementType;
};

template <typename List>
void MetaList<List>::write( MetaWriter& writer, const void* data ) const
{
	const List& list = *static_cast<const List*>( data );

	writer.startArray();
	for ( auto it = list.begin(), last = list.end(); it != last; ++it )
	{
		if ( it != list.begin() )
			writer.delimitArray();

		m_elementType->write( writer, stdx::to_address( it ) );
	}
	writer.endArray();
}

template <typename List>
void MetaList<List>::read( MetaReader& reader, void* data ) const
{
	List& list = *static_cast<List*>( data );

	reader.startArray();

	if constexpr ( stdx::is_detected_v<detail::emplace_back_default_t, List> )
	{
		const size_t maxSize = list.max_size();
		for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
		{
			if ( count == maxSize )
				throw MetaIOException( stdx::format( "Exceeding max size of list: {}", maxSize ) );

			auto& element = list.emplace_back();
			m_elementType->read( reader, std::addressof( element ) );
		}
	}
	else
	{
		// assume fixed size array
		const size_t size = std::size( list );
		for ( size_t i = 0; i < size; ++i )
		{
			if ( !reader.hasNextArrayElement( i ) )
				throw MetaIOException( stdx::format( "Only {} elements were read into array of size {}", i, size ) );

			m_elementType->read( reader, std::addressof( list[ i ] ) );
		}

		if ( reader.hasNextArrayElement( size ) )
			throw MetaIOException( stdx::format( "Too many elements read into array of size {}", size ) );
	}
}

#define DECLARE_META_LIST( List ) \
template<typename T> \
struct MetaTypeResolver<List<T>> { \
	static const MetaType* get() { \
		static const MetaList<List<T>> s_metaType( getMetaType<T>() ); \
		return &s_metaType; \
	}};

DECLARE_META_LIST( std::vector )
// add std::list and std::array?

template <typename Map>
class MetaMap : public MetaType
{
public:
	using key_type = typename Map::key_type;
	using mapped_type = typename Map::mapped_type;

	MetaMap( const MetaType* keyType, const MetaType* valueType )
		: MetaType( stdx::reflection::type_name_v<Map> )
		, m_keyType{ keyType }
		, m_valueType{ valueType }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const MetaType* getKeyType() const { return m_keyType; }
	const MetaType* getValueType() const { return m_valueType; }

private:
	const MetaType* m_keyType;
	const MetaType* m_valueType;
};

template <typename Map>
void MetaMap<Map>::write( MetaWriter& writer, const void* data ) const
{
	const Map& map = *static_cast<const Map*>( data );

	writer.startArray();

	for ( auto it = map.begin(), last = map.end(); it != last; ++it )
	{
		if ( it != map.begin() )
			writer.delimitArray();

		auto[ key, value ] = *it;

		writer.startObject();

		writer.startVariable( "K" );
		m_keyType->write( writer, std::addressof( key ) );
		writer.endVariable();

		writer.delimitObject();

		writer.startVariable( "V" );
		m_valueType->write( writer, std::addressof( value ) );
		writer.endVariable();

		writer.endObject();
	}

	writer.endArray();
}

template <typename Map>
void MetaMap<Map>::read( MetaReader& reader, void* data ) const
{
	Map& map = *static_cast<Map*>( data );

	reader.startArray();
	
	for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
	{
		reader.startObject();
		auto expectedKeyKey = reader.startVariable();
		if ( expectedKeyKey != "K" )
			throw MetaIOException( stdx::format( "Expected variable K at map element {}", count ) );

		key_type key{};
		m_keyType->read( reader, std::addressof( key ) );
		reader.endVariable();

		if ( !reader.hasNextObjectVariable( 1 ) )
			throw MetaIOException( stdx::format( "Missing value in map element {}", count ) );

		auto expectedValueKey = reader.startVariable();
		if ( expectedValueKey != "V" )
			throw MetaIOException( stdx::format( "Expected variable V at map element {}", count ) );

		mapped_type value{};
		m_valueType->read( reader, std::addressof( value ) );
		reader.endVariable();

		if constexpr ( stdx::is_detected_v<detail::insert_value_t, Map> )
		{
			auto result = map.insert( { std::move( key ), std::move( value ) } );
			if ( !result.second )
				throw MetaIOException( stdx::format( "Key {} duplicate found at element {}", key, count ) );
		}
		else
		{
			// less safe but supports stdx::enum_map
			map[ std::move( key ) ] = std::move( value );
		}

		if ( reader.hasNextObjectVariable( 2 ) )
			throw MetaIOException( "Expected end of object in map element" );
	}
}

#define DECLARE_META_MAP( type ) \
template<typename K, typename V> \
struct MetaTypeResolver<type<K, V>>{ \
	static const MetaType* get() { \
		static const MetaMap<type<K, V>> s_metaType( getMetaType<K>(), getMetaType<V>() ); \
		return &s_metaType; \
	}};

DECLARE_META_MAP( std::unordered_map )
DECLARE_META_MAP( std::map )
DECLARE_META_MAP( stdx::enum_map )
DECLARE_META_MAP( stdx::flat_map )
DECLARE_META_MAP( stdx::simple_map )

template <typename Set>
class MetaSet : public MetaType
{
public:
	MetaSet( const MetaType* type )
		: MetaType( stdx::reflection::type_name_v<Set> )
		, m_elementType{ type }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const MetaType* getElementType() const { return m_elementType; }

private:
	const MetaType* m_elementType;
};

template <typename Set>
void MetaSet<Set>::write( MetaWriter& writer, const void* data ) const
{
	const Set& list = *static_cast<const Set*>( data );

	writer.startArray();
	for ( auto it = list.begin(), last = list.end(); it != last; ++it )
	{
		if ( it != list.begin() )
			writer.delimitArray();

		m_elementType->write( writer, stdx::to_address( it ) );
	}
	writer.endArray();
}

template <typename Set>
void MetaSet<Set>::read( MetaReader& reader, void* data ) const
{
	Set& set = *static_cast<Set*>( data );

	reader.startArray();

	const size_t maxSize = set.max_size();
	for ( size_t count = 0; reader.hasNextArrayElement( count ); ++count )
	{
		if ( count >= maxSize )
			throw MetaIOException( stdx::format( "Exceeding max size of set: {}", maxSize ) );

		typename Set::value_type value{};
		m_elementType->read( reader, std::addressof( value ) );

		auto result = set.insert( std::move( value ) );
		if ( !result.second )
			throw MetaIOException( stdx::format( "Duplicate set value at element {}", count ) );
	}
}

#define DECLARE_META_SET( Set ) \
template<typename T> \
struct MetaTypeResolver<Set<T>>{ \
	static const MetaType* get() { \
		static const MetaSet<Set<T>> s_metaType( getMetaType<T>() ); \
		return &s_metaType; \
	}};

DECLARE_META_SET( std::set )
DECLARE_META_SET( std::unordered_set )

template <typename T>
class MetaUniquePointer : public MetaType
{
public:
	MetaUniquePointer( const MetaType* type )
		: MetaType( stdx::reflection::type_name_v<std::unique_ptr<T>> )
		, m_type{ type }
	{}

	void write( MetaWriter& writer, const void* data ) const override;
	void read( MetaReader& reader, void* data ) const override;

	const MetaType* getType() const { return m_type; }

private:
	const MetaType* m_type;
};

template <typename T>
void MetaUniquePointer<T>::write( MetaWriter& writer, const void* data ) const
{
	const std::unique_ptr<T>& ptr = *static_cast<const std::unique_ptr<T>*>( data );

	if ( ptr == nullptr )
		writer.writeNull();
	else
		m_type->write( writer, ptr.get() );
}

template <typename T>
void MetaUniquePointer<T>::read( MetaReader& reader, void* data ) const
{
	std::unique_ptr<T>& ptr = *static_cast<std::unique_ptr<T>*>( data );

	if ( reader.isNull() )
	{
		ptr = nullptr;
	}
	else
	{
		ptr = std::make_unique<T>();
		m_type->read( reader, ptr.get() );
	}
}

template <typename T>
struct MetaTypeResolver<std::unique_ptr<T>>
{
	static const MetaType* get()
	{
		static const MetaUniquePointer<T> s_metaType( getMetaType<T>() );
		return &s_metaType;
	}
};