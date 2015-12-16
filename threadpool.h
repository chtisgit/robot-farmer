#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
#include <list>
#include <chrono>
#include <mutex>

class Worker{
	int id;
	std::thread thr;

public:
	Worker(int id, std::thread&& thr);
	inline auto join() -> void
	{
		thr.join();
	}
};

template<class Workset>
class ThreadPool{
	std::atomic<bool> running;
	std::atomic<int> thr_id_count;
	const int max_threads;

	std::list<Worker> workers;

	std::mutex sets_mutex;
	std::list<Workset> sets_queue;

	std::list<Workset> sets_waiting;


	inline auto list_merge() -> void {
		sets_queue.splice(sets_queue.end(), sets_waiting);
	}
	auto distribute() -> void;
	auto add_worker() -> void;
public:

	ThreadPool(const int max_thr);

	auto load(Workset set) -> void;
	auto run(std::chrono::milliseconds = 250) -> void;
	auto stopall() -> void;
	auto waitall() -> void;
};


template<class Workset>
ThreadPool<Workset>::ThreadPool(const int max_thr)
 : running(false),thr_id_count(0),max_threads(max_thr)
{
}

template<class Workset>
void ThreadPool<Workset>::distribute()
{
	if(sets_waiting.size() == 0)
		return;

	const auto size = sets_queue.size();

	if(size < static_cast<decltype(size)>(max_threads)*2/3){
		sets_mutex.lock();
		list_merge();
		sets_mutex.unlock();
	}
}

template<class Workset>
void ThreadPool<Workset>::add_worker()
{
	workers.emplace_back(thr_id_count++, std::thread([this](){
		
		while(running){

			sets_mutex.lock();
			if(sets_queue.size() >= 1){

				auto ws = sets_queue.front();
				sets_queue.pop_front();

				sets_mutex.unlock();

				ws();
			}else{
				sets_mutex.unlock();
			}
		}
		
	}));

}

template<class Workset>
void ThreadPool<Workset>::load(Workset set)
{
	if(running){
		sets_waiting.push_back(set);
		distribute();
	}else{
		sets_queue.push_back(set);
	}
}

template<class Workset>
void ThreadPool<Workset>::run(std::chrono::milliseconds sleepfor)
{
	running = true;
	for(auto i = max_threads; i > 0; i--)
		add_worker();

	while(running){
		std::this_thread::sleep_for(sleepfor);
		distribute();
	}
}

template<class Workset>
void ThreadPool<Workset>::stopall()
{
	running = false;
}

template<class Workset>
void ThreadPool<Workset>::waitall()
{
	for(auto& w : workers)
		w.join();
}


#endif
