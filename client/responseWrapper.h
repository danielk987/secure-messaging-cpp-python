#pragma once

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>

#include "config.h"
#include "utils.h"
#include "user.h"
#include "message.h"
class Response {
private:
	uint8_t r_version; // Server version
	uint16_t r_code; // Response code
	uint32_t r_payload_size;// Response payload size
	std::vector<uint8_t> r_payload;// Response payload
public:

	Response():r_version(), r_code(), r_payload_size(), r_payload() {}

	bool createHeader(const std::vector<uint8_t>&);// Create and unpack little endian header from the server response

	uint16_t getCode() const;// Get the response code
	uint32_t getPayloadSize() const;// Get the response payload size
	
	void setPayload(const std::vector<uint8_t>&);// Set the response payload
	std::vector<uint8_t> getPayload() const;// Get the response payload

};
class ResponseHandler {
private:
	const Response& r_response; // Response object
	User& r_user;// current user
	std::map<std::string, User>& r_contact_list; // user list
	std::map<std::string, std::vector<unsigned char>>& r_unknown_users; // unkown users
	void checkError();// Checks if server responded with an error
	User& findUserByID(std::string );// Finding user by ID
public:

	ResponseHandler(const Response&, User&, std::map<std::string, User>&, std::map<std::string, std::vector<unsigned char>>&);
	void registerHandler();// Register response handler
	void userListHandler();// Get the user list
	void publicKeyHandler();// Get the public key
	void fetchMessagesHandler();// Fetch messages response handler
	void sendMessageHandler();// Send message response handler
	void requestSymmetricHandler();// Request symmetric key response handler
	void sendSymmetricHandler();// Sequest symmetric key response handler
	void sendFileHandler();// Send file response handler
};