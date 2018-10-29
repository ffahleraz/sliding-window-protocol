all: recvfile sendfile

recvfile: helpers.h helpers.cpp recvfile.cpp
	g++ helpers.h helpers.cpp recvfile.cpp -o recvfile

sendfile:helpers.h helpers.cpp sendfile.cpp
	g++ helpers.h helpers.cpp sendfile.cpp -o sendfile