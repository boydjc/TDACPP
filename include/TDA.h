#pragma once

#include <curl/curl.h>

class TDA {

	public:
		TDA();

	private:
		CURL *curl;
		CURLcode res;

		void testLibCurl();
		
};
