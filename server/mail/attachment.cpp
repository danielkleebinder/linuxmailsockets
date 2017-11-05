/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */


#include "attachment.h"
#include <string>
#include <memory>
#include <stdexcept>


attachment::attachment() : size(0) {}
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


void attachment::set_size(uint64_t size) {
	if (size < 0) {
		throw std::runtime_error("Attachment size can not be a negative value");
	}
	this->size = size;
}

uint64_t attachment::get_size() {
	return size;
}
