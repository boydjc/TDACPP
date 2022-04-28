#include <iostream>
#include "../include/TDA.h"

TDA::TDA() {	
	std::cout << "TDA!" << std::endl;
	testLibCurl();
}

void TDA::testLibCurl(){
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/");

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		/* Check for errors */

		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* always clean up */

		curl_easy_cleanup(curl);
		curl_global_cleanup();
		std::cout << "Success!" << std::endl;
	}
}

