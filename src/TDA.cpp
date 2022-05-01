#include <iostream>
#include <sstream>
#include "../include/TDA.h"

TDA::TDA() {	
	std::cout << "TDA!" << std::endl;

	// set up libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);

	readEnvVars();	

	createAccessToken();
}

TDA::~TDA() {
	// clean up libcurl
	curl_global_cleanup();
	std::cout << "TDA DECONSTRUCTOR" << std::endl;
}

// Retrieves the values for the refresh, access tokens, and the client id
// through environment variables on the system
void TDA::readEnvVars() {
	// get the refresh token from environ variable
	if(std::getenv("TDA_REFRESH")){
		refreshToken = std::getenv("TDA_REFRESH");
	} else {
		std::cout << "ERROR! Could not find refresh token." << std::endl;
		std::cout << "Please make sure your refresh token is stored in an env variable named \"TDA_REFRESH\"" << std::endl;
	}

	// get the access token from environ variable
	if(std::getenv("TDA_ACCESS")) {
		accessToken = std::getenv("TDA_ACCESS");
	} else {
		std::cout << "ERROR! Could not find access token." << std::endl;
		std::cout << "Please make sure your access token is stored in an env variable named \"TDA_ACCESS\"" << std::endl;
	}

	// get the client id from environ variable
	if(std::getenv("TDA_CLIENT")) {
		clientId = std::getenv("TDA_CLIENT");
	} else {
		std::cout << "ERROR! Could not find client id." << std::endl;
		std::cout << "Please make sure your client id is stored in an env variable named \"TDA_CLIENT\"" << std::endl;
	}

}

int TDA::sendReq(){
	std::cout << "Initializing curl" << std::endl;

	int reqStatus = 0;
	curl = curl_easy_init();

	if(curl) {
		std::cout << "Setting url opt" << std::endl;
		std::cout << reqUrl << std::endl;
		curl_easy_setopt(curl, CURLOPT_URL, reqUrl.c_str());
		
		/* Setting Headers */
		struct curl_slist *headers=NULL;

		headers = curl_slist_append(headers, "Accept-Encoding: gzip");
		headers = curl_slist_append(headers, "Accept-Language: en-US");
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

		/* Setting post fields if we are doing a post request */
		if(postData != "") {
			std::cout << "Setting post fields" << std::endl;
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		}

		std::cout << "Setting callback function" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, TDA::saveLibCurlRes);
		std::cout << "Setting callback variable" << std::endl;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resResults);

		// for debugging
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		
		/* Perform the request, res will get the return code */
		std::cout << "Performing request" << std::endl;
		res = curl_easy_perform(curl);

		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			reqStatus = 1;
		}

		std::cout << "REQUEST RESULT: " << res << std::endl;

		/* always clean up */
		std::cout << "Cleaning up" << std::endl;
		curl_easy_cleanup(curl);
	}
	
	return reqStatus;
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

	reqUrl = "https://api.tdameritrade.com/v1/oauth2/token";

	// using curl here to encode the refresh token
	curl = curl_easy_init();

	std::string refreshEncode = curl_easy_escape(curl, refreshToken.c_str(), refreshToken.length());

	postData = "grant_type=refresh_token&refresh_token=" + refreshEncode + "&access_type=&code=&client_id=" + clientId + "&redirect_uri=";

	curl_easy_cleanup(curl);

	int reqStatus = sendReq();

	if(reqStatus == 0) {
		std::cout << "SUCCESS" << std::endl;
		std::cout << "New Access Token: " << resResults << std::endl;
	} else {
		std::cout << "Access Token Request Failed" << std::endl;
	}
	
}

