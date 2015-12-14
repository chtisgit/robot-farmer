#include <list>
#include <iostream>
#include <string>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <sys/stat.h>

#include "crawler.h"
#include "strexception.h"

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

    for(int i = 0; i < lvl; i++) {
        const char* c = domain.substr(i, 1).c_str();

        cur.append("/").append(c);
        make_dir(cur);
    }
}

std::string Crawler::get_dir_struct(std::string path, std::string domain, int lvl) {
    std::string cur = path;

    for(int i = 0; i < lvl; i++) {
        const char* c = domain.substr(i, 1).c_str();

        cur.append("/").append(c);
    }

    return cur;
}

#if FILE_OUTPUT
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
#else
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *usr) {
    size_t real = size * nmemb;
    ((std::string*)usr)->append((char*)ptr, real);

    return real;
}
#endif

void Crawler::crawl(std::string domain) {
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

#if FILE_OUTPUT
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
#else
    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
#endif
   
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

#if FILE_OUTPUT
    fclose(fp);
#endif

}
