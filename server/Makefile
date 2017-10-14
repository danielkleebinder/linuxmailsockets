all: mailserver

mailserver: *.cpp
	g++ -ggdb -std=c++11 -pthread -Wall -o mailserver net/*.cpp *.cpp

clean:
	rm -f mailserver *.o
