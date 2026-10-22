#include "user.h"

User::User():u_name(), u_id(), u_private_key(), u_public_key(), u_symmetric_key(), u_active(false) {
	readUserFromFile();
	generateSymmetricKey();
}

User::User(const std::string& name, const std::string& id) :u_name(name), u_id(id), u_private_key(), u_public_key(), u_symmetric_key(), u_active(false) {}

// Reading the user info from the local file if exists
void User::readUserFromFile() {
	std::ifstream file(Config::CLIENT_FILE_NAME.c_str());
	// Check if file exists 
	if (!file.is_open()) {
		return;
	}
	std::string line;
	if (!std::getline(file, line)) {
		return;
	}
	if (line.size() > Config::MAX_NAME_SIZE) {
		std::cerr << "Warning: User file is corrupted " << std::endl;
	}
	u_name = line;
	if (!std::getline(file, line)) {
		return;
	}
	// convert the user id
	u_id = Utils::hexToBytes(line);
	std::string privateKey;
	while (std::getline(file, line)) {
		privateKey += line;
	}
	// decode the private key
	try {
		u_private_key = Base64Wrapper::decode(privateKey);
	}
	catch (...) {
		return;
	}
	u_active = true; // flag for active user
	file.close();
}
// Setting the current user active
void User::setActiveUser(const std::string& name,const std::string& id) {
	u_name = name;
	u_id = id;
	u_active = true;
}

// Check if user active
bool User::isActive() {
	return u_active;
}

// Generate private and public key for the user
std::string User::generateRSAKeys() {
	RSAPrivateWrapper keysWrapper;
	u_private_key = keysWrapper.getPrivateKey();
	return keysWrapper.getPublicKey();
}
// Generate a symmetric key for the user
void User::generateSymmetricKey() {
	AESWrapper keyWrapper;
	const unsigned char* key = keyWrapper.getKey();
	u_symmetric_key = std::vector<unsigned char>(key, key + AESWrapper::DEFAULT_KEYLENGTH);
}

// Set the public key
void User::setPublicKey(const std::string& public_key) {
	u_public_key = public_key;
};

// Get the public key
std::string User::getPublicKey() {
	return u_public_key;
};
// Get the private key
std::string User::getPrivateKey() {
	return u_private_key;
};
// Set the symmetric key
void User::setSymmetricKey(const std::vector<unsigned char>& symmetic_key) {
	u_symmetric_key = symmetic_key;
};
// Get the symmetric key
const std::vector<unsigned char>& User::getSymmetricKey() const {
	return u_symmetric_key;
};
// Set the name
void User::setName(const std::string& name) {
	u_name = name;
}
// Get the name
std::string User::getName() const {
	return u_name;
};
// Set the ID
void User::setID(const std::string& id) {
	u_id = id;
}
// Get the ID
std::string User::getID() const {
	return u_id;
};

// Save user info the me file
bool User::saveToFile() {
	std::string privateKey;
	try {
		privateKey = Base64Wrapper::encode(u_private_key);
	}
	catch (...) {
		throw std::runtime_error("Couln't convert private key to base64");
	}
	std::ofstream file(Config::CLIENT_FILE_NAME.c_str());
	if (file.is_open()) {
		file << u_name << std::endl;
		file << Utils::bytesToHex(u_id) << std::endl;
		file << privateKey << std::endl;
		file.close();
	}
	else {
		throw std::runtime_error("Unable to open file");
	}
	u_active = true;
	return true;
}