build:
	g++ -std=c++11 -o ./bin/main -lcurl -I./include/ -I../lib/json/ ./src/*
