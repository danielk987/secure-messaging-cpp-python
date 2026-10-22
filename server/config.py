class Config:
    """
    Configuration class for the server
    """
    DEFAUTL_HOST = ''
    DEFAULT_PORT = 1357
    PORT_FILE_NAME = 'myport.info'
    DB_NAME = 'defensive.db'
    SERVER_VERSION = 2
    MAX_CLIENTES_PER_CONNECTION = 10
    MAX_PACKAGE_SIZE = 4096
    
    # Request header sizes
    CLIENT_ID_SIZE = 16
    VESRSION_SIZE = 1
    CODE_SIZE = 2
    REQUEST_PAYLOAD_SIZE = 4
    REQUEST_HEADER_SIZE = CLIENT_ID_SIZE + VESRSION_SIZE + CODE_SIZE + REQUEST_PAYLOAD_SIZE  # 23 bytes

    
    # Request codes
    REGISTRATION_CODE = 600
    USER_LIST_CODE = 601
    PUBLIC_KEY_CODE = 602
    SEND_MESSAGE_CODE = 603
    FETCH_MESSAGES_CODE = 604
    
    # Request Sizes
    NAME_SIZE = 255
    PUBLIC_KEY_SIZE = 160
    REGISTRATION_REQUEST_SIZE = NAME_SIZE + PUBLIC_KEY_SIZE  # 255 (Name) + 160 (PublicKey)
    MESSAGE_MIN_SIZE = CLIENT_ID_SIZE + 5
    MESSAGE_ID_SIZE = 4
    
    # Response Codes
    REGISTRATION_SUCCESS = 2100
    USER_LIST_SUCCESS = 2101
    PUBLIC_KEY_SUCCESS = 2102
    SEND_MESSAGE_SUCCESS = 2103
    FETCH_MESSAGES_SUCCESS = 2104
    ERROR_RESPONSE = 9000
    
    
    @classmethod
    def get_port(cls):
        """
        Read the port from a file and validate it
        if valid return it , otherwise return the default port
        """
        try:
            with open(cls.PORT_FILE_NAME, 'r') as file:
                port = int(file.read().strip())
                if 1 <= port <= 65535:
                    return port
        except (FileNotFoundError, ValueError):
            return cls.DEFAULT_PORT
        return cls.DEFAULT_PORT