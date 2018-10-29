all: recvfile sendfile

recvfile: helpers.h helpers.cpp recvfile.h recvfile.cpp
	g++ helpers.h helpers.cpp recvfile.h recvfile.cpp -o recvfile

sendfile:helpers.h helpers.cpp sendfile.h sendfile.cpp
	g++ helpers.h helpers.cpp sendfile.h sendfile.cpp -o sendfile