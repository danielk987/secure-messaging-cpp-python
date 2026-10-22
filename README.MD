# Encrypted Messaging System (C++ Client + Python Server)

A secure end-to-end encrypted messaging system: a **C++ client** and a **multi-client Python server**
communicating over a custom binary protocol. Users register, exchange keys, and send encrypted
messages and files to each other. The server only ever stores ciphertext.

Built as the final project of the Open University course
[20937 – Defensive Systems Programming](https://www.openu.ac.il/courses/20937.htm?t=h).

**Final project grade: 100.**

## Features

- **Custom binary protocol** – fixed little-endian headers (client ID, version, code, payload size) with strict validation on both sides.
- **End-to-end encryption** – RSA key pair per user for key exchange, AES symmetric keys for message and file content.
- **Messaging** – text messages, file transfer, and symmetric key request/send between users.
- **Multi-client server** – non-blocking socket server using `selectors`, with per-connection buffering for large payloads.
- **Persistence** – SQLite database (users and pending messages) with `defensive.db`.
- **Defensive programming** – input and size validation, bounded buffers, generic error handling, and server-side logging.

## Architecture

```
┌──────────────┐     custom binary protocol (TCP)     ┌───────────────────┐
│  C++ Client  │ ───────────────────────────────────▶ │   Python Server   │
│ RSA + AES    │ ◀─────────────────────────────────── │ selectors + SQLite│
└──────────────┘                                      └───────────────────┘
```

### Request codes

| Code | Action                  |
|------|-------------------------|
| 600  | Register                |
| 601  | Get users list          |
| 602  | Get a user's public key |
| 603  | Send message            |
| 604  | Fetch waiting messages  |

Message types: request symmetric key, send symmetric key, text message, file.

## Running

The server and client use the wrapper classes for Base64, AES and RSA, which come from the
[Crypto++](https://www.cryptopp.com/) library, so it must be available when building the client.

**Server** (Python 3):

```bash
cd server
python main.py
```

The port is read from `myport.info` (falls back to `1357`).

**Client** (C++17):

Build all files in `client/` (for example in Visual Studio, linked against Crypto++), then run it next to
`info.server` which holds the server `address:port`.

## Project structure

### Server (`server/`)

| File          | Responsibility                                                                                           |
|---------------|----------------------------------------------------------------------------------------------------------|
| `main.py`     | Entry point; creates the server with generic error handling.                                             |
| `server.py`   | Sets up the DB and logger (`server.log`), listens for connections via a selector, buffers requests by payload size, and dispatches to `Request`/`Response`. |
| `config.py`   | All constants (codes, sizes, port loading from `myport.info`).                                           |
| `database.py` | SQLite logic: creates tables if missing and runs queries.                                                |
| `requests.py` | `RequestHeader` (unpack and validate) and `Request` (handle each request code and store/fetch data).     |
| `response.py` | Builds response headers and payloads for each action.                                                    |

### Client (`client/`)

| File                               | Responsibility                                                                                   |
|------------------------------------|--------------------------------------------------------------------------------------------------|
| `main.cpp`                         | Entry point; runs the `Client` with generic error handling.                                      |
| `client.cpp/h`                     | Reads `info.server`, shows the menu, sends requests and receives responses; stores contacts and unknown users. |
| `requestWrapper.cpp/h`             | `Request` and `RequestHandler`: build, validate, and serialize requests.                         |
| `responseWrapper.cpp/h`            | `Response` and `ResponseHandler`: parse and validate server responses.                           |
| `message.cpp/h`                    | `Message` base class and one subclass per message type (decryption and display).                 |
| `user.cpp/h`                       | Active user and contact details (keys, IDs).                                                     |
| `RSAWrapper`, `AESWrapper`, `Base64Wrapper` | Cryptography and encoding helpers.                                                      |
| `config.cpp/h`, `utils.cpp/h`      | Constants, hex conversion, little-endian pack/unpack, etc.                                       |

## Course

Open University of Israel – 20937 Defensive Systems Programming (B.Sc. Computer Science).
