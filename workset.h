#ifndef WORKSET_H
#define WORKSET_H


class Workset{
	CURL *curl;
	std::string url;

public:

	Workset(CURL *curl, std::string url) : curl(curl), url(url)
	{
	}

	bool operator()()
	{
		return true;
	}
};



#endif
