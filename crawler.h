#ifndef CRAWLER_H
#define CRAWLER_H

#include <string>
#include <list>
#include <regex>
#include <curl/curl.h>
#include "curlprovider.h"

#include <cctype>

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
    typedef std::function<void (const std::string &)> DomainFoundFunc;

    Crawler();
    Crawler(CURL *c);

    void setCURL(CURL*);
    void crawl(std::string);
    void setCallback(DomainFoundFunc);

    inline operator bool() const
    {
        return curl;
    }

private:

    inline bool is_safe_char(char x) {
        return isalnum(x);
    }

    void download_robots(const std::string & domain);
    void parse_domains(const std::string & domain);

    void make_dir(const std::string &);
    void make_dir_struct(const std::string &, const std::string &, unsigned int);

    void new_domain(const std::string &);
    bool domain_is_new(const std::string &);
    bool domain_is_valid(const std::string &);

    CURL *curl;

    std::string get_dir_struct(const std::string &, const std::string &, unsigned int);

    DomainFoundFunc df_callback;

    static std::regex domain_regex;

};

#endif
