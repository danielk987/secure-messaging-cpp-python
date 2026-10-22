#include "utils.h"

// Convert hex to string
std::string Utils::hexToBytes(std::string& hex) {
	std::string bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(static_cast<char>(byte));
    }
    return bytes;
}

// Convert string to hex
std::string Utils::bytesToHex(std::string& bytes) {
    std::stringstream ss;
    ss << std::hex;
    for (unsigned char byte : bytes) {
        if (byte < 16) ss << '0'; 
        ss << static_cast<int>(byte);
    }
    return ss.str();
}

// Getting the TMP dir
std::string Utils::getEnvTemp() {
#if defined(_WIN32)
    char* buff = nullptr;
    size_t cnt = 0;
    if (_dupenv_s(&buff, &cnt, "TMP") != 0 || !buff) {
        return ""; 
    }
    std::string value(buff);
    free(buff);
    return value;
#elif defined(__linux__)
    const char* s = getenv("TMPDIR");
    if (!s) return ""; 
    return std::string(s); 
#endif
}

// Check if file exists
bool Utils::fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

// Generate a random string starting with messageU_
std::string Utils::generateRandomString() {
    std::random_device rd;
    return "messageU_" + std::to_string(rd());
}