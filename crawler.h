#ifndef CRAWLER_H
#define CRAWLER_H

#include <string>
#include <list>
#include <curl/curl.h>

#define ROBOTS_TXT_URL "/robots.txt"
#define INDEX_URL "/"

#define DIR_STRUCT_LEVEL 4

// has to be 1 as it is the only output which is implemented by now
#define FILE_OUTPUT 1

#if FILE_OUTPUT
#define OUTPUT_DIR "data"
#endif

class Crawler {
public:
    void crawl(std::string);

private:
    CURL* init_curl();
    std::list<std::string> parse_domains();

    void make_dir(std::string);
    void make_dir_struct(std::string, std::string, int);

    std::string get_dir_struct(std::string, std::string, int);
};

#endif
