#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <random>
#include "config.h"

class Utils {
public:
	static std::string hexToBytes(std::string&);
	static std::string bytesToHex(std::string&);
    static std::string getEnvTemp();
    static bool fileExists(const std::string& path);
    static std::string generateRandomString();
    // Pack integer in little-endian format
    template<typename T>
    static std::vector<uint8_t> packLittleEndian(T value) {
        std::vector<uint8_t> bytes(sizeof(T));
        for (size_t i = 0; i < sizeof(T); ++i) {
            bytes[i] = (value >> (i * 8)) & 0xFF;
        }
        return bytes;
    }

    // Unpack integer from little-endian format
    template<typename T>
    static T unpackLittleEndian(const std::vector<uint8_t>& bytes, size_t offset) {
        T value = 0;
        for (size_t i = 0; i < sizeof(T) && offset + i < bytes.size(); ++i) {
            value |= static_cast<T>(bytes[offset + i]) << (i * 8);
        }
        return value;
    };

	
};