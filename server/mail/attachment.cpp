/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */


#include "attachment.h"

#include <string>
#include <stdexcept>


attachment::attachment() {}
attachment::~attachment() {}


void attachment::set_name(std::string name) {
	this->name = name;
}

std::string attachment::get_name() {
	return name;
}


void attachment::set_data(uint8_t* data) {
	this->data = data;
}

uint8_t* attachment::get_data() {
	return data;
}
