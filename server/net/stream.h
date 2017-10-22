#ifndef STREAM_H
#define STREAM_H

#include <string>
#include <cstdint>


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

	// Read/write as character stream
	char readchar();
	std::string readline();
	void writechar(char ch);
	void writeline(std::string str);

	// Read/write as byte stream
	uint8_t readbyte();
	void readbytes(uint8_t* bytes, int n);
	void writebyte(uint8_t b);
	void writebytes(uint8_t* bytes, int n);
	void writebytes(uint8_t* bytes, int offset, int size);

protected:
	int _handler;
};

#endif // STREAM_H
