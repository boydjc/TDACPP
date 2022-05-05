#include <iostream>
#include "../include/TDA.h"

int main() {

	std::cout << "Testing C++" << std::endl;
	
	PriceData test;
	test.date = "2022-01-10";
	test.open = 10.10;
	test.high = 10.20;
	test.low = 10.02;
	test.close = 10.35;
	test.volume = 14500000;

	std::cout << "Price Data" << std::endl;
	std::cout << "--------------------------" << std::endl;
	std::cout << "Date: " << test.date << std::endl;
	std::cout << "Open: " << test.open << std::endl;
	std::cout << "High: " << test.high << std::endl;
	std::cout << "Low: " << test.low << std::endl;
	std::cout << "Close: " << test.close << std::endl;
	std::cout << "Volume: " << test.volume << std::endl;

	//TDA testTda;

	//testTda.getHistPrice("AAPL");

	return 0;
}
