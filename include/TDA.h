#pragma once

#include <curl/curl.h>

class TDA {

	public:
		TDA();
		~TDA();

	private:
		CURL *curl;
		CURLcode res;

		// refresh is the authorization token give by 
		// TD Ameritrade. It is good for 90 days and is used to 
		std::string refreshToken;

		// accessToken is retrieved from TD Ameritrade's API using the refresh token.
		// accessToken is good for 30 minutes
		// for 30 minutes
		char* accessToken; 

		// the clientId is the consumer key of your registered app in the 
		// TD Ameritrade developer api site
		char* clientId;

		// This is the variable that curl will use to write the request 
		// html results to using CURLOPT_WRITEDATA	
		std::string resResults;

		std::string reqUrl;

		void readEnvVars();
		void testLibCurl();
		static size_t saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s);
		void createAccessToken();
		
};
