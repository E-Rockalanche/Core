#ifndef STDX_MEMORY_HPP
#define STDX_MEMORY_HPP

#include <memory>

namespace stdx
{

template <typename T,
	std::enable_if_t<!std::is_array_v<T>, int> = 0>
std::unique_ptr<T> make_unique_for_overwrite()
{
	return std::unique_ptr<T>( new T );
}

template <typename T,
	std::enable_if_t<std::is_array_v<T>, int> = 0>
std::unique_ptr<T> make_unique_for_overwrite( std::size_t size )
{
	return std::unique_ptr<T>( new typename std::remove_extent<T>::type[ size ] );
}

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_for_overwrite( Args&&... args ) = delete;


} // namespace stdx

#endif