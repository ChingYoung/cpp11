#ifndef __SYNCQUEUE_H__
#define __SYNCQUEUE_H__

#include <mutex>
#include <condition_variable>
#include <iostream>

template<typename T>
class SyncQueue
{
public:
	SyncQueue(int maxSize)
		: m_maxSize(maxSize)
	{

	}
	~SyncQueue()
	{

	}

public:
	void Put(const T&x)
	{
		Add(x);
	}

	void Put(T&& x)
	{
		Add(std::forward<T>(x));
	}

	void Take(std::list<T>& list)
	{
		std::unique_lock<std::mutex> locker(m_mutex);

		m_notEmpty.wait(locker, [this]() {return m_needStop || NotEmpty(); });

		if (m_needStop)
			return;

		list = std::move(m_queue);
		m_notFull.notify_one();
	}

	void Take(T& t)
	{
		std::unique_lock<std::mutex> locker(m_mutex);

		m_notEmpty.wait(locker, [this]() {return m_needStop || NotEmpty(); });

		if (m_needStop)
			return;

		t = m_queue.front();
		m_queue.pop_front();
		m_notFull.notify_one();
	}

	void Stop()
	{
		{
			std::lock_guard<std::mutex> locker(m_mutex);
			m_needStop = true;
		}

		//被唤醒的线程不需要等待释放mutex锁就可以获取mutex，性能会好一点

		m_notFull.notify_all();
		m_notEmpty.notify_all();
	}

	bool Empty()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.empty();
	}
	bool Full()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.size() == m_maxSize;
	}
	std::size_t Size()
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		return m_queue.size();
	}
	int Count()
	{
		return m_queue.size();
	}

private:
	bool NotFull() const
	{
		bool full = m_queue.size() >= m_maxSize;
		if (full)
			std::cout << "buffer has fulled, wait..." << std::this_thread::get_id() <<std::endl;

		return !full;
	}
	bool NotEmpty() const
	{
		bool empty = m_queue.empty();
		if (empty)
			std::cout << "buffer has emptied, wait..." << std::this_thread::get_id() << std::endl;

		return !empty;
	}

	template<typename F>
	void Add(F&& x)
	{
		std::unique_lock<std::mutex> locker(m_mutex);
		m_notFull.wait(locker, [this] {return m_needStop || NotFull(); });
		if (m_needStop)
			return;

		m_queue.push_back(std::forward<F>(x));
		m_notEmpty.notify_one();
	}

private:
	std::list<T> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_notEmpty;
	std::condition_variable m_notFull;
	std::size_t m_maxSize;
	bool m_needStop;
};

#endif //__SYNCQUEUE_H__