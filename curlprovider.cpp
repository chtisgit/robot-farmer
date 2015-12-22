#include "curlprovider.h"

Curlpp::Curlpp()
{
	curl = curl_easy_init();
}

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

CurlProvider::CurlProvider(int num) 
{
	curls.reserve(num);
	for(auto i = num; i > 0; i--){
		curls.emplace_back(Curlpp{}, 0);
	}
}

auto CurlProvider::getCURL() -> CURL*
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

auto CurlProvider::freeCURL(CURL *c) -> void
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
