/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */


#include "mailpoolservice.h"
#include "email.h"
#include "../filesystem.h"
#include "../filelock.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>

#include <uuid/uuid.h>
#include <sys/time.h>
#include <stdlib.h>


// Defines
#define MAIL_NAME "mail.crn"
#define LOCK_NAME "lock.lck"
#define ATTACHMENT_DIR "attachments"



mailpoolservice::mailpoolservice(std::string basedir)
	: basedir(basedir), archive_name("DEL_ARCHIVE_USR"), current_id(0) {
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

	// Check if receiver is the archive
	if (mail.get_receiver() == archive_name) {
		return false;
	}

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
	if (username == archive_name) {
		throw std::runtime_error("Username is not allowed to be the same as the archive name");
	}
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
	if (!fs::exists(userdir) || username == archive_name) {
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
	std::string dirname = files.at(mail_id - 1);
	std::string maildir = concat_dir(userdir, dirname);
	std::string deledir = concat_dir(udeldir, dirname);
	std::string lockfil = concat_dir(maildir, LOCK_NAME);

	// Try to obtain a file lock on system level
	filelock fl(lockfil);
	if (!fl.try_lock(true)) {
		return false;
	}

	return fs::move_dir(maildir, deledir);
}


std::vector<email> mailpoolservice::load_user_mails(std::string username) {
	std::vector<email> result;
	std::string userdir = concat_dir(basedir, username);

	// Check if the user even has an email account
	if (!fs::exists(userdir) || username == archive_name) {
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
	if (archive_name.empty() || archive_name.length() <= 0) {
		throw std::runtime_error("Archive name can't be empty!");
	}
	this->archive_name = archive_name;
}


std::string mailpoolservice::get_archive_name() {
	return archive_name;
}




email mailpoolservice::parse_mail_dir(std::string mail_dir) {
	std::string mailtxt = concat_dir(mail_dir, MAIL_NAME);
	std::string lockfil = concat_dir(mail_dir, LOCK_NAME);

	// Check if the email even exists
	if (!fs::exists(mailtxt)) {
		throw std::runtime_error("Mail file does not exist!");
	}

	// Obtain exclusive file lock by OS
	filelock fl(lockfil);
	if (!fl.try_lock(true)) {
		throw std::runtime_error("Not able to obtain lock!");
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
	// Artificial ID
	struct timeval now;
	gettimeofday(&now, NULL);
	long ms = now.tv_sec * 1000 + now.tv_usec / 1000;

	std::stringstream result;
	result << ms;
	result << '-';
	result << (++current_id);
	result << '-';
	for (int i = 0; i < 16; i++) {
		result << ((char) ((rand() % 25) + 97));
		if ((i + 1) % 5 == 0) {
			result << '-';
		}
	}
	return result.str();
}
