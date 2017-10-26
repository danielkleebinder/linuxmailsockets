/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 26.10.2017
 *
 * (c) All rights reserved
 */


#include "virtuallogin.h"
#include "../user.h"


virtuallogin::virtuallogin() {}
virtuallogin::~virtuallogin() {}

bool virtuallogin::login(user& usr) {
	if (usr.get_username().empty()) {
		return false;
	}
	if (usr.get_password().empty()) {
		return false;
	}
	usr.set_logged_in(usr.is_fhtw_user());
	return usr.is_logged_in();
}

bool virtuallogin::logout(user& usr) {
	usr.set_logged_in(false);
	return true;
}

