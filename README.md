# TDACPP

A class for interacting with TD Ameritrades API

## Dev Notes

- API refresh token needs to be stored in an environment variable named TDA_REFRESH
- The refresh token does not need to be encoded before storing in env var. The
	class will encode it for you before sending requests
- The client ID of the developer app used to access the API should be stored in an environment
  variable named TDA_CLIENT
- If you have an access token already you can store it in the env var TDA_ACCESS
	However, it's not really needed. If your TDA_REFESH and TDA_CLIENT are valid
	then the class will reach out for a new access token if it doesn't find one.
