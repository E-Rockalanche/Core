#pragma once

#include <Meta/MetaClass.h>
#include <stdx/json.h>

class MetaEditor
{
public:


private:
	const Meta::MetaClass* m_type = nullptr;
	stdx::json m_document;
};