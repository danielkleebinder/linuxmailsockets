#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "mail/smtpservice.h"
#include "mail/mailpoolservice.h"
#include "net/serversocket.h"

#include <memory>
#include <map>
#include <vector>
#include <string>


/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.10.2017
 *
 * (c) All rights reserved
 */
class appcontext final {
public:
	// Internal attempt data structure
	struct attempt_t {
		int num_attempts;
		time_t last_sec;
	};

	// Methods
	virtual ~appcontext();

	static void initialize(int port, mailpoolservice& mps);
	static bool is_initialized();

	static net::serversocket* get_serversocket();
	static std::map<std::string, std::shared_ptr<struct attempt_t>>* get_blacklist();
	static std::vector<smtpservice*>* get_smtpservices();

	static void set_debug_mode(bool debug_mode);
	static bool is_debug_mode();
	static void debug_log(std::string msg);
	static void debug_log(std::string msg, int layer);
	static void serialize_blacklist(mailpoolservice& mps);

	static void dispose();

private:
	appcontext();

	static bool initialized;
	static bool debug_mode;
	static net::serversocket* ss;
	static std::map<std::string, std::shared_ptr<struct attempt_t>>* blacklist;
	static std::vector<smtpservice*>* smtpservices;
};

#endif // APPCONTEXT_H
