#ifndef CURLPROVIDER_H
#define CURLPROVIDER_H

#include <mutex>
#include <vector>
#include <curl/curl.h>

class Curlpp{
	CURL *curl;
public:
	Curlpp();
	Curlpp(CURL *c);
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

	auto get_raw_curl() -> CURL*;
	auto free_raw_curl(CURL* c) -> void;
public:
	class Curltmp{
		CURL *curl;
		CurlProvider& parent;
	public:
		Curltmp(CurlProvider& p);
		~Curltmp();
		auto get() -> CURL*;
		auto operator*() -> CURL*;
		inline operator bool()
		{
			return curl != nullptr;
		}
	};

	CurlProvider(int num);
	
	auto get_curl_temporary() -> Curltmp;
};

#endif

