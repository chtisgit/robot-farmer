#ifndef CURLPROVIDER_H
#define CURLPROVIDER_H

#include <mutex>
#include <vector>
#include <curl/curl.h>

class Curlpp{
	CURL *curl;
public:
	Curlpp();
	~Curlpp();
	auto get() -> CURL*;
	auto operator*() -> CURL*;
	inline operator bool() const
	{
		return curl != nullptr;
	}
};

class CurlProvider{
	std::vector<std::pair<Curlpp,int>> curls;
	std::mutex mtx;

public:
	CurlProvider(int num);

	auto getCURL() -> CURL*;
	auto freeCURL(CURL* c) -> void;
};

#endif

