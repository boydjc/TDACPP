# TDACPP

A class for interacting with TD Ameritrades API

## Dev Notes

- API refresh token needs to be stored in an environment variable named TDA_REFRESH
- The refresh token does not need to be encoded before storing in env var. The
	class will encode it for you before sending requests
- API access token needs to be stored in an environment variable named TDA_ACCESS
- The client ID of the developer app used to access the API should be stored in an environment
  variable named TDA_CLIENT

