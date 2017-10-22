#ifndef USER_H
#define USER_H

#include <string>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 22.10.2017
 *
 * (c) All rights reserved
 */
class user {
public:
	user();
	user(std::string username, std::string password);
	virtual ~user();

	void set_username(std::string from);
	std::string get_username();

	void set_password(std::string to);
	std::string get_password();

	void set_logged_in(bool logged_in);
	bool is_logged_in();

	bool is_fhtw_user();

protected:
	// Username and Password represent internally used
	// user characteristics.
	std::string username;
	std::string password;

	bool logged_in;
};

#endif // USER_H
