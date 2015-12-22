#ifndef WORKSET_H
#define WORKSET_H

#include "crawler.h"
#include "threadpool.h"

class CrawlWorkset{
	Crawler crawler;
	std::string url;

public:
	explicit CrawlWorkset(std::string url);
	explicit CrawlWorkset(const char *url);
	void operator()(ThreadPool<CrawlWorkset,CurlProvider>& pool, CurlProvider& curlpr);
};



#endif
