#include "workset.h"

CrawlWorkset::CrawlWorkset(std::string url) : url(url) {}
CrawlWorkset::CrawlWorkset(const char *url) : url(url) {}

void CrawlWorkset::operator()(ThreadPool<CrawlWorkset,CurlProvider>& pool, CurlProvider& curlpr)
{
	auto curl = curlpr.get_curl_temporary();

	Crawler crawler(curl->get());
	crawler.setCallback([&pool](const std::string& _url){
		pool.load( CrawlWorkset(_url) );
	});

	crawler.crawl(url);
}
