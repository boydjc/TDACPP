#include <iostream>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <time.h>
#include <stdlib.h>

#include "../include/TDA.h"

TDA::TDA() {	
	std::cout << "TDA!" << std::endl;

	// set up libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);

	readConfig();	

}

TDA::~TDA() {
	// clean up libcurl
	curl_global_cleanup();
}

// Retrieves values like refresh token and client id through config.json
void TDA::readConfig() {

	std::ifstream configFile("../tda-config.json");
	configFile >> configJSON;
	configFile.close();

	// get the refresh token from environ variable
	if(!(configJSON.contains("refresh_token"))){
		std::cout << "ERROR! Could not find refresh token." << std::endl;
		std::cout << "Please make sure your refresh token is in your config.json file!" << std::endl;
	}
	/* if we have a refresh token but no creation date, then we can assume that this is the baseline 
	/ refresh token that the user created when first using this class
	/ although is fine for the first 90 days, we try to do the user a favor and create a new refresh 
	/ token now so that the class knows when it was created. We can't accurately assume when 
	/ the user created the baseline refresh token. This is purely to keep the user from having to manually
	/ authenticate with TD Ameritrade again after 90 days*/
	else if(configJSON.contains("refresh_token") && !(configJSON.contains("refresh_create_date"))) {
		std::cout << "Found refresh token but no creation date. Creating new refresh token with creation date" << std::endl;
		createAccessToken(true);
		saveConfig();
		readConfig();
	}else if(configJSON.contains("refresh_token") && configJSON.contains("refresh_create_date")) {
		// check for expired refresh token

		int64_t refreshCreation = configJSON["refresh_create_date"].get<int64_t>();

		int64_t timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		// 60 days
		int64_t refreshExpirationDate = refreshCreation + 5184000000;

		if(timeNow >= refreshExpirationDate) {
			std::cout << "Refresh Token will expire in 30 days or less. Performing early renewal" << std::endl;
			createAccessToken(true);
			saveConfig();
			readConfig();
		}

		// check to see if there is an access token in the config file
		// if there is not then create one, if there is then check to make sure that it is still valid
		if(configJSON.contains("access_token")) {
			// check the date that it was created and see if it has expired
			// access_tokens are only good for 30 minutes
			
			if(checkAccessExpire()) {
				createAccessToken();
				saveConfig();
				readConfig();
			}

			// get the client id from environ variable
			if(!(configJSON.contains("client_id"))) {
				std::cout << "ERROR! Could not find client id." << std::endl;
				std::cout << "Please make sure your client id is in your config.json file!" << std::endl;
			}

		}else if(!(configJSON.contains("access_token"))) {
			std::cout << "Did not find access token. Creating new one" << std::endl;
			createAccessToken();
			saveConfig();
			readConfig();
		}
	}
}

bool TDA::checkAccessExpire() {

	int64_t accessCreation = configJSON["access_create_date"].get<int64_t>();
	int64_t timeNow = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			
	// 30 minutes
	int64_t accessExpirationDate = accessCreation + 1800000;
			
	if(timeNow > accessExpirationDate) {
		return true;
	}

	return false;

}

void TDA::sendReq(){
	std::cout << "Initializing curl" << std::endl;

	curl = curl_easy_init();

	if(curl) {
		std::cout << "Setting url opt" << std::endl;
		std::cout << reqUrl << std::endl;
		curl_easy_setopt(curl, CURLOPT_URL, reqUrl.c_str());	
		
		/* Setting post fields if we are doing a post request */
		if(postData != "") {
			/* Setting Headers */
			struct curl_slist *headers=NULL;

			headers = curl_slist_append(headers, "Accept-Encoding: gzip");
			headers = curl_slist_append(headers, "Accept-Language: en-US");
			headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			std::cout << "Setting post fields" << std::endl;
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		}else {
			// set bearer token
			std::cout << "SETTING BEARER" << std::endl;
			struct curl_slist *headers=NULL;
			headers = curl_slist_append(headers, ("Authorization: Bearer " + configJSON["access_token"].get<std::string>()).c_str());
			curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, configJSON["access_token"].get<std::string>().c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}

		std::cout << "Setting callback function" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TDA::saveLibCurlRes);
		std::cout << "Setting callback variable" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resResults);

		// for debugging
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		
		/* Perform the request, res will get the return code */
		std::cout << "Performing request" << std::endl;
		res = curl_easy_perform(curl);

		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* always clean up */
		std::cout << "Cleaning up" << std::endl;
		curl_easy_cleanup(curl);
	}
}

/*  Call back function for libcurl to save our requested data
	instead of it getting printed to stdout */
size_t TDA::saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s) {

	size_t dataLen = size*nmemb;

	try {
		s->append((char*)buffer, dataLen);
	} catch(std::bad_alloc &e) {
		// handle memory problem
		return 0;
	}

	return dataLen;
}

void TDA::saveConfig() {
	std::ofstream configFile("../tda-config.json", std::ios::trunc);
	configFile << configJSON.dump();
	configFile.close();
}

// refresh=true will also get a new refresh_token
// this only needs to be done every 90 days
void TDA::createAccessToken(bool refresh) {

	reqUrl = "https://api.tdameritrade.com/v1/oauth2/token";

	// using curl here to encode the refresh token
	curl = curl_easy_init();

	std::string refreshEncode = curl_easy_escape(curl, 
												 configJSON["refresh_token"].get<std::string>().c_str(), 
												 configJSON["refresh_token"].get<std::string>().length());
	
	if(refresh){
		postData = "grant_type=refresh_token&refresh_token=" +
											   refreshEncode +
											   "&access_type=offline&code=&client_id=" + 
											   configJSON["client_id"].get<std::string>() + "&redirect_url=";
	}else{
		postData = "grant_type=refresh_token&refresh_token=" + 
											   refreshEncode + 
											   "&access_type=&code=&client_id=" + 
											   configJSON["client_id"].get<std::string>() + "&redirect_uri=";
	}

	curl_easy_cleanup(curl);

	sendReq();
	
	// clear the post data
	postData = "";

	// parse the request results into JSON object
	nlohmann::json resJSON = nlohmann::json::parse(resResults);

	std::cout << resJSON.dump() << std::endl;

	if(resJSON.contains("access_token")) {
		// store the access token in configJSON
		configJSON["access_token"] = resJSON["access_token"].get<std::string>();
		configJSON["access_create_date"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); 
	} 

	if(resJSON.contains("refresh_token")) {
		std::cout << "WARNING!!!! CREATING NEW REFRESH TOKEN" << std::endl;
		configJSON["refresh_token"] = resJSON["refresh_token"].get<std::string>();
		configJSON["refresh_create_date"] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	if(resJSON.contains("error")) {
		std::cout << "ERROR in fetching new Access or Refresh Token" << std::endl;
		std::cout << resJSON["error"] << std::endl;
	}	
}


std::vector<Candle> TDA::getHistPrice(std::string ticker, std::string periodType, 
					   std::string period, std::string freqType,
					   std::string freq, unsigned int endDate,
					   unsigned int startDate, bool extHourData) {

	histPriceData.clear();

	if(!(checkAccessExpire())) {

		reqUrl = "https://api.tdameritrade.com/v1/marketdata/" + ticker + "/pricehistory?";

		// append the values for each parameter if they are there
		reqUrl.append("periodType=" + periodType);

		if(period != "") {
			std::cout << "Period Present" << std::endl;
			reqUrl.append("&period=" + period);
		}

		if(freqType != "") {
			std::cout << "Frequence Type Present" << std::endl;
			reqUrl.append("&frequencyType=" + freqType);
		}

		if(freq != "") {
			std::cout << "Frequence Present" << std::endl;
			reqUrl.append("&frequency=" + freq);
		}

		if(endDate != 0) {
			std::cout << "End Date Present" << std::endl;
			reqUrl.append("&endDate=" + endDate);
		}

		if(startDate != 0) {
			std::cout << "Start date present" << std::endl;
			reqUrl.append("&startDate=" + startDate);
		}

		if(extHourData) {
			reqUrl.append("&needExtendedHoursData=true");
		}else {
			reqUrl.append("&needExtendedHoursData=false");
		}

		sendReq();
	
		nlohmann::json resJSON = nlohmann::json::parse(resResults);

		nlohmann::json candlesJSON = resJSON["candles"];

		for (auto it = candlesJSON.begin(); it != candlesJSON.end(); ++it) {

			Candle newCandle;
	
			time_t datetime = (*it)["datetime"].get<int64_t>() / 1000;
			struct tm *tm = localtime(&datetime);
			char humanDate[20];

			strftime(humanDate, 20, "%Y-%m-%d", tm);

			newCandle.date = humanDate;

			float open = (*it)["open"].get<float>();

			newCandle.open = open;

			float high = (*it)["high"].get<float>();

			newCandle.high = high;

			float low = (*it)["low"].get<float>();

			newCandle.low = low;

			float close = (*it)["close"].get<float>();
			
			newCandle.close = close;

			unsigned long volume = (*it)["volume"].get<int>();

			newCandle.volume = volume;
	
			histPriceData.push_back(newCandle);

		}
	}

	return histPriceData;
}


void TDA::getQuote(std::string symbol) {

	if(!(checkAccessExpire())) {
		
		reqUrl = "https://api.tdameritrade.com/v1/marketdata/" + symbol + "/quotes";

		sendReq();

		nlohmann::json resJSON = nlohmann::json::parse(resResults);

		nlohmann::json symbolJSON = resJSON[symbol];

		std::cout << symbolJSON << std::endl;
		
		Quote newQuote;

		newQuote.symbol = symbolJSON.contains("symbol") ? symbolJSON["symbol"].get<std::string>() : "";
		newQuote.assetMainType = symbolJSON.contains("assetMainType") ? symbolJSON["assetMainType"].get<std::string>() : "";
		newQuote.assetSubType = symbolJSON.contains("assetSubType") ? symbolJSON["assetSubType"].get<std::string>() : "";
		newQuote.assetType = symbolJSON.contains("assetType") ? symbolJSON["assetType"].get<std::string>() : "";
		newQuote.exchange = symbolJSON.contains("exchange") ? symbolJSON["exchange"].get<std::string>() : "";
		newQuote.exchangeName = symbolJSON.contains("exchangeName") ? symbolJSON["exchangeName"].get<std::string>() : "";
		newQuote.divDate = symbolJSON.contains("divDate") ? symbolJSON["divDate"].get<std::string>() : "";
		newQuote.securityStatus = symbolJSON.contains("securityStatus") ? symbolJSON["securityStatus"].get<std::string>() : "";
		newQuote.bidId = symbolJSON.contains("bidId") ? symbolJSON["bidId"].get<std::string>() : "";
		newQuote.askId = symbolJSON.contains("askId") ? symbolJSON["askId"].get<std::string>() : "";
		newQuote.description = symbolJSON.contains("description") ? symbolJSON["description"].get<std::string>() : "";
		newQuote.lastId = symbolJSON.contains("lastId") ? symbolJSON["lastId"].get<std::string>() : "";
		newQuote.product = symbolJSON.contains("product") ? symbolJSON["product"].get<std::string>() : "";
		newQuote.futurePriceFormat = symbolJSON.contains("futurePriceFormat") ? symbolJSON["futurePriceFormat"].get<std::string>() : "";
		newQuote.futureTradingHours = symbolJSON.contains("futureTradingHours") ? symbolJSON["futureTradingHours"].get<std::string>() : "";
		newQuote.futureActiveSymbol = symbolJSON.contains("futureActiveSymbol") ? symbolJSON["futureActiveSymbol"].get<std::string>() : "";
		newQuote.futureExpirationDate = symbolJSON.contains("futureExpirationDate") ? symbolJSON["futureExpirationDate"].get<std::string>() : "";
		newQuote.contractType = symbolJSON.contains("contractType") ? symbolJSON["contractType"].get<std::string>() : "";
		newQuote.underlying = symbolJSON.contains("underlying") ? symbolJSON["underlying"].get<std::string>() : "";
		newQuote.expriationType = symbolJSON.contains("expirationType") ? symbolJSON["expirationType"].get<std::string>() : "";
		newQuote.exerciseType = symbolJSON.contains("exerciseType") ? symbolJSON["exerciseType"].get<std::string>() : "";
		newQuote.delieverables = symbolJSON.contains("deliverables") ? symbolJSON["deliverables"].get<std::string>() : "";
		newQuote.uvExpirationType = symbolJSON.contains("uvExpirationType") ? symbolJSON["uvExpirationType"].get<std::string>() : "";
		newQuote.settlementType = symbolJSON.contains("settlementType") ? symbolJSON["settlementType"].get<std::string>() : "";
		newQuote.tradingHours = symbolJSON.contains("tradingHours") ? symbolJSON["tradingHours"].get<std::string>() : "";
		newQuote.marketMaker = symbolJSON.contains("marketMaker") ? symbolJSON["marketMaker"].get<std::string>() : "";
		newQuote.cusip = symbolJSON.contains("cusip") ? symbolJSON["cusip"].get<std::string>() : "";
		newQuote.lastPrice = symbolJSON.contains("lastPrice") ? symbolJSON["lastPrice"].get<float>() : 0.00;
		newQuote.openPrice = symbolJSON.contains("openPrice") ? symbolJSON["openPrice"].get<float>() : 0.00;
		newQuote.highPrice = symbolJSON.contains("highPrice") ? symbolJSON["highPrice"].get<float>() : 0.00;
		newQuote.lowPrice = symbolJSON.contains("lowPrice") ? symbolJSON["lowPrice"].get<float>() : 0.00;
		newQuote.closePrice = symbolJSON.contains("closePrice") ? symbolJSON["closePrice"].get<float>() : 0.00;
		newQuote.bidPrice = symbolJSON.contains("bidPrice") ? symbolJSON["bidPrice"].get<float>() : 0.00;
		newQuote.netChange = symbolJSON.contins("netChange") ? symbolJSON["netChange"].get<float>() : 0.00;
		newQuote.fiftyTwoWeekHigh = symbolJSON.contains("52WkHigh") ? symbolJSON["52WkHigh"].get<float>() : 0.00;
		newQuote.fiftyTwoWeekLow = symbolJSON.contains("52WkLow") ? symbolJSON["52WkHigh"].get<float>() : 0.00;
		newQuote.peRatio = symbolJSON.contains("peRatio") ? symbolJSON["peRatio"].get<float>() : 0.00;
		newQuote.divAmount = symbolJSON.contains("divAmount") ? symbolJSON["divAmount"].get<float>() : 0.00;
		newQuote.divYield = symbolJSON.contains("divYield") ? symbolJSON["divYield"].get<float>() : 0.00;
		newQuote.futurePercentChange = symbolJSON.contains("futurePercentChange") ? symbolJSON["futurePercentChange"].get<float>() : 0.00;
		newQuote.moneyIntrinsicValue = symbolJSON.contains("moneyIntrinsicValue") ? symbolJSON["moneyIntrinsicValue"].get<float>() : 0.00;
		newQuote.mark = symbolJSON.contains("mark") ? symbolJSON["mark"].get<float>() : 0.00;
		newQuote.tick = symbolJSON.contains("tick") ? symbolJSON["tick"].get<float>() : 0.00;
		newQuote.fiftyWkHigh = symbolJSON.contains("fiftyWkHigh") ? symbolJSON["fiftyWkHigh"].get<float>() : 0.00;
		newQuote.fiftyWkLow = symbolJSON.contains("fiftyWkLow") ? symbolJSON["fiftyWkLow"].get<float>() : 0.00;
		newQuote.askPrice = symbolJSON.contains("askPrice") ? symbolJSON["askPrice"].get<float>() : 0.00;
		newQuote.volatility = symbolJSON.contains("volatility") ? symbolJSON["volatlity"].get<float>() : 0.00;
		newQuote.futureSettlementPrice = symbolJSON.contains("futureSettlementPrice") ? symbolJSON["futureSettlementPrice"].get<float>() : 0.00;
		newQuote.strikePrice = symbolJSON.contains("strikePrice") ? symbolJSON["strikePrice"].get<float>() : 0.00;
		newQuote.timeValue = symbolJSON.contains("timeValue") ? symbolJSON["timeValue"].get<float>() : 0.00;
		newQuote.delta = symbolJSON.contains("delta") ? symbolJSON["delta"].get<float>() : 0.00;
		newQuote.gamma = symbolJSON.contains("gamma") ? symbolJSON["gamma"].get<float>() : 0.00;
		newQuote.theta = symbolJSON.contains("theta") ? symbolJSON["theta"].get<float>() : 0.00;
		newQuote.vega = symbolJSON.contains("vega") ? symbolJSON["vega"].get<float>() : 0.00;
		newQuote.rho = symbolJSON.contains("rho") ? symbolJSON["rho"].get<float>() : 0.00;
		newQuote.theoreticalOptionValue = symbolJSON.contains("theoreticalOptionValue") ? symbolJSON["theoreticalOptionValue"].get<float>() : 0.00;
		newQuote.underlyingPrice = symbolJSON.contains("underlyingPrice") ? symbolJSON["underlyingPrice"].get<float>() : 0.00;
		newQuote.percentChange = symbolJSON.contains("percentChange") ? symbolJSON["percentChange"].get<float>() : 0.00;
		newQuote.regularMarketLastPrice = symbolJSON.contains("regularMarketLastPrice") ? symbolJSON["regularMarketLastPrice"].get<float>(): 0.00;
		newQuote.regularMarketNetChange = symbolJSON.contains("regularMarketNetChange") ? symbolJSON["regularMarketNetChange"].get<float>() : 0.00;
		newQuote.digits = symbolJSON.contains("digits") ? symbolJSON["digits"].get<int>() : 0;
		newQuote.nAV = symbolJSON.contains("nAV") ? symbolJSON["nAV"].get<int>() : 0;
		newQuote.openInterest = symbolJSON.contains("openInterest") ? symbolJSON["openInterest"].get<int>() : 0;
		newQuote.futureMultiplier = symbolJSON.contains("futureMultiplier") ? symbolJSON["futureMultiplier"].get<int>() : 0;
		newQuote.tickAmount = symbolJSON.contains("tickAmount") ? symbolJSON["tickAmount"].get<int>() : 0;

	}
}
