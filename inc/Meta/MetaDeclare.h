#pragma once

#include "MetaClass.h"
#include <string_view>

#define META_DECLARE_CLASS \
private: \
	static const MetaClass s_metaClass; \
public: \
	static const MetaClass* staticGetMetaClass(); \
	static std::string_view staticGetClassName(); \
	const MetaClass* getMetaClass() const { return staticGetMetaClass(); } \
	std::string_view getClassName() const { return staticGetClassName(); }

#define META_DECLARE_BASE_CLASS \
private: \
	static const MetaClass s_metaClass; \
public: \
	static const MetaClass* staticGetMetaClass(); \
	static std::string_view staticGetClassName(); \
	virtual const MetaClass* getMetaClass() const { return staticGetMetaClass(); } \
	virtual std::string_view getClassName() const { return staticGetClassName(); }

#define META_DECLARE_DERIVED_CLASS \
private: \
	static const MetaClass s_metaClass; \
public: \
	static const MetaClass* staticGetMetaClass(); \
	static std::string_view staticGetClassName(); \
	const MetaClass* getMetaClass() const override { return staticGetMetaClass(); } \
	std::string_view getClassName() const override { return staticGetClassName(); }