all: mailserver

mailserver: *.cpp login/*.cpp mail/*.cpp net/*.cpp
	g++ -ggdb -std=c++11 -pthread -Wall -o mailserver login/*.cpp mail/*.cpp net/*.cpp *.cpp -lldap -DLDAP_DEPRECATED
 
clean:
	rm -f mailserver *.o
