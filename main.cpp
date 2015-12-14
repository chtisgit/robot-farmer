#include <iostream>

#include "log.h"
#include "crawler.h"
#include "threadpool.h"
#include "workset.h"

ThreadPool<Workset> *global_pool = nullptr;
void signal_handler_exit()
{
    if(global_pool != nullptr)
            global_pool->stopall();
}


#include "util.h"

void usage() {
    std::cerr << "Usage: robot_farmer seed_domain" << std::endl; 
    exit(1);
}

int main(int argc, char **argv) {
    Log::create(std::cout);
    LOG << "Starting..." << std::endl;

    signal(SIGINT, signal_handler_exit);
    signal(SIGTERM, signal_handler_exit);

    if(argc < 2)
        usage();
    
    Crawler c;
    ThreadPool<Workset> pool(100);
    global_pool = &pool;

    c.crawl(argv[1]);

    return 0;
}

