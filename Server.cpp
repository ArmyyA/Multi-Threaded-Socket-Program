#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <memory>
#include <string>

using namespace Sync;

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
                    break;
                }

                std::string transformData = TransformString(recData);

                clientSocket.Write(ByteArray(transformData));
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

    try{
        SocketServer serverSocket(3000);
        std::cout << "I am a server running on port 3000." << std::endl;

        while(true){
            try{
                // Accepting connections
                Socket clientSocket = serverSocket.Accept();

                // Create and start new Client thread
                ClientHandlers.emplace_back(new ClientHandler(std::move(clientSocket)));
            }
            catch (const std::string& e){
                std::cerr <<"Error while accepting a new connection: " << e << std::endl;
            }
        }
    }
    catch (const std::string& e){
        std::cerr <<"Error while creating server socket: " << e << std::endl;
    }

    return 0;
}
