#ifndef MAILPOOLSERVICE_H
#define MAILPOOLSERVICE_H


#include <string>
#include "email.h"


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */
class mailpoolservice {
public:
	mailpoolservice(std::string basedir);
	~mailpoolservice();

	bool save_mail(email& mail);
	email load_mail(std::string name);

	std::string get_basedir();

private:
	std::string basedir;
	std::string archive_name;

	void set_archive_name(std::string archive_name);
	std::string get_archive_name();
	void create_dir_hierarchy(std::string dir);
};


#endif // MAILPOOLSERVICE_H
