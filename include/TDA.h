#pragma once

#include <curl/curl.h>

#include "../include/json.hpp"

class TDA {

	public:
		TDA();
		~TDA();

	private:
		CURL *curl;
		CURLcode res;

		// variable used to store our config values
		// like refresh token and client id
		nlohmann::json configJSON;

		// This is the variable that curl will use to write the request 
		// html results to using CURLOPT_WRITEDATA	
		std::string resResults;

		std::string reqUrl;

		std::string postData;

		void readConfig();
		void saveConfig();
		void sendReq();
		static size_t saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s);
		void createAccessToken(bool refresh=false);
		
};
