#include "Meta/MetaPrimitive.h"

namespace Meta
{

#define DECLARE_META_PRIMITIVE( type )	\
const MetaPrimitive<type> s_metaPrimitive_##type( #type );	\
const MetaType* MetaTypeResolver<type>::get() {	\
	return &s_metaPrimitive_##type;	\
}

DECLARE_META_PRIMITIVE( bool )

DECLARE_META_PRIMITIVE( uint8_t )
DECLARE_META_PRIMITIVE( uint16_t )
DECLARE_META_PRIMITIVE( uint32_t )
DECLARE_META_PRIMITIVE( uint64_t )

DECLARE_META_PRIMITIVE( int8_t )
DECLARE_META_PRIMITIVE( int16_t )
DECLARE_META_PRIMITIVE( int32_t )
DECLARE_META_PRIMITIVE( int64_t )

DECLARE_META_PRIMITIVE( float )
DECLARE_META_PRIMITIVE( double )

#undef DECLARE_META_PRIMITIVE

}