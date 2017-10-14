/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */


#include "mailpoolservice.h"
#include "email.h"
#include "../filesystem.h"

#include <string>
#include <iostream>
#include <stdexcept>


mailpoolservice::mailpoolservice(std::string basedir)
	: basedir(basedir) {
	create_dir_hierarchy(basedir);
}

mailpoolservice::~mailpoolservice() {}


void mailpoolservice::create_dir_hierarchy(std::string dir) {
	fs::make_dir_rec(dir);
}


bool mailpoolservice::save_mail(email& mail) {
	return true;
}

email mailpoolservice::load_mail(std::string name) {
	return email();
}

std::string mailpoolservice::get_basedir() {
	return basedir;
}


void mailpoolservice::set_archive_name(std::string archive_name) {
	this->archive_name = archive_name;
}

std::string mailpoolservice::get_archive_name() {
	return archive_name;
}
