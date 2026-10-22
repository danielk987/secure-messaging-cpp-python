#pragma once
#include <string>

class Config {
public:

    static constexpr uint8_t CLIENT_VERSION = 2;
    static constexpr int MAX_NAME_SIZE = 254;
    static constexpr int RESPONSE_HEADER_SIZE = 7;

    // Request Codes
    static constexpr uint16_t REGISTRATION_CODE = 600;
    static constexpr uint16_t USER_LIST_CODE = 601;
    static constexpr uint16_t PUBLIC_KEY_CODE = 602;
    static constexpr uint16_t SEND_MESSAGE_CODE = 603;
    static constexpr uint16_t FETCH_MESSAGES_CODE = 604;
    // Response Codes
    static constexpr uint16_t REGISTRATION_SUCCESS = 2100;
    static constexpr uint16_t USER_LIST_SUCCESS = 2101;
    static constexpr uint16_t PUBLIC_KEY_SUCCESS = 2102;
    static constexpr uint16_t SEND_MESSAGE_SUCCESS = 2103;
    static constexpr uint16_t FETCH_MESSAGES_SUCCESS = 2104;
    static constexpr uint16_t ERROR_RESPONSE = 9000;

    // Response sizes
    static constexpr int VERSION_SIZE = 1;
    static constexpr int MESSAGE_TYPE_SIZE = 1;
    static constexpr int CODE_SIZE = 2;
    static constexpr int CLIENT_ID_SIZE = 16;
    static constexpr int MESSAGE_ID_SIZE = 4;
    static constexpr int MESSAGE_SIZE_FIELD_SIZE = 4;

    // Messages Types
    static constexpr uint8_t REQUEST_SYMMETRIC_TYPE = 1;
    static constexpr uint8_t SEND_SYMMETRIC_TYPE = 2;
    static constexpr uint8_t SEND_MESSAGE_TYPE = 3;
    static constexpr uint8_t SEND_FILE_TYPE = 4;


    static const std::string CLIENT_FILE_NAME;
    static const std::string SERVER_FILE_NAME;

    // User Choice
    static const std::string USER_REGISTER_CHOICE;
    static const std::string USER_LIST_CHOICE;
    static const std::string PUBLIC_KEY_CHOICE;
    static const std::string FETCH_MESSAGES_CHOICE;
    static const std::string SEND_MESSAGE_CHOICE;
    static const std::string REQUEST_SYMMETRIC_CHOICE;
    static const std::string SEND_SYMMETRIC_CHOICE;
    static const std::string SEND_FILE_CHOICE;
    static const std::string EXIT_CHOICE;
};