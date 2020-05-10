#pragma once

#include "MetaClass.h"
#include "MetaTypeDetail.h"

#define META_CLASS_BEGIN_COMMON( ClassType ) \
const MetaClass* ClassType::staticGetMetaClass() { return static_cast<const MetaClass*>( getMetaType<ClassType>() ); } \
std::string_view ClassType::staticGetClassName() { return ClassType::staticGetMetaClass()->getName(); } \
template<> const MetaType* MetaTypeResolver<ClassType>::get() { \
	using LocalClassType = ClassType; \
	static const  MetaClass s_metaType( \
		stdx::reflection::type_name_v<ClassType>.c_str(), \
		std::is_polymorphic_v<ClassType>, \
		std::is_abstract_v<ClassType>,

#define META_CLASS_BEGIN( ClassType ) META_CLASS_BEGIN_COMMON( ClassType ) {

#define META_BASE_CLASS_BEGIN( ClassType ) META_CLASS_BEGIN_COMMON( ClassType ) {

#define META_DERIVED_CLASS_BEGIN( ClassType, ParentType ) \
	META_CLASS_BEGIN_COMMON( ClassType ) \
	ParentType::staticGetMetaClass(), {

#define META_VAR( name ) \
	MetaVariable( getMetaType<decltype(LocalClassType::name)>(), #name, offsetof( LocalClassType, name ) ),

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