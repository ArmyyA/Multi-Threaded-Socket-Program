#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

int main(void)
{

	// std::cout << "I am a client" << std::endl;

	std::string serverIP = "127.0.0.1";
	unsigned int serverPort = 3000;

	try
	{
		// Initialize the socket
		Socket clientSocket(serverIP, serverPort);

		if (clientSocket.Open() < 0)
		{
			std::cerr << "Unable to connect to the server." << std::endl;
			return -1;
		}

		std::cout << "I am a client connected to server on port 3000." << std::endl;

		while (true)
		{
			std::cout << "Enter a string (anything) or enter 'done' to exit: ";
			std::string input;
			std::getline(std::cin, input);

			if (input == "done")
			{
				break;
			}

			if (clientSocket.Write(ByteArray(input)) < 0)
			{
				std::cerr << "Unable to write to the server." << std::endl;
				break;
			}

			ByteArray response;
			if (clientSocket.Read(response) < 0)
			{
				std::cerr << "Unable to read response from the server" << std::endl;
				break;
			}

			std::cout << "Message from server: " << response.ToString() << std::endl;
		}

		clientSocket.Close();
	}
	catch (const std::string &e)
	{
		std::cerr << "Error has occured: " << e << std::endl;
		return -1;
	}
	return 0;
}
