#include "workset.h"

#include <list>

CrawlWorkset::CrawlWorkset(std::string url) : url(url) {}
CrawlWorkset::CrawlWorkset(const char *url) : url(url) {}

void CrawlWorkset::operator()(ThreadPool<CrawlWorkset,CurlProvider>& pool, CurlProvider& curlpr)
{
	auto curl = curlpr.get_curl_temporary();
	auto newsets = std::list<CrawlWorkset>();

	Crawler crawler(curl->get());
	crawler.setCallback([&newsets](const std::string& _url){
		newsets.emplace_back(_url);
	});

	crawler.crawl(url);

	pool.load(newsets);
}
