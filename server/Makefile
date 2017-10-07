all: mailserver

mailserver: *.cpp
	g++ -ggdb -std=c++11 -pthread -Wall -o mailserver *.cpp

clean:
	rm -f mailserver *.o
