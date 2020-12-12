#pragma once

namespace Math
{

template <typename T>
struct Size2
{
	T width;
	T height;
};

using Size2i = Size2<int>;
using Size2f = Size2<float>;

template <typename T>
struct Border
{
	T left;
	T top;
	T right;
	T bottom;
};

template <typename T>
constexpr bool operator==( const Border<T>& lhs, const Border<T>& rhs ) noexcept
{
	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.right == rhs.right && lhs.bottom == rhs.bottom;
}

template <typename T>
constexpr bool operator!=( const Border<T>& lhs, const Border<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

using Borderi = Border<int>;
using Borderf = Border<float>;

template <typename T>
struct Rect
{
	T left;
	T top;
	T width;
	T height;

	constexpr T Right() const noexcept { return left + width; }
	constexpr T Bottom() const noexcept { return top + height; }
};

template <typename T>
constexpr bool operator==( const Rect<T>& lhs, const Rect<T>& rhs ) noexcept
{
	return lhs.left == rhs.left && lhs.top == rhs.top && lhs.width == rhs.width && lhs.height == rhs.height;
}

template <typename T>
constexpr bool operator!=( const Rect<T>& lhs, const Rect<T>& rhs ) noexcept
{
	return !( lhs == rhs );
}

using Recti = Rect<int>;
using Rectf = Rect<float>;

} // namespace Math