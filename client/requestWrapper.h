#pragma once

#include <string>
#include <vector>
#include <map>
#include "utils.h"
#include "config.h"
#include "user.h"
#include <stdexcept>

class Request {
private:
	const std::string r_client_id; // Request client id
	uint16_t r_code; // Request code
	uint32_t r_payload_size; // Request payload size
	const std::vector<uint8_t> r_payload; // Request payload

public:
	Request(const std::string&, uint16_t, const std::vector<uint8_t>&);
	Request(const std::string&, uint16_t);
	std::vector<uint8_t> createPayload() const; // Function to create the payload
};



class RequestHandler {
private:
	User& r_user; // Active user
	std::map<std::string, User>& r_contact_list; // user list
	void hasUser(); // Check if current user is active
	std::string getUserName(); // Gets username from input and checks if valid
	User& getUser();// Gets username from input the checks if user in the users list
public:

	RequestHandler(User&, std::map<std::string, User>&);

	Request registerHandler();// Register action handler
	Request userListHandler();// Get user list
	Request publicKeyHandler();// Get target user public key
	Request fetchMessagesHandler(); // Get the active user pending messages
	Request sendMessageHandler();// Send message to target user
	Request requestSymmetricHandler();// Request a symmetric key
	Request sendSymmetricHandler();// Send your symmetric key to target user
	Request sendFileHandler();// Function to send file
};