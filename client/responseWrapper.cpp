#include "responseWrapper.h"

// Create and unpack little endian header from the server response
bool Response::createHeader(const std::vector<uint8_t>& header) {
	if (header.size() != Config::RESPONSE_HEADER_SIZE) {
		return false;
	}
	r_version = header[0];
	r_code = Utils::unpackLittleEndian<uint16_t>(header, Config::VERSION_SIZE);
	r_payload_size = Utils::unpackLittleEndian<uint32_t>(header, Config::VERSION_SIZE + Config::CODE_SIZE);
}

// Get the response code
uint16_t Response::getCode() const {
	return r_code;
}
// Get the response payload size
uint32_t Response::getPayloadSize() const {
	return r_payload_size;
}
// Set the response payload
void Response::setPayload(const std::vector<uint8_t>& payload) {
	r_payload = payload;
}
// Get the response payload
std::vector<uint8_t> Response::getPayload() const {
	return r_payload;
}

ResponseHandler::ResponseHandler(const Response& response, User& user, std::map<std::string, User>& contact_list, std::map<std::string, std::vector<unsigned char>>& unknown_users) :
	r_response(response),
	r_user(user),
	r_contact_list(contact_list),
	r_unknown_users(unknown_users)
{}

// Checks if server responded with an error
void ResponseHandler::checkError() {
	if (r_response.getCode() == Config::ERROR_RESPONSE) {
		throw std::runtime_error("server responded with an error");
	}
}
// Finding user by ID
User& ResponseHandler::findUserByID(std::string id) {
	for (auto& pair : r_contact_list) {
		if (pair.second.getID() == id) {
			return pair.second;
		}
	}
	throw std::runtime_error("Didn't find user with given id");
}
// Register response handler
void ResponseHandler::registerHandler() {
	checkError();
	// Check if payload size and code matches
	if (r_response.getPayloadSize() != Config::CLIENT_ID_SIZE) {
		throw std::runtime_error("Got wrong payload size");
	}
	if (r_response.getCode() != Config::REGISTRATION_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	auto payload = r_response.getPayload();
	std::string clientID(payload.begin(), payload.end());
	// save the active user
	r_user.setID(clientID);
	if (r_user.saveToFile()) {
		std::cout << "Registered Successfully !!" << std::endl;
		std::cout << std::endl;
	}
}
// Get the user list
void ResponseHandler::userListHandler() {
	checkError();
	// Checks the response code
	if (r_response.getCode() != Config::USER_LIST_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	// Get metadata for single user
	auto payload = r_response.getPayload();
	auto payloadSize = r_response.getPayloadSize();
	auto singleUserSize = Config::CLIENT_ID_SIZE + Config::MAX_NAME_SIZE + 1;
	auto current_contact_list = std::move(r_contact_list);
	// build a new contact list
	for (size_t i = 0; i < payloadSize; i += singleUserSize)
	{
		std::string clientID(payload.begin() + i, payload.begin() + i + Config::CLIENT_ID_SIZE);
		std::string name(payload.begin() + i + Config::CLIENT_ID_SIZE , payload.begin() + i + singleUserSize);
		std::string trimmedName = name.substr(0, name.find('\0'));
		// only add if not exists or client id changed (maybe deleted)
		if (current_contact_list.find(trimmedName) == current_contact_list.end() || current_contact_list[trimmedName].getID() != clientID) {
			r_contact_list[trimmedName] = User(trimmedName, clientID);
		}
		else {
			r_contact_list[trimmedName] = current_contact_list[trimmedName];
		}
		// Check if we have this user symmetric key from the messages recived earlier
		if (r_unknown_users.find(clientID) != r_unknown_users.end()) {
			r_contact_list[trimmedName].setSymmetricKey(r_unknown_users[clientID]);
			r_unknown_users.erase(clientID);
		}

		std::cout << trimmedName << std::endl;
	}
	std::cout << std::endl;
}
// Get the public key
void ResponseHandler::publicKeyHandler() {
	checkError();
	// Check the payload size and the response code
	if (r_response.getPayloadSize() != Config::CLIENT_ID_SIZE + RSAPublicWrapper::KEYSIZE) {
		throw std::runtime_error("Got wrong payload size");
	}
	if (r_response.getCode() != Config::PUBLIC_KEY_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	// Set the public key to the user
	auto payload = r_response.getPayload();
	std::string clientID(payload.begin() , payload.begin() + Config::CLIENT_ID_SIZE);
	std::string publicKey(payload.begin() + Config::CLIENT_ID_SIZE, payload.end());
	User& user = findUserByID(clientID);
	user.setPublicKey(publicKey);
	std::cout << "Public key for user " << user.getName() << " received" << std::endl;
}

// Fetch messages response handler
void ResponseHandler::fetchMessagesHandler() {
	checkError();
	// Check the response code
	if (r_response.getCode() != Config::FETCH_MESSAGES_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	// Get message one by one and process it
	auto payload = r_response.getPayload();
	int messageCounter = 0;
	// Store all the messages in messagesList
	std::vector<std::unique_ptr<Message>> messagesList;
	auto it = payload.begin();
	while (it < payload.end()) {
		// First check the message type 
		auto current = std::vector<uint8_t>(it, payload.end());
		if (current.size() < Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE)
			throw std::runtime_error("Got wrong payload size");
		auto messageType = Utils::unpackLittleEndian<uint8_t>(current, Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE);
		std::unique_ptr<Message> currentMessage;
		// Get the message by the type
		switch (messageType) {
			case Config::REQUEST_SYMMETRIC_TYPE:
				currentMessage = std::make_unique<RequestSymmetricMessage>(current, r_contact_list, r_unknown_users);
				break;
			case Config::SEND_SYMMETRIC_TYPE:
				currentMessage = std::make_unique<SendSymmetricMessage>(current, r_contact_list, r_unknown_users, r_user);
				break;
			case Config::SEND_MESSAGE_TYPE:
				currentMessage = std::make_unique<SendMessage>(current, r_contact_list, r_unknown_users);
				break;
			case Config::SEND_FILE_TYPE:
				currentMessage = std::make_unique<FileMessage>(current, r_contact_list, r_unknown_users);
				break;
			default:
				continue;
		}
		it += currentMessage->getTotalSize();
		messagesList.push_back(std::move(currentMessage));
		messageCounter++;
	}
	// Sorting the message so we get symmetric key first (so we can dycrpt)
	std::sort(messagesList.begin(), messagesList.end(), [](const std::unique_ptr<Message>& msg1, const std::unique_ptr<Message>& msg2) {
		return msg1->getType() < msg2->getType();
	});
	// Handle each message by order
	for (auto& msg : messagesList) {
		msg->handleMessage();
	}
}

// Send message response handler
void ResponseHandler::sendMessageHandler() {
	checkError();
	// Check the payload size and the response code
	if (r_response.getPayloadSize() != Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE) {
		throw std::runtime_error("Got wrong payload size");
	}
	if (r_response.getCode() != Config::SEND_MESSAGE_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	std::cout << "Message was sent successfully" << std::endl;
}

// Request symmetric key response handler
void ResponseHandler::requestSymmetricHandler() {
	checkError();
	// Check the payload size and the response code
	if (r_response.getPayloadSize() != Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE) {
		throw std::runtime_error("Got wrong payload size");
	}
	if (r_response.getCode() != Config::SEND_MESSAGE_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	std::cout << "Request for symmetric key was sent successfully" << std::endl;
}

// Sequest symmetric key response handler
void ResponseHandler::sendSymmetricHandler() {
	checkError();
	// Check the payload size and the response code
	if (r_response.getPayloadSize() != Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE) {
		throw std::runtime_error("Got wrong payload size");
	}
	if (r_response.getCode() != Config::SEND_MESSAGE_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	std::cout << "Symmetric key was sent successfully" << std::endl;
}

// Send file response handler
void ResponseHandler::sendFileHandler() {
	checkError();
	// Check the payload size and the response code
	if (r_response.getPayloadSize() != Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE) {
		throw std::runtime_error("Got wrong payload size");
	}
	if (r_response.getCode() != Config::SEND_MESSAGE_SUCCESS) {
		throw std::runtime_error("Server sent a wrong code");
	}
	std::cout << "File was sent successfully" << std::endl;
}



