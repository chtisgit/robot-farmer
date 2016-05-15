
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
    LOG << "sending stop to all threads!" << std::endl;

    if(global_pool != nullptr)
        global_pool->send_stop();
}


#include "util.h"

void usage() {
    using std::cerr;
    using std::endl;
    cerr << "Usage: " << progname << " num_threads" << endl; 
    cerr << "num_threads   ...   (maximum) number of threads to use" << endl;
}

int main(int argc, char **argv) {
    using namespace std::literals::chrono_literals;

    progname = argv[0];

    if(argc != 2){
        usage();
        return 1;
    }
    
    const int num_threads = atoi(argv[1]);

    if(num_threads < 1 || num_threads > 5000){
        std::cerr << "num_threads must be between 1 and 5000" << std::endl;
        return 1;
    }

    CurlProvider provider(num_threads);
    ThreadPool<CrawlWorkset,CurlProvider> pool(num_threads, provider);
    global_pool = &pool;

    std::cout << "Please enter some URLs. EOF or blank line will start the download. Abort with Ctrl+C." << std::endl;

    std::string seed_url;
    do{
        getline(std::cin, seed_url);
        pool.load( CrawlWorkset(seed_url) );
    }while(!std::cin.eof() && !seed_url.empty());

    Log::create(std::cout);
    LOG << "Starting..." << std::endl;

    signal(SIGINT, signal_handler_exit);
    signal(SIGTERM, signal_handler_exit);

    pool.run(20ms);

    pool.join();
    global_pool = nullptr;

    return 0;
}

