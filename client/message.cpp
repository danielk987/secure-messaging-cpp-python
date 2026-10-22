#include "message.h"

Message::Message(const std::vector<uint8_t>& payload, std::map<std::string, User>& users, std::map<std::string, std::vector<unsigned char>>& unknown_users):m_contact_list(users), m_unknown_users(unknown_users){
	// check minimum header size
	size_t headerSize = Config::CLIENT_ID_SIZE +
		Config::MESSAGE_ID_SIZE +
		Config::MESSAGE_TYPE_SIZE +
		Config::MESSAGE_SIZE_FIELD_SIZE;

	if (payload.size() < headerSize) {
		throw std::runtime_error("Got wrong payload size");
	}
	// Get the Message metadata
	std::string clientID(payload.begin(), payload.begin() + Config::CLIENT_ID_SIZE);
	m_client_id = clientID;

	m_id = Utils::unpackLittleEndian<uint32_t>(payload, Config::CLIENT_ID_SIZE);
	m_type = Utils::unpackLittleEndian<uint8_t>(payload, Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE);
	m_size = Utils::unpackLittleEndian<uint32_t>(payload, Config::CLIENT_ID_SIZE + Config::MESSAGE_ID_SIZE + Config::MESSAGE_TYPE_SIZE);
	m_totalSize = headerSize + m_size;
	if (payload.size() < m_totalSize) {
		throw std::runtime_error("Got wrong payload size");
	}
	// Get the message content
	std::string content(payload.begin() + headerSize, payload.begin() + headerSize + m_size);
	m_content = content;
}
// Get the message size
uint32_t Message::getSize() {
	return m_size;
}
// Get the message total size
size_t Message::getTotalSize() {
	return m_totalSize;
}
// Get the message client id
std::string Message::getClientID() {
	return m_client_id;
}
// Get the message content
std::string Message::getContent() {
	return m_content;
}
// Get the message type
uint8_t Message::getType() const {
	return m_type;
}
// Find a user by ID
User* Message::findUserByID(std::string id) {
	for (auto& pair : m_contact_list) {
		if (pair.second.getID() == id) {
			return &(pair.second);
		}
	}
	return nullptr;
}
// print the header of the message
void Message::printHeader() {
	auto* user = findUserByID(m_client_id);
	std::string from = "unknown";
	if (user != nullptr) {
		from = user->getName();
	}
	std::cout << "From: " << from << std::endl;
	std::cout << "Content: " << std::endl;
}
// Print the footter
void Message::printFooter() {
	std::cout << "-----<EOM>-----" << "\n" << std::endl;
}

// Print the whole message
void Message::handleMessage() {
	printHeader();
	printContent();
	printFooter();
}


RequestSymmetricMessage::RequestSymmetricMessage(const std::vector<uint8_t>& payload, std::map<std::string, User>& users, std::map<std::string, std::vector<unsigned char>>& unknown_users):Message(payload, users, unknown_users) {
}

// Just prints "Request for symmetric key"
void RequestSymmetricMessage::printContent() {
	std::cout << "Request for symmetric key" << std::endl;
}

SendSymmetricMessage::SendSymmetricMessage(const std::vector<uint8_t>& payload, std::map<std::string, User>& users, std::map<std::string, std::vector<unsigned char>>& unknown_users, User& user):Message(payload, users, unknown_users), s_user(user) {
}

// Decrypt with private key, save to contact list or unknown, print "symmetric key received"
void SendSymmetricMessage::printContent() {
	// Try to decrypt the symmetric key
	std::string symmetricKey;
	try {
		symmetricKey = RSAPrivateWrapper(s_user.getPrivateKey()).decrypt(m_content);
	}
	catch (...) {
		std::cout << "cant decrypt key" << std::endl;
		return;
	}
	// Check if we have a user to set the symmetric key to
	auto* user = findUserByID(m_client_id);
	std::vector<unsigned char> symmetricVec(symmetricKey.begin(), symmetricKey.end());
	if (user != nullptr) {
		user->setSymmetricKey(symmetricVec);
	}else{
		// No user found store the key in unknown users 
		m_unknown_users[m_client_id] = symmetricVec;
	}
	std::cout << "symmetric key received" << std::endl;
}

SendMessage::SendMessage(const std::vector<uint8_t>& payload, std::map<std::string, User>& users, std::map<std::string, std::vector<unsigned char>>& unknown_users):Message(payload, users, unknown_users) {
}

// Decrypt with symmetric key, if not "cant decrypt message", or print message
void SendMessage::printContent() {
	auto* user = findUserByID(m_client_id);
	std::vector<unsigned char> symmetricVec;
	// Check if we fetched this user
	if (user != nullptr) {
		symmetricVec = user->getSymmetricKey();
	}
	// Check if got this user's symmetric key but not fetched
	else if(m_unknown_users.find(m_client_id) != m_unknown_users.end()){
		symmetricVec = m_unknown_users[m_client_id];
	}
	// No symmetric key no party
	if (symmetricVec.empty())
	{
		std::cout << "cant decrypt message" << std::endl;
		return;
	}
	// Found a symmetric key, try to decrypt
	try {
		auto content = AESWrapper(symmetricVec.data(), AESWrapper::DEFAULT_KEYLENGTH).decrypt(m_content.c_str(), m_size);
		std::cout << content << std::endl;
	}
	catch (...) {
		std::cout << "cant decrypt message" << std::endl;
	}
}

FileMessage::FileMessage(const std::vector<uint8_t>& payload, std::map<std::string, User>& users, std::map<std::string, std::vector<unsigned char>>& unknown_users):Message(payload, users, unknown_users) {
}
// Handles a file message
void FileMessage::printContent() {
	// Try to find the symmetric key
	auto* user = findUserByID(m_client_id);
	std::vector<unsigned char> symmetricVec;
	if (user != nullptr) {
		symmetricVec = user->getSymmetricKey();
	}
	else if(m_unknown_users.find(m_client_id) != m_unknown_users.end()){
		symmetricVec = m_unknown_users[m_client_id];
	}
	if (symmetricVec.empty())
	{
		std::cout << "cant decrypt message" << std::endl;
		return;
	}
	// Symmetric key found try to decrypt
	std::string content;
	try {
		content = AESWrapper(symmetricVec.data(), AESWrapper::DEFAULT_KEYLENGTH).decrypt(m_content.c_str(), m_size);
	}
	catch (...) {
		std::cout << "cant decrypt message" << std::endl;
		return;
	}
	// Find and store the file in the TMP dir
	std::string tempDir = Utils::getEnvTemp();
	if (tempDir.empty()) {
		std::cout << "cant find TMP folder" << std::endl;
		return;
	}
	// Make sure the file name is unique
	std::string fullPath;
	do {
		std::string filename = Utils::generateRandomString();
		fullPath = tempDir + "\\" + filename;
	} while (Utils::fileExists(fullPath));

	std::ofstream file(fullPath, std::ios::binary);
	file.write(content.data(), content.size());
	file.close();
	// Print the path
	std::cout << fullPath << std::endl;
}