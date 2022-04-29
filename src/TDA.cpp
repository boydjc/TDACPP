#include <iostream>
#include "../include/TDA.h"

TDA::TDA() {	
	std::cout << "TDA!" << std::endl;

	// set upi libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);

	// get the refresh token from environ variable
	if(std::getenv("TDA_REFRESH")){
		refreshToken = std::getenv("TDA_REFRESH");
	} else {
		std::cout << "ERROR! Could not find refresh token." << std::endl;
		std::cout << "Please make sure your refresh token is stored in an env variable named \"TDA_REFRESH\"" << std::endl;
	}


	if(std::getenv("TDA_ACCESS")) {
		accessToken = std::getenv("TDA_ACCESS");
	} else {
		std::cout << "ERROR! Could not find access token." << std::endl;
		std::cout << "Please make sure your access token is stored in an env variable named \"TDA_ACCESS\"" << std::endl;
	}

	testLibCurl();
}

TDA::~TDA() {
	// clean up libcurl
	curl_global_cleanup();
	std::cout << "DECONSTRUCTOR" << std::endl;
}

void TDA::testLibCurl(){
	std::cout << "Initializing curl" << std::endl;
	curl = curl_easy_init();

	if(curl) {
		std::cout << "Setting url opt" << std::endl;
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/");
		std::cout << "Setting callback function" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TDA::saveLibCurlRes);
		std::cout << "Setting callback variable" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resResults);
		
		/* Perform the request, res will get the return code */
		std::cout << "Performing request" << std::endl;
		res = curl_easy_perform(curl);

		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* always clean up */
		std::cout << "Cleaning up" << std::endl;
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


void TDA::createAccessToken() {

}

