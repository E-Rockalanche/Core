#pragma once

#include <Meta/MetaEnum.h>

class MetaEditorEnum : public Meta::MetaType
{
public:
	MetaEditorEnum( std::string name ) : Meta::MetaType( std::move( name ) ) {}

	void save( ByteWriter& ) const override { dbBreak(); } // saving only for meta export

	void load( ByteReader& in ) override
	{
		dbVerify( in.ReadHeader( static_cast<uint32_t>( Meta::TypeTag::Enum ) ) == 0 );
		auto count = in.ReadUint32();
		m_values.reserve( count );
		for ( ; count > 0; --count )
		{
			auto value = in.ReadInt64();
			auto name = in.ReadString<std::string>();
			m_values.push_back( std::make_pair( value, std::move( name ) ) );
		}
		m_isBitset = in.ReadBool();
	}

private:
	std::vector<std::pair<int64_t, std::string>> m_values;
	bool m_isBitset = false;
};
