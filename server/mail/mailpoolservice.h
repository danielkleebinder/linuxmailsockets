#ifndef MAILPOOLSERVICE_H
#define MAILPOOLSERVICE_H


#include <string>
#include <vector>
#include <map>
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
	email load_mail(std::string username, unsigned int mail_id);
	bool delete_mail(std::string username, unsigned int mail_id);
	std::vector<email> load_user_mails(std::string username);

	bool save_blacklist(std::map<std::string, time_t> bl);
	std::map<std::string, time_t> load_blacklist();

	std::string get_basedir();

private:
	// Base mailpool directory
	std::string basedir;
	// Archive name should never have less than 8 characters
	// to prevent users from accessing the "trash" folder.
	// This will be prevented by the system anyways!
	std::string archive_name;
	// Running ID for mail naming
	int current_id;

	void set_archive_name(std::string archive_name);
	std::string get_archive_name();
	void create_dir_hierarchy(std::string dir);

	email parse_mail_dir(std::string mail_dir);
	std::string concat_dir(std::string first, std::string last); 
	std::string next_uuid();
};


#endif // MAILPOOLSERVICE_H
