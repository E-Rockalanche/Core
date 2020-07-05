#pragma once

#include <utility>

namespace stdx
{

template <typename T, typename Priority, typename Compare = std::less<Priority>>
class priority_queue
{
	using value_type = T;
	using key_type = Priority;
	using 
};

}