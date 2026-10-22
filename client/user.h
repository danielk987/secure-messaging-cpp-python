#pragma once

#include <string>
#include "config.h"
#include "utils.h"
#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"
#include <fstream>
#include <vector>
#include <iostream>

class User {
private:
	std::string u_name;
	std::string u_id;
	std::string u_private_key;
	std::string u_public_key;
	std::vector<unsigned char> u_symmetric_key;
	bool u_active;
	void readUserFromFile();
public:
	User();
	User(const std::string&,const std::string&);
	void setActiveUser(const std::string&,const std::string&);
	void generateSymmetricKey();
	bool isActive();
	std::string generateRSAKeys();

	void setPublicKey(const std::string&);
	std::string getPublicKey();

	std::string getPrivateKey();

	void setSymmetricKey(const std::vector<unsigned char>&);
	const std::vector<unsigned char>& getSymmetricKey() const;

	void setName(const std::string&);
	std::string getName() const;

	void setID(const std::string&);
	std::string getID() const;

	bool saveToFile();
};