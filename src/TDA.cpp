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
	//std::cout << "Initializing curl" << std::endl;

	resResults = "";

	curl = curl_easy_init();

	if(curl) {
		//std::cout << "Setting url opt" << std::endl;
		//std::cout << reqUrl << std::endl;
		curl_easy_setopt(curl, CURLOPT_URL, reqUrl.c_str());	
		
		/* Setting post fields if we are doing a post request */
		if(postData != "") {
			/* Setting Headers */
			struct curl_slist *headers=NULL;

			headers = curl_slist_append(headers, "Accept-Encoding: gzip");
			headers = curl_slist_append(headers, "Accept-Language: en-US");
			headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			//std::cout << "Setting post fields" << std::endl;
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		}else {
			// set bearer token
			//std::cout << "SETTING BEARER" << std::endl;
			struct curl_slist *headers=NULL;
			headers = curl_slist_append(headers, ("Authorization: Bearer " + configJSON["access_token"].get<std::string>()).c_str());
			curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, configJSON["access_token"].get<std::string>().c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}

		//std::cout << "Setting callback function" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TDA::saveLibCurlRes);
		//std::cout << "Setting callback variable" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resResults);

		// for debugging
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		
		/* Perform the request, res will get the return code */
		//std::cout << "Performing request" << std::endl;
		res = curl_easy_perform(curl);

		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* always clean up */
		//std::cout << "Cleaning up" << std::endl;
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

	std::cout << "Creating Access Token" << std::endl;

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
	} else {
		createAccessToken();
		saveConfig();
	}

	return histPriceData;
}


Quote TDA::getQuote(std::string symbol) {

	Quote newQuote;

	if(!(checkAccessExpire())) {
		
		reqUrl = "https://api.tdameritrade.com/v1/marketdata/" + symbol + "/quotes";

		sendReq();

		//std::cout << resResults << std::endl;

		nlohmann::json resJSON = nlohmann::json::parse(resResults);
		
		if(resJSON.contains(symbol)) {
			nlohmann::json symbolJSON = resJSON[symbol];

			newQuote = createQuote(symbolJSON);
		}else {
			std::cout << "WARNING: Unable to obtain quote for symbol: " << symbol << std::endl;
		}
	}else {
		createAccessToken();
		saveConfig();
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
	}else {
		createAccessToken();
		saveConfig();
	}

	return quotes;
}

Quote TDA::createQuote(nlohmann::json quoteJSON) {
	Quote newQuote;

	//std::cout << "Setting Symbol" << std::endl;
	newQuote.symbol = quoteJSON.contains("symbol") ? quoteJSON["symbol"].get<std::string>() : "";
	//std::cout << "Setting assetMainType" << std::endl;
	newQuote.assetMainType = quoteJSON.contains("assetMainType") ? quoteJSON["assetMainType"].get<std::string>() : "";
	//std::cout << "Setting assetSubType" << std::endl;
	newQuote.assetSubType = quoteJSON.contains("assetSubType") ? quoteJSON["assetSubType"].get<std::string>() : "";
	//std::cout << "Setting assetType" << std::endl;
	newQuote.assetType = quoteJSON.contains("assetType") ? quoteJSON["assetType"].get<std::string>() : "";
	//std::cout << "Setting exchange" << std::endl;
	newQuote.exchange = quoteJSON.contains("exchange") ? quoteJSON["exchange"].get<std::string>() : "";
	//std::cout << "Setting exchangeName" << std::endl;
	newQuote.exchangeName = quoteJSON.contains("exchangeName") ? quoteJSON["exchangeName"].get<std::string>() : "";
	//std::cout << "Setting divDate" << std::endl;
	newQuote.divDate = quoteJSON.contains("divDate") ? quoteJSON["divDate"].get<std::string>() : "";
	//std::cout << "Setting securityStatus" << std::endl;
	newQuote.securityStatus = quoteJSON.contains("securityStatus") ? quoteJSON["securityStatus"].get<std::string>() : "";
	//std::cout << "Setting bidId" << std::endl;
	newQuote.bidId = quoteJSON.contains("bidId") ? quoteJSON["bidId"].get<std::string>() : "";
	//std::cout << "Setting askId" << std::endl;
	newQuote.askId = quoteJSON.contains("askId") ? quoteJSON["askId"].get<std::string>() : "";
	//std::cout << "Setting description" << std::endl;
	newQuote.description = quoteJSON.contains("description") ? quoteJSON["description"].get<std::string>() : "";
	//std::cout << "Setting lastId" << std::endl;
	newQuote.lastId = quoteJSON.contains("lastId") ? quoteJSON["lastId"].get<std::string>() : "";
	//std::cout << "Setting product" << std::endl;
	newQuote.product = quoteJSON.contains("product") ? quoteJSON["product"].get<std::string>() : "";
	//std::cout << "Setting futurePriceFormat" << std::endl;
	newQuote.futurePriceFormat = quoteJSON.contains("futurePriceFormat") ? quoteJSON["futurePriceFormat"].get<std::string>() : "";
	//std::cout << "Setting futureTradingHours" << std::endl;
	newQuote.futureTradingHours = quoteJSON.contains("futureTradingHours") ? quoteJSON["futureTradingHours"].get<std::string>() : "";
	//std::cout << "Setting futureActiveSymbol" << std::endl;
	newQuote.futureActiveSymbol = quoteJSON.contains("futureActiveSymbol") ? quoteJSON["futureActiveSymbol"].get<std::string>() : "";
	//std::cout << "Setting futureExpirationDate" << std::endl;
	newQuote.futureExpirationDate = quoteJSON.contains("futureExpirationDate") ? quoteJSON["futureExpirationDate"].get<std::string>() : "";
	//std::cout << "Setting contractType" << std::endl;
	newQuote.contractType = quoteJSON.contains("contractType") ? quoteJSON["contractType"].get<std::string>() : "";
	//std::cout << "Settin underlying" << std::endl;
	newQuote.underlying = quoteJSON.contains("underlying") ? quoteJSON["underlying"].get<std::string>() : "";
	//std::cout << "Setting expirationType" << std::endl;
	newQuote.expirationType = quoteJSON.contains("expirationType") ? quoteJSON["expirationType"].get<std::string>() : "";
	//std::cout << "Setting exerciseType" << std::endl;
	newQuote.exerciseType = quoteJSON.contains("exerciseType") ? quoteJSON["exerciseType"].get<std::string>() : "";
	//std::cout << "Setting deliverables" << std::endl;
	newQuote.deliverables = quoteJSON.contains("deliverables") ? quoteJSON["deliverables"].get<std::string>() : "";
	//std::cout << "Setting uvExpirationType" << std::endl;
	newQuote.uvExpirationType = quoteJSON.contains("uvExpirationType") ? quoteJSON["uvExpirationType"].get<std::string>() : "";
	//std::cout << "Setting settlementType" << std::endl;
	newQuote.settlementType = quoteJSON.contains("settlementType") ? quoteJSON["settlementType"].get<std::string>() : "";
	//std::cout << "Setting tradingHours" << std::endl;
	newQuote.tradingHours = quoteJSON.contains("tradingHours") ? quoteJSON["tradingHours"].get<std::string>() : "";
	//std::cout << "Setting marketMaker" << std::endl;
	newQuote.marketMaker = quoteJSON.contains("marketMaker") ? quoteJSON["marketMaker"].get<std::string>() : "";
	//std::cout << "Setting cusip" << std::endl;
	if(quoteJSON["assetMainType"] != "FOREX") {
		// this is because cusip is null for FOREX
		newQuote.cusip = quoteJSON.contains("cusip") ? quoteJSON["cusip"].get<std::string>() : "";
	}
	//std::cout << "Setting lastPrice" << std::endl;
	newQuote.lastPrice = quoteJSON.contains("lastPrice") ? quoteJSON["lastPrice"].get<float>() : 0.00;
	//std::cout << "Setting openPrice" << std::endl;
	newQuote.openPrice = quoteJSON.contains("openPrice") ? quoteJSON["openPrice"].get<float>() : 0.00;
	//std::cout << "Setting highPrice" << std::endl;
	newQuote.highPrice = quoteJSON.contains("highPrice") ? quoteJSON["highPrice"].get<float>() : 0.00;
	//std::cout << "Setting lowPrice" << std::endl;
	newQuote.lowPrice = quoteJSON.contains("lowPrice") ? quoteJSON["lowPrice"].get<float>() : 0.00;
	//std::cout << "Setting closePrice" << std::endl;
	newQuote.closePrice = quoteJSON.contains("closePrice") ? quoteJSON["closePrice"].get<float>() : 0.00;
	//std::cout << "Setting bidPrice" << std::endl;
	newQuote.bidPrice = quoteJSON.contains("bidPrice") ? quoteJSON["bidPrice"].get<float>() : 0.00;
	//std::cout << "Setting netChange" << std::endl;
	newQuote.netChange = quoteJSON.contains("netChange") ? quoteJSON["netChange"].get<float>() : 0.00;
	//std::cout << "Setting 52WkHigh" << std::endl;
	newQuote.fiftyTwoWeekHigh = quoteJSON.contains("52WkHigh") ? quoteJSON["52WkHigh"].get<float>() : 0.00;
	//std::cout << "setting 52WkLow" << std::endl;
	newQuote.fiftyTwoWeekLow = quoteJSON.contains("52WkLow") ? quoteJSON["52WkHigh"].get<float>() : 0.00;
	//std::cout << "Setting peRatio" << std::endl;
	newQuote.peRatio = quoteJSON.contains("peRatio") ? quoteJSON["peRatio"].get<float>() : 0.00;
	//std::cout << "Setting divAmount" << std::endl;
	newQuote.divAmount = quoteJSON.contains("divAmount") ? quoteJSON["divAmount"].get<float>() : 0.00;
	//std::cout << "Setting divYield" << std::endl;
	newQuote.divYield = quoteJSON.contains("divYield") ? quoteJSON["divYield"].get<float>() : 0.00;
	//std::cout << "Setting futurePercentChange" << std::endl;
	newQuote.futurePercentChange = quoteJSON.contains("futurePercentChange") ? quoteJSON["futurePercentChange"].get<float>() : 0.00;
	//std::cout << "Setting moneyIntrinsicValue" << std::endl;
	newQuote.moneyIntrinsicValue = quoteJSON.contains("moneyIntrinsicValue") ? quoteJSON["moneyIntrinsicValue"].get<float>() : 0.00;
	//std::cout << "Setting mark" << std::endl;
	newQuote.mark = quoteJSON.contains("mark") ? quoteJSON["mark"].get<float>() : 0.00;
	//std::cout << "Setting tick" << std::endl;
	newQuote.tick = quoteJSON.contains("tick") ? quoteJSON["tick"].get<float>() : 0.00;
	//std::cout << "Settting fiftyWkHigh" << std::endl;
	newQuote.fiftyWkHigh = quoteJSON.contains("fiftyWkHigh") ? quoteJSON["fiftyWkHigh"].get<float>() : 0.00;
	//std::cout << "Setting fiftyWkLow" << std::endl;
	newQuote.fiftyWkLow = quoteJSON.contains("fiftyWkLow") ? quoteJSON["fiftyWkLow"].get<float>() : 0.00;
	//std::cout << "Setting askPrice" << std::endl;
	newQuote.askPrice = quoteJSON.contains("askPrice") ? quoteJSON["askPrice"].get<float>() : 0.00;
	//std::cout << "Setting volatility" << std::endl;
	newQuote.volatility = quoteJSON.contains("volatility") ? quoteJSON["volatility"].get<float>() : 0.00;
	//std::cout << "Setting futureSettlementPrice" << std::endl;
	newQuote.futureSettlementPrice = quoteJSON.contains("futureSettlementPrice") ? quoteJSON["futureSettlementPrice"].get<float>() : 0.00;
	//std::cout << "Setting strikePrice" << std::endl;
	newQuote.strikePrice = quoteJSON.contains("strikePrice") ? quoteJSON["strikePrice"].get<float>() : 0.00;
	//std::cout << "Setting timeValue" << std::endl;
	newQuote.timeValue = quoteJSON.contains("timeValue") ? quoteJSON["timeValue"].get<float>() : 0.00;
	//std::cout << "Setting delta" << std::endl;
	newQuote.delta = quoteJSON.contains("delta") ? quoteJSON["delta"].get<float>() : 0.00;
	//std::cout << "Setting gamma" << std::endl;
	newQuote.gamma = quoteJSON.contains("gamma") ? quoteJSON["gamma"].get<float>() : 0.00;
	//std::cout << "Setting theta" << std::endl;
	newQuote.theta = quoteJSON.contains("theta") ? quoteJSON["theta"].get<float>() : 0.00;
	//std::cout << "Setting vega" << std::endl;
	newQuote.vega = quoteJSON.contains("vega") ? quoteJSON["vega"].get<float>() : 0.00;
	//std::cout << "Setting rho" << std::endl;
	newQuote.rho = quoteJSON.contains("rho") ? quoteJSON["rho"].get<float>() : 0.00;
	//std::cout << "Setting theoreticalOptionValue" << std::endl;
	newQuote.theoreticalOptionValue = quoteJSON.contains("theoreticalOptionValue") ? quoteJSON["theoreticalOptionValue"].get<float>() : 0.00;
	//std::cout << "Setting underlyingPrice" << std::endl;
	newQuote.underlyingPrice = quoteJSON.contains("underlyingPrice") ? quoteJSON["underlyingPrice"].get<float>() : 0.00;
	//std::cout << "Setting percentChange" << std::endl;
	newQuote.percentChange = quoteJSON.contains("percentChange") ? quoteJSON["percentChange"].get<float>() : 0.00;
	//std::cout << "Setting regularMarketLastPrice" << std::endl;
	newQuote.regularMarketLastPrice = quoteJSON.contains("regularMarketLastPrice") ? quoteJSON["regularMarketLastPrice"].get<float>(): 0.00;
	//std::cout << "Setting regularMarketNetChange" << std::endl;
	newQuote.regularMarketNetChange = quoteJSON.contains("regularMarketNetChange") ? quoteJSON["regularMarketNetChange"].get<float>() : 0.00;
	//std::cout << "Setting digits" << std::endl;
	newQuote.digits = quoteJSON.contains("digits") ? quoteJSON["digits"].get<int>() : 0;
	//std::cout << "Setting nAV" << std::endl;
	newQuote.nAV = quoteJSON.contains("nAV") ? quoteJSON["nAV"].get<int>() : 0;
	//std::cout << "Setting openInterest" << std::endl;
	newQuote.openInterest = quoteJSON.contains("openInterest") ? quoteJSON["openInterest"].get<int>() : 0;
	//std::cout << "Setting futureMultiplier" << std::endl;
	newQuote.futureMultiplier = quoteJSON.contains("futureMultiplier") ? quoteJSON["futureMultiplier"].get<int>() : 0;
	//std::cout << "Setting tickAmount" << std::endl;
	newQuote.tickAmount = quoteJSON.contains("tickAmount") ? quoteJSON["tickAmount"].get<int>() : 0;
	//std::cout << "Setting totalVolume" << std::endl;
	newQuote.totalVolume = quoteJSON.contains("totalVolume") ? quoteJSON["totalVolume"].get<int>() : 0;
	//std::cout << "Setting bidSize" << std::endl;
	newQuote.bidSize = quoteJSON.contains("bidSize") ? quoteJSON["bidSize"].get<int>() : 0;
	//std::cout << "Setting askSize" << std::endl;
	newQuote.askSize = quoteJSON.contains("askSize") ? quoteJSON["askSize"].get<int>() : 0;
	//std::cout << "Setting lastSize" << std::endl;
	newQuote.lastSize = quoteJSON.contains("lastSize") ? quoteJSON["lastSize"].get<int>() : 0;
	//std::cout << "Setting multiplier" << std::endl;
	newQuote.multiplier = quoteJSON.contains("multiplier") ? quoteJSON["multiplier"].get<int>() : 0;
	//std::cout << "Setting regularMarketLastSize" << std::endl;
	newQuote.regularMarketLastSize = quoteJSON.contains("regularMarketLastSize") ? quoteJSON["regularMarketLastSize"].get<int>() : 0;
	//std::cout << "Setting tradeTimeInLong" << std::endl;
	newQuote.tradeTimeInLong = quoteJSON.contains("tradeTimeInLong") ? quoteJSON["tradeTimeInLong"].get<long>() : 0;
	//std::cout << "Setting quoteTimeOnLong" << std::endl;
	newQuote.quoteTimeInLong = quoteJSON.contains("quoteTimeInLong") ? quoteJSON["quoteTimeInLong"].get<long>() : 0;
	//std::cout << "Setting askSizeInLong" << std::endl;
	newQuote.askSizeInLong = quoteJSON.contains("askSizeInLong") ? quoteJSON["askSizeInLong"].get<long>() : 0;
	//std::cout << "Setting bidSizeInLong" << std::endl;
	newQuote.bidSizeInLong = quoteJSON.contains("bidSizeInLong") ? quoteJSON["bidSizeInLong"].get<long>() : 0;
	//std::cout << "Setting lastSizeInLong" << std::endl;
	newQuote.lastSizeInLong = quoteJSON.contains("lastSizeInLong") ? quoteJSON["lastSizeInLong"].get<long>() : 0;
	//std::cout << "Setting regularMarketTradeTimeInLong" << std::endl;
	newQuote.regularMarketTradeTimeInLong = quoteJSON.contains("regularMarketTradeTimeInLong") ? quoteJSON["regularMarketTradeTimeInLong"].get<long>() : 0;
	//std::cout << "Setting bidPriceInDouble" << std::endl;
	newQuote.bidPriceInDouble = quoteJSON.contains("bidPriceInDouble") ? quoteJSON["bidPriceInDouble"].get<double>() : 0;
	//std::cout << "Setting askPriceInDouble" << std::endl;
	newQuote.askPriceInDouble = quoteJSON.contains("askPriceInDouble") ? quoteJSON["askPriceInDouble"].get<double>() : 0;
	//std::cout << "Setting lastPriceInDouble" << std::endl;
	newQuote.lastPriceInDouble = quoteJSON.contains("lastPriceInDouble") ? quoteJSON["lastPriceInDouble"].get<double>() : 0;
	//std::cout << "Setting highPriceInDouble" << std::endl;
	newQuote.highPriceInDouble = quoteJSON.contains("highPriceInDouble") ? quoteJSON["highPriceInDouble"].get<double>() : 0;
	//std::cout << "Setting lowPriceInDouble" << std::endl;
	newQuote.lowPriceInDouble = quoteJSON.contains("lowPriceInDouble") ? quoteJSON["lowPriceInDouble"].get<double>() : 0;
	//std::cout << "Settin closePriceInDouble" << std::endl;
	newQuote.closePriceInDouble = quoteJSON.contains("closePriceInDouble") ? quoteJSON["closePriceInDouble"].get<double>() : 0;
	//std::cout << "Setting openPriceInDouble" << std::endl;
	newQuote.openPriceInDouble = quoteJSON.contains("openPriceInDouble") ? quoteJSON["openPriceInDouble"].get<double>() : 0;
	//std::cout << "Setting netChangeInDouble" << std::endl;
	newQuote.netChangeInDouble = quoteJSON.contains("netChangeInDouble") ? quoteJSON["netChangeInDouble"].get<double>() : 0;
	//std::cout << "Setting moneyIntrinsicValueInDouble" << std::endl;
	newQuote.moneyIntrinsicValueInDouble = quoteJSON.contains("moneyIntrinsicValueInDouble") ? quoteJSON["moneyIntrinsicValueInDouble"].get<double>() : 0;
	//std::cout << "Setting markChangeInDouble" << std::endl;
	newQuote.markChangeInDouble = quoteJSON.contains("markChangeInDouble") ? quoteJSON["markChangeInDouble"].get<double>() : 0;
	//std::cout << "Setting markPercentChangeInDouble" << std::endl;
	newQuote.markPercentChangeInDouble = quoteJSON.contains("markPercentChangeInDouble") ? quoteJSON["markPercentChangeInDouble"].get<double>() : 0;
	//std::cout << "Setting netPercentChangeInDouble" << std::endl;
	newQuote.netPercentChangeInDouble = quoteJSON.contains("netPercentChangeInDouble") ? quoteJSON["netPercentChangeInDouble"].get<double>() : 0;
	//std::cout << "Setting regularMarketPercentChangeInDouble" << std::endl;
	newQuote.regularMarketPercentChangeInDouble = quoteJSON.contains("regularMarketPercentChangeInDouble") ? quoteJSON["regularMarketPercentChangeInDouble"].get<double>() : 0;
	//std::cout << "Setting multiplierInDouble" << std::endl;
	newQuote.multiplierInDouble = quoteJSON.contains("multiplierInDouble") ? quoteJSON["multiplierInDouble"].get<double>() : 0;
	//std::cout << "Setting strikePriceInDouble" << std::endl;
	newQuote.strikePriceInDouble = quoteJSON.contains("strikePriceInDouble") ? quoteJSON["strikePriceInDouble"].get<double>() : 0;
	//std::cout << "Setting timeValueInDouble" << std::endl;
	newQuote.timeValueInDouble = quoteJSON.contains("timeValueInDouble") ? quoteJSON["timeValueInDouble"].get<double>() : 0;
	//std::cout << "Setting deltaInDouble" << std::endl;
	newQuote.deltaInDouble = quoteJSON.contains("deltaInDouble") ? quoteJSON["deltaInDouble"].get<double>() : 0;
	//std::cout << "Setting gammaInDouble" << std::endl;
	newQuote.gammaInDouble = quoteJSON.contains("gammaInDouble") ? quoteJSON["gammaInDouble"].get<double>() : 0;
	//std::cout << "Setting thetaInDouble" << std::endl;
	newQuote.thetaInDouble = quoteJSON.contains("thetaInDouble") ? quoteJSON["thetaInDouble"].get<double>() : 0;
	//std::cout << "Setting vegaInDouble" << std::endl;
	newQuote.vegaInDouble = quoteJSON.contains("vegaInDouble") ? quoteJSON["vegaInDouble"].get<double>() : 0;
	//std::cout << "Setting rhoInDouble" << std::endl;
	newQuote.rhoInDouble = quoteJSON.contains("rhoInDouble") ? quoteJSON["rhoInDouble"].get<double>() : 0;
	//std::cout << "Setting changeInDouble" << std::endl;
	newQuote.changeInDouble = quoteJSON.contains("changeInDouble") ? quoteJSON["changeInDouble"].get<double>() : 0;
	//std::cout << "Setting 52WkHighInDouble" << std::endl;
	newQuote.fiftyTwoWkHighInDouble = quoteJSON.contains("52WkHighInDouble") ? quoteJSON["52WkHighInDouble"].get<double>() : 0;
	//std::cout << "Setting 52WkLowInDouble" << std::endl;
	newQuote.fiftyTwoWkLowInDouble = quoteJSON.contains("52WkLowInDouble") ? quoteJSON["52WkLowInDouble"].get<double>() : 0;
	//std::cout << "Setting futureIsTradable" << std::endl;
	newQuote.futureIsTradable = quoteJSON.contains("futureIsTradable") ? quoteJSON["futureIsTradable"].get<bool>() : false;
	//std::cout << "Setting futureIsActive" << std::endl;
	newQuote.futureIsActive = quoteJSON.contains("futureIsActive") ? quoteJSON["futureIsActive"].get<bool>() : false;
	//std::cout << "Setting inTheMoney" << std::endl;
	newQuote.inTheMoney = quoteJSON.contains("inTheMoney") ? quoteJSON["inTheMoney"].get<bool>() : false;
	//std::cout << "Setting isTradable" << std::endl;
	newQuote.isTradable = quoteJSON.contains("isTradable") ? quoteJSON["isTradable"].get<bool>() : false;
	//std::cout << "Setting marginable" << std::endl;
	newQuote.marginable = quoteJSON.contains("marginable") ? quoteJSON["marginable"].get<bool>() : false;
	//std::cout << "Setting shortable" << std::endl;
	newQuote.shortable = quoteJSON.contains("shortable") ? quoteJSON["shortable"].get<bool>() : false;
	//std::cout << "Setting realTimeEntitled" << std::endl;
	newQuote.realTimeEntitled = quoteJSON.contains("realTimeEntitled") ? quoteJSON["realTimeEntitled"].get<bool>() : false;
	//std::cout << "Setting delayed" << std::endl;
	newQuote.delayed = quoteJSON.contains("delayed") ? quoteJSON["delayed"].get<bool>() : false;

	return newQuote;
}
