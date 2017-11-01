/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 26.10.2017
 *
 * (c) All rights reserved
 */


#include "ldaplogin.h"
#include "../user.h"

#include <stdlib.h>

// LDAP library
#define LDAP_DEPRECATED 1
#include <ldap.h>

// C++11 libraries
#include <sstream>
#include <stdexcept>
#include <iostream>


ldaplogin::ldaplogin() {}
ldaplogin::~ldaplogin() {}

bool ldaplogin::login(user& usr) {
	usr.set_logged_in(false);
	if (!usr.is_fhtw_user() || usr.get_username().empty() || usr.get_password().empty()) {
		return false;
	}

	// LDAP authentification
	LDAP* ldap;

	// Pre setup variables
	int ldap_version = LDAP_VERSION3;
	void* ldap_referrals = LDAP_OPT_OFF;

	int ldap_port = 389;

	const char* ldap_host = "ldap.technikum-wien.at";
	const char* ldap_pw = usr.get_password().c_str();

	// Create Base DN
	std::stringstream ss;
	ss << "uid=" << usr.get_username();
	ss << ",ou=People,dc=technikum-wien,dc=at";
	std::string res_bn = ss.str();
	const char* base_dn = res_bn.c_str();

	// Try to establish a connection to the LDAP server
	if ((ldap = ldap_init(ldap_host, ldap_port)) == NULL) {
		throw std::runtime_error("Could not connect to the LDAP server");
	}

	// Set LDAP options
	ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
	ldap_set_option(ldap, LDAP_OPT_REFERRALS, &ldap_referrals);

	// Try to bind the user identification
	int result = ldap_simple_bind_s(ldap, base_dn, ldap_pw);
	usr.set_logged_in(result == LDAP_SUCCESS);

	// Disconnect from LDAP
	ldap_unbind(ldap);
	return usr.is_logged_in();
}

bool ldaplogin::logout(user& usr) {
	usr.set_logged_in(false);
	return true;
}

