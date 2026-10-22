import struct
from config import Config
class Response:
    
    def _create_response_header(code, payload_length):
        """
        Create a response header
        """
        return struct.pack('<BHI', Config.SERVER_VERSION, code, payload_length)
    
    @classmethod
    def registration_success(cls,payload):
        """Create a registration success response"""
        if len(payload) != Config.CLIENT_ID_SIZE:
            return False
        response = cls._create_response_header(Config.REGISTRATION_SUCCESS, len(payload))
        return response + struct.pack(f'<{Config.CLIENT_ID_SIZE}s', payload)
    
    
    @classmethod
    def create_error_response(cls):
        return cls._create_response_header(Config.ERROR_RESPONSE, 0)
    
    @classmethod
    def user_list_success(cls, users):
        """Create a user list success response"""
        payload  = b''
        for user in users:
            client_id, name, _, __ = user
            name_bytes = name.encode('ascii') + b'\x00'
            payload += struct.pack(f'<{Config.CLIENT_ID_SIZE}s{Config.NAME_SIZE}s', client_id, name_bytes)
        header = cls._create_response_header(Config.USER_LIST_SUCCESS, len(payload))
        return header + payload
    
    @classmethod
    def public_key_success(cls, client_id, public_key):
        """Create a public key success response"""
        payloadLen = len(client_id) + len(public_key)
        if payloadLen != Config.CLIENT_ID_SIZE + Config.PUBLIC_KEY_SIZE:
            return False
        header = cls._create_response_header(Config.PUBLIC_KEY_SUCCESS, payloadLen)
        return header + struct.pack(f'<{Config.CLIENT_ID_SIZE}s{Config.PUBLIC_KEY_SIZE}s', client_id, public_key)
    
    @classmethod
    def send_message_success(cls,target_client_id,message_id):
        """Create a send message success response"""
        payload = struct.pack(f'<{Config.CLIENT_ID_SIZE}sI', target_client_id, message_id)
        header = cls._create_response_header(Config.SEND_MESSAGE_SUCCESS, Config.CLIENT_ID_SIZE + Config.MESSAGE_ID_SIZE)
        return header + payload
    
    @classmethod
    def fetch_messages_success(cls, messages):
        """Create a fetch messages success response"""
        payload = b''
        for msg in messages:
            message_id, _, sender_client_id,message_type, message_content = msg
            message_size = len(message_content)
            payload += struct.pack(f'<{Config.CLIENT_ID_SIZE}sIBI{message_size}s', sender_client_id,message_id, message_type, message_size, message_content)
        header = cls._create_response_header(Config.FETCH_MESSAGES_SUCCESS, len(payload))
        return header + payload