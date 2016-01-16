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

	class Worker{
		std::thread thr;
	public:
		Worker(ThreadPool* p) : thr(&ThreadPool::worker_do, p) {}
		~Worker(){ if(thr.joinable()) thr.join(); }
		auto running() -> bool { return !thr.joinable(); }
		auto join() -> void { thr.join(); }
	};

	enum Status{
		STOPPED, RUNNING, SHUTDOWN
	};

	std::atomic<Status> status;
	const std::size_t max_threads;

	// number of worksets currently processed
	std::atomic<int> processing; 
	std::list<Worker> workers;

	std::mutex sets1_mutex;
	std::list<Workset> sets1;

	std::mutex sets2_mutex;
	std::list<Workset> sets2;

	GlobData& gdata;

	inline auto list_merge() -> void
	{
		sets1.splice(sets1.end(), sets2);
	}

	auto worker_do() -> void
	{
		using namespace std::chrono_literals;
		auto work = false;
		while(status == Status::RUNNING){

			sets1_mutex.lock();
			if(status != Status::RUNNING){
				sets1_mutex.unlock();
				break;
			}
			if(sets1.size() >= 1){

				auto ws = sets1.front();
				sets1.pop_front();

				sets1_mutex.unlock();

				DEBUG_LOG("Workset is processed ..." << std::endl);
				if(!work){
					++processing;
					work = true;
				}
				ws(*this, gdata);
			}else{
				sets1_mutex.unlock();
				if(work){
					--processing;
					work = false;
				}
			}
			// FIXME: Hardcoded sleep time
			std::this_thread::sleep_for(40ms);
		}
	}

	auto distribute() -> void
	{
		std::lock_guard<std::mutex> lock2( sets2_mutex );
		std::lock_guard<std::mutex> lock1( sets1_mutex );

		if(sets1.size() == 0 && sets2.size() == 0){
			DEBUG_LOG("both work-queues are empty!" << std::endl);
			if(processing == 0){
				status = Status::SHUTDOWN;
				LOG << "ThreadPool shutdown! No more work." << std::endl;
			}
			return;
		}

		if(sets1.size() < max_threads*2/3)
			list_merge();

	}

	auto add_worker() -> void
	{
		workers.emplace_back(this);
	}

public:

	ThreadPool(const std::size_t max_thr, GlobData& gdata)
		: status(Status::STOPPED),max_threads(max_thr),processing(0),gdata(gdata)
	{
	}

	auto load(Workset set) -> void
	{
		if(status != Status::STOPPED){
			sets2_mutex.lock();
			sets2.push_back(set);
			sets2_mutex.unlock();
		}else{
			sets1.push_back(set);
		}
	}

	auto run(std::chrono::milliseconds sleepfor) -> void
	{
		status = Status::RUNNING;
		for(auto i = max_threads; i > 0; i--)
			add_worker();

		LOG << "ThreadPool is running..." << std::endl;
		while(status == Status::RUNNING){
			std::this_thread::sleep_for(sleepfor);
			distribute();
		}
	}

	auto send_stop() -> void
	{
		status = Status::SHUTDOWN;
	}

	auto join() -> void
	{
		workers.clear();
		status = Status::STOPPED;
	}
};

#endif
