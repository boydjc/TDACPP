#pragma once

#include <curl/curl.h>

#include "../include/json.hpp"

struct Candle {
	std::string date;
	float open;
	float high;
	float low;
	float close;
	unsigned long volume;
};


/* Not all of these will be used at once. It depends on the type of security 
	that you are getting the quote for */
struct Quote {
	std::string symbol = "";
	std::string assetMainType = "";
	std::string assetSubType = "";
	std::string assetType = "";
	std::string exchange = "";
	std::string exchangeName = "";
	std::string divDate = "";
	std::string securityStatus = "";
	std::string bidId = "";
	std::string askId = "";
	std::string description = "";
	std::string lastId = "";
	std::string product = "";
	std::string futurePriceFormat = "";
	std::string futureTradingHours = "";
	std::string futureActiveSymbol = "";
	std::string futureExpirationDate = "";
	std::string contractType = "";
	std::string underlying = "";
	std::string expirationType = "";
	std::string exerciseType = "";
	std::string deliverables = "";
	std::string uvExpirationType = "";
	std::string settlementType = "";
	std::string tradingHours = "";
	std::string marketMaker = "";
	std::string cusip = "";
	float lastPrice = 0.00;
	float openPrice = 0.00;
	float highPrice = 0.00;
	float lowPrice = 0.00;
	float closePrice = 0.00;
	float bidPrice = 0.00;
	float netChange = 0.00;
	float fiftyTwoWeekHigh = 0.00;
	float fiftyTwoWeekLow = 0.00;
	float peRatio = 0.00;
	float divAmount = 0.00;
	float divYield = 0.00;
	float futurePercentChange = 0.00;
	float moneyIntrinsicValue = 0.00;
	float mark = 0.00;
	float tick = 0.00;
	float fiftyWkHigh = 0.00;
	float fiftyWkLow = 0.00;
	float askPrice = 0.00;
	float volatility = 0.00;
	float futureSettlementPrice = 0.00;
	float strikePrice = 0.00;
	float timeValue = 0.00;
	float delta = 0.00;
	float gamma = 0.00;
	float theta = 0.00;;
	float vega = 0.00;
	float rho = 0.00;
	float theoreticalOptionValue = 0.00; 
	float underlyingPrice = 0.00;
	float percentChange = 0.00;
	float regularMarketLastPrice = 0.00;
	float regularMarketNetChange = 0.00;
	int digits = 0;
	int nAV = 0;
	int openInterest = 0;
	int futureMultiplier = 0;
	int tickAmount = 0;
	int totalVolume = 0;
	int bidSize = 0;
	int askSize = 0;
	int lastSize = 0;
	int multiplier = 0;
	int regularMarketLastSize = 0;
	long tradeTimeInLong = 0;
	long quoteTimeInLong = 0;
	long bidSizeInLong = 0;
	long lastSizeInLong = 0;
	long askSizeInLong = 0;
	long regularMarketTradeTimeInLong = 0;
	double bidPriceInDouble = 0.00;
	double askPriceInDouble = 0.00;
	double lastPriceInDouble = 0.00;
	double highPriceInDouble = 0.00;
	double lowPriceInDouble = 0.00;
	double closePriceInDouble = 0.00;
	double openPriceInDouble = 0.00;
	double netChangeInDouble = 0.00;
	double moneyIntrinsicValueInDouble = 0.00;
	double markChangeInDouble = 0.00;
	double markPercentChangeInDouble = 0.00;
	double netPercentChangeInDouble = 0.00;
	double regularMarketPercentChangeInDouble = 0.00;
	double multiplierInDouble = 0.00;
	double strikePriceInDouble = 0.00;
	double timeValueInDouble = 0.00;
	double deltaInDouble = 0.00;
	double gammaInDouble = 0.00;
	double thetaInDouble = 0.00;
	double vegaInDouble = 0.00;
	double rhoInDouble = 0.00;
	double changeInDouble = 0.00;
	double fiftyTwoWkHighInDouble = 0.00;
	double fiftyTwoWkLowInDouble = 0.00;
	bool futureIsTradable = false;
	bool futureIsActive = false;
	bool inTheMoney = false;
	bool isTradable = false;
	bool marginable = false;
	bool shortable = false;
	bool realTimeEntitled = false;
	bool delayed = false;
};

class TDA {

	public:
		TDA();
		~TDA();

		/* BEGIN set & get functions */

		
		/*  setHistPrice()
			
			Gets the historic price for a given ticker

			params:

			ticker - ticker symbol of company

			periodType - The type of period to show. Valid values are day, month, year, or ytd (year to date). 
						 Default is day.

			period - The number of periods to show.

					 Example: For a 2 day / 1 min chart, the values would be:

					 period: 2
					 periodType: day
					 frequency: 1
					 frequencyType: min

					 Valid periods by periodType (defaults marked with an asterisk):

					 day: 1, 2, 3, 4, 5, 10*
					 month: 1*, 2, 3, 6
					 year: 1*, 2, 3, 5, 10, 15, 20
					 ytd: 1*

			freqType - The type of frequency with which a new candle is formed.

					   Valid frequencyTypes by periodType (defaults marked with an asterisk):

					   day: minute*
					   month: daily, weekly*
					   year: daily, weekly, monthly*
					   ytd: daily, weekly*

			freq - The number of the frequencyType to be included in each candle.

				   Valid frequencies by frequencyType (defaults marked with an asterisk):

				   minute: 1*, 5, 10, 15, 30
				   daily: 1*
				   weekly: 1*
				   monthly: 1*

			endDate - End date as milliseconds since epoch. If startDate and endDate are provided, period should not be provided. 
					  Default is previous trading day.

			startDate - Start date as milliseconds since epoch. If startDate and endDate are provided, period should not be provided.

			extHourData - true to return extended hours data, false for regular market hours only. Default is true */

		std::vector<Candle> getHistPrice(std::string ticker, std::string periodType="day",
					  	  std::string period="", std::string freqType="",
					  	  std::string freq="", unsigned int endDate=0,
					  	  unsigned int startDate=0, bool extHourData=true);

		Quote getQuote(std::string symbol);
		std::vector<Quote> getQuotes(std::string symbols);

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

		std::vector<Candle> histPriceData;
		std::vector<Quote> quotes;

		void readConfig();
		void saveConfig();
		void sendReq();
		static size_t saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s);
		void createAccessToken(bool refresh=false);
		bool checkAccessExpire();

		Quote createQuote(nlohmann::json quoteJSON);
		
};
