all: mailclient

mailclient: *.cpp
	g++ -ggdb -std=c++11 -pthread -Wall -o mailclient *.cpp

clean:
	rm -f mailclient *.o
