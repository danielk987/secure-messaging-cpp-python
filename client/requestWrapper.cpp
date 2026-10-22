
#include "requestWrapper.h"

Request::Request(const std::string& id, uint16_t code, const std::vector<uint8_t>& payload) :
	r_client_id(id),
	r_code(code),
	r_payload_size(static_cast<uint32_t>(payload.size())),
	r_payload(payload) {
}

Request::Request(const std::string& id, uint16_t code) :
	r_client_id(id),
	r_code(code),
	r_payload_size(0),
	r_payload() {
}

// Function to create the payload
std::vector<uint8_t> Request::createPayload() const {
	// Add the client id
	std::vector<uint8_t> requestVec(r_client_id.begin(), r_client_id.end());
	// Add the version
	requestVec.push_back(Config::CLIENT_VERSION);
	// Add the code
	auto codeVec = Utils::packLittleEndian<uint16_t>(r_code);
	requestVec.insert(requestVec.end(), codeVec.begin(), codeVec.end());
	// add the payload size
	auto sizeVec = Utils::packLittleEndian<uint32_t>(r_payload_size);
	requestVec.insert(requestVec.end(), sizeVec.begin(), sizeVec.end());
	// add the payload
	requestVec.insert(requestVec.end(), r_payload.begin(), r_payload.end());
	return requestVec;
}

RequestHandler::RequestHandler(User& user, std::map<std::string, User>& contact_list) :r_user(user), r_contact_list(contact_list) {}

// Check if current user is active
void RequestHandler::hasUser() {
	if (!r_user.isActive()) {
		throw std::runtime_error("You need to register first");
	}
}
// Gets username from input and checks if valid
std::string RequestHandler::getUserName() {
	std::cout << "Enter username: ";
	std::string username;
	std::getline(std::cin, username);
	if (username.length() <= 0 || username.length() > Config::MAX_NAME_SIZE) {
		throw std::runtime_error("User name must conatin at least one char and no more then " + std::to_string(Config::MAX_NAME_SIZE));
	}
	return username;
}

// Gets username from input the checks if user in the users list
User& RequestHandler::getUser() {
	hasUser();
	std::string username = getUserName();
	if (r_contact_list.find(username) == r_contact_list.end()) {
		throw std::runtime_error("This username doesn't exsits try to fetch again");
	}
	return r_contact_list[username];
}

// Register action handler
Request RequestHandler::registerHandler() {
	if (r_user.isActive()) {
		throw std::runtime_error("You are already registered");
	}
	// Getting the user name
	std::string username = getUserName();
	r_user.setName(username);
	// Resize because server expects Config::MAX_NAME_SIZE + 1 length
	username.resize(Config::MAX_NAME_SIZE + 1);
	// Generate the private and public keys
	auto privateKey = r_user.generateRSAKeys();
	privateKey.resize(RSAPublicWrapper::KEYSIZE);
	// Create the vector payload
	std::vector<uint8_t> payloadVec(username.begin(), username.end());
	payloadVec.insert(payloadVec.end(), privateKey.begin(), privateKey.end());
	// Generate a dummy client id
	std::string dummyClientID(16, '\0');
	auto request = Request(dummyClientID, Config::REGISTRATION_CODE, payloadVec);
	return request;
}
// Get user list
Request RequestHandler::userListHandler() {
	hasUser();
	auto request = Request(r_user.getID(), Config::USER_LIST_CODE);
	return request;
}
// Get target user public key
Request RequestHandler::publicKeyHandler() {
	auto user = getUser();
	auto userID = user.getID();
	std::vector<uint8_t> payloadVec(userID.begin(), userID.end());
	auto request = Request(r_user.getID(), Config::PUBLIC_KEY_CODE, payloadVec);
	return request;
}
// Get the active user pending messages
Request RequestHandler::fetchMessagesHandler() {
	hasUser();
	auto request = Request(r_user.getID(), Config::FETCH_MESSAGES_CODE);
	return request;
}
// Send message to target user
Request RequestHandler::sendMessageHandler() {
	auto& user = getUser();
	auto userID = user.getID();
	const auto& symmetricKey = user.getSymmetricKey();
	// Check if we share a symmetric key with the user
	if (symmetricKey.empty()) {
		throw std::runtime_error("You dont share a symmetric key with this user");
	}
	// Get the message 
	std::cout << "Enter Your Message: ";
	std::string msg;
	std::getline(std::cin, msg);
	// Add the target user
	std::vector<uint8_t> payloadVec(userID.begin(), userID.end());
	// Add the Type
	payloadVec.insert(payloadVec.end(), Config::SEND_MESSAGE_TYPE);
	// Encrypt the messagee content with the symmetric key 
	auto encrypted = AESWrapper(symmetricKey.data(), AESWrapper::DEFAULT_KEYLENGTH).encrypt(msg.data(), msg.length());
	// Convert to little endian
	auto contentSize = Utils::packLittleEndian(static_cast<uint32_t>(encrypted.size()));
	// Add content size
	payloadVec.insert(payloadVec.end(), contentSize.begin(), contentSize.end());
	// Add the content
	payloadVec.insert(payloadVec.end(), encrypted.begin(), encrypted.end());
	auto request = Request(r_user.getID(), Config::SEND_MESSAGE_CODE, payloadVec);
	return request;
}
// Request a symmetric keyt
Request RequestHandler::requestSymmetricHandler() {
	auto user = getUser();
	auto userID = user.getID();
	// Add the target user
	std::vector<uint8_t> payloadVec(userID.begin(), userID.end());
	// Add the Type
	payloadVec.insert(payloadVec.end(), Config::REQUEST_SYMMETRIC_TYPE);
	// Convert to little endian
	auto contentSize = Utils::packLittleEndian(static_cast<uint32_t>(0));
	// Add the content size
	payloadVec.insert(payloadVec.end(), contentSize.begin(), contentSize.end());

	auto request = Request(r_user.getID(), Config::SEND_MESSAGE_CODE, payloadVec);
	return request;
}
// Send your symmetric key to target user
Request RequestHandler::sendSymmetricHandler() {
	auto& targetUser = getUser();
	auto targetUserID = targetUser.getID();
	// Get the target user public key
	auto targetPublicKey = targetUser.getPublicKey();
	if (targetPublicKey.empty()) {
		throw std::runtime_error("First you need to get the user's public key");
	}
	// Add the target user
	std::vector<uint8_t> payloadVec(targetUserID.begin(), targetUserID.end());
	// Add the Type
	payloadVec.insert(payloadVec.end(), Config::SEND_SYMMETRIC_TYPE);

	const auto& symmetricKey = r_user.getSymmetricKey();
	// Encrypt your symmetric key with the targest public key 
	auto encryptedSymmetricKey = RSAPublicWrapper(targetPublicKey).encrypt(reinterpret_cast<const char*>(symmetricKey.data()), AESWrapper::DEFAULT_KEYLENGTH);
	// Build the payload
	std::vector<uint8_t> content(encryptedSymmetricKey.begin(), encryptedSymmetricKey.end());
	// Convert to little endian
	auto contentSize = Utils::packLittleEndian(static_cast<uint32_t>(content.size()));
	payloadVec.insert(payloadVec.end(), contentSize.begin(), contentSize.end());

	payloadVec.insert(payloadVec.end(), content.begin(), content.end());

	auto request = Request(r_user.getID(), Config::SEND_MESSAGE_CODE,payloadVec);
	// Set Your symmetric key to the target user
	targetUser.setSymmetricKey(symmetricKey);
	return request;
}
// Function to send file
Request RequestHandler::sendFileHandler() {
	auto targetUser = getUser();
	auto targetUserID = targetUser.getID();
	const auto& symmetricKey = targetUser.getSymmetricKey();
	// Check if we share a symmetric key with the user
	if (symmetricKey.empty()) {
		throw std::runtime_error("You dont share a symmetric key with this user");
	}
	// Get the file path and the file
	std::string filePath;
	std::cout << "Enter file path: ";
	std::getline(std::cin, filePath);
	std::ifstream testFile(filePath);
	if (!testFile.good()) {
		throw "File does not exist";
	}
	testFile.close();
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open()) {
		throw "File can't open";
	}

	// Get file size
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	// Reserve space in vector
	char* buffer = new char[length];
	// Read file into vector
	file.read(buffer, length);
	if (!file) {
		delete[] buffer;
		throw "File can't be read";
	}
	file.close();
	// Encrypt the file content with the symmetric key 
	auto encrypted = AESWrapper(symmetricKey.data(), AESWrapper::DEFAULT_KEYLENGTH).encrypt(buffer, length);

	// Add the target user
	std::vector<uint8_t> payloadVec(targetUserID.begin(), targetUserID.end());
	// Add the Type
	payloadVec.insert(payloadVec.end(), Config::SEND_FILE_TYPE);
	// Convert to little endian
	auto contentSize = Utils::packLittleEndian(static_cast<uint32_t>(encrypted.size()));
	payloadVec.insert(payloadVec.end(), contentSize.begin(), contentSize.end());

	payloadVec.insert(payloadVec.end(), encrypted.begin(), encrypted.end());
	auto request = Request(r_user.getID(), Config::SEND_MESSAGE_CODE, payloadVec);
	return request;
}
