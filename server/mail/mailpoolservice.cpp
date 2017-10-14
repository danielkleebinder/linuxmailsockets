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
#include <vector>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <sstream>

#include <uuid/uuid.h>
#include <stdlib.h>


// Defines
#define MAIL_NAME "mail.txt"
#define ATTACHMENT_DIR "attachments"


mailpoolservice::mailpoolservice(std::string basedir)
	: basedir(basedir), archive_name("DEL_ARCHIVE_USR") {
	if (basedir.empty()) {
		throw std::runtime_error("Mailpool directory can't be empty!");
	}

	// Check if last character is a '/'
	if (basedir.back() != '/') {
		basedir += '/';
	}

	// Create directory hierarchy
	create_dir_hierarchy(basedir);
}

mailpoolservice::~mailpoolservice() {}


void mailpoolservice::create_dir_hierarchy(std::string dir) {
	fs::make_dir_rec(dir);
}


bool mailpoolservice::save_mail(email& mail) {
	std::string uuid = next_uuid();

	// Create all directory strings
	std::string userdir = concat_dir(basedir, mail.get_receiver());
	std::string uuiddir = concat_dir(userdir, uuid);
	std::string mailtxt = concat_dir(uuiddir, MAIL_NAME);
	std::string attadir = concat_dir(uuiddir, ATTACHMENT_DIR);

	// Check if user directory exists
	if (!fs::exists(attadir)) {
		fs::make_dir(userdir);
		fs::make_dir(uuiddir);
		fs::make_dir(attadir);
	}

	// Write E-Mail
	fs::file_append_text(mailtxt, mail.get_sender(), true);
	fs::file_append_text(mailtxt, mail.get_receiver(), true);
	fs::file_append_text(mailtxt, mail.get_subject(), true);
	fs::file_append_text(mailtxt, mail.get_message(), true);
	return true;
}

email mailpoolservice::load_mail(std::string username, unsigned int mail_id) {
	std::vector<email> mails = load_user_mails(username);
	if (mails.size() < mail_id) {
		throw std::runtime_error("Mail ID is out of range!");
	}
	return mails.at(mail_id - 1);
}

bool mailpoolservice::delete_mail(std::string username, unsigned int mail_id) {
	// Create all used directory strings
	std::string archdir = concat_dir(basedir, archive_name);
	std::string udeldir = concat_dir(archdir, username);
	std::string userdir = concat_dir(basedir, username);

	// Check if user even has an email account with emails
	if (!fs::exists(userdir)) {
		return false;
	}

	// Create directory hierarchy
	if (!fs::exists(udeldir)) {
		fs::make_dir(archdir);
		fs::make_dir(udeldir);
	}

	// Check if email exists
	std::vector<std::string> files = fs::list_files(userdir);
	if (files.size() < mail_id) {
		return false;
	}

	// Finally move the email to the archive
	std::string maildir = concat_dir(userdir, files.at(mail_id - 1));
	return fs::move_dir(maildir, udeldir);
}

std::vector<email> mailpoolservice::load_user_mails(std::string username) {
	std::vector<email> result;
	std::string userdir = concat_dir(basedir, username);

	// Check if the user even has an email account
	if (!fs::exists(userdir)) {
		return result;
	}

	// Receive all available emails from this user
	std::vector<std::string> files = fs::list_files(userdir);
	for (std::string current : files) {
		std::string mail_dir = concat_dir(userdir, current);
		result.push_back(parse_mail_dir(mail_dir));
	}

	// Return all emails
	return result;
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




email mailpoolservice::parse_mail_dir(std::string mail_dir) {
	std::string mailtxt = concat_dir(mail_dir, MAIL_NAME);

	// Check if the email even exists
	if (!fs::exists(mailtxt)) {
		throw std::runtime_error("Mail file does not exist!");
	}

	email result;
	std::ifstream input(mailtxt);
	
	// Read and parse the email file
	std::string line;
	std::stringstream ss;
	int line_count = 0;
	while (std::getline(input, line)) {
		if (line_count == 0) {					// First line: Sender name
			result.set_sender(line);
		} else if (line_count == 1) {			// Second line: Receiver name
			result.set_receiver(line);
		} else if (line_count == 2) {			// Third line: Subject
			result.set_subject(line);
		} else {								// Rest: Mail message
			ss << line << std::endl;
		}
		line_count++;
	}

	// Add message to the email object and return everything
	result.set_message(ss.str());
	return result;
}

std::string mailpoolservice::concat_dir(std::string first, std::string last) {
	if (first.back() != '/') {
		first += '/';
	}
	return first + last;
}

std::string mailpoolservice::next_uuid() {
//	uuid_t uuid;
//	uuid_generate(uuid);

	// REPLACE WITH UUID!!
	int uuid = rand();

	std::stringstream result;
	result << uuid;
	return result.str();
}

