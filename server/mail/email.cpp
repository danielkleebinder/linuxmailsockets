/**
 * AUTHOR: KLEEBINDER Daniel
 * CREATED ON: 30.09.2017
 *
 * (c) All rights reserved
 */


#include "email.h"

#include <string>
#include <stdexcept>

email::email() {}

email::email(std::string from, std::string to, std::string subject, std::string message) {
	set_sender(from);
	set_receiver(to);
	set_subject(subject);
	set_message(message);
}

email::~email() {}

void email::set_sender(std::string from) {
	if (from.length() <= 0 || from.length() > 8) {
		throw std::invalid_argument("Sender length is not allowed to exceed 8 characters and can't be empty!");
	}
	this->from = from;
}

std::string email::get_sender() {
	return from;
}

void email::set_receiver(std::string to) {
	if (to.length() <= 0 || to.length() > 8) {
		throw std::invalid_argument("Receiver length is not allowed to exceed 8 characters and can't be empty!");
	}
	this->to = to;
}

std::string email::get_receiver() {
	return to;
}

void email::set_subject(std::string subject) {
	if (subject.length() <= 0 || subject.length() > 80) {
		throw std::invalid_argument("Subject length is not allowed to exceed 80 characters and can't be empty!");
	}
	this->subject = subject;
}

std::string email::get_subject() {
	return subject;
}

void email::set_message(std::string message) {
	this->message = message;
}

std::string email::get_message() {
	return message;
}
