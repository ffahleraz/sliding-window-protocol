all: recvfile sendfile

recvfile: src/helpers.h src/helpers.cpp src/recvfile.cpp
	g++ -std=c++11 -pthread src/helpers.cpp src/recvfile.cpp -o recvfile

sendfile: src/helpers.h src/helpers.cpp src/sendfile.cpp
	g++ -std=c++11 -pthread src/helpers.cpp src/sendfile.cpp -o sendfile

clean: recvfile sendfile
	rm -f recvfile sendfile