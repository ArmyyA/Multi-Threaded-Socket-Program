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

std::atomic<bool> serverRunning(true);

void ShutdownListener(SocketServer &serverSocket)
{
    std::string command;
    while (serverRunning)
    {
        std::cin >> command;
        if (command == "shutdown")
        {
            serverRunning = false;
            try
            {
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

class ClientHandler : public Thread
{
private:
    Socket clientSocket;

public:
    ClientHandler(Socket clientSocket) : clientSocket(std::move(clientSocket)) {}

protected:
    virtual long ThreadMain() override
    {
        try
        {
            while (true)
            {
                ByteArray data;
                this->clientSocket.Read(data);

                // convert ByteArray to String
                std::string recData = data.ToString();

                if (recData == "done")
                {
                    std::cout << "A client is disconnecting."<< std::endl;
                    break;
                }

                std::cout << "Client connected and sent the string: " << recData << std::endl;

                std::string transformData = TransformString(recData);

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
    std::string TransformString(const std::string &input)
    {
        return std::string(input.rbegin(), input.rend());
    }
};

int main(void)
{
    // Vector to track client threads
    std::vector<std::unique_ptr<Thread>> ClientHandlers;

    try
    {
        SocketServer serverSocket(3000);
        std::thread listenerShutdown(ShutdownListener, std::ref(serverSocket));
        std::cout << "I am a server running on port 3000." << std::endl;

        while (serverRunning)
        {
            try
            {
                // Accepting connections
                Socket clientSocket = serverSocket.Accept();

                if (!serverRunning)
                    break;

                // Create and start new Client thread
                ClientHandlers.emplace_back(new ClientHandler(std::move(clientSocket)));
            }
            // Will be caught due to Shutdown
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

        if (listenerShutdown.joinable())
        {
            listenerShutdown.join();
        }
        ClientHandlers.clear();
        std::cout << "The Server has shutdown gracefully." << std::endl;
    }
    catch (const std::string &e)
    {
        std::cerr << "Error while creating server socket: " << e << std::endl;
    }

    return 0;
}
