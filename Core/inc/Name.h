#pragma once

#include <string_view>

struct StringIntern;

class Name
{
public:
	using size_type = uint8_t;
	using view_type = std::string_view;

	Name() : m_str{ EmptyString } {}
	Name( const char* str );

	Name( const Name& ) = default;
	Name& operator=( const Name& ) = default;

	operator view_type() const noexcept;
	operator bool() const noexcept;

	const char* c_str() const noexcept;
	const char* data() const noexcept;
	size_type size() const noexcept;

	friend bool operator==( Name lhs, Name rhs ) noexcept { return lhs.m_str == rhs.m_str; }
	friend bool operator!=( Name lhs, Name rhs ) noexcept { return lhs.m_str != rhs.m_str; }
	friend bool operator<( Name lhs, Name rhs ) noexcept { return lhs.m_str < rhs.m_str; }
	friend bool operator>( Name lhs, Name rhs ) noexcept { return lhs.m_str > rhs.m_str; }
	friend bool operator<=( Name lhs, Name rhs ) noexcept { return lhs.m_str <= rhs.m_str; }
	friend bool operator>=( Name lhs, Name rhs ) noexcept { return lhs.m_str >= rhs.m_str; }

private:
	static const StringIntern* EmptyString;

private:
	const StringIntern* m_str;
};