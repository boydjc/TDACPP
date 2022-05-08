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

	testTda.getQuote(ticker);

	return 0;
}
