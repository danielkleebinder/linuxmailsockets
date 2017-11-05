#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#include <string>
#include <cstdint>
#include <memory>

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

	void set_data_ptr(std::shared_ptr<uint8_t> sp);
	uint8_t* get_data();

	void set_size(uint64_t size);
	uint64_t get_size();

protected:
	// File name which is usually limited to 255 characters
	std::string name;

	// Data as byte array
	std::shared_ptr<uint8_t> sp;
	uint64_t size;
};

#endif // ATTACHMENT_H
