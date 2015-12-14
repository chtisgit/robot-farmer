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
	Worker(int id, std::thread thr);
	inline void join()
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


	inline void list_merge(){
		sets_queue.splice(sets_queue.end(), sets_waiting);
	}
	void distribute();
	void add_worker();
public:

	ThreadPool(const int max_thr);

	void load(Workset set);
	void run(std::chrono::milliseconds = 250);
	void stopall();
};


template<class Workset>
ThreadPool<Workset>::ThreadPool(const int max_thr)
 : max_threads(max_thr), running(false),thr_id_count(0)
{
}

template<class Workset>
void ThreadPool<Workset>::distribute()
{
	if(sets_waiting.size() == 0)
		return;

	if(sets_queue.size() < max_threads*2/3){
		sets_mutex.lock();
		list_merge();
		sets_mutex.unlock();
	}
}

template<class Workset>
void ThreadPool<Workset>::add_worker()
{
	workers.emplace_back(thr_id_count++, [this](){
		
		while(running){

			sets_mutex.lock();
			if(sets_queue.size() >= 1){

				auto ws = sets_queue.front();
				sets_queue.pop_front();

				sets_mutex.unlock();

				running = ws();
			}else{
				sets_mutex.unlock();
			}
		}
		
	});

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
	for(auto& w : workers)
		w.join();
}


#endif
