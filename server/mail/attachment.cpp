/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */


#include "attachment.h"
#include <string>
#include <memory>

attachment::attachment() {}
attachment::~attachment() {}


void attachment::set_name(std::string name) {
	this->name = name;
}

std::string attachment::get_name() {
	return name;
}


void attachment::set_data_ptr(std::shared_ptr<uint8_t> sp) {
	this->sp = sp;
}

uint8_t* attachment::get_data() {
	return sp.get();
}
