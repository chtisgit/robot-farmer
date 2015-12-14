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
};


#endif
