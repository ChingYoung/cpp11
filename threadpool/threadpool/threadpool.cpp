// threadpool.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <thread>
#include <functional>
#include <list>
#include <atomic>
#include "syncqueue.h"

const int MaxTaskCount = 100;
class ThreadPool
{
public:
	using Task = std::function<void()>;
	ThreadPool(int numThreads = std::thread::hardware_concurrency())
		: m_queue(MaxTaskCount)
		, m_running(false)
	{
		Start(numThreads);
	}

	~ThreadPool()
	{
		Stop();
	}

	void Stop()
	{
		std::call_once(m_flag, [this] {StopThreadInGroup(); });
	}

	void AddTask(Task&& task)
	{
		m_queue.Put(std::forward<Task>(task));
	}

	void AddTask(const Task& task)
	{
		m_queue.Put(task);
	}

private:
	void Start(int numThreads)
	{
		std::cout << "thread num:" << numThreads << std::endl;

		m_running = true;
		//创建线程组
		for (int i = 0; i < numThreads; ++i)
		{
			m_threadgroup.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
		}
	}

	void RunInThread()
	{
		while (m_running)
		{
			//去任务分别执行
			std::list<Task> list;
			m_queue.Take(list);

			for (auto& task : list)
			{
				if (!m_running)
					return;

				task();
			}
		}
	}

	void StopThreadInGroup()
	{
		m_queue.Stop();	//同步队列线程停止
		m_running = false;

		for (auto thread : m_threadgroup)
		{
			if (thread)
				thread->join();
		}

		m_threadgroup.clear();
	}

private:
	std::list<std::shared_ptr<std::thread>> m_threadgroup;
	SyncQueue<Task> m_queue;
	std::atomic_bool m_running;
	std::once_flag m_flag;
};

void TestThreadPool()
{
	ThreadPool pool;
	std::thread thd1([&pool] {
		for (int i = 0; i < 100; ++i)
		{
			auto thdId = std::this_thread::get_id();
			pool.AddTask([thdId] {
				std::cout << "同步层线程1的线程id：" << thdId << std::endl;
			});
		}
	});

	std::thread thd2([&pool] {
		for (int i = 0; i < 100; ++i)
		{
			auto thdId = std::this_thread::get_id();
			pool.AddTask([thdId] {
				std::cout << "同步层线程2的线程id：" << thdId << std::endl;
			});
		}
	});

	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::getchar();

	pool.Stop();

	thd1.join();
	thd2.join();

	std::cout << "结束" << std::endl;
}

int main()
{
	TestThreadPool();

	std::getchar();
    return 0;
}

