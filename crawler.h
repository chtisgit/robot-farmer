#ifndef CRAWLER_H
#define CRAWLER_H

#include <string>
#include <list>
#include <regex>
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
    typedef std::function<void (std::string)> DomainFoundFunc;

    Crawler();
    void crawl(std::string);
    void setCallback(DomainFoundFunc);

    inline operator bool()
    {
        return curl != NULL;
    }

private:

    void download_robots(std::string domain);
    void parse_domains(std::string domain);

    void make_dir(std::string);
    void make_dir_struct(std::string, std::string, int);

    void new_domain(std::string);
    bool domain_is_new(std::string);
    bool domain_is_valid(std::string);

    CURL *curl;

    std::string get_dir_struct(std::string, std::string, int);

    DomainFoundFunc df_callback;

    static std::regex domain_regex;

};

#endif
