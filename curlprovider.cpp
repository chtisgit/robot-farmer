#include "curlprovider.h"
#include "log.h"

Curlpp::Curlpp()
{
	curl = curl_easy_init();
}

Curlpp::Curlpp(CURL *c) : curl(c) {}

Curlpp::~Curlpp()
{
	if(curl != nullptr)
		curl_easy_cleanup(curl);
}

auto Curlpp::get() -> CURL*
{
	return curl;
}

CurlProvider::Curltmp::Curltmp(CurlProvider& p) : parent(p)
{
	curl = parent.get_raw_curl();
}

CurlProvider::Curltmp::~Curltmp()
{
	if(curl != nullptr)
		parent.free_raw_curl(curl);
}

auto CurlProvider::Curltmp::get() -> CURL*
{
	return curl;
}

CurlProvider::CurlProvider(int num) 
	: curls(num)
{
	for(auto& p : curls){
		p.second = 0;
	}
}

auto CurlProvider::get_raw_curl() -> CURL*
{
	CURL* c = nullptr;
	std::lock_guard<decltype(mtx)> mtx_guard(mtx);

	for(auto& p : curls){
		if(p.second == 0){
			p.second = 1;
			c = p.first.get();
			break;
		}
	}

	return c;
}

auto CurlProvider::free_raw_curl(CURL *c) -> void
{
	std::lock_guard<decltype(mtx)> mtx_guard(mtx);
	for(auto& p : curls){
		if(p.first.get() == c){
			p.second = 0;
			break;
		}
	}
	// TODO: throw if not found ?
}

auto CurlProvider::get_curl_temporary() -> CurlProvider::Curltmp
{
	return CurlProvider::Curltmp(*this);
}
