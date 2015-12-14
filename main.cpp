#include <iostream>

#include "crawler.h"
#include "threadpool.h"
#include "workset.h"

ThreadPool<Workset> *global_pool = nullptr;
void signal_handler_exit()
{
    if(global_pool != nullptr)
            global_pool->stopall();
}


int main() {
    Crawler c;
    ThreadPool<Workset> pool(100);
    global_pool = &pool;

    c.crawl("orf.at");


    return 0;
}

