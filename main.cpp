#include <iostream>

#include "log.h"
#include "crawler.h"

#include "util.h"

void usage() {
    std::cout << "Usage: robot_farmer seed_domain" << std::endl; 
    exit(1);
}

int main(int argc, char **argv) {
    Log::create(std::cout);
    LOG << "Starting..." << std::endl;

    if(argc < 2)
        usage();
    
    Crawler c;
    c.crawl(argv[1]);

    return 0;
}
