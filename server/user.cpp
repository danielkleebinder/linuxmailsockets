/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 22.10.2017
 *
 * (c) All rights reserved
 */


#include "user.h"

#include <string>
#include <stdexcept>
#include <regex>


user::user() {}

user::user(std::string username, std::string password)
	: logged_in(false) {
	set_username(username);
	set_password(password);
}

user::~user() {}


/**
 * Sets the username.
 *
 * @param username Username.
 */
void user::set_username(std::string username) {
	if (username.empty()) {
		throw std::invalid_argument("Username length is not allowed to be empty!");
	}
	this->username = username;
}

std::string user::get_username() {
	return username;
}


/**
 * Sets the password.
 *
 * @param password Password.
 */
void user::set_password(std::string password) {
	if (password.empty()) {
		throw std::invalid_argument("Password length is not allowed to be empty!");
	}
	this->password = password;
}

std::string user::get_password() {
	return password;
}


/**
 * Sets if the user is logged in or not.
 *
 * @param logged_in True if the user is logged in, otherwise false.
 */
void user::set_logged_in(bool logged_in) {
	this->logged_in = logged_in;
}

bool user::is_logged_in() {
	return logged_in;
}


/**
 * Checks if the username matches the FHTW user pattern.
 *
 * @return True if the username pattern matches the FHTW user patter.
 */
bool user::is_fhtw_user() {
	std::regex rgx("^[A-Za-z]{2}[0-9]{2}[A-Za-z][0-9]{3}$");
	return std::regex_match(username, rgx);
}
