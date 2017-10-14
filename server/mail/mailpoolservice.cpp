/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */


#include "mailpoolservice.h"
#include "email.h"

#include <string>
#include <iostream>



mailpoolservice::mailpoolservice(std::string basedir)
	: basedir(basedir) {}

mailpoolservice::~mailpoolservice() {}

bool mailpoolservice::save_mail(email& mail) {
	return true;
}

email mailpoolservice::load_mail(std::string name) {
	return email();
}

std::string mailpoolservice::get_basedir() {
	return basedir;
}


