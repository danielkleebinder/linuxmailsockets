#ifndef EMAIL_H
#define EMAIL_H

#include <string>

/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */
class email {
public:
	email();
	email(std::string from, std::string to, std::string subject, std::string message);
	virtual ~email();

	void set_sender(std::string from);
	std::string get_sender();

	void set_receiver(std::string to);
	std::string get_receiver();

	void set_subject(std::string subject);
	std::string get_subject();

	void set_message(std::string message);
	std::string get_message();

protected:
	// From and To represent internally used names.
	// These are limited to 8 characters
	std::string from;
	std::string to;

	// The subject is limited to 80 characters
	std::string subject;

	// The message is terminated my "\n.\n" sequence
	// and has no limit in size
	std::string message;
};

#endif // EMAIL_H
