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
	
	bool operator()()
	{
		crawler.crawl(domain);
		return true;
	}
};



#endif
