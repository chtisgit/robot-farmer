#ifndef WORKSET_H
#define WORKSET_H

#include <string>

#include "crawler.h"

class Workset{
	// TODO
	Crawler crawler;
	std::string domain;

public:
	Workset(std::string domain);
	Workset(const char *domain);

	inline auto new_data(std::string new_domain) -> void
	{
		domain = new_domain;
	}

	auto operator()() -> void
	{
		crawler.crawl(domain);
	}
};



#endif
