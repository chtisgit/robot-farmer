#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <thread>
//#include <vector>
#include <list>
#include <chrono>
#include <mutex>

#include "log.h"

template<class Workset, class GlobData>
class ThreadPool{
	std::atomic<bool> running;
	const int max_threads;

	std::list<std::thread> workers;

	std::mutex sets1_mutex;
	std::list<Workset> sets1;

	std::mutex sets2_mutex;
	std::list<Workset> sets2;

	GlobData& gdata;

	inline void list_merge(){
		sets1.splice(sets1.end(), sets2);
	}
	void distribute();
	void add_worker();
public:

	ThreadPool(const int max_thr, GlobData& gdata);

	void load(Workset set);
	void run(std::chrono::milliseconds);
	void send_stop();
	void join();
};


template<class Workset, class GlobData>
ThreadPool<Workset,GlobData>::ThreadPool(const int max_thr, GlobData& gdata)
 : running(false),max_threads(max_thr),gdata(gdata)
{
}

template<class Workset, class GlobData>
void ThreadPool<Workset,GlobData>::distribute()
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

template<class Workset, class GlobData>
void ThreadPool<Workset,GlobData>::add_worker()
{
	using std::literals::chrono_literals::operator""ms;

	workers.emplace_back([this](){
		
		while(running){

			sets1_mutex.lock();
			if(sets1.size() >= 1){
				
				auto ws = sets1.front();
				sets1.pop_front();

				sets1_mutex.unlock();
				
				LOG << "Workset is processed ..." << std::endl;
				ws(*this, gdata);
			}else{
				sets1_mutex.unlock();
			}

			// FIXME: Hardcoded sleep time
			std::this_thread::sleep_for(50ms);

		}
		
	});

}

template<class Workset, class GlobData>
void ThreadPool<Workset,GlobData>::load(Workset set)
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

template<class Workset, class GlobData>
void ThreadPool<Workset,GlobData>::run(std::chrono::milliseconds sleepfor)
{
	running = true;
	for(auto i = max_threads; i > 0; i--)
		add_worker();

	LOG << "ThreadPool is running..." << std::endl;
	while(running){
		std::this_thread::sleep_for(sleepfor);
		distribute();
	}
}

template<class Workset, class GlobData>
void ThreadPool<Workset,GlobData>::send_stop()
{
	running = false;
}

template<class Workset, class GlobData>
void ThreadPool<Workset,GlobData>::join()
{
	for(auto& w : workers)
		w.join();
	workers.clear();
}

#endif
