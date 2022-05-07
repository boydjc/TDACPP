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
	std::string symbol;
	std::string exchange;
	std::string exchangeName;
	std::string divDate;
	std::string securityStatus;
	std::string bidId;
	std::string askId;
	std::string description;
	std::string lastId;
	std::string product;
	std::string futurePriceFormat;
	std::string futureTradingHours;
	std::string futureActiveSymbol;
	std::string futureExpirationDate;
	std::string contractType;
	std::string underlying;
	std::string expirationType;
	std::string exerciseType;
	std::string deliverables;
	std::string uvExpirationType;
	std::string settlementType;
	std::string tradingHours;
	std::string marketMaker;
	float lastPrice;
	float openPrice;
	float highPrice;
	float lowPrice;
	float closePrice;
	float bidPrice;
	float netChange;
	float fiftyTwoWeekHigh;
	float fiftyTwoWeekLow;
	float peRation;
	float divAmount;
	float divYield;
	float futurePercentChange;
	float moneyIntrinsicValue;
	float mark;
	float tick;
	float fiftyWkHigh;
	float fiftyWkLow;
	float askPrice;
	float volatility;
	float futureSettlementPrice;
	float strikePrice;
	float timeValue;
	float delta;
	float gamma;
	float theta;
	float vega;
	float rho;
	float theoreticalOptionValue;
	float underlyingPrice;
	float percentChange;
	float regularMarketLastPrice;
	float regularMarketNetChange;
	int digits;
	int nAV;
	int openInterest;
	int futureMultiplier;
	int tickAmount;
	int futureExpirationDateInt;
	int totalVolume;
	int bidSize;
	int askSize;
	int lastSize;
	int multiplier;
	int regularMarketLastSize;
	long tradeTimeInLong;
	long quoteTimeInLong;
	long regularMarketTradeTimeInLong;
	double bidPriceInDouble;
	double askPriceInDouble;
	double lastPriceInDouble;
	double highPriceInDouble;
	double lowPriceInDouble;
	double closePriceInDouble;
	double openPriceInDouble;
	double netChangeInDouble;
	double moneyIntrinsicValueInDouble;
	double multiplierInDouble;
	double strikePriceInDouble;
	double timeValueInDouble;
	double deltaInDouble;
	double gammaInDouble;
	double thetaInDouble;
	double vegaInDouble;
	double rhoInDouble;
	double changeInDouble;
	double fiftyTwoWkHighInDouble;
	double fiftyTwoWkLowInDouble;
	bool futureIsTradable;
	bool futureIsActive;
	bool inTheMoney;
	bool isTradable;
	bool marginable;
	bool shortable;
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

		void readConfig();
		void saveConfig();
		void sendReq();
		static size_t saveLibCurlRes(void *buffer, size_t size, size_t nmemb, std::string *s);
		void createAccessToken(bool refresh=false);
		bool checkAccessExpire();
		
};
