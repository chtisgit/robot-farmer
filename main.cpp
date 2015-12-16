#include <iostream>
#include <csignal>

#include "log.h"
#include "crawler.h"
#include "threadpool.h"
#include "workset.h"

ThreadPool<Workset> *global_pool = nullptr;
void signal_handler_exit(int sig)
{
    if(global_pool != nullptr)
            global_pool->stopall();
}


#include "util.h"

void usage() {
    std::cerr << "Usage: robot_farmer seed_domain" << std::endl; 
}

int main(int argc, char **argv) {
    using namespace std::literals::chrono_literals;

    Log::create(std::cout);
    LOG << "Starting..." << std::endl;

    signal(SIGINT, signal_handler_exit);
    signal(SIGTERM, signal_handler_exit);

    if(argc < 2){
        usage();
	return 1;
    }
    
    Crawler c;
    ThreadPool<Workset> pool(2);
    global_pool = &pool;

    c.setCallback([&pool](const std::string& s){
        pool.load(s);
    });

    pool.load(argv[1]);
    pool.run(20ms);

    return 0;
}

