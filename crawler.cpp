#include <list>
#include <iostream>
#include <string>
#include <regex>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <sys/stat.h>
#include "log.h"

#include "crawler.h"
#include "strexception.h"
#include "tld.h"

// thanks [http://myregexp.com/examples.html]
std::regex Crawler::domain_regex("(([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,6})");

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void Crawler::setCallback(DomainFoundFunc cb) {
    df_callback = cb;
}

CURL* Crawler::init_curl() {
    CURL *curl = curl_easy_init();
    
    if(!curl)
        throw StringException("Could not initialize curl");

}

void Crawler::make_dir(std::string name) {
    const int err = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    if(err == -1 && errno != EEXIST) {
        throw StringException(std::string("Could not create string ").append(name));
    }
}

void Crawler::make_dir_struct(std::string path, std::string domain, int lvl) {
    std::string cur = path;

    if(domain.length() < lvl) {
        make_dir(cur.append("/").append(domain));
        return;
    }

    for(int i = 0; i < lvl; i++) {
        const char* c = domain.substr(i, 1).c_str();

        switch(c[0]) {
            case '.':
                continue;
                break;
        }

        cur.append("/").append(c);
        make_dir(cur);
    }
}

std::string Crawler::get_dir_struct(std::string path, std::string domain, int lvl) {
    std::string cur = path;

    if(domain.length() < lvl)
        return cur.append("/").append(domain);

    for(int i = 0; i < lvl; i++) {
       const char* c = domain.substr(i, 1).c_str();

        switch(c[0]) {
            case '.':
                continue;
                break;
        }

        cur.append("/").append(c);
    }

    return cur;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static size_t write_data_var(void *ptr, size_t size, size_t nmemb, void *usr) {
    size_t real = size * nmemb;
    ((std::string*)usr)->append((char*)ptr, real);

    return real;
}

void Crawler::crawl(std::string domain) {
    download_robots(domain);
    parse_domains(domain);
}

bool Crawler::domain_is_valid(std::string domain) {
    for(const auto &c_tld : Tld::tlds) {
        if(ends_with(domain, c_tld)) {
            LOG << "Domain " << domain << " ends with " << c_tld << std::endl;
            return true;
        }
    }
    
    LOG << "Domain " << domain << " seems to be invalid" << std::endl;
    return false;
}

bool Crawler::domain_is_new(std::string domain) {
    std::string fname = get_dir_struct(std::string(OUTPUT_DIR), domain, DIR_STRUCT_LEVEL).append("/").append(domain);

    LOG << "Checking if " << fname << " exists...";

    struct stat buffer;   
    bool res = (stat (fname.c_str(), &buffer) == 0); 

    if(res)
        LOG_INTERNAL << "yes";
    else
        LOG_INTERNAL << "no";

    LOG_INTERNAL << std::endl;

    return !res;
}

void Crawler::new_domain(std::string domain) {
    // TODO: add logpool

    LOG << "New domain found: " << domain << std::endl;
    download_robots(domain);

    if(df_callback)
        df_callback(domain);
}

void Crawler::parse_domains(std::string domain) {
    LOG << "Searching index of " << domain << " for new domains" << std::endl;

    std::string url = domain;
    url.append(INDEX_URL);

    CURL *curl = init_curl();

    std::string buffer;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_var);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "robot-farmer V0.1");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    std::smatch domain_match;

    const std::sregex_token_iterator end;
    for (std::sregex_token_iterator i(buffer.cbegin(), buffer.cend(), domain_regex);
            i != end;
            ++i)
    {
        LOG << "Checking domain " << *i << std::endl;

        if(domain_is_new(*i)) {
            if(domain_is_valid(*i)) {
                LOG << "Calling new_domain" << std::endl;
                new_domain(*i);
            }
        }
    }
}

void Crawler::download_robots(std::string domain) {
    LOG << "Downloading robots.txt from " << domain << std::endl;

    std::string url = domain;
    url.append(ROBOTS_TXT_URL);

#if FILE_OUTPUT
    make_dir(std::string(OUTPUT_DIR));
    make_dir_struct(std::string(OUTPUT_DIR), domain, DIR_STRUCT_LEVEL);

    std::string fname = get_dir_struct(std::string(OUTPUT_DIR), domain, DIR_STRUCT_LEVEL).append("/").append(domain);
    const char* filename = fname.c_str();

    FILE *fp = fopen(filename, "wb");
    
    if(!fp)
        throw StringException(std::string("Could not open output file ").append(filename));
#endif

    CURL *curl = init_curl();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "robot-farmer V0.1");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

#if FILE_OUTPUT
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
#else
    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_var);
#endif
   
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

#if FILE_OUTPUT
    fclose(fp);
#endif



}