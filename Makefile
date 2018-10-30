all: recvfile sendfile

recvfile: helpers.h helpers.cpp recvfile.cpp
	g++ -std=c++11 -pthread helpers.h helpers.cpp recvfile.cpp -o recvfile

sendfile:helpers.h helpers.cpp sendfile.cpp
	g++ -std=c++11 -pthread helpers.h helpers.cpp sendfile.cpp -o sendfile

clean: recvfile sendfile
	rm -f recvfile sendfile