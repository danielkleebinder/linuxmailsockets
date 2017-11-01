/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 14.10.2017
 *
 * (c) All rights reserved
 */


#include "mailpoolservice.h"
#include "email.h"
#include "attachment.h"
#include "../filesystem.h"
#include "../filelock.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>

#include <uuid/uuid.h>
#include <sys/time.h>
#include <stdlib.h>


// Defines
#define MAIL_NAME "mail.crn"
#define LOCK_NAME "lock.lck"
#define DEL_ARCHIVE_USR "DEL_ARCHIVE_USR"
#define ATTACHMENT_DIR "attachments"
#define BLACKLIST_NAME "blacklist.blk"



mailpoolservice::mailpoolservice(std::string basedir)
	: basedir(basedir), archive_name(DEL_ARCHIVE_USR), current_id(0) {
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

	// Write Attachments
	for (attachment current : mail.get_attachments()) {
		std::string attachment_file = concat_dir(attadir, current.get_name());
		fs::file_write_bytes(attachment_file, current.get_data());
	}
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


bool mailpoolservice::save_blacklist(std::map<std::string, time_t> bl) {
	std::string blistfile = concat_dir(basedir, BLACKLIST_NAME);

	// Clear the file content and create it if it
	// does not already exist
	if (!fs::clear_file(blistfile)) {
		return false;
	}


	// Write blacklist using the
	//   key:value
	// form.
	std::stringstream ss;
	for (auto const& entry : bl) {
		ss.str(std::string(entry.first));
		ss << ":" << entry.second;
		if (!fs::file_append_text(blistfile, ss.str(), true)) {
			return false;
		}
	}
	return true;
}


std::map<std::string, time_t> mailpoolservice::load_blacklist() {
	std::map<std::string, time_t> result;
	std::string blistfile = concat_dir(basedir, BLACKLIST_NAME);

	// Return empty result if blacklist file does not exist
	if (!fs::exists(blistfile)) {
		return result;
	}

	std::ifstream input(blistfile);
	
	// Read and parse the blacklist file
	std::string delimiter = ":";
	std::string line;
	std::stringstream ss;
	int line_count = 0;

	// Parse token
	while (std::getline(input, line)) {
		// Skip empty lines
		if (line.empty()) {
			continue;
		}

		// Skip Comments
		if (line.find("#") == 0) {
			continue;
		}

		// Parse key:value delimiter
		size_t pos = line.find(delimiter);
		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + delimiter.length(), line.length());

		// Insert key and value into result map
		std::string::size_type sz;
		result[key] = (time_t) std::stol(value, &sz);
		line_count++;
	}

	// Return the result
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
	std::string attadir = concat_dir(mail_dir, ATTACHMENT_DIR);

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

	// Add message to the email object
	result.set_message(ss.str());

	// Try to read all attachments regarding this email
	std::vector<std::string> files = fs::list_files(attadir);
	for (std::string current : files) {
		std::string attachment_name = concat_dir(attadir, current);

		// Check if stream could be opened
		std::ifstream ai(attachment_name, std::ifstream::binary);
		if (!ai) {
			throw std::runtime_error("Could not open attachment file!");
		}

		// Seek stream positions and retreive file size
		ai.seekg(0, std::ios::end);
		std::streamsize size = ai.tellg();
		ai.seekg(0, std::ios::beg);

		// Read data
		std::shared_ptr<uint8_t> sp(new uint8_t[size], std::default_delete<uint8_t[]>());
		char* buffer = (char*) sp.get();
		ai.read(buffer, size);
		ai.close();

		// Save attachment
		attachment att;
		att.set_name(current);
		att.set_data_ptr(sp);
		//att.set_data((uint8_t*) buffer);
		result.get_attachments().push_back(att);
	}

	// Return the email and all its attachments
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
