#pragma once

#include <cstdint>
#include <future>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

namespace stdx
{

class thread_pool
{
private:
	enum class signal
	{
		run,
		stop,
		kill
	};

public:

	thread_pool( size_t threadCount = std::thread::hardware_concurrency() )
	{
		threadCount = std::max<size_t>( threadCount, 1 );

		m_threads.reserve( threadCount );
		for ( size_t i = 0; i < threadCount; ++i )
		{
			m_threads.push_back( std::thread( [this] {
				while ( true )
				{
					std::function<void()> task;

					{
						std::unique_lock lock{ m_queueLock };
						m_condition.wait( lock, [this] { return m_signal == signal::stop || !m_taskQueue.empty(); } );

						if ( m_signal == signal::stop && m_taskQueue.empty() )
						{
							return;
							m_condition.notify_one();
						}

						task = std::move( m_taskQueue.front() );
						m_taskQueue.pop();
					}

					task();
				}
			} ) );
		}
	}

	~thread_pool()
	{
		{
			std::lock_guard{ m_queueLock };
			m_signal = signal::stop;
		}

		m_condition.notify_all();

		for ( auto& t : m_threads )
		{
			t.join();
		}
	}

	thread_pool& operator=( const thread_pool& ) = delete;
	thread_pool& operator=( thread_pool&& ) = delete;

	template <typename F>
	[[nodiscard]] auto push( F&& f ) -> std::future<std::invoke_result_t<F>>
	{
		auto task = std::make_shared<std::packaged_task<std::invoke_result_t<F>()>>( std::forward<F>( f ) );
		auto result = task->get_future();

		{
			std::lock_guard{ m_queueLock };
			m_taskQueue.emplace( [ task = std::move( task ) ]{ ( *task )( ); } );
		}

		m_condition.notify_one();

		return result;
	}

private:

	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_taskQueue;

	std::mutex m_queueLock;
	std::condition_variable m_condition;

	signal m_signal = signal::run;
};

}