import struct
from config import Config
from response import Response
import uuid

class RequestHeader:
    
    def parse_request_header(self, header_bytes):
        """Parse the request header from bytes"""
        if len(header_bytes) < Config.REQUEST_HEADER_SIZE:
            return False
        
        # Parse little-endian header
        client_id, version, code, payload_size = struct.unpack(f'<{Config.CLIENT_ID_SIZE}sBHI', header_bytes[:Config.REQUEST_HEADER_SIZE])
        self.client_id = client_id
        self.version = version
        self.code = code
        self.payload_size = payload_size
        return True
    
    

class Request:
    def __init__(self, header, payload,logger,database):
        self._header = header
        self._payload = payload
        self._logger = logger
        self._database = database   
    
    def request_handler(self):
        # Handle request based on header code
        if self._header.code == Config.REGISTRATION_CODE:
            return self._register_handler()
        elif self._header.code == Config.USER_LIST_CODE:
            return self._user_list_handler()
        elif self._header.code == Config.PUBLIC_KEY_CODE:
            return self._public_key_handler()
        elif self._header.code == Config.SEND_MESSAGE_CODE:
            return self._send_message_handler()
        elif self._header.code == Config.FETCH_MESSAGES_CODE:
            return self._fetch_messages_handler()
        else:
            self._logger.error(f"Unknown request code: {self._header.code}")
            return False
        
    def _register_handler(self):
        """Handle client registration"""
        if len(self._payload) < Config.REGISTRATION_REQUEST_SIZE:
            self._logger.error("Invalid registration payload size")
            return False
        name_bytes, public_key = struct.unpack(f'<{Config.NAME_SIZE}s{Config.PUBLIC_KEY_SIZE}s', self._payload[:Config.REGISTRATION_REQUEST_SIZE])
        # find the null terminator
        null_index = name_bytes.find(b'\x00')
        # Check if null terminator exists
        if null_index == -1:
            self._logger.error("Invalid name in registration payload")
            return False
        name = name_bytes[:null_index].decode('ascii', errors='ignore')
        # check if name not empty
        if not name:
            self._logger.error("Empty name in registration payload")
            return False
        # check if client already registered
        if self._database.get_client_by_name(name):
            self._logger.error(f"Client {name} already registered")
            return False        
        unique_id = uuid.uuid4().bytes
        # check if id already exists
        while self._database.get_client_by_id(unique_id):
            unique_id = uuid.uuid4().bytes
        
        self._database.add_client(unique_id, name, public_key)
        return Response.registration_success(unique_id)
        
    def _check_user_exists(self):
        """Check if a client exists in the database"""
        client_id = self._header.client_id
        client = self._database.get_client_by_id(client_id)
        if not client:
            self._logger.error(f"Client ID {client_id.hex()} not found")
            return False
        # Update the last seen for the client
        self._database.update_client_last_seen(client_id)
        return True
    
    def _user_list_handler(self):
        """Handle user list request"""
        if not self._check_user_exists():
            return False
        users = self._database.get_clients_list(self._header.client_id)
        self._logger.info(f"Sending user list to {self._header.client_id.hex()}")
        return Response.user_list_success(users)
        
    def _public_key_handler(self):
        """Handle public key request"""
        if len(self._payload) < Config.CLIENT_ID_SIZE:
            self._logger.error("Invalid public key request payload size")
            return False
        if not self._check_user_exists():
            return False
        target_client_id = struct.unpack(f"<{Config.CLIENT_ID_SIZE}s",self._payload[:Config.CLIENT_ID_SIZE])[0]
        target_client = self._database.get_client_by_id(target_client_id)
        if not target_client:
            self._logger.error(f"Target client ID {target_client_id.hex()} not found")
            return False
        
        _, name, public_key, __ = target_client
        self._logger.info(f"Sending public key of {name} to {self._header.client_id.hex()}")
        return Response.public_key_success(target_client_id, public_key)
        
    def _send_message_handler(self):
        """Handle send message request"""
        if len(self._payload) < Config.MESSAGE_MIN_SIZE:
            self._logger.error("Invalid send message payload size")
            return False
        if not self._check_user_exists():
            return False
        
        target_client_id, message_type, content_size = struct.unpack(f"<{Config.CLIENT_ID_SIZE}sBI",self._payload[:Config.MESSAGE_MIN_SIZE])
        
        if len(self._payload) < Config.MESSAGE_MIN_SIZE + content_size:
            self._logger.error("Message Smaller than expected")
            return False
        
        message_content = struct.unpack(f"<{content_size}s", self._payload[Config.MESSAGE_MIN_SIZE:])[0] 
        
        # Check if target client exists
        target_client = self._database.get_client_by_id(target_client_id)
        if not target_client:
            self._logger.error(f"Target client ID {target_client_id.hex()} not found")
            return False
        
        # Store the message in the database
        message_id = self._database.add_message(target_client_id, self._header.client_id, message_type, message_content)
        if message_id is None:
            self._logger.error("Failed to store message")
            return False
        return Response.send_message_success(target_client_id,message_id)
    
    def _fetch_messages_handler(self):
        """Handle fetch messages request"""
        if not self._check_user_exists():
            return False
        
        messages = self._database.get_all_messages(self._header.client_id)
        self._logger.info(f"Sending {len(messages)} messages to {self._header.client_id.hex()}")
        return Response.fetch_messages_success(messages)