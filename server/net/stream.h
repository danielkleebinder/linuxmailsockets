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

	// Read/write primitive data types
	uint16_t readuint16();
	uint32_t readuint32();
	uint64_t readuint64();

	void writeuint16(uint16_t v);
	void writeuint32(uint32_t v);
	void writeuint64(uint64_t v);

protected:
	int _handler;
};

#endif // STREAM_H
