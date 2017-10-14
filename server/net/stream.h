#ifndef STREAM_H
#define STREAM_H

#include <string>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 10.10.2017
 *
 * (c) All rights reserved
 */
class stream {
public:
	stream(int handler);
	virtual ~stream();

	void close();

	char read();
	std::string readline();
	void write(char ch);
	void write(std::string str);

protected:
	int _handler;
};

#endif // STREAM_H
