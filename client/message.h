#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "config.h"
#include "utils.h"
#include "user.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

class Message {
protected:
	std::string m_client_id; // Message client id
	uint32_t m_id; // Message id
	uint8_t m_type; // Message type
	uint32_t m_size; // Message size
	std::string m_content; // Message Content
	size_t m_totalSize;// Total size including header
	std::map<std::string, User>& m_contact_list; // contact list
	std::map<std::string, std::vector<unsigned char>>& m_unknown_users; // unknown users
	User* findUserByID(std::string); // finding user by id
	void printHeader();// print the message header
	void printFooter();// print the message footer
public:
	Message(const std::vector<uint8_t>&, std::map<std::string, User>&, std::map<std::string, std::vector<unsigned char>>&);
	uint32_t getSize(); // get the message size
	size_t getTotalSize(); // get the total size
	std::string getClientID(); // get the client id
	std::string getContent();// get the content 
	uint8_t getType() const;// get the type
	void handleMessage();// Print the whole message
	virtual void printContent() {};// prints the content (will be overriden)
};

class RequestSymmetricMessage : public Message {
public:
	RequestSymmetricMessage(const std::vector<uint8_t>&, std::map<std::string, User>&, std::map<std::string, std::vector<unsigned char>>&);
	void printContent() override;
};


class SendSymmetricMessage : public Message {
private:
	User& s_user;
public:
	SendSymmetricMessage(const std::vector<uint8_t>&, std::map<std::string, User>&, std::map<std::string, std::vector<unsigned char>>&,User&);
	void printContent() override;
};


class SendMessage : public Message {
public:
	SendMessage(const std::vector<uint8_t>&, std::map<std::string, User>&, std::map<std::string, std::vector<unsigned char>>&);
	void printContent() override;
};

class FileMessage : public Message {
public:
	FileMessage(const std::vector<uint8_t>&, std::map<std::string, User>&, std::map<std::string, std::vector<unsigned char>>&);
	void printContent() override;
};