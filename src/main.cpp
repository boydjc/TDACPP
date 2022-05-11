#include <iostream>
#include <chrono>
#include <thread>
#include "../include/TDA.h"

int main() {

	TDA testTda;

	std::cout << "Please choose an option" << std::endl;
	std::cout << "1. Get Historical Price" << std:: endl;
	std::cout << "2. Get Quote " << std::endl;
	std::cout << "3. Get Quotes " << std::endl;
	std::cout << "4. Monitor Mode" << std::endl;
	std::cout << "5. Paper Mode" << std::endl;

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

		std::cout << "Enter a ticker: " << std::endl;
	
		std::string ticker;

		std::cout << ": ";
		getline(std::cin, ticker);

		std::cout << "MONITOR MODE: Press CTRL+C TO STOP" << std::endl;
		
		bool stop = false;
	
		Quote lastQuote;
		Quote testQuote = testTda.getQuote(ticker);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	
		while(!(stop)) {
			testQuote = testTda.getQuote(ticker);

			if((testQuote.lastPrice != lastQuote.lastPrice) && (testQuote.bidPrice != lastQuote.bidPrice) &&
				(testQuote.askPrice != lastQuote.askPrice) && (testQuote.bidSize != lastQuote.bidSize) &&
				(testQuote.askSize != lastQuote.askSize)) {

				std::cout << "Last Price: " << testQuote.lastPrice << std::endl;
				std::cout << "Last Size: " << testQuote.lastSize << std::endl;
				std::cout << "Bid Price: " << testQuote.bidPrice << std::endl;
				std::cout << "Bid Size: " << testQuote.bidSize << std::endl;
				std::cout << "Ask Price: " << testQuote.bidPrice << std::endl;
				std::cout << "Ask Size: " << testQuote.askSize << std::endl;
				std::cout << "Total Volume: " << testQuote.totalVolume << std::endl;
				std::cout << "--------------------------------" << std::endl;
				lastQuote = testQuote;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));

		}
		
	}else if(userChoice == "5") {
		std::cout << "Paper Mode" << std::endl;

		std::cout << "Enter a ticker: " << std::endl;
	
		std::string ticker;

		std::cout << ": ";
		getline(std::cin, ticker);
	
		double money;
		std::string userInput;

		std::cout << "Enter starting amount" << std::endl;
		std::cout << ":";
		
		getline(std::cin, userInput);

		money = std::stof(userInput);

		int stockShares = 0;

		std::cout << "Money: $" << money << std::endl;
		if(stockShares != 0) {
			std::cout << "Shares of " << ticker << ": " << stockShares << std::endl;
		}
		std::cout << "-----------------------------------------" << std::endl;

		bool stop = false;
	
		Quote lastQuote;
		Quote testQuote = testTda.getQuote(ticker);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	
		while(!(stop)) {
						
			testQuote = testTda.getQuote(ticker);

			if((testQuote.lastPrice != lastQuote.lastPrice) && (testQuote.bidPrice != lastQuote.bidPrice) &&
				(testQuote.askPrice != lastQuote.askPrice) && (testQuote.bidSize != lastQuote.bidSize) &&
				(testQuote.askSize != lastQuote.askSize)) {

				std::cout << "Money: $" << money << std::endl;
				if(stockShares != 0) {
					std::cout << "Shares of " << ticker << ": " << stockShares << std::endl;
				}
				std::cout << "-----------------------------------------" << std::endl;

				std::cout << "Symbol: " << testQuote.symbol << std::endl;
				std::cout << "Last Price: " << testQuote.lastPrice << "\t";
				std::cout << "Bid Price: " << testQuote.bidPrice << "\t";
				std::cout << "Ask Price: " << testQuote.askPrice << std::endl;
				std::cout << "Last Size: " << testQuote.lastSize << "\t\t";	
				std::cout << "Bid Size: " << testQuote.bidSize << "\t\t";
				std::cout << "Ask Size: " << testQuote.askSize << std::endl << std::endl;

				
				
				// Play strategy
				// if we have an uptick with an increase in volume then we will buy
				// if we have some stock with we encounter a down tick then we will sell
				if(testQuote.lastPrice > lastQuote.lastPrice && testQuote.lastSize > lastQuote.lastSize) {
					std::cout << "Bought 1 share of " << ticker << std::endl;
					stockShares++;
					// buy at the ask
					money -= testQuote.askPrice;
				}else if(testQuote.lastPrice < lastQuote.lastPrice && stockShares != 0) {
					// sell at the bid
					std::cout << "Sold all shares of " << ticker << std::endl << std::endl;
					money += (stockShares * testQuote.bidPrice);
					stockShares = 0;
				}

				lastQuote = testQuote;

			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

	}
	return 0;
}
