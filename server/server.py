from config import Config
from database import Database
from response import Response
import requests
import logging
import os
import socket
import selectors


class Server:
    """
    Server class that initializes the server with configuration settings
    """
    def __init__(self):
        self._host = Config.DEFAUTL_HOST
        self._port = Config.get_port()
        self._configure_logging()
        self._database = Database(self._logger)
        self.sel = selectors.DefaultSelector()
        self.connection_buffer = {} # this will hold partial data for each connection
    
    def accept_connection(self, sock, mask):
        """Accept new client connection"""
        try:
            conn, _ = sock.accept()
            conn.setblocking(False)
            self.connection_buffer[conn] = b''
            self.sel.register(conn, selectors.EVENT_READ, self.handle_client_data)
            
        except Exception as e:
            self._logger.error(f"Error accepting connection: {e}")
            
    def handle_client_data(self, conn, mask):
        """Handle data from client"""
        try:
            data = conn.recv(Config.MAX_PACKAGE_SIZE)
            if not data:
                conn.send(Response.create_error_response())
                self.close_client_connection(conn)
                return
            
            self.connection_buffer[conn] += data
            # Try to process message from buffer
            self.process_buffer(conn)
            
        except Exception as e:
            self._logger.error(f"Error handling client data: {e}")
            self.close_client_connection(conn)
    
    
    def process_buffer(self, conn):
        """Process complete messages from buffer"""
        buffer = self.connection_buffer[conn]
        # Not enough data for header
        if len(buffer) < Config.REQUEST_HEADER_SIZE:
            return
        
        header_bytes = buffer[:Config.REQUEST_HEADER_SIZE]
        header = requests.RequestHeader()
        if not header.parse_request_header(header_bytes):
            self._logger.error("Invalid header received, closing connection")
            conn.send(Response.create_error_response())
            self.close_client_connection(conn)
            return
        
        if len(buffer) < Config.REQUEST_HEADER_SIZE + header.payload_size:
            return  # Wait for more data
        
        payload = buffer[Config.REQUEST_HEADER_SIZE:Config.REQUEST_HEADER_SIZE + header.payload_size]
        request_response = requests.Request(header, payload, self._logger, self._database).request_handler()
        if not request_response:
            self._logger.error("Request handling failed, closing connection")
            conn.send(Response.create_error_response())
            self.close_client_connection(conn)
            return
        conn.send(request_response)
        if header.code == Config.FETCH_MESSAGES_CODE:
            self._database.delete_all_messages(header.client_id)
        self.close_client_connection(conn)
        
        
    def close_client_connection(self, conn):
        """Clean up a client connection"""
        
        if conn in self.connection_buffer:
            del self.connection_buffer[conn]
        
        try:
            self.sel.unregister(conn)
        except KeyError:
            pass
        try:
            conn.close()
        except:
            pass
    
    def run(self):
        try:
            # Create the server socket
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.bind((self._host, self._port))
            sock.listen(Config.MAX_CLIENTES_PER_CONNECTION)
            sock.setblocking(False)
            self.sel.register(sock, selectors.EVENT_READ, self.accept_connection)
            self._logger.info(f'Server Listenning to requests')
            while True:
                events = self.sel.select() 
                for key, mask in events:
                    callback = key.data 
                    callback(key.fileobj, mask) 
        except Exception as e:
            print(f"Server error: {e}")
        finally:
            self.sel.close()
        pass
    
    
    def _configure_logging(self):
        """
        Configures logging for the server
        """
        script_dir = os.path.dirname(os.path.abspath(__file__))
        log_file = os.path.join(script_dir, 'server.log')
        
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file),
                logging.StreamHandler()
            ]
        )
        self._logger = logging.getLogger(__name__)
        self._logger.info(f'Server started on {self._host}:{self._port}')