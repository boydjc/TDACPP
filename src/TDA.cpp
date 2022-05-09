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


Quote TDA::getQuote(std::string symbol) {

	Quote newQuote;

	if(!(checkAccessExpire())) {
		
		reqUrl = "https://api.tdameritrade.com/v1/marketdata/" + symbol + "/quotes";

		sendReq();

		nlohmann::json resJSON = nlohmann::json::parse(resResults);

		nlohmann::json symbolJSON = resJSON[symbol];

		//std::cout << symbolJSON << std::endl;	

		//std::cout << "Setting symbol" << std::endl;
		newQuote.symbol = symbolJSON.contains("symbol") ? symbolJSON["symbol"].get<std::string>() : "";
		//std::cout << "Setting assetMainType" << std::endl;
		newQuote.assetMainType = symbolJSON.contains("assetMainType") ? symbolJSON["assetMainType"].get<std::string>() : "";
		//std::cout << "Setting assetSubType" << std::endl;
		newQuote.assetSubType = symbolJSON.contains("assetSubType") ? symbolJSON["assetSubType"].get<std::string>() : "";
		//std::cout << "Setting assetType" << std::endl;
		newQuote.assetType = symbolJSON.contains("assetType") ? symbolJSON["assetType"].get<std::string>() : "";
		//std::cout << "Setting exchange" << std::endl;
		newQuote.exchange = symbolJSON.contains("exchange") ? symbolJSON["exchange"].get<std::string>() : "";
		//std::cout << "Setting exchangeName" << std::endl;
		newQuote.exchangeName = symbolJSON.contains("exchangeName") ? symbolJSON["exchangeName"].get<std::string>() : "";
		//std::cout << "Setting divDate" << std::endl;
		newQuote.divDate = symbolJSON.contains("divDate") ? symbolJSON["divDate"].get<std::string>() : "";
		//std::cout << "Setting securityStatus" << std::endl;
		newQuote.securityStatus = symbolJSON.contains("securityStatus") ? symbolJSON["securityStatus"].get<std::string>() : "";
		//std::cout << "Setting bidId" << std::endl;
		newQuote.bidId = symbolJSON.contains("bidId") ? symbolJSON["bidId"].get<std::string>() : "";
		//std::cout << "Setting askId" << std::endl;
		newQuote.askId = symbolJSON.contains("askId") ? symbolJSON["askId"].get<std::string>() : "";
		//std::cout << "Setting description" << std::endl;
		newQuote.description = symbolJSON.contains("description") ? symbolJSON["description"].get<std::string>() : "";
		//std::cout << "SettinglastId" << std::endl;
		newQuote.lastId = symbolJSON.contains("lastId") ? symbolJSON["lastId"].get<std::string>() : "";
		//std::cout << "Setting product" << std::endl;
		newQuote.product = symbolJSON.contains("product") ? symbolJSON["product"].get<std::string>() : "";
		//std::cout << "Setting futurePriceFormat" << std::endl;
		newQuote.futurePriceFormat = symbolJSON.contains("futurePriceFormat") ? symbolJSON["futurePriceFormat"].get<std::string>() : "";
		//std::cout << "Setting futureTradingHours" << std::endl;
		newQuote.futureTradingHours = symbolJSON.contains("futureTradingHours") ? symbolJSON["futureTradingHours"].get<std::string>() : "";
		//std::cout << "Setting futureActiveSymbol" << std::endl;
		newQuote.futureActiveSymbol = symbolJSON.contains("futureActiveSymbol") ? symbolJSON["futureActiveSymbol"].get<std::string>() : "";
		//std::cout << "Setting futureExpirationDate" << std::endl;
		newQuote.futureExpirationDate = symbolJSON.contains("futureExpirationDate") ? symbolJSON["futureExpirationDate"].get<std::string>() : "";
		//std::cout << "Setting contractType" << std::endl;
		newQuote.contractType = symbolJSON.contains("contractType") ? symbolJSON["contractType"].get<std::string>() : "";
		//std::cout << "Setting underlying" << std::endl;
		newQuote.underlying = symbolJSON.contains("underlying") ? symbolJSON["underlying"].get<std::string>() : "";
		//std::cout << "Setting expirationType" << std::endl;
		newQuote.expirationType = symbolJSON.contains("expirationType") ? symbolJSON["expirationType"].get<std::string>() : "";
		//std::cout << "Setting exerciseType" << std::endl;
		newQuote.exerciseType = symbolJSON.contains("exerciseType") ? symbolJSON["exerciseType"].get<std::string>() : "";
		//std::cout << "Setting deliverables" << std::endl;
		newQuote.deliverables = symbolJSON.contains("deliverables") ? symbolJSON["deliverables"].get<std::string>() : "";
		//std::cout << "Setting uvExpirationType" << std::endl;
		newQuote.uvExpirationType = symbolJSON.contains("uvExpirationType") ? symbolJSON["uvExpirationType"].get<std::string>() : "";
		//std::cout << "Setting settlementType" << std::endl;
		newQuote.settlementType = symbolJSON.contains("settlementType") ? symbolJSON["settlementType"].get<std::string>() : "";
		//std::cout << "Setting tradingHours" << std::endl;
		newQuote.tradingHours = symbolJSON.contains("tradingHours") ? symbolJSON["tradingHours"].get<std::string>() : "";
		//std::cout << "Setting marketMaker" << std::endl;
		newQuote.marketMaker = symbolJSON.contains("marketMaker") ? symbolJSON["marketMaker"].get<std::string>() : "";
		//std::cout << "Setting cusip" << std::endl;
		newQuote.cusip = symbolJSON.contains("cusip") ? symbolJSON["cusip"].get<std::string>() : "";
		//std::cout << "Setting lastPrice" << std::endl;
		newQuote.lastPrice = symbolJSON.contains("lastPrice") ? symbolJSON["lastPrice"].get<float>() : 0.00;
		//std::cout << "Setting openPrice" << std::endl;
		newQuote.openPrice = symbolJSON.contains("openPrice") ? symbolJSON["openPrice"].get<float>() : 0.00;
		//std::cout << "Setting highPrice" << std::endl;
		newQuote.highPrice = symbolJSON.contains("highPrice") ? symbolJSON["highPrice"].get<float>() : 0.00;
		//std::cout << "Setting lowPrice" << std::endl;
		newQuote.lowPrice = symbolJSON.contains("lowPrice") ? symbolJSON["lowPrice"].get<float>() : 0.00;
		//std::cout << "Setting closePrice" << std::endl;
		newQuote.closePrice = symbolJSON.contains("closePrice") ? symbolJSON["closePrice"].get<float>() : 0.00;
		//std::cout << "Setting bidPrice" << std::endl;
		newQuote.bidPrice = symbolJSON.contains("bidPrice") ? symbolJSON["bidPrice"].get<float>() : 0.00;
		//std::cout << "Setting netChange" << std::endl;
		newQuote.netChange = symbolJSON.contains("netChange") ? symbolJSON["netChange"].get<float>() : 0.00;
		//std::cout << "Setting 52WkHigh" << std::endl;
		newQuote.fiftyTwoWeekHigh = symbolJSON.contains("52WkHigh") ? symbolJSON["52WkHigh"].get<float>() : 0.00;
		//std::cout << "Setting 52WkLow" << std::endl;
		newQuote.fiftyTwoWeekLow = symbolJSON.contains("52WkLow") ? symbolJSON["52WkHigh"].get<float>() : 0.00;
		//std::cout << "Setting peRatio" << std::endl;
		newQuote.peRatio = symbolJSON.contains("peRatio") ? symbolJSON["peRatio"].get<float>() : 0.00;
		//std::cout << "Setting divAmount" << std::endl;
		newQuote.divAmount = symbolJSON.contains("divAmount") ? symbolJSON["divAmount"].get<float>() : 0.00;
		//std::cout << "Setting divYield" << std::endl;
		newQuote.divYield = symbolJSON.contains("divYield") ? symbolJSON["divYield"].get<float>() : 0.00;
		//std::cout << "Setting futurePercentChange" << std::endl;
		newQuote.futurePercentChange = symbolJSON.contains("futurePercentChange") ? symbolJSON["futurePercentChange"].get<float>() : 0.00;
		//std::cout << "Setting moneyIntrinsicValue" << std::endl;
		newQuote.moneyIntrinsicValue = symbolJSON.contains("moneyIntrinsicValue") ? symbolJSON["moneyIntrinsicValue"].get<float>() : 0.00;
		//std::cout << "Setting mark" << std::endl;
		newQuote.mark = symbolJSON.contains("mark") ? symbolJSON["mark"].get<float>() : 0.00;
		//std::cout << "Setting tick" << std::endl;
		newQuote.tick = symbolJSON.contains("tick") ? symbolJSON["tick"].get<float>() : 0.00;
		//std::cout << "Setting fiftyWkHigh" << std::endl;
		newQuote.fiftyWkHigh = symbolJSON.contains("fiftyWkHigh") ? symbolJSON["fiftyWkHigh"].get<float>() : 0.00;
		//std::cout << "Setting fiftyWkLow" << std::endl;
		newQuote.fiftyWkLow = symbolJSON.contains("fiftyWkLow") ? symbolJSON["fiftyWkLow"].get<float>() : 0.00;
		//std::cout << "Setting askPrice" << std::endl;
		newQuote.askPrice = symbolJSON.contains("askPrice") ? symbolJSON["askPrice"].get<float>() : 0.00;
		//std::cout << "Setting volatility" << std::endl;
		newQuote.volatility = symbolJSON.contains("volatility") ? symbolJSON["volatility"].get<float>() : 0.00;
		//std::cout << "Setting futureSettlementPrice" << std::endl;
		newQuote.futureSettlementPrice = symbolJSON.contains("futureSettlementPrice") ? symbolJSON["futureSettlementPrice"].get<float>() : 0.00;
		//std::cout << "Setting strikePrice" << std::endl;
		newQuote.strikePrice = symbolJSON.contains("strikePrice") ? symbolJSON["strikePrice"].get<float>() : 0.00;
		//std::cout << "Setting timeValue" << std::endl;
		newQuote.timeValue = symbolJSON.contains("timeValue") ? symbolJSON["timeValue"].get<float>() : 0.00;
		//std::cout << "Setting delta" << std::endl;
		newQuote.delta = symbolJSON.contains("delta") ? symbolJSON["delta"].get<float>() : 0.00;
		//std::cout << "Setting gamma" << std::endl;
		newQuote.gamma = symbolJSON.contains("gamma") ? symbolJSON["gamma"].get<float>() : 0.00;
		//std::cout << "Setting theta" << std::endl;
		newQuote.theta = symbolJSON.contains("theta") ? symbolJSON["theta"].get<float>() : 0.00;
		//std::cout << "Setting vega" << std::endl;
		newQuote.vega = symbolJSON.contains("vega") ? symbolJSON["vega"].get<float>() : 0.00;
		//std::cout << "Setting rho" << std::endl;
		newQuote.rho = symbolJSON.contains("rho") ? symbolJSON["rho"].get<float>() : 0.00;
		//std::cout << "Setting theoreticalOptionValue" << std::endl;
		newQuote.theoreticalOptionValue = symbolJSON.contains("theoreticalOptionValue") ? symbolJSON["theoreticalOptionValue"].get<float>() : 0.00;
		//std::cout << "Setting underlyingPrice" << std::endl;
		newQuote.underlyingPrice = symbolJSON.contains("underlyingPrice") ? symbolJSON["underlyingPrice"].get<float>() : 0.00;
		//std::cout << "Setting percentChange" << std::endl;
		newQuote.percentChange = symbolJSON.contains("percentChange") ? symbolJSON["percentChange"].get<float>() : 0.00;
		//std::cout << "Setting regularMarketLastPrice" << std::endl;
		newQuote.regularMarketLastPrice = symbolJSON.contains("regularMarketLastPrice") ? symbolJSON["regularMarketLastPrice"].get<float>(): 0.00;
		//std::cout << "Setting regularMarketNetChange" << std::endl;
		newQuote.regularMarketNetChange = symbolJSON.contains("regularMarketNetChange") ? symbolJSON["regularMarketNetChange"].get<float>() : 0.00;
		//std::cout << "Setting digits" << std::endl;
		newQuote.digits = symbolJSON.contains("digits") ? symbolJSON["digits"].get<int>() : 0;
		//std::cout << "Seting nAV" << std::endl;
		newQuote.nAV = symbolJSON.contains("nAV") ? symbolJSON["nAV"].get<int>() : 0;
		//std::cout << "Setting openInterest" << std::endl;
		newQuote.openInterest = symbolJSON.contains("openInterest") ? symbolJSON["openInterest"].get<int>() : 0;
		//std::cout << "Setting futureMultiplier" << std::endl;
		newQuote.futureMultiplier = symbolJSON.contains("futureMultiplier") ? symbolJSON["futureMultiplier"].get<int>() : 0;
		//std::cout << "Setting tickAmount" << std::endl;
		newQuote.tickAmount = symbolJSON.contains("tickAmount") ? symbolJSON["tickAmount"].get<int>() : 0;
		//std::cout << "Setting totalVolume" << std::endl;
		newQuote.totalVolume = symbolJSON.contains("totalVolume") ? symbolJSON["totalVolume"].get<int>() : 0;
		//std::cout << "Setting bidSize" << std::endl;
		newQuote.bidSize = symbolJSON.contains("bidSize") ? symbolJSON["bidSize"].get<int>() : 0;
		//std::cout << "Setting askSize" << std::endl;
		newQuote.askSize = symbolJSON.contains("askSize") ? symbolJSON["askSize"].get<int>() : 0;
		//std::cout << "Setting lastSize" << std::endl;
		newQuote.lastSize = symbolJSON.contains("lastSize") ? symbolJSON["lastSize"].get<int>() : 0;
		//std::cout << "Setting multiplier" << std::endl;
		newQuote.multiplier = symbolJSON.contains("multiplier") ? symbolJSON["multiplier"].get<int>() : 0;
		//std::cout << "Setting regularMarketLastSize" << std::endl;
		newQuote.regularMarketLastSize = symbolJSON.contains("regularMarketLastSize") ? symbolJSON["regularMarketLastSize"].get<int>() : 0;
		//std::cout << "Setting tradeTimeInLong" << std::endl;
		newQuote.tradeTimeInLong = symbolJSON.contains("tradeTimeInLong") ? symbolJSON["tradeTimeInLong"].get<long>() : 0;
		//std::cout << "Setting quoteTimeInLong" << std::endl;
		newQuote.quoteTimeInLong = symbolJSON.contains("quoteTimeInLong") ? symbolJSON["quoteTimeInLong"].get<long>() : 0;
		//std::cout << "Setting regularMarketTradeTimeInLong" << std::endl;
		newQuote.regularMarketTradeTimeInLong = symbolJSON.contains("regularMarketTradeTimeInLong") ? symbolJSON["regularMarketTradeTimeInLong"].get<long>() : 0;
		//std::cout << "Setting bidPriceInDouble" << std::endl;
		newQuote.bidPriceInDouble = symbolJSON.contains("bidPriceInDouble") ? symbolJSON["bidPriceInDouble"].get<double>() : 0;
		//std::cout << "Setting askPriceInDouble" << std::endl;
		newQuote.askPriceInDouble = symbolJSON.contains("askPriceInDouble") ? symbolJSON["askPriceInDouble"].get<double>() : 0;
		//std::cout << "Setting lastPriceInDouble" << std::endl;
		newQuote.lastPriceInDouble = symbolJSON.contains("lastPriceInDouble") ? symbolJSON["lastPriceInDouble"].get<double>() : 0;
		//std::cout << "Setting highPriceInDouble" << std::endl;
		newQuote.highPriceInDouble = symbolJSON.contains("highPriceInDouble") ? symbolJSON["highPriceInDouble"].get<double>() : 0;
		//std::cout << "Setting highPriceInDouble" << std::endl;
		newQuote.lowPriceInDouble = symbolJSON.contains("lowPriceInDouble") ? symbolJSON["lowPriceInDouble"].get<double>() : 0;
		//std::cout << "Setting closePriceInDouble" << std::endl;
		newQuote.closePriceInDouble = symbolJSON.contains("closePriceInDouble") ? symbolJSON["closePriceInDouble"].get<double>() : 0;
		//std::cout << "Setting openPriceInDouble" << std::endl;
		newQuote.openPriceInDouble = symbolJSON.contains("openPriceInDouble") ? symbolJSON["openPriceInDouble"].get<double>() : 0;
		//std::cout << "Setting netChangeInDouble" << std::endl;
		newQuote.netChangeInDouble = symbolJSON.contains("netChangeInDouble") ? symbolJSON["netChangeInDouble"].get<double>() : 0;
		//std::cout << "Setting moneyIntrinicValueInDouble" << std::endl;
		newQuote.moneyIntrinsicValueInDouble = symbolJSON.contains("moneyIntrinsicValueInDouble") ? symbolJSON["moneyIntrinsicValueInDouble"].get<double>() : 0;
		//std::cout << "Setting markChangeInDouble" << std::endl;
		newQuote.markChangeInDouble = symbolJSON.contains("markChangeInDouble") ? symbolJSON["markChangeInDouble"].get<double>() : 0;
		//std::cout << "Setting markPercentChangeInDouble" << std::endl;
		newQuote.markPercentChangeInDouble = symbolJSON.contains("markPercentChangeInDouble") ? symbolJSON["markPercentChangeInDouble"].get<double>() : 0;
		//std::cout << "Setting netPercentChangeInDouble" << std::endl;
		newQuote.netPercentChangeInDouble = symbolJSON.contains("netPercentChangeInDouble") ? symbolJSON["netPercentChangeInDouble"].get<double>() : 0;
		//std::cout << "Setting regularMarketPercentChangeInDouble" << std::endl;
		newQuote.regularMarketPercentChangeInDouble = symbolJSON.contains("regularMarketPercentChangeInDouble") ? symbolJSON["regularMarketPercentChangeInDouble"].get<double>() : 0;
		//std::cout << "Setting multiplierInDouble" << std::endl;
		newQuote.multiplierInDouble = symbolJSON.contains("multiplierInDouble") ? symbolJSON["multiplierInDouble"].get<double>() : 0;
		//std::cout << "Setting strikePriceInDouble" << std::endl;
		newQuote.strikePriceInDouble = symbolJSON.contains("strikePriceInDouble") ? symbolJSON["strikePriceInDouble"].get<double>() : 0;
		//std::cout << "Setting timeValueInDouble" << std::endl;
		newQuote.timeValueInDouble = symbolJSON.contains("timeValueInDouble") ? symbolJSON["timeValueInDouble"].get<double>() : 0;
		//std::cout << "Setting deltaInDouble" << std::endl;
		newQuote.deltaInDouble = symbolJSON.contains("deltaInDouble") ? symbolJSON["deltaInDouble"].get<double>() : 0;
		//std::cout << "Setting gammaInDouble" << std::endl;
		newQuote.gammaInDouble = symbolJSON.contains("gammaInDouble") ? symbolJSON["gammaInDouble"].get<double>() : 0;
		//std::cout << "Setting thetaInDouble" << std::endl;
		newQuote.thetaInDouble = symbolJSON.contains("thetaInDouble") ? symbolJSON["thetaInDouble"].get<double>() : 0;
		//std::cout << "Setting vegaInDouble" << std::endl;
		newQuote.vegaInDouble = symbolJSON.contains("vegaInDouble") ? symbolJSON["vegaInDouble"].get<double>() : 0;
		//std::cout << "Setting rhoInDouble" << std::endl;
		newQuote.rhoInDouble = symbolJSON.contains("rhoInDouble") ? symbolJSON["rhoInDouble"].get<double>() : 0;
		//std::cout << "Setting changeInDouble" << std::endl;
		newQuote.changeInDouble = symbolJSON.contains("changeInDouble") ? symbolJSON["changeInDouble"].get<double>() : 0;
		//std::cout << "Setting 52WkHighInDouble" << std::endl;
		newQuote.fiftyTwoWkHighInDouble = symbolJSON.contains("52WkHighInDouble") ? symbolJSON["52WkHighInDouble"].get<double>() : 0;
		//std::cout << "Setting 52WkLowInDouble" << std::endl;
		newQuote.fiftyTwoWkLowInDouble = symbolJSON.contains("52WkLowInDouble") ? symbolJSON["52WkLowInDouble"].get<double>() : 0;
		//std::cout << "Setting futureIsTradable" << std::endl;
		newQuote.futureIsTradable = symbolJSON.contains("futureIsTradable") ? symbolJSON["futureIsTradable"].get<bool>() : false;
		//std::cout << "Setting futureIsActive" << std::endl;
		newQuote.futureIsActive = symbolJSON.contains("futureIsActive") ? symbolJSON["futureIsActive"].get<bool>() : false;
		//std::cout << "Setting inTheMoney" << std::endl;
		newQuote.inTheMoney = symbolJSON.contains("inTheMoney") ? symbolJSON["inTheMoney"].get<bool>() : false;
		//std::cout << "Setting isTradable" << std::endl;
		newQuote.isTradable = symbolJSON.contains("isTradable") ? symbolJSON["isTradable"].get<bool>() : false;
		//std::cout << "Setting marginalbe" << std::endl;
		newQuote.marginable = symbolJSON.contains("marginable") ? symbolJSON["marginable"].get<bool>() : false;
		//std::cout << "Setting shortable" << std::endl;
		newQuote.shortable = symbolJSON.contains("shortable") ? symbolJSON["shortable"].get<bool>() : false;
		//std::cout << "Setting realTimeEntitled" << std::endl;
		newQuote.realTimeEntitled = symbolJSON.contains("realTimeEntitled") ? symbolJSON["realTimeEntitled"].get<bool>() : false;
		//std::cout << "Setting delayed" << std::endl;
		newQuote.delayed = symbolJSON.contains("delayed") ? symbolJSON["delayed"].get<bool>() : false;
	}

	return newQuote;
}
