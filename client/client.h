#pragma once
#include "user.h"
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include "config.h"
#include "utils.h"
#include "requestWrapper.h"
#include "responseWrapper.h"
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using boost::asio::ip::tcp;

class Client {
private:
	std::string c_address; // Url address
	std::string c_port; // port number
	boost::asio::io_context c_ioContext;
	User c_user; // the active user
	std::map<std::string, User> c_contact_list; // user list
	std::map<std::string, std::vector<unsigned char>> c_unknown_users; // users that got only symmetric key
	bool readAddressFromFile(); // read address and port from info.server
	void showMenu(); // show the menu
	void genericHandler(Request(RequestHandler::* requestMethod)(),
		void (ResponseHandler::* responseMethod)()); // generic request and response handler
	Response sendRequest(const Request&); // send reqeust funciton
public:
	Client();
	void run();
};