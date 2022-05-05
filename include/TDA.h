#pragma once

#include <curl/curl.h>

#include "../include/json.hpp"

struct PriceData {
	std::string date;
	float open;
	float high;
	float low;
	float close;
	unsigned long volume;
};

class TDA {

	public:
		TDA();
		~TDA();

		/* BEGIN set & get functions */

		void getHistPrice(std::string ticker, std::string periodType="day",
					  std::string period="", std::string freqType="",
					  std::string freq="", unsigned int endDate=0,
					  unsigned int startDate=0, bool extHourData=true);

		/* END set & get functions */

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
