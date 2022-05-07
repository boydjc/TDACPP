# TDACPP

A class for interacting with TD Ameritrades API

## Table of Contents
<a href="#getting-started">Getting Started</a><br>
<a href="#how-to-use">How to use</a><br>
<a href="#todo-api-implementations">TODO API Implementations</a>

## Getting Started

<sub><i>The info for getting started was taken from https://developer.tdameritrade.com/content/authentication-faq and https://developer.tdameritrade.com/content/phase-1-authentication-update-xml-based-api. Check there if you run into problems.</i></sub>

Before using this you will need to have an account with TD Ameritrade for stock trading. You will also need to create a developer account at https://developer.tdameritrade.com/. Your developer account is separate from your trading account. After creating a developer account and logging in, the next step is to register a new app. Once you have an app registered, that app will have a unique Consumer Key.

Once you have your app consumer key, you will invoke the authentication window in the browser with the URL 

```
https://auth.tdameritrade.com/auth?response_type=code&redirect_uri={Callback URL}&client_id={Consumer Key}@AMER.OAUTHAP
```

For personal use your callback url can be localhost (https://127.0.0.1:8080). The url should look something like this:

```
https://auth.tdameritrade.com/auth?response_type=code&redirect_uri=https%3A%2F%2F127.0.0.1%3A8080&client_id=ABC1234%40AMER.OAUTHAP
```

If the url is correct you should be redirected to a TD Ameritrade login screen. On this screen you will <strong>login with your TD Ameritrade trading account</strong>. After logging in, it will redirect to an error page, however, the url you are at should have a long string of text after `code=`

It should look something like the following:

```
https://127.0.0.1/?code=zU9cQBabeNVOa6e7KsZIaR3S8F0dJvz8pj0WxJA0hYFMVNokRY3xSuQDnSCsnGDZD4DjEKbjayXyDOdo6hahoBShvGnc7LG9p9Eu1nZaIYruOviUb532ir8DeH9ZFegIta3raPzXtNlSeUPiV5Y2vvEcvmVWF0OjlbOvwrlf7lA9cKMpfXPSU8eJ7xtP6Lx9LA2FBKjhf8hTx3CAsLZF7GWbunm2wiQOeiLYca04CPi6VhKhhkBt46dDgXfVd15tIDPsfzDJzEPMLqDKvbBBOoMAHwqvY6CgbQs8OCRcIpYLhyAa5oFNYu8ekmB7DuD0BThLT3cSDkELib5k2U3RoQxsfFcQbjg8ziDUlupK72KnvGEeEW2k2nvNUMmvhMBVAJPf9LIYYj68ffsEsPehHFtDTzsLKxYuYQYJqcyqEPrFqSnaJffpBKccULvVBnaCSpjN4l0vtR7SZwWc2PhvTZJAdOTXDLniyDEnQyd2UwWivdTBNiAvjfJOROeFJxSA6Qpz5d2gTlbpxRR7TpMgPrZpnGAR3d5scFLdghd7gXybIB5sFYDmuHUZtNpAcmcAtH045gYTMxBAV2wfzgR7VbkoOo1KIWCegwcGnXBd6wElvyDxAeRRG86I4Zwl2z5Eq4O9NKnzXFZn93VN1C7lYsq0ueEhCxFH2NXhG8GsqTN1vCmX4CFH2ZwS3QZjABqVyDqN4CY49MThqwfl9siyIF5M7rN6fYPJAu5jFvvPXEWOPQTv9SLhnFx9yCfgVfdHfQMYZ33FTguzcb76cn6aRyJOHwiN69c4aEAaGNb1vXIXM8nbQU449aaxq2Jdpidz2MCfFcvJU682tcorBJIlClwFcKU42W6yxJKQNt0Dxft8mn9F8TvQVaPabmsepix4CKgnzUv87KIkdEbK3N1aRGa2Hgqz4HZim2E0IKWfJ2Qtmcz1gqa0egYxOF6hR4lGG5BlNwqeXgTwszcfMRtm60rCRhnAHSPYdj4Q9pU4
```

Copy everything after `code=` and push it through a urldecoder like the one at https://www.urldecoder.org/

This is your authentication code to get your first refresh token.

In the final step, go to https://developer.tdameritrade.com/authentication/apis/post/token-0 and fill out the form with the following information:

```
grant_type: authorization_code
refresh_token: blank
access_type: blank
code: {URL Decoded Auth Code}
client_id: {Consumer Key}
redirect_uri: {Callback URL} (https://127.0.0.1:8080 if thats what you used earlier)
```

If everything goes well, then you should see a 200 response code followed by your refresh token. Congratz! 🥳 You're ready to start using TDACPP.

In order for the class to begin working, the initial API refresh token that is first created through <a href="#getting-started">Getting Started</a> and client id (also known as the consumer key of your developer app) need to be stored in a file named config.json in the parent directory of your project. The file needs to look like the following:

```
{
  "refresh_token": "Ucy8sYKEZhcpTRqQGgi6LNY1uzplV2iwHl42rkcjlGnkbMIm4Q0Ihc05NrZZFmNhm2js67E6m72qpoGND6sKQ9aOqYSIZxpa5KaFG8FBVhomst48Ztq
0yaRDJ8PlJNd0Z4i0WUslJXAURV2Fat7QtCNfG7CysOFdidNO0Cnhvpf513VGsvEB9s0PQX7glo2s7LtH79Hc0S4u5A7FoTT2ozgnUiLTR91jeRsHydTH6vCb1R1f2ovo4mLWzf4Pj3HZ
IAvfrrA9tpRmyAQpba270gMmbOU4kQn8XckbIU19P2GZFU3spWNRgJobx2biqMEgVM8fVVNsQ2Nd406ZiJN3F4eAjw7V2v8u7PDHkoaRxGpw8abVpJiAj3bsNeQMV69M5YvzurUKEFhiC
pmB7f8vRkNLBxreHEGKUtV0iqdJ6TkjUsHccYwOZSvzkFgJk1xviLhsff6k3zXXkzYVq1qolTszQ9GYwDgieebQPuXr4WdnfBbrtFqaKhPoLTOGt87LOgm0l610ikmhqKF71arOfTeMj
Woi8BvbM4oUwmMNIYGFzsNxJ3lyvd8GNo3bCAvBc5trKmr8mzurYXNlcktHXiRjMA4HuVrZY0aSwgE8d3XNAICtQVUW8Cu2s1945bEFoY2xqfzLIfayHdf6y2ZqJoZyj3QrKypnh1Vi8s
hc8ULoUUdPiDZSGE8y1SycOVhio0OAhsoz1zWv8KVaO8r4jZSdJ405zTYt5GelO31NaQ0XNr18IqxQszqROjAJzHjkr8URhGie01ERY1olfGSRjwAvEwSDNprVN8fdnLZJgq5eHIvR
jDtB0Mrxjov3YEosFS10nQrQXLqub5LtCvYrqgEZjJXxVcsNNmg1WKmBjDtzGeXa9e5D3EGoTSFP61TnN0ZCSFaWmnisYfRAYj",

  "client_id": "CNLSNSEQHSWSGTEINUKLCNLSNSE"
}

```

- The refresh token does not need to be encoded before storing in config.json. The class will encode it for you before sending requests

- There is no need to manually keep track of when a refresh token or access token is created. The class will check the access token and refresh token creation dates and make new ones when it's time. However, if you go more than 90 days without running the class and your refresh token expires, you will need to manually create one again and insert it into config.json.


## How to use

<strong>Getting Price History</strong>

```C++
void setHistPrice(std::string ticker, std::string periodType, 
		  std::string period, std::string freqType,
		  std::string freq, unsigned int endDate,
		  unsigned int startDate, bool extHourData)
```

Corresonds to <a href="https://developer.tdameritrade.com/price-history/apis/get/marketdata/%7Bsymbol%7D/pricehistory">Get Price History</a>.

<details><summary>Click for Params</summary>

-	ticker
	
	<i>ticker symbol of company</i>

-	periodType

	<i>The type of period to show. Valid values are day, month, year, or ytd (year to date). 
	   Default is day</i>

-	period

	<i>The number of periods to show.

	Example: For a 2 day / 1 min chart, the values would be:
					 					
	period: 2<br>
	periodType: day<br>			
	frequency: 1<br>
	frequencyType: min<br>

	Valid periods by periodType (defaults marked with an asterisk):

	day: 1, 2, 3, 4, 5, 10*<br>
	month: 1*, 2, 3, 6<br>
	year: 1*, 2, 3, 5, 10, 15, 20<br>
	ytd: 1*<br></i>

-	freqType

	<i>The type of frequency with which a new candle is formed.

	Valid frequencyTypes by periodType (defaults marked with an asterisk):

	day: minute*<br>
	month: daily, weekly*<br>
	year: daily, weekly, monthly*<br>
	ytd: daily, weekly*<br></i>

-	freq

	<i>The number of the frequencyType to be included in each candle.

	Valid frequencies by frequencyType (defaults marked with an asterisk):

	minute: 1*, 5, 10, 15, 30<br>
	daily: 1*<br>
	weekly: 1*<br>
	monthly: 1*<br></i>

-	endDate

	<i>End date as milliseconds since epoch. If startDate and endDate are provided, period should not be provided. 
	   Default is previous trading day.</i>

-	startDate

	<i>Start date as milliseconds since epoch. If startDate and endDate are provided, period should not be provided.</i>

-	extHourData

	<i>true to return extended hours data, false for regular market hours only. Default is true</i>
</details>

The price data retrieved will be stored in a public `std::vector<Candle>` variable named `histPriceData` to be accessed. The Candle struct has the following attribues:

```C++
struct Candle {
	std::string date;
	float open;
	float high;
	float low;
	float close;
	unsigned long volume;
};
```

## Examples

<strong>Getting Price History</strong>
```C++
#include "../include/TDA.h"

int main() {
    
    TDA testTDA;
    
    testTDA.setHistPrice("AAPL");
    
    for(int i=0; i<5; i++) {
	std::cout << "Date: " << testTda.histPriceData[i].date << std::endl;
	std::cout << "Open: " << testTda.histPriceData[i].open << std::endl;
	std::cout << "High: " << testTda.histPriceData[i].high << std::endl;
	std::cout << "Low: " << testTda.histPriceData[i].low << std::endl;
	std::cout << "Close: " << testTda.histPriceData[i].close << std::endl;
	std::cout << "Volume: " << testTda.histPriceData[i].volume << std::endl;
	std::cout << "---------------------------------" << std::endl;
    }
    return 0;
}
```
<details><summary>Click for Output</summary>
Date: 2022-04-22<br>
Open: 166<br>
High: 166.09<br>
Low: 166<br>
Close: 166.09<br>
Volume: 979<br>
---------------------------------<br>
Date: 2022-04-22<br>
Open: 166.01<br>
High: 166.01<br>
Low: 166<br>
Close: 166<br>
Volume: 1990<br>
---------------------------------<br>
Date: 2022-04-22<br>
Open: 165.96<br>
High: 165.96<br>
Low: 165.93<br>
Close: 165.93<br>
Volume: 1163<br>
---------------------------------<br>
Date: 2022-04-22<br>
Open: 166.06<br>
High: 166.07<br>
Low: 166.05<br>
Close: 166.05<br>
Volume: 1171<br>
---------------------------------<br>
Date: 2022-04-22<br>
Open: 166.05<br>
High: 166.05<br>
Low: 166.05<br>
Close: 166.05<br>
Volume: 110<br>
---------------------------------<br>
</details>


## TODO API Implementations

<details><summary>Click for Details</summary>

#### Accounts and Trading
	
- [ ] Cancel Order
- [ ] Get Order
- [ ] Get Orders By Path
- [ ] Get Orders By Query
- [ ] Place Order
- [ ] Replace Order

- [ ] Create Saved Order
- [ ] Delete Saved Order
- [ ] Get Saved Order
- [ ] Get Saved Orders By Path
- [ ] Replace Saved Order

- [ ] Get Account
- [ ] Get Accounts

#### Authentication
- [X] Post Access Token

#### Instruments
- [ ] Search Instruments
- [ ] Get Instruments

#### Market Hours
- [ ] Get Hours for Multiple Markets
- [ ] Get Hours for Single Market

#### Movers
- [ ] Get Movers

#### Option Chains
- [ ] Get Option Chain

#### Price History
- [X] Get Price History

#### Quotes
- [ ] Get Quote
- [ ] Get Quotes

#### Transaction History
- [ ] Get Transaction
- [ ] Get Transactions

#### User Info & Preferences
- [ ] Get Preferences
- [ ] Get Streamer Subscription Keys
- [ ] Get User Principals
- [ ] Update Preferences

#### Watchlist
- [ ] Create Watchlist
- [ ] Delete Watchlist
- [ ] Get Watchlist
- [ ] Get Watchlists for Multiple Accounts
- [ ] Get Watchlists for Single Account
- [ ] Replace Watchlist
- [ ] Update WatchList
</details>
   


