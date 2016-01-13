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

auto Curlpp::get() const -> CURL*
{
	return curl;
}

CurlProvider::CurlProvider(int num) 
	: curls(num)
{
	for(auto& c : curls){
		c = std::make_shared<Curlpp>();
	}
}

auto CurlProvider::get_curl_temporary() -> std::shared_ptr<Curlpp>
{
	std::lock_guard<decltype(mtx)> lock(mtx);

	for(auto& c : curls){
		if(c && c.unique())
			return c;
	}
	return nullptr;
}
