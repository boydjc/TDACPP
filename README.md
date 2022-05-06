# TDACPP

A class for interacting with TD Ameritrades API


## How to use

<strong>Getting Price History</strong>

```
	void setHistPrice(std::string ticker, std::string periodType, 
					  std::string period, std::string freqType,
					  std::string freq, unsigned int endDate,
					  unsigned int startDate, bool extHourData)
```

Corresonds to <a href="https://developer.tdameritrade.com/price-history/apis/get/marketdata/%7Bsymbol%7D/pricehistory">Get Price History</a>.


	params:

		-	<strong>ticker</strong> = ticker symbol of company

		-	<strong>periodType</strong> = The type of period to show. Valid values are day, month, year, or ytd (year to date). 
						 				  Default is day.

		-	<strong>period</strong> = The number of periods to show.

				Example: For a 2 day / 1 min chart, the values would be:
					 					
				period: 2
				periodType: day
				frequency: 1
				frequencyType: min

				Valid periods by periodType (defaults marked with an asterisk):

				day: 1, 2, 3, 4, 5, 10*
				month: 1*, 2, 3, 6
				year: 1*, 2, 3, 5, 10, 15, 20
				ytd: 1*

		-	<strong>freqType</strong> = The type of frequency with which a new candle is formed.

				Valid frequencyTypes by periodType (defaults marked with an asterisk):

				day: minute*
				month: daily, weekly*
				year: daily, weekly, monthly*
				ytd: daily, weekly*

		-	<strong>freq</strong> = The number of the frequencyType to be included in each candle.

				Valid frequencies by frequencyType (defaults marked with an asterisk):

				minute: 1*, 5, 10, 15, 30
				daily: 1*
				weekly: 1*
				monthly: 1*

		-	<strong>endDate</strong> = End date as milliseconds since epoch. If startDate and endDate are provided, period should not be provided. 
				Default is previous trading day.

		-	<strong>startDate</strong> = Start date as milliseconds since epoch. If startDate and endDate are provided, period should not be provided.

		-	<strong>extHourData</strong> = true to return extended hours data, false for regular market hours only. Default is true */


## Dev Notes

- The initial API refresh token that is first created through TD Ameritrade's authentication guide (https://developer.tdameritrade.com/content/authentication-faq) and client id need to be stored in a file named config.json in the parent directory. The file needs to look like the following:

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

- The refresh token does not need to be encoded before storing in config.json. The
	class will encode it for you before sending requests

- There is no need to manually keep track of when a refresh token or access token is created. The class will check the access token and refresh token creation dates and make new ones when it's time. However, if you go more than 90 days without running the class and your refresh token expires, you will need to manually create one again and insert it into config.json.

