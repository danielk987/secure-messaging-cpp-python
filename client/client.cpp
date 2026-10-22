#include "client.h"

Client::Client() :c_address(), c_port(), c_ioContext(), c_user(User{}), c_contact_list({}), c_unknown_users({}) {
}

// Reading the ip and port from the info.server
bool Client::readAddressFromFile() {
	std::ifstream file(Config::SERVER_FILE_NAME.c_str());
	if (!file.is_open()) {
		return false;
	}
	std::string line;
	if (std::getline(file, line)) {
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			c_address = line.substr(0, colonPos);
			c_port = line.substr(colonPos + 1);
			return true;
		}
	}
	return false;
}

// Show the client menu
void Client::showMenu() {
	std::cout << "MessageU client at your service." << std::endl;
	std::cout << std::endl;
	std::cout << Config::USER_REGISTER_CHOICE << ") Register" << std::endl;
	std::cout << Config::USER_LIST_CHOICE << ") Request for clients list" << std::endl;
	std::cout << Config::PUBLIC_KEY_CHOICE << ") Request for public key" << std::endl;
	std::cout << Config::FETCH_MESSAGES_CHOICE << ") Request for waiting messages" << std::endl;
	std::cout << Config::SEND_MESSAGE_CHOICE << ") Send a text message" << std::endl;
	std::cout << Config::REQUEST_SYMMETRIC_CHOICE << ") Send a request for symmetric key" << std::endl;
	std::cout << Config::SEND_SYMMETRIC_CHOICE << ") Send your symmetric key" << std::endl;
	std::cout << Config::SEND_FILE_CHOICE << ") Send File" << std::endl;
	std::cout << Config::EXIT_CHOICE << ") Exit client" << std::endl;
	std::cout << "?" << std::endl;
}

// Start poitn of the client, show the menu and get the users action
void Client::run() {
	bool isReadSuccess = readAddressFromFile();
	if (!isReadSuccess) {
		std::cerr << "Couldn't read server file: " << Config::SERVER_FILE_NAME << std::endl;
		return;
	}
	std::string line;
	while (true) {
		showMenu();
		std::getline(std::cin, line);
		try {
			if (line == Config::USER_REGISTER_CHOICE) {
				genericHandler(&RequestHandler::registerHandler, &ResponseHandler::registerHandler);
			}
			else if (line == Config::USER_LIST_CHOICE) {
				genericHandler(&RequestHandler::userListHandler, &ResponseHandler::userListHandler);
			}
			else if (line == Config::PUBLIC_KEY_CHOICE) {
				genericHandler(&RequestHandler::publicKeyHandler, &ResponseHandler::publicKeyHandler);
			}
			else if (line == Config::FETCH_MESSAGES_CHOICE) {
				genericHandler(&RequestHandler::fetchMessagesHandler, &ResponseHandler::fetchMessagesHandler);
			}
			else if (line == Config::SEND_MESSAGE_CHOICE) {
				genericHandler(&RequestHandler::sendMessageHandler, &ResponseHandler::sendMessageHandler);
			}
			else if (line == Config::REQUEST_SYMMETRIC_CHOICE) {
				genericHandler(&RequestHandler::requestSymmetricHandler, &ResponseHandler::requestSymmetricHandler);
			}
			else if (line == Config::SEND_SYMMETRIC_CHOICE) {
				genericHandler(&RequestHandler::sendSymmetricHandler, &ResponseHandler::sendSymmetricHandler);
			}
			else if (line == Config::SEND_FILE_CHOICE) {
				genericHandler(&RequestHandler::sendFileHandler, &ResponseHandler::sendFileHandler);
			}
			else if (line == Config::EXIT_CHOICE) {
				break;
			}
			else {
				std::cout << "Invalid choice please try again" << std::endl;
			}
		}
		catch (const std::runtime_error& e) {
			std::cout << e.what() << std::endl;
		}
		catch (const boost::system::system_error& e) {
			std::cout << "Connection Error: " << e.what() << std::endl;
		}
	}

}
// Funciton to send the request and reutrn the response
Response Client::sendRequest(const Request& request) {
	tcp::socket socket(c_ioContext);

	// Resolve server address
	tcp::resolver resolver(c_ioContext);
	tcp::resolver::results_type endpoints = resolver.resolve(c_address, c_port);

	// Connect to server
	boost::asio::connect(socket, endpoints);
	// Send request
	boost::asio::write(socket, boost::asio::buffer(request.createPayload()));

	// Receive response header (7 bytes)
	std::vector<uint8_t> header(Config::RESPONSE_HEADER_SIZE);
	boost::asio::read(socket, boost::asio::buffer(header));

	Response serverResponse;
	serverResponse.createHeader(header);

	// Receive payload if any
	if (serverResponse.getPayloadSize() > 0) {
		std::vector<uint8_t> payload(serverResponse.getPayloadSize());
		boost::asio::read(socket, boost::asio::buffer(payload));
		serverResponse.setPayload(payload);
	}
	socket.close();
	return serverResponse;
}
// Generic function to handle the request and response for each action
void Client::genericHandler(
	Request(RequestHandler::* requestMethod)(),
	void (ResponseHandler::* responseMethod)()){
	auto payload = (RequestHandler(c_user, c_contact_list).*requestMethod)();
	auto response = sendRequest(payload);
	ResponseHandler responseHandler(response, c_user, c_contact_list, c_unknown_users);
	(responseHandler.*responseMethod)();
}


