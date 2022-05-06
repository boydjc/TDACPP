#include <iostream>
#include "../include/TDA.h"

int main() {

	TDA testTda;

	std::cout << "Enter a ticker to get price data for" << std::endl;
	
	std::string ticker;

	std::cout << ": ";
	getline(std::cin, ticker);

	// testing
	testTda.setHistPrice(ticker);

	for(int i=0; i<testTda.histPriceData.size(); i++) {
		std::cout << "Date: " << testTda.histPriceData[i].date << std::endl;
		std::cout << "Open: " << testTda.histPriceData[i].open << std::endl;
		std::cout << "High: " << testTda.histPriceData[i].high << std::endl;
		std::cout << "Low: " << testTda.histPriceData[i].low << std::endl;
		std::cout << "Close: " << testTda.histPriceData[i].close << std::endl;
		std::cout << "Volume: " << testTda.histPriceData[i].volume << std::endl;
	}
	return 0;
}
