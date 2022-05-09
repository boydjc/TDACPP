#include <iostream>
#include "../include/TDA.h"

int main() {

	TDA testTda;

	std::cout << "Enter a ticker: " << std::endl;
	
	std::string ticker;

	std::cout << ": ";
	getline(std::cin, ticker);

	// testing
	/*std::vector<Candle> histPriceData = testTda.getHistPrice(ticker);

	for(int i=0; i<5; i++) {
		std::cout << "Date: " << histPriceData[i].date << std::endl;
		std::cout << "Open: " << histPriceData[i].open << std::endl;
		std::cout << "High: " << histPriceData[i].high << std::endl;
		std::cout << "Low: " << histPriceData[i].low << std::endl;
		std::cout << "Close: " << histPriceData[i].close << std::endl;
		std::cout << "Volume: " << histPriceData[i].volume << std::endl;
		std::cout << "---------------------------------" << std::endl;
	}*/

	Quote testQuote = testTda.getQuote(ticker);

	std::cout << "Symbol: " << testQuote.symbol << std::endl;
	std::cout << "Last Price: " << testQuote.lastPrice << std::endl;
	std::cout << "Last Size: " << testQuote.lastSize << std::endl;
	std::cout << "Bid Price: " << testQuote.bidPrice << std::endl;
	std::cout << "Bid Size: " << testQuote.bidSize << std::endl;
	std::cout << "Ask Price: " << testQuote.bidPrice << std::endl;
	std::cout << "Ask Size: " << testQuote.askSize << std::endl;
	std::cout << "Total Volume: " << testQuote.totalVolume << std::endl;

	return 0;
}
