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
	enum Status{
		STOPPED, RUNNING, SHUTDOWN
	};

	std::atomic<Status> status;
	const std::size_t max_threads;

	std::list<std::thread> workers;

	std::mutex sets1_mutex;
	std::list<Workset> sets1;

	std::mutex sets2_mutex;
	std::list<Workset> sets2;

	GlobData& gdata;

	inline void list_merge(){
		sets1.splice(sets1.end(), sets2);
	}

	void distribute() {
		std::lock_guard<std::mutex> lock2( sets2_mutex );
		if(sets2.size() == 0)
			return;

		std::lock_guard<std::mutex> lock1( sets1_mutex );
		if(sets1.size() < max_threads*2/3)
			list_merge();

	}

	void add_worker() {
		using namespace std::chrono_literals;

		workers.emplace_back([this](){
			while(status == Status::RUNNING){

				sets1_mutex.lock();
				if(sets1.size() >= 1){

					auto ws = sets1.front();
					sets1.pop_front();

					sets1_mutex.unlock();

					LOG << "Workset is processed ..." << std::endl;
					ws(*this, gdata);
				}else{
					sets1_mutex.unlock();
					// FIXME: Hardcoded sleep time
					std::this_thread::sleep_for(50ms);
				}


			}
		});
	}

	public:

	ThreadPool(const std::size_t max_thr, GlobData& gdata)
		: status(Status::STOPPED),max_threads(max_thr),gdata(gdata)
	{
	}

	void load(Workset set) {
		if(status != Status::STOPPED){
			sets2_mutex.lock();
			sets2.push_back(set);
			sets2_mutex.unlock();
			distribute();
		}else{
			sets1.push_back(set);
		}
	}

	void run(std::chrono::milliseconds sleepfor) {
		status = Status::RUNNING;
		for(auto i = max_threads; i > 0; i--)
			add_worker();

		LOG << "ThreadPool is running..." << std::endl;
		while(status == Status::RUNNING){
			std::this_thread::sleep_for(sleepfor);
			distribute();
		}
	}

	void send_stop() {
		status = Status::SHUTDOWN;
	}

	void join() {
		for(auto& w : workers)
			w.join();
		workers.clear();
		status = Status::STOPPED;
	}
};

#endif
