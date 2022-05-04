# TDACPP

A class for interacting with TD Ameritrades API

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

