#pragma once

#include "MetaClass.h"
#include "MetaEnum.h"
#include "MetaList.h"
#include "MetaMap.h"
#include "MetaPointer.h"
#include "MetaPrimitive.h"
#include "MetaSet.h"
#include "MetaString.h"

#include <stdx/type_traits.h>

namespace Meta
{

// meta type resolver defaults to primitive type

template<> const MetaType* MetaTypeResolver<bool>::get();
template<> const MetaType* MetaTypeResolver<uint8_t>::get();
template<> const MetaType* MetaTypeResolver<uint16_t>::get();
template<> const MetaType* MetaTypeResolver<uint32_t>::get();
template<> const MetaType* MetaTypeResolver<uint64_t>::get();
template<> const MetaType* MetaTypeResolver<int8_t>::get();
template<> const MetaType* MetaTypeResolver<int16_t>::get();
template<> const MetaType* MetaTypeResolver<int32_t>::get();
template<> const MetaType* MetaTypeResolver<int64_t>::get();
template<> const MetaType* MetaTypeResolver<float>::get();
template<> const MetaType* MetaTypeResolver<double>::get();

template <typename T>
const MetaType* MetaTypeResolver<T>::get()
{
	if constexpr ( std::is_enum_v<T> )
	{
		static const MetaEnum<T> s_metaEnum;
		return &s_metaEnum;
	}
	else if constexpr ( std::is_pointer_like_v<T> )
	{
		static const MetaPointer<T> s_metaPointer;
		return &s_metaPointer;
	}
	else if constexpr ( std::is_convertible_v<T, std::string_view> )
	{
		static const MetaString<T> s_metaString;
		return &s_metaString;
	}
	else if constexpr ( stdx::is_map_v<T> )
	{
		static const MetaMap<T> s_metaMap;
		return &s_metaMap;
	}
	else if constexpr ( stdx::is_set_v<T> )
	{
		static const MetaSet<T> s_metaSet;
		return &s_metaSet;
	}
	else if constexpr ( stdx::is_list_v<T> || stdx::is_array_like_v<T> )
	{
		static const MetaList<T> s_metaList;
		return &s_metaMap;
	}
	else
	{
		static const MetaPrimitive<T> s_metaPrimitive;
		return &s_metaPrimitive;
	}
}

} // namespace Meta

#define META_CLASS_BEGIN_COMMON( ClassType ) \
const Meta::MetaClass* ClassType::staticGetMetaClass() { return static_cast<const Meta::MetaClass*>( Meta::getMetaType<ClassType>() ); } \
std::string_view ClassType::staticGetClassName() { return ClassType::staticGetMetaClass()->getName(); } \
template<> const Meta::MetaType* Meta::MetaTypeResolver<ClassType>::get() { \
	using LocalClassType = ClassType; \
	static const Meta::MetaClass s_metaType = MetaClass::MakeMetaClass<ClassType>( \
		stdx::reflection::type_name_v<ClassType>, \
		std::is_polymorphic_v<ClassType>, \
		std::is_abstract_v<ClassType>,

#define META_CLASS_BEGIN( ClassType ) META_CLASS_BEGIN_COMMON( ClassType ) {

#define META_BASE_CLASS_BEGIN( ClassType ) META_CLASS_BEGIN_COMMON( ClassType ) {

#define META_DERIVED_CLASS_BEGIN( ClassType, ParentType ) \
	static_assert( std::is_base_of_v<ParentType, ClassType> ); \
	META_CLASS_BEGIN_COMMON( ClassType ) \
	ParentType::staticGetMetaClass(), {

#define META_VAR( name ) \
	Meta::MetaVariable{ getMetaType<decltype(LocalClassType::name)>(), #name, offsetof( LocalClassType, name ) },

#define META_CLASS_END } ); return &s_metaType; }



#ifndef META_DECLARE_CLASS
#define META_DECLARE_CLASS static_assert( "META_DECLARE_CLASS is defined in Meta/MetaDeclare.h" );
#endif

#ifndef META_DECLARE_BASE_CLASS
#define META_DECLARE_BASE_CLASS static_assert( "META_DECLARE_BASE_CLASS is defined in Meta/MetaDeclare.h" );
#endif

#ifndef META_DECLARE_DERRIVED_CLASS
#define META_DECLARE_DERRIVED_CLASS static_assert( "META_DECLARE_DERRIVED_CLASS is defined in Meta/MetaDeclare.h" );
#endif