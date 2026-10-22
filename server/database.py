import sqlite3
from config import Config
import os

class Database:
    """
    Database class that initializes the database connection and tables
    will handle database operations
    """
    def __init__(self,logger):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        self._name = os.path.join(script_dir, Config.DB_NAME)
        self._logger = logger
        self._initialize_database() 

    def _initialize_database(self):
        """
        Initializes the database connection and creates tables
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                # create users table
                cursor.execute('''
                CREATE TABLE IF NOT EXISTS clients (
                    ID BLOB PRIMARY KEY,
                    UserName TEXT NOT NULL,
                    PublicKey BLOB,
                    LastSeen DATETIME DEFAULT CURRENT_TIMESTAMP
                )''')
                
                # create messages table
                cursor.execute('''
                CREATE TABLE IF NOT EXISTS messages (
                    ID INTEGER PRIMARY KEY AUTOINCREMENT,
                    ToClient BLOB NOT NULL,
                    FromClient BLOB NOT NULL,
                    Type INTEGER NOT NULL,
                    Content BLOB NOT NULL,
                    FOREIGN KEY (ToClient) REFERENCES clients (ID)
                    FOREIGN KEY (FromClient) REFERENCES clients (ID)
                )''')
                
                conn.commit()
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while initializing the database: {e}")
    
    def get_client_by_name(self, username):
        """
        Retrieves a client from the database by username
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('SELECT * FROM clients WHERE UserName = ?', (username,))
                return cursor.fetchone()
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while retrieving client: {e}")
            return None
    
    def get_client_by_id(self, client_id):
        """
        Retrieves a client from the database by client ID
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('SELECT * FROM clients WHERE ID = ?', (client_id,))
                return cursor.fetchone()
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while retrieving client: {e}")
            return None
    
    def add_client(self, client_id, username, public_key):
        """
        Adds a new client to the database
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('INSERT INTO clients (ID, UserName, PublicKey) VALUES (?, ?, ?)',
                               (client_id, username, public_key))
                conn.commit()
                self._logger.info(f"Client {username} added to the database")
                return True
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while adding client: {e}")
            
        return False
            
    def update_client_last_seen(self, client_id):
        """
        Updates the last seen timestamp for a client
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('UPDATE clients SET LastSeen = CURRENT_TIMESTAMP WHERE ID = ?', (client_id,))
                conn.commit()
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while updating last seen: {e}")
            
    def get_clients_list(self,client_id):
        """
        Retrieves a list of all clients from the database
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('SELECT * FROM clients WHERE ID != ?', (client_id,))
                return cursor.fetchall()
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while retrieving clients list: {e}")
        return []
    
    def add_message(self, to_client, from_client, message_type, content):
        """
        Adds a new message to the database
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('INSERT INTO messages (ToClient, FromClient, Type, Content) VALUES (?, ?, ?, ?)',
                               (to_client, from_client, message_type, content))
                conn.commit()
                message_id = cursor.lastrowid
                self._logger.info(f"Message from {from_client.hex()} to {to_client.hex()} added to the database with ID {message_id}")
                return message_id
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while adding message: {e}")
        return None
    
    def get_all_messages(self, client_id):
        """
        Retrieves messages for a specific client
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('SELECT * FROM messages WHERE ToClient = ?', (client_id,))
                self._logger.info(f"All messages for client {client_id.hex()} were retrieved")
                return cursor.fetchall()
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while retrieving messages: {e}")
            return []
        
    def delete_all_messages(self, client_id):
        """
        Deletes all messages for a specific client
        """
        try:
            with sqlite3.connect(self._name) as conn:
                cursor = conn.cursor()
                cursor.execute('DELETE FROM messages WHERE ToClient = ?', (client_id,))
                conn.commit()
                self._logger.info(f"All messages for client {client_id.hex()} were deleted")
        except sqlite3.Error as e:
            self._logger.error(f"An error occurred while deleting messages: {e}")