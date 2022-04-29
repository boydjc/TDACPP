#include <iostream>
#include "../include/TDA.h"

TDA::TDA() {	
	std::cout << "TDA!" << std::endl;

	// get the refresh token from environ variable
	if(std::getenv("TDA_REFRESH")){
		std::cout << "REFRESH TOKEN FOUND!" << std::endl;
		std::cout << std::getenv("TDA_REFRESH") << std::endl;
	} else {
		std::cout << "ERROR! Could not find refresh token." << std::endl;
		std::cout << "Please make sure your refresh token is stored in an env variable named \"TDA_REFRESH\"" << std::endl;
	}

	//testLibCurl();
}

void TDA::testLibCurl(){
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TDA::saveLibCurlRes);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resResults);
		
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* always clean up */
		curl_easy_cleanup(curl);
		curl_global_cleanup();
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

