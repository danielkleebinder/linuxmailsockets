#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#include <string>
#include <cstdint>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */
class attachment {
public:
	attachment();
	virtual ~attachment();

	void set_name(std::string name);
	std::string get_name();

	void set_data(uint8_t* data);
	uint8_t* get_data();

protected:
	// File name which is usually limited to 255 characters
	std::string name;

	// Data as byte array
	uint8_t* data;
};

#endif // ATTACHMENT_H
