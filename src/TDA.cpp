#include <iostream>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <time.h>
#include <stdlib.h>
#include <sstream>

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
			// if for some reason the access token creation date gets deleted or doesn't exist, 
			// then make a new access token
				

			if(!(configJSON.contains("access_create_date"))) {
				createAccessToken();
				saveConfig();
				readConfig();
			}else {
				if(checkAccessExpire()) {
					createAccessToken();
					saveConfig();
					readConfig();
				}
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

	resResults = "";

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

		std::cout << resResults << std::endl;

		nlohmann::json resJSON = nlohmann::json::parse(resResults);
		
		if(resJSON.contains(symbol)) {
			nlohmann::json symbolJSON = resJSON[symbol];

			newQuote = createQuote(symbolJSON);
		}else {
			std::cout << "WARNING: Unable to obtain quote for symbol: " << symbol << std::endl;
		}
	}

	return newQuote;
}

std::vector<Quote> TDA::getQuotes(std::string symbols) {

	quotes.clear();

	if(!(checkAccessExpire())) {

		// split the symbol string by the delimiter (',')
		// and store the tickers so we can use them later to
		// parse the returned JSON
		std::vector<std::string> tickerList;

		int start = 0;
		int end = symbols.find(",");
		std::string del = ",";
		while(end != -1) {
			tickerList.push_back(symbols.substr(start, end-start));
			start = end + del.size();
			end = symbols.find(del, start);
		}
		tickerList.push_back(symbols.substr(start, end-start));

		reqUrl = "https://api.tdameritrade.com/v1/marketdata/quotes?symbol=" + symbols;

		sendReq();

		// parse the returned JSON into separate quotes for each ticker we stored earlier
		std::cout << resResults << std::endl;

		nlohmann::json resJSON = nlohmann::json::parse(resResults);

		for(int i=0; i<tickerList.size(); i++) {
			if(resJSON.contains(tickerList[i])) {
				nlohmann::json symbolJSON = resJSON[tickerList[i]];
				Quote newQuote = createQuote(symbolJSON);
				quotes.push_back(newQuote);
			} else {
				std::cout << "WARNING: Unable to obtain quote for symbol: " << tickerList[i] << std::endl;
			}
		}
	}

	return quotes;
}

Quote TDA::createQuote(nlohmann::json quoteJSON) {
	Quote newQuote;

	newQuote.symbol = quoteJSON.contains("symbol") ? quoteJSON["symbol"].get<std::string>() : "";
	newQuote.assetMainType = quoteJSON.contains("assetMainType") ? quoteJSON["assetMainType"].get<std::string>() : "";
	newQuote.assetSubType = quoteJSON.contains("assetSubType") ? quoteJSON["assetSubType"].get<std::string>() : "";
	newQuote.assetType = quoteJSON.contains("assetType") ? quoteJSON["assetType"].get<std::string>() : "";
	newQuote.exchange = quoteJSON.contains("exchange") ? quoteJSON["exchange"].get<std::string>() : "";
	newQuote.exchangeName = quoteJSON.contains("exchangeName") ? quoteJSON["exchangeName"].get<std::string>() : "";
	newQuote.divDate = quoteJSON.contains("divDate") ? quoteJSON["divDate"].get<std::string>() : "";
	newQuote.securityStatus = quoteJSON.contains("securityStatus") ? quoteJSON["securityStatus"].get<std::string>() : "";
	newQuote.bidId = quoteJSON.contains("bidId") ? quoteJSON["bidId"].get<std::string>() : "";
	newQuote.askId = quoteJSON.contains("askId") ? quoteJSON["askId"].get<std::string>() : "";
	newQuote.description = quoteJSON.contains("description") ? quoteJSON["description"].get<std::string>() : "";
	newQuote.lastId = quoteJSON.contains("lastId") ? quoteJSON["lastId"].get<std::string>() : "";
	newQuote.product = quoteJSON.contains("product") ? quoteJSON["product"].get<std::string>() : "";
	newQuote.futurePriceFormat = quoteJSON.contains("futurePriceFormat") ? quoteJSON["futurePriceFormat"].get<std::string>() : "";
	newQuote.futureTradingHours = quoteJSON.contains("futureTradingHours") ? quoteJSON["futureTradingHours"].get<std::string>() : "";
	newQuote.futureActiveSymbol = quoteJSON.contains("futureActiveSymbol") ? quoteJSON["futureActiveSymbol"].get<std::string>() : "";
	newQuote.futureExpirationDate = quoteJSON.contains("futureExpirationDate") ? quoteJSON["futureExpirationDate"].get<std::string>() : "";
	newQuote.contractType = quoteJSON.contains("contractType") ? quoteJSON["contractType"].get<std::string>() : "";
	newQuote.underlying = quoteJSON.contains("underlying") ? quoteJSON["underlying"].get<std::string>() : "";
	newQuote.expirationType = quoteJSON.contains("expirationType") ? quoteJSON["expirationType"].get<std::string>() : "";
	newQuote.exerciseType = quoteJSON.contains("exerciseType") ? quoteJSON["exerciseType"].get<std::string>() : "";
	newQuote.deliverables = quoteJSON.contains("deliverables") ? quoteJSON["deliverables"].get<std::string>() : "";
	newQuote.uvExpirationType = quoteJSON.contains("uvExpirationType") ? quoteJSON["uvExpirationType"].get<std::string>() : "";
	newQuote.settlementType = quoteJSON.contains("settlementType") ? quoteJSON["settlementType"].get<std::string>() : "";
	newQuote.tradingHours = quoteJSON.contains("tradingHours") ? quoteJSON["tradingHours"].get<std::string>() : "";
	newQuote.marketMaker = quoteJSON.contains("marketMaker") ? quoteJSON["marketMaker"].get<std::string>() : "";
	newQuote.cusip = quoteJSON.contains("cusip") ? quoteJSON["cusip"].get<std::string>() : "";
	newQuote.lastPrice = quoteJSON.contains("lastPrice") ? quoteJSON["lastPrice"].get<float>() : 0.00;
	newQuote.openPrice = quoteJSON.contains("openPrice") ? quoteJSON["openPrice"].get<float>() : 0.00;
	newQuote.highPrice = quoteJSON.contains("highPrice") ? quoteJSON["highPrice"].get<float>() : 0.00;
	newQuote.lowPrice = quoteJSON.contains("lowPrice") ? quoteJSON["lowPrice"].get<float>() : 0.00;
	newQuote.closePrice = quoteJSON.contains("closePrice") ? quoteJSON["closePrice"].get<float>() : 0.00;
	newQuote.bidPrice = quoteJSON.contains("bidPrice") ? quoteJSON["bidPrice"].get<float>() : 0.00;
	newQuote.netChange = quoteJSON.contains("netChange") ? quoteJSON["netChange"].get<float>() : 0.00;
	newQuote.fiftyTwoWeekHigh = quoteJSON.contains("52WkHigh") ? quoteJSON["52WkHigh"].get<float>() : 0.00;
	newQuote.fiftyTwoWeekLow = quoteJSON.contains("52WkLow") ? quoteJSON["52WkHigh"].get<float>() : 0.00;
	newQuote.peRatio = quoteJSON.contains("peRatio") ? quoteJSON["peRatio"].get<float>() : 0.00;
	newQuote.divAmount = quoteJSON.contains("divAmount") ? quoteJSON["divAmount"].get<float>() : 0.00;
	newQuote.divYield = quoteJSON.contains("divYield") ? quoteJSON["divYield"].get<float>() : 0.00;
	newQuote.futurePercentChange = quoteJSON.contains("futurePercentChange") ? quoteJSON["futurePercentChange"].get<float>() : 0.00;
	newQuote.moneyIntrinsicValue = quoteJSON.contains("moneyIntrinsicValue") ? quoteJSON["moneyIntrinsicValue"].get<float>() : 0.00;
	newQuote.mark = quoteJSON.contains("mark") ? quoteJSON["mark"].get<float>() : 0.00;
	newQuote.tick = quoteJSON.contains("tick") ? quoteJSON["tick"].get<float>() : 0.00;
	newQuote.fiftyWkHigh = quoteJSON.contains("fiftyWkHigh") ? quoteJSON["fiftyWkHigh"].get<float>() : 0.00;
	newQuote.fiftyWkLow = quoteJSON.contains("fiftyWkLow") ? quoteJSON["fiftyWkLow"].get<float>() : 0.00;
	newQuote.askPrice = quoteJSON.contains("askPrice") ? quoteJSON["askPrice"].get<float>() : 0.00;
	newQuote.volatility = quoteJSON.contains("volatility") ? quoteJSON["volatility"].get<float>() : 0.00;
	newQuote.futureSettlementPrice = quoteJSON.contains("futureSettlementPrice") ? quoteJSON["futureSettlementPrice"].get<float>() : 0.00;
	newQuote.strikePrice = quoteJSON.contains("strikePrice") ? quoteJSON["strikePrice"].get<float>() : 0.00;
	newQuote.timeValue = quoteJSON.contains("timeValue") ? quoteJSON["timeValue"].get<float>() : 0.00;
	newQuote.delta = quoteJSON.contains("delta") ? quoteJSON["delta"].get<float>() : 0.00;
	newQuote.gamma = quoteJSON.contains("gamma") ? quoteJSON["gamma"].get<float>() : 0.00;
	newQuote.theta = quoteJSON.contains("theta") ? quoteJSON["theta"].get<float>() : 0.00;
	newQuote.vega = quoteJSON.contains("vega") ? quoteJSON["vega"].get<float>() : 0.00;
	newQuote.rho = quoteJSON.contains("rho") ? quoteJSON["rho"].get<float>() : 0.00;
	newQuote.theoreticalOptionValue = quoteJSON.contains("theoreticalOptionValue") ? quoteJSON["theoreticalOptionValue"].get<float>() : 0.00;
	newQuote.underlyingPrice = quoteJSON.contains("underlyingPrice") ? quoteJSON["underlyingPrice"].get<float>() : 0.00;
	newQuote.percentChange = quoteJSON.contains("percentChange") ? quoteJSON["percentChange"].get<float>() : 0.00;
	newQuote.regularMarketLastPrice = quoteJSON.contains("regularMarketLastPrice") ? quoteJSON["regularMarketLastPrice"].get<float>(): 0.00;
	newQuote.regularMarketNetChange = quoteJSON.contains("regularMarketNetChange") ? quoteJSON["regularMarketNetChange"].get<float>() : 0.00;
	newQuote.digits = quoteJSON.contains("digits") ? quoteJSON["digits"].get<int>() : 0;
	newQuote.nAV = quoteJSON.contains("nAV") ? quoteJSON["nAV"].get<int>() : 0;
	newQuote.openInterest = quoteJSON.contains("openInterest") ? quoteJSON["openInterest"].get<int>() : 0;
	newQuote.futureMultiplier = quoteJSON.contains("futureMultiplier") ? quoteJSON["futureMultiplier"].get<int>() : 0;
	newQuote.tickAmount = quoteJSON.contains("tickAmount") ? quoteJSON["tickAmount"].get<int>() : 0;
	newQuote.totalVolume = quoteJSON.contains("totalVolume") ? quoteJSON["totalVolume"].get<int>() : 0;
	newQuote.bidSize = quoteJSON.contains("bidSize") ? quoteJSON["bidSize"].get<int>() : 0;
	newQuote.askSize = quoteJSON.contains("askSize") ? quoteJSON["askSize"].get<int>() : 0;
	newQuote.lastSize = quoteJSON.contains("lastSize") ? quoteJSON["lastSize"].get<int>() : 0;
	newQuote.multiplier = quoteJSON.contains("multiplier") ? quoteJSON["multiplier"].get<int>() : 0;
	newQuote.regularMarketLastSize = quoteJSON.contains("regularMarketLastSize") ? quoteJSON["regularMarketLastSize"].get<int>() : 0;
	newQuote.tradeTimeInLong = quoteJSON.contains("tradeTimeInLong") ? quoteJSON["tradeTimeInLong"].get<long>() : 0;
	newQuote.quoteTimeInLong = quoteJSON.contains("quoteTimeInLong") ? quoteJSON["quoteTimeInLong"].get<long>() : 0;
	newQuote.regularMarketTradeTimeInLong = quoteJSON.contains("regularMarketTradeTimeInLong") ? quoteJSON["regularMarketTradeTimeInLong"].get<long>() : 0;
	newQuote.bidPriceInDouble = quoteJSON.contains("bidPriceInDouble") ? quoteJSON["bidPriceInDouble"].get<double>() : 0;
	newQuote.askPriceInDouble = quoteJSON.contains("askPriceInDouble") ? quoteJSON["askPriceInDouble"].get<double>() : 0;
	newQuote.lastPriceInDouble = quoteJSON.contains("lastPriceInDouble") ? quoteJSON["lastPriceInDouble"].get<double>() : 0;
	newQuote.highPriceInDouble = quoteJSON.contains("highPriceInDouble") ? quoteJSON["highPriceInDouble"].get<double>() : 0;
	newQuote.lowPriceInDouble = quoteJSON.contains("lowPriceInDouble") ? quoteJSON["lowPriceInDouble"].get<double>() : 0;
	newQuote.closePriceInDouble = quoteJSON.contains("closePriceInDouble") ? quoteJSON["closePriceInDouble"].get<double>() : 0;
	newQuote.openPriceInDouble = quoteJSON.contains("openPriceInDouble") ? quoteJSON["openPriceInDouble"].get<double>() : 0;
	newQuote.netChangeInDouble = quoteJSON.contains("netChangeInDouble") ? quoteJSON["netChangeInDouble"].get<double>() : 0;
	newQuote.moneyIntrinsicValueInDouble = quoteJSON.contains("moneyIntrinsicValueInDouble") ? quoteJSON["moneyIntrinsicValueInDouble"].get<double>() : 0;
	newQuote.markChangeInDouble = quoteJSON.contains("markChangeInDouble") ? quoteJSON["markChangeInDouble"].get<double>() : 0;
	newQuote.markPercentChangeInDouble = quoteJSON.contains("markPercentChangeInDouble") ? quoteJSON["markPercentChangeInDouble"].get<double>() : 0;
	newQuote.netPercentChangeInDouble = quoteJSON.contains("netPercentChangeInDouble") ? quoteJSON["netPercentChangeInDouble"].get<double>() : 0;
	newQuote.regularMarketPercentChangeInDouble = quoteJSON.contains("regularMarketPercentChangeInDouble") ? quoteJSON["regularMarketPercentChangeInDouble"].get<double>() : 0;
	newQuote.multiplierInDouble = quoteJSON.contains("multiplierInDouble") ? quoteJSON["multiplierInDouble"].get<double>() : 0;
	newQuote.strikePriceInDouble = quoteJSON.contains("strikePriceInDouble") ? quoteJSON["strikePriceInDouble"].get<double>() : 0;
	newQuote.timeValueInDouble = quoteJSON.contains("timeValueInDouble") ? quoteJSON["timeValueInDouble"].get<double>() : 0;
	newQuote.deltaInDouble = quoteJSON.contains("deltaInDouble") ? quoteJSON["deltaInDouble"].get<double>() : 0;
	newQuote.gammaInDouble = quoteJSON.contains("gammaInDouble") ? quoteJSON["gammaInDouble"].get<double>() : 0;
	newQuote.thetaInDouble = quoteJSON.contains("thetaInDouble") ? quoteJSON["thetaInDouble"].get<double>() : 0;
	newQuote.vegaInDouble = quoteJSON.contains("vegaInDouble") ? quoteJSON["vegaInDouble"].get<double>() : 0;
	newQuote.rhoInDouble = quoteJSON.contains("rhoInDouble") ? quoteJSON["rhoInDouble"].get<double>() : 0;
	newQuote.changeInDouble = quoteJSON.contains("changeInDouble") ? quoteJSON["changeInDouble"].get<double>() : 0;
	newQuote.fiftyTwoWkHighInDouble = quoteJSON.contains("52WkHighInDouble") ? quoteJSON["52WkHighInDouble"].get<double>() : 0;
	newQuote.fiftyTwoWkLowInDouble = quoteJSON.contains("52WkLowInDouble") ? quoteJSON["52WkLowInDouble"].get<double>() : 0;
	newQuote.futureIsTradable = quoteJSON.contains("futureIsTradable") ? quoteJSON["futureIsTradable"].get<bool>() : false;
	newQuote.futureIsActive = quoteJSON.contains("futureIsActive") ? quoteJSON["futureIsActive"].get<bool>() : false;
	newQuote.inTheMoney = quoteJSON.contains("inTheMoney") ? quoteJSON["inTheMoney"].get<bool>() : false;
	newQuote.isTradable = quoteJSON.contains("isTradable") ? quoteJSON["isTradable"].get<bool>() : false;
	newQuote.marginable = quoteJSON.contains("marginable") ? quoteJSON["marginable"].get<bool>() : false;
	newQuote.shortable = quoteJSON.contains("shortable") ? quoteJSON["shortable"].get<bool>() : false;
	newQuote.realTimeEntitled = quoteJSON.contains("realTimeEntitled") ? quoteJSON["realTimeEntitled"].get<bool>() : false;
	newQuote.delayed = quoteJSON.contains("delayed") ? quoteJSON["delayed"].get<bool>() : false;

	return newQuote;
}
