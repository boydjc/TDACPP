#pragma once

#include <curl/curl.h>

class TDA {

	public:
		TDA();

	private:
		CURL *curl;
		CURLcode res;

		// This is the variable that curl will use to write the request 
		// html results to using CURLOPT_WRITEDATA	
		std::string resResults;

		void testLibCurl();
		static size_t saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s);
		
};
