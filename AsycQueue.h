// Func: An implemention of asynchronous Queue
// Author: hoxzheng
// date: 2019/5/31  
#ifndef _ASYC_QUEUE_H_
#define _ASYC_QUEUE_H_

#include<queue>
#include<condition_variable>
#include<mutex>

template<class T>
class AsycQueue
{
public:
	using container_type = typename std::queue<T>::container_type;
	// Constructors.
	AsycQueue()=default;
	//delete: do not support copy construct / copy operation.
	AsycQueue(const AsycQueue<T> &another_queue) = delete;
	AsycQueue& operator=(const AsycQueue<T> &another_queue) = delete;
	explicit AsycQueue(const container_type &c) :_queue(c) {}
	template<typename _InputIterator>
	AsycQueue(_InputIterator first, _InputIterator last) {
		for (auto it = first; it != last; ++it) {
			this->_queue.push(*it);
		}
	}
	bool empty();
	auto size(); //auto : maybe size_t or other.

	void push(const T & new_value);
	T wait_pop(); //Be in blocking when the queue is empty.
	bool try_pop(T & poped_value);  //return false when the queue is empty.


	~AsycQueue()=default;
private:
	// mutable : maybe modify these variables in class's const functions.
	mutable std::mutex mtx; 
	mutable std::condition_variable data_con; 
	std::queue<T> _queue;
};

#endif

template<class T>
inline bool AsycQueue<T>::empty()
{
	std::lock_guard<std::mutex> lck(mtx);
	return this->_queue.empty();
}

template<class T>
inline auto AsycQueue<T>::size()
{
	std::lock_guard<std::mutex> lck(mtx);
	return this->_queue.size();
}

template<class T>
inline void AsycQueue<T>::push(const T & new_value)
{
	std::lock_guard<std::mutex> lck(mtx);
	// move:: do not create temporary var.
	this->_queue.push(std::move(new_value));
	//if(_queue.size() == 1)
	this->data_con.notify_one(); // awake one of the consumer.

#ifndef NO_DEBUG
	std::cout << "Producer " << std::this_thread::get_id()
		<< " create a num: " << new_value << std::endl;
#endif
}

template<class T>
inline T AsycQueue<T>::wait_pop()
{
	std::unique_lock<std::mutex> lck(mtx);
	// block and auto-unlock when pred false <==> empty.
	// !!!!! can not directly use this->empty, which can cause dead lock.
	this->data_con.wait(lck, [this] {return !this->_queue.empty(); });

	T pop_val = std::move(this->_queue.front());
	this->_queue.pop();

#ifndef NO_DEBUG
	std::cout << "Consumer " << std::this_thread::get_id()
		<< " remove a num: " << pop_val << std::endl;
#endif

	return pop_val;
}

template<class T>
inline bool AsycQueue<T>::try_pop(T & poped_value)
{
	std::lock_guard<std::mutex> lck(mtx);
	if (this->empty())
		return false;
	poped_value = std::move(this->_queue.front());
	this->_queue.pop();
	return true;
}
