#include <list>
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

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
std::regex Crawler::safe_chars_regex("[a-zA-Z0-9]");

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void Crawler::setCallback(DomainFoundFunc cb) {
    df_callback = cb;
}

void Crawler::make_dir(std::string name) {
    const int err = mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    
    if(err == -1 && errno != EEXIST) {
        throw StringException(std::string("Could not create string ").append(name));
    }
}

void Crawler::make_dir_struct(std::string path, std::string domain, unsigned int lvl) {
    std::string cur = path;

    if(domain.length() < lvl) {
        make_dir(cur.append("/").append(domain));
        return;
    }

    for(unsigned int i = 0; i < lvl; i++) {
        const char* c = domain.substr(i, 1).c_str();

        DEBUG_LOG("cur is: " << cur << std::endl);
        if(!std::regex_match(c, safe_chars_regex)) {
            continue;
        }

        cur.append("/").append(c);
        make_dir(cur);
    }
}

std::string Crawler::get_dir_struct(std::string path, std::string domain, unsigned int lvl) {
    std::string cur = path;

    if(domain.length() < lvl)
        return cur.append("/").append(domain);

    for(unsigned int i = 0; i < lvl; i++) {
       const char* c = domain.substr(i, 1).c_str();

        if(!std::regex_match(c, safe_chars_regex)) {
            continue;
        }

        cur.append("/").append(c);
        DEBUG_LOG("cur2 is: " << cur << std::endl);
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
    // convert domain name to lowercase
    std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);

    download_robots(domain);
    parse_domains(domain);
}

bool Crawler::domain_is_valid(std::string domain) {
    for(const auto &c_tld : Tld::tlds) {
        if(ends_with(domain, c_tld)) {
            DEBUG_LOG("Domain " << domain << " ends with " << c_tld << std::endl);
            return true;
        }
    }
    
    DEBUG_LOG("Domain " << domain << " seems to be invalid" << std::endl);
    return false;
}

bool Crawler::domain_is_new(std::string domain) {
    std::string fname = get_dir_struct(std::string(OUTPUT_DIR), domain, DIR_STRUCT_LEVEL).append("/").append(domain);

    DEBUG_LOG("Checking if " << fname << " exists...");

    struct stat buffer;   
    bool res = (stat (fname.c_str(), &buffer) == 0); 

    if(res)
        DEBUG_LOG_NONL("yes");
    else
        DEBUG_LOG_NONL("no");

    DEBUG_LOG_NONL(std::endl);

    return !res;
}

void Crawler::new_domain(std::string domain) {
    DEBUG_LOG("New domain found: " << domain << std::endl);
    crawl(domain);

    if(df_callback)
        df_callback(domain);
}

void Crawler::parse_domains(std::string domain) {
    DEBUG_LOG("Searching index of " << domain << " for new domains" << std::endl);

    std::string url = domain;
    url.append(INDEX_URL);

    std::string buffer;

    curl_easy_setopt(*curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(*curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(*curl, CURLOPT_WRITEFUNCTION, write_data_var);
    curl_easy_setopt(*curl, CURLOPT_TIMEOUT, 1L);
    curl_easy_setopt(*curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(*curl, CURLOPT_USERAGENT, "robot-farmer V0.1");

    curl_easy_perform(*curl);
    
    std::smatch domain_match;

    const std::sregex_token_iterator end;
    for (std::sregex_token_iterator i(buffer.cbegin(), buffer.cend(), domain_regex);
            i != end;
            ++i)
    {
        std::string domain = std::string(*i);

        // convert domain name to lowercase
        std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);

        DEBUG_LOG("Checking domain " << domain << std::endl);

        if(domain_is_new(domain)) {
            if(domain_is_valid(domain)) {
                DEBUG_LOG("Calling new_domain" << std::endl);
                new_domain(*i);
            }
        }
    }
}

void Crawler::download_robots(std::string domain) {
    LOG << domain << std::endl;

    DEBUG_LOG("Downloading robots.txt from " << domain << std::endl);

    std::string url = domain;
    url.append(ROBOTS_TXT_URL);

#if FILE_OUTPUT
    make_dir(std::string(OUTPUT_DIR));
    make_dir_struct(std::string(OUTPUT_DIR), domain, DIR_STRUCT_LEVEL);

    std::string fname = get_dir_struct(std::string(OUTPUT_DIR), domain, DIR_STRUCT_LEVEL).append("/").append(domain);
    const char* filename = fname.c_str();
    
    DEBUG_LOG("Downloading to " << filename << std::endl);

    FILE *fp = fopen(filename, "wb");
    
    if(!fp) {
        DEBUG_LOG("Could not open output file " << filename);
        return;
    }

#endif

    curl_easy_setopt(*curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(*curl, CURLOPT_TIMEOUT, 1L);
    curl_easy_setopt(*curl, CURLOPT_USERAGENT, "robot-farmer V0.1");
    curl_easy_setopt(*curl, CURLOPT_FOLLOWLOCATION, 1);

#if FILE_OUTPUT
    curl_easy_setopt(*curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(*curl, CURLOPT_WRITEDATA, fp);
#else
    std::string buffer;
    curl_easy_setopt(*curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(*curl, CURLOPT_WRITEFUNCTION, write_data_var);
#endif
   
    curl_easy_perform(*curl);

#if FILE_OUTPUT
    fclose(fp);
#endif



}
