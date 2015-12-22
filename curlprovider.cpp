#include "curlprovider.h"

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
auto Curlpp::operator*() -> CURL*
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
auto CurlProvider::Curltmp::operator*() -> CURL*
{
	return curl;
}

CurlProvider::CurlProvider(int num) 
{
	curls.reserve(num);
	for(auto i = num; i > 0; i--){
		curls.emplace_back(Curlpp{}, 0);
	}
}

auto CurlProvider::get_raw_curl() -> CURL*
{
	CURL* c = nullptr;

	mtx.lock();
	for(auto& p : curls){
		if(p.second == 0){
			p.second = 1;
			c = p.first.get();
			break;
		}
	}
	mtx.unlock();

	return c;
}

auto CurlProvider::free_raw_curl(CURL *c) -> void
{
	mtx.lock();
	for(auto& p : curls){
		if(p.first.get() == c){
			p.second = 0;
			break;
		}
	}
	mtx.unlock();
	// TODO: throw if not found ?
}

auto CurlProvider::get_curl_temporary() -> CurlProvider::Curltmp
{
	return CurlProvider::Curltmp(*this);
}
