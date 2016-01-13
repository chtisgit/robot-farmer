#ifndef CURLPROVIDER_H
#define CURLPROVIDER_H

#include <mutex>
#include <vector>
#include <memory>
#include <curl/curl.h>

class Curlpp{
	CURL *curl;
public:
	explicit Curlpp();
	explicit Curlpp(CURL *c);
	~Curlpp();
	auto get() const -> CURL*;
	inline operator bool() const
	{
		return curl != nullptr;
	}
};

class CurlProvider{
	std::vector<std::shared_ptr<Curlpp>> curls;
	std::mutex mtx;
public:
	CurlProvider(int num);
	auto get_curl_temporary() -> std::shared_ptr<Curlpp>;
};

#endif

