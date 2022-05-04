#pragma once

#include <curl/curl.h>

#include "../include/json.hpp"

class TDA {

	public:
		TDA();
		~TDA();

	void getHistPrice(std::string ticker, std::string periodType="day",
					  std::string period="", std::string freqType="",
					  std::string freq="", unsigned int endDate=0,
					  unsigned int startDate=0, bool extHourData=true);


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

		// the unix time stamp for when the access token was created
		int64_t accessTokenCreationDate;
		int64_t refreshTokenCreationDate;

		void readConfig();
		void saveConfig();
		void sendReq();
		static size_t saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s);
		void createAccessToken(bool refresh=false);
		bool checkAccessExpire();
		
};
