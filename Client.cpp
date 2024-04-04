#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// Main function
int main(void)
{
	// std::cout << "I am a client" << std::endl;

	// IP and Port of the server
	std::string serverIP = "127.0.0.1";
	unsigned int serverPort = 3000;

	try
	{
		// Initialize the socket
		Socket clientSocket(serverIP, serverPort);

		// Attemps to open a connection with the server
		if (clientSocket.Open() < 0)
		{
			std::cerr << "Unable to connect to the server." << std::endl;
			return -1;
		}

		std::cout << "I am a client connected to server on port 3000." << std::endl;

		// Loop until user enters 'done'
		while (true)
		{
			std::cout << "Enter a string (anything) or enter 'done' to exit: ";
			std::string input;
			std::getline(std::cin, input);

			// If user enters done, the string is written to the server and client socket closes
			if (input == "done")
			{
				clientSocket.Write(ByteArray(input)); // Notify the server that client is disconnecting
				break;
			}

			// Write the input to the server (unless it fails)
			if (clientSocket.Write(ByteArray(input)) < 0)
			{
				std::cerr << "Unable to write to the server." << std::endl;
				break;
			}

			// Wait for the response from the server
			ByteArray response;
			
			// Read the response, unless it fails to do so
			if (clientSocket.Read(response) < 0)
			{
				std::cerr << "Unable to read response from the server" << std::endl;
				break;
			}

			// Print the response
			std::cout << "Message from server: " << response.ToString() << std::endl;
		}

		// If the loop was broken, then socket should be closed
		clientSocket.Close();
	}
	catch (const std::string &e)
	{
		std::cerr << "Error has occured: " << e << std::endl;
		return -1;
	}
	return 0;
}
