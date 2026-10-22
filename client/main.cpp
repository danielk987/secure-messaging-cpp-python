#include "client.h"
#include <iostream>

int main() {
	try {
		// Start the Client
		Client client;
		client.run();
	}
	catch (...) {
		// Shouldn't get here
		std::cerr << "Some Thing Went Wrong" << std::endl;
	}
	return 0;
}