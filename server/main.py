from server import Server
def main():
    try:
        server = Server()
        server.run()
    except Exception as e:
        print(f"Something went wrong: {e}")

if __name__ == "__main__":
    main()