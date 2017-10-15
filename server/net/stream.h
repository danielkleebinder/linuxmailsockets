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

	void close_stream();

	char sread();
	std::string sreadline();
	void swrite(char ch);
	void swrite(std::string str);

protected:
	int _handler;
};

#endif // STREAM_H
