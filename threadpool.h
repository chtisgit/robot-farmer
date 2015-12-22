#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
//#include <vector>
#include <list>
#include <chrono>
#include <mutex>

template<class Workset>
class ThreadPool{
	std::atomic<bool> running;
	const int max_threads;

	std::list<std::thread> workers;

	std::mutex sets1_mutex;
	std::list<Workset> sets1;

	std::mutex sets2_mutex;
	std::list<Workset> sets2;


	inline void list_merge(){
		sets1.splice(sets1.end(), sets2);
	}
	void distribute();
	void add_worker();
public:

	ThreadPool(const int max_thr);

	void load(Workset set);
	void run(std::chrono::milliseconds = 250);
	void send_stop();
	void join();
};


template<class Workset>
ThreadPool<Workset>::ThreadPool(const int max_thr)
 : running(false),max_threads(max_thr)
{
}

template<class Workset>
void ThreadPool<Workset>::distribute()
{
	if(sets2.size() == 0)
		return;

	if(sets1.size() < max_threads*2/3){
		sets1_mutex.lock();
		sets2_mutex.lock();
		list_merge();
		sets2_mutex.unlock();
		sets1_mutex.unlock();
	}
}

template<class Workset>
void ThreadPool<Workset>::add_worker()
{
	workers.emplace_back([this](){
		
		while(running){

			sets1_mutex.lock();
			if(sets1.size() >= 1){

				auto ws = sets1.front();
				sets1.pop_front();

				sets1_mutex.unlock();

				running = ws();
			}else{
				sets1_mutex.unlock();
			}
		}
		
	});

}

template<class Workset>
void ThreadPool<Workset>::load(Workset set)
{
	if(running){
		sets2_mutex.lock();
		sets2.push_back(set);
		sets2_mutex.unlock();
		distribute();
	}else{
		sets1.push_back(set);
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
void ThreadPool<Workset>::send_stop()
{
	running = false;
}

template<class Workset>
void ThreadPool<Workset>::join()
{
	for(auto& w : workers)
		w.join();
	workers.clear();
}

#endif
