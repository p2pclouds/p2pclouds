#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

/* for example:
#include <iostream>
#include <vector>
#include <chrono>

#include "threadpool.h"

int main()
{
	ThreadPool pool(4);
	std::vector< std::future<int> > results;

	for (int i = 0; i < 8; ++i) {
		results.emplace_back(
			pool.enqueue([i] {
			std::cout << "hello " << i << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "world " << i << std::endl;
			return i*i;
		})
			);
	}

	for (auto && result : results)
		std::cout << result.get() << ' ';
	std::cout << std::endl;

	return 0;
}
*/

namespace P2pClouds {

	class ThreadContex
	{
	public:
		ThreadContex()
		{
		}

		virtual ~ThreadContex()
		{
		}

		virtual void onTaskStart()
		{
		}

		virtual void onTaskEnd()
		{
		}
	};

	template<class THREAD_CONTEXT = ThreadContex>
	class ThreadPool {
	public:
		ThreadPool(size_t threads = std::thread::hardware_concurrency());
		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
			->std::future<typename std::result_of<F(P2pClouds::ThreadContex& context, Args...)>::type>;
		~ThreadPool();
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;
		// the task queue
		std::queue< std::function<void(THREAD_CONTEXT&)> > tasks;

		// synchronization
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};

	// the constructor just launches some amount of workers
	template<class THREAD_CONTEXT>
	inline ThreadPool<THREAD_CONTEXT>::ThreadPool(size_t threads)
		: stop(false)
	{
		for (size_t i = 0; i < threads; ++i)
			workers.emplace_back(
				[this]
		{
			THREAD_CONTEXT context;

			for (;;)
			{
				std::function<void(THREAD_CONTEXT&)> task;

				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock,
						[this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}

				context.onTaskStart();
				task(context);
				context.onTaskEnd();
			}
		}
		);
	}

	// add new work item to the pool
	template<class THREAD_CONTEXT>
	template<class F, class... Args>
	auto ThreadPool<THREAD_CONTEXT>::enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(P2pClouds::ThreadContex& context, Args...)>::type>
	{
		using return_type = typename std::result_of<F(P2pClouds::ThreadContex& context, Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type(P2pClouds::ThreadContex& context)> >(
			std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task](THREAD_CONTEXT& context) { (*task)(context); });
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	template<class THREAD_CONTEXT>
	inline ThreadPool<THREAD_CONTEXT>::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
			worker.join();
	}

}

