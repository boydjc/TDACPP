#include <iostream>
#include <chrono>
#include <thread>
#include "TDA.h"

int main() {

	TDA testTda;

	std::cout << "Please choose an option" << std::endl;
	std::cout << "1. Get Historical Price" << std:: endl;
	std::cout << "2. Get Quote " << std::endl;
	std::cout << "3. Get Quotes " << std::endl;
	std::cout << "4. Place Order" << std::endl;

	std::string userChoice;
	std::cout << ": ";
	getline(std::cin, userChoice);

	if(userChoice == "1") {

		std::cout << "Enter a ticker: " << std::endl;
	
		std::string ticker;

		std::cout << ": ";
		getline(std::cin, ticker);

		// testing
		std::vector<Candle> histPriceData = testTda.getHistPrice(ticker);

		for(int i=0; i<5; i++) {
			std::cout << "Date: " << histPriceData[i].date << std::endl;
			std::cout << "Open: " << histPriceData[i].open << std::endl;
			std::cout << "High: " << histPriceData[i].high << std::endl;
			std::cout << "Low: " << histPriceData[i].low << std::endl;
			std::cout << "Close: " << histPriceData[i].close << std::endl;
			std::cout << "Volume: " << histPriceData[i].volume << std::endl;
			std::cout << "---------------------------------" << std::endl;
		}
	
	} else if(userChoice == "2") {

		std::cout << "Enter a ticker: " << std::endl;
	
		std::string ticker;

		std::cout << ": ";
		getline(std::cin, ticker);


		Quote testQuote = testTda.getQuote(ticker);

		std::cout << "Quote-Time: " << testQuote.quoteTimeInLong << std::endl;
		std::cout << "Symbol: " << testQuote.symbol << std::endl;
		std::cout << "Last Price: " << testQuote.lastPrice << std::endl;
		std::cout << "Last Size: " << testQuote.lastSize << std::endl;
		std::cout << "Bid Price: " << testQuote.bidPrice << std::endl;
		std::cout << "Bid Size: " << testQuote.bidSize << std::endl;
		std::cout << "Ask Price: " << testQuote.bidPrice << std::endl;
		std::cout << "Ask Size: " << testQuote.askSize << std::endl;
		std::cout << "Total Volume: " << testQuote.totalVolume << std::endl;
	} else if(userChoice == "3") {

		std::cout << "Enter some tickers: " << std::endl;
	
		std::string tickers;

		std::cout << ": ";
		getline(std::cin, tickers);

		std::vector<Quote> testQuotes = testTda.getQuotes(tickers);

		for(int i=0; i<testQuotes.size(); i++) {
			std::cout << "Symbol: " << testQuotes[i].symbol << std::endl;
			if(testQuotes[i].assetMainType != "FOREX") {
				std::cout << "Last Price: " << testQuotes[i].lastPrice << std::endl;
				std::cout << "Last Size: " << testQuotes[i].lastSize << std::endl;
				std::cout << "Bid Price: " << testQuotes[i].bidPrice << std::endl;
				std::cout << "Bid Size: " << testQuotes[i].bidSize << std::endl;
				std::cout << "Ask Price: " << testQuotes[i].bidPrice << std::endl;
				std::cout << "Ask Size: " << testQuotes[i].askSize << std::endl;
				std::cout << "Total Volume: " << testQuotes[i].totalVolume << std::endl;
				std::cout << "--------------------------------" << std::endl;
			} else {
				std::cout << "Last Price: " << testQuotes[i].lastPriceInDouble << std::endl;
				std::cout << "Last Size: " << testQuotes[i].lastSizeInLong << std::endl;
				std::cout << "Bid Price: " << testQuotes[i].bidPriceInDouble << std::endl;
				std::cout << "Bid Size: " << testQuotes[i].bidSizeInLong << std::endl;
				std::cout << "Ask Price: " << testQuotes[i].bidPriceInDouble << std::endl;
				std::cout << "Ask Size: " << testQuotes[i].askSizeInLong << std::endl;
				std::cout << "--------------------------------" << std::endl;
			}
		}
	} else if(userChoice == "4") {
		testTda.placeOrder();
	}
	return 0;
}
