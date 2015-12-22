#include <iostream>
#include <csignal>

#include "log.h"
#include "crawler.h"
#include "threadpool.h"
#include "workset.h"

const char *progname = nullptr;
ThreadPool<Workset> *global_pool = nullptr;

void signal_handler_exit(int sig)
{
    if(global_pool != nullptr)
            global_pool->send_stop();
}


#include "util.h"

void usage() {
    using std::cerr;
    using std::endl;
    cerr << "Usage: " << progname << " seed_domain num_threads" << endl; 
    cerr << "seed_domain   ...   the domain, where farming is started" << endl;
    cerr << "num_threads   ...   (maximum) number of threads to use" << endl;
}

int main(int argc, char **argv) {
    progname = argv[0];

    signal(SIGINT, signal_handler_exit);
    signal(SIGTERM, signal_handler_exit);

    Log::create(std::cout);
    LOG << "Starting..." << std::endl;

    if(argc < 2){
        usage();
	return 1;
    }
    
    Crawler c;
    ThreadPool<Workset> pool(100);
    global_pool = &pool;

    c.crawl(argv[1]);

    pool.join();
    global_pool = nullptr;

    return 0;
}

