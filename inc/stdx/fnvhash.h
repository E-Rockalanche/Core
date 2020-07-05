#include <stdx/int.h>
#include <stdx/span.h>

namespace stdx
{

namespace detail
{

template <typename T>
struct fnv_constants {};

template <>
struct fnv_constants<uint32_t>
{
	static constexpr uint32_t prime = 0x01000193;
	static constexpr uint32_t basis = 0x811c9dc5;
};

template <>
struct fnv_constants<uint64_t>
{
	static constexpr uint64_t prime = 0x00000100000001B3;
	static constexpr uint64_t basis = 0xcbf29ce484222325;
};

}

template <typename T>
constexpr T fnv1a( stdx::span<const uint8_t> data ) noexcept
{
	T hash = fnv_constants<T>::basis;

	for ( uint8_t byte : data )
	{
		hash = ( hash ^ byte ) * fnv_constants<T>::prime;
	}

	return hash;
}

} // namespace stdx