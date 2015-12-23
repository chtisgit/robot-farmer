#define DEBUG

#include <iostream>
#include <csignal>

#include "log.h"
#include "crawler.h"
#include "threadpool.h"
#include "curlprovider.h"
#include "workset.h"

const char *progname = nullptr;
ThreadPool<CrawlWorkset,CurlProvider> *global_pool = nullptr;

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
    using std::literals::chrono_literals::operator""ms;

    progname = argv[0];

    signal(SIGINT, signal_handler_exit);
    signal(SIGTERM, signal_handler_exit);

    Log::create(std::cout);
    LOG << "Starting..." << std::endl;

    if(argc < 3){
        usage();
	return 1;
    }
    
    const char *start_url = argv[1];
    const int num_threads = atoi(argv[2]);

    if(num_threads < 1 || num_threads > 5000){
        std::cerr << "num_threads must be between 1 and 5000" << std::endl;
	return 1;
    }

    CurlProvider provider(num_threads);
    ThreadPool<CrawlWorkset,CurlProvider> pool(num_threads, provider);
    global_pool = &pool;

    pool.load( CrawlWorkset(start_url) );
    pool.run(20ms);

    pool.join();
    global_pool = nullptr;

    return 0;
}

