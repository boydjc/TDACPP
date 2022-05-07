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

struct MutualFundQuote {
	std::string symbol;
	std::string desc;
	float closePrice;
	float netChange;
	float totalVolume;
	int64_t tradeTimeInLong;
	std::string exchange;
	std::string exchangeName;
	int digits;
	float fiftyTwoWeekHigh;
	float fiftyTwoWeekLow;
	int nAV;
	float peRation;
	float divAmount;
	float divYield;
	std::string divDate;
	std::string securityStatus;
};

struct FutureQuote {
	std::string symbol;
	double bidPriceInDouble;
	double askPriceInDouble;
	double lastPriceInDouble;
	std::string bidId;
	std::string askId;
	double highPriceInDouble;
	double lowPriceInDouble;
	double closePriceInDouble;
	std::string exchange;
	std::string description;
	std::string lastId;
	double openPriceInDouble;
	float changeInDouble;
	float futurePercentChange;
	std::string exchangeName;
	std::string securityStatus;
	int openInterest;
	float mark;
	float tick;
	int tickAmount;
	std::string product;
	std::string futurePriceFormat;
	std::string futureTradingHours;
	bool futureIsTradable;
	int futureMultiplier;
	bool futureIsActive;
	float futureSettlementPrice;
	std::string futureActiveSymbol;
	std::string futureExpirationDate;
};

struct FutureOptionQuote {
	std::string symbol;
	double bidPriceInDouble;
	double askPriceInDouble;
	double lastPriceInDouble;
	double closePriceInDouble;
	std::string description;
	double openPriceInDouble;
	double netChangeInDouble;
	int openInterest;
	std::string exchangeName;
	std::string securityStatus;
	double moneyIntrinsicValueInDouble;
	double multiplierInDouble;
	int digits;
	double strikePriceInDouble;
	std::string contractType;
	std::string underlying;
	double timeValueInDouble;
	double deltaInDouble;
	double gammaInDouble;
	double thetaInDouble;
	double vegaInDouble;
	double rhoInDouble;
	float mark;
	float tick;
	int tickAmount;
	bool futureIsTradable;
	std::string futureTradingHours;
	float futurePercentChange;
	bool futureIsActive;
	int futureExpirationDate;
	std::string expirationType;
	std::string exerciseType;
	bool inTheMoney;
};

struct IndexQuote {
	std::string symbol;
	std::string description;
	float lastPrice;
	float openPrice;
	float highPrice;
	float lowPrice;
	float closePrice;
	float netChange;
	int totalVolume;
	long tradeTimeInLong;
	std::string exchange;
	std::string exchangeName;
	int digits;
	float fiftyWkHigh;
	float fiftyWkLow;
	std::string securityStatus;
};

struct OptionQuote {
	std::string symbol;
	std::string description;
	float bidPrice;
	int bidSize;
	float askPrice;
	int askSize;
	float lastPrice;
	int lastSize;
	float openPrice;
	float highPrice;
	float lowPrice;
	float closePrice;
	float netChange;
	int totalVolume;
	long quoteTimeInLong;
	long tradeTimeInLong;
	float mark;
	int openInterest;
	float volatility;
	float moneyIntrinsicValue;
	int multiplier;
	float strikePrice;
	std::string contractType;
	std::string underlying;
	float timeValue;
	std::string deliverables;
	float delta;
	float gamma;
	float theta;
	float vega;
	float rho;
	std::string securityStatus;
	float theoreticalOptionValue;
	float underlyingPrice;
	std::string uvExpirationType;
	std::string exchange;
	std::string exchangeName;
	std::string settlementType;
};

struct ForexQuote {
	std::string symbol;
	double bidPriceInDouble;
	double askPriceInDouble;
	double lastPriceInDouble;
	double highPriceInDouble;
	double lowPriceInDouble;
	double closePriceInDouble;
	std::string exchange;
	std::string description;
	double openPriceInDouble;
	double changeInDouble;
	float percentChange;
	std::string exchangeName;
	int digits;
	std::string securityStatus;
	float tick;
	int tickAmount;
	std::string product;
	std::string tradingHours;
	bool isTradable;
	std::string marketMaker;
	double fiftyTwoWkHighInDouble;
	double fiftyTwoWkLowInDouble;
	float mark;
};

struct ETFQuote {
	std::string symbol;
	std::string description;
	float bidPrice;
	int bidSize;
	std::string bidId;
	float askPrice;
	int askSize;
	std::string askId;
	float lastPrice;
	int lastSize;
	std::string lastId;
	float openPrice;
	float highPrice;
	float lowPrice;
	float closePrice;
	float netChange;
	int totalVolume;
	long quoteTimeInLong;
	long tradeTimeInLong;
	float mark;
	std::string exchange;
	std::string exchangeName;
	bool marginable;
	bool shortable;
	float volatility;
	int digits;
	float fiftyTwoWkHigh;
	float fiftyTwoWkLow;
	float peRatio;
	float divAmount;
	float divYield;
	std::string divDate;
	std::string securityStatus;
	float regularMarketLastPrice;
	int regularMarketLastSize;
	float regularMarketNetChange;
	long regularMarketTradeTimeInLong;
};

struct EquityQuote {
	std::string symbol;
	std::string description;
	float bidPrice;
	int bidSize;
	float askPrice;
	int askSize;
	std::string askId;
	float lastPrice;
	int lastSize;
	std::string lastId;
	float openPrice;
	float highPrice;
	float lowPrice;
	float closePrice;
	float netChange;
	int totalVolume;
	long quoteTimeInLong;
	long tradeTimeInLong;
	float mark;
	std::string exchange;
	std::string exchangeName;
	bool marginable;
	bool shortable;
	float volatility;
	int digits;
	float fiftyTwoWkHigh;
	float fiftyTwoWkLow;
	float peRatio;
	float divAmount;
	float divYield;
	std::string divDate;
	std::string securityStatus;
	float regularMarketLastPrice;
	int regularMarketLastSize;
	float regularMarketNetChange;
	long regularMarketTradeTimeInLong;
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

		void setHistPrice(std::string ticker, std::string periodType="day",
					  	  std::string period="", std::string freqType="",
					  	  std::string freq="", unsigned int endDate=0,
					  	  unsigned int startDate=0, bool extHourData=true);

		/* END set & get functions */

		std::vector<Candle> histPriceData;

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
