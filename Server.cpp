#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <thread>

using namespace Sync;

// Boolean to indicate server status
std::atomic<bool> serverRunning(true);

// Listens for the 'shutdown' command to close the server socket
void ShutdownListener(SocketServer &serverSocket)
{
    std::string command;
    while (serverRunning)
    {
        std::cin >> command;
        if (command == "shutdown")
        {
            // Sets the boolean to false
            serverRunning = false;
            try
            {
                // Uses shutdown in server socket
                serverSocket.Shutdown();
                break;
            }
            catch (const std::string &e)
            {
                std::cerr << "Error while shutting down server socket: " << e << std::endl;
            }
        }
    }
}

// A client class to handle client connections
class ClientHandler : public Thread
{
private:
    Socket clientSocket; // Client Socket

public:
    ClientHandler(Socket clientSocket) : clientSocket(std::move(clientSocket)) {}

protected:
    // Main function that handles client handler thread
    virtual long ThreadMain() override
    {
        try
        {
            while (true)
            {
                ByteArray data;
                // Read the data recieved
                this->clientSocket.Read(data);

                // convert ByteArray to String
                std::string recData = data.ToString();

                // If client disconects, break and close the socket.
                if (recData == "done")
                {
                    std::cout << "A client is disconnecting."<< std::endl;
                    break;
                }

                std::cout << "Client connected and sent the string: " << recData << std::endl;

                // Transform the incoming string and make it reversed
                std::string transformData = TransformString(recData);

                // Write the string back to the client
                clientSocket.Write(ByteArray(transformData));

                std::cout << "Sending client reversed string: " << transformData << std::endl;
            }
        }
        catch (...)
        {
            std::cerr << "An error has occured with the handler!" << std::endl;
        }

        clientSocket.Close();
        return 0;
    }

private:
    // For this lab, I simply chose to reverse the string when writing to client
    std::string TransformString(const std::string &input)
    {
        return std::string(input.rbegin(), input.rend());
    }
};

// Main function
int main(void)
{
    // Vector to track client threads
    std::vector<std::unique_ptr<Thread>> ClientHandlers;

    try
    {
        // Create the server socket on port 3000
        SocketServer serverSocket(3000);
        std::thread listenerShutdown(ShutdownListener, std::ref(serverSocket)); // Create a thread for shutdown listener
        std::cout << "I am a server running on port 3000." << std::endl;

        // Run infintely until 'shutdown' is inputted
        while (serverRunning)
        {
            try
            {
                // Accepting connections from client
                Socket clientSocket = serverSocket.Accept();

                // If server running happens to be false, commence shutdown
                if (!serverRunning)
                    break;

                // Create and start new Client thread
                ClientHandlers.emplace_back(new ClientHandler(std::move(clientSocket)));
            }
            // Exception will be caught due to Shutdown
            catch (const TerminationException&)
            {
                if (!serverRunning)
                {
                    std::cout << "Initiating Server shutdown." << std::endl;
                }
                break;
            }
            catch (const std::string &e)
            {
                if (!serverRunning)
                {
                    break;
                }
                std::cerr << "Error while accepting a new connection: " << e << std::endl;
            }
        }

        // Ensure the thread has finished
        if (listenerShutdown.joinable())
        {
            listenerShutdown.join();
        }

        // Clear all handlers to ensure threads are joined before exiting
        ClientHandlers.clear();
        std::cout << "The Server has shutdown gracefully." << std::endl;
    }
    catch (const std::string &e)
    {
        std::cerr << "Error while creating server socket: " << e << std::endl;
    }

    return 0;
}
