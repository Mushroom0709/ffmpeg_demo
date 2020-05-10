#ifndef _X_SAFE_QUEUE_H_
#define _X_SAFE_QUEUE_H_

#include <queue>
#include <mutex>


namespace x
{
	template<class _T>
	class xQueue
	{
	private:
		std::queue<_T> queue_;
		std::mutex queue_lock_;
	public:
		xQueue()
		{
			//
		}
		~xQueue()
		{
			Clear();
		}
	public:
		void Push(_T _value)
		{
			std::lock_guard<std::mutex> auto_lock(queue_lock_);
			queue_.push(_value);
		}

		void MaxSziePush(_T _value,volatile bool* flag_,int _max_size = 256)
		{
			int size = -1;
			//int diff = _max_size / 10;

			while (*flag_)
			{
				queue_lock_.lock();
				size = queue_.size();
				queue_lock_.unlock();

				if (size >= _max_size)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
				}
				else
				{
					break;
				}
			}
			
			queue_lock_.lock();
			queue_.push(_value);
			queue_lock_.unlock();
		}

		int Size()
		{
			std::lock_guard<std::mutex> auto_lock(queue_lock_);
			return queue_.size();
		}

		bool TryPop(_T& _value)
		{
			std::lock_guard<std::mutex> auto_lock(queue_lock_);
			if (queue_.empty())
				return false;
			_value = queue_.front();
			queue_.pop();

			return true;
		}

		void Clear()
		{
			std::lock_guard<std::mutex> auto_lock(queue_lock_);
			while (queue_.size() > 0)
			{
				queue_.pop();
			}
		}
	};
}

#endif //_X_SAFE_QUEUE_H_