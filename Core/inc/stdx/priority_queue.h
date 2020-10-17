#pragma once

#include <queue>


namespace stdx
{

template <typename Priority, typename T>
struct priority_queue_entry
{
	Priority priority;
	T value;

	bool operator<( const priority_queue_entry& other ) const noexcept
	{
		return priority > other.priority;
	}
};

template <typename Priority, typename T>
using priority_queue = std::priority_queue<priority_queue_entry<Priority, T>>;

}