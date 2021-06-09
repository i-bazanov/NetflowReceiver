#include "CommonLibsAndFuncs.h"

#include "FileWriter.h"

#include <fstream>
#include <iostream>
#include <ctime>

#ifdef PLATFORM_WINDOWS
#include <thread>
#else
#include <pthread.h>
#include <cstring>
#endif // PLATFORM_WINDOWS


unsigned int bufFWCopy[9];

#ifdef PLATFORM_WINDOWS
void FW::FileWriter::writeToFile()
#else
void* writeToFile(void* path)
#endif // PLATFORM_WINDOWS
{
    init_sockets();

    socktype_t s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == SOCKET_ERROR)
    {
        std::cout << "Error creating socket" << std::endl;
        return_error();
    }

    struct sockaddr_in addr;
    int addr_size = sizeof(addr);
    addr.sin_family = AF_INET;

#ifdef PLATFORM_WINDOWS
    addr.sin_addr.S_un.S_addr = ADDR_ANY;
#else
    addr.sin_addr.s_addr = INADDR_ANY;
#endif // PLATFORM_WINDOWS

    addr.sin_port = htons(9996);

    if (bind(s, (sockaddr*)&addr, addr_size) == SOCKET_ERROR)
    {
        std::cout << "Can't bind socket!" << std::endl;
        return_error();
    }

    sockaddr_in client; // Use to hold the client information (port / ip address)
#ifdef PLATFORM_WINDOWS
    int clientLength = sizeof(client); // The size of the client information
#else
    unsigned int clientLength = sizeof(client); // The size of the client information
#endif // PLATFORM_WINDOWS

    ////////////////////////////// [0] -> V9, [1] -> IPFIX, [2] -> other //////////////////////////////
    int foutputTraffic[3] = { 0, 0, 0 };

    ////////////////////////////////////// [0] -> Ip, [1] -> Port //////////////////////////////////////
    int IpAndPort[2] = { 0, 0 };
    size_t bufIpAndPortSize = 2;

    char clientIp[15];
    std::memset(clientIp, 0, sizeof(clientIp));

    ////////////////////////////////////// Creating file stream ///////////////////////////////////////
    std::ofstream foutput((const char*)path);

    if (!foutput)
    {
        std::cout << "Error: file isn't able to open" << std::endl;
        return_error();
    }

    ////////////////////////////////////////// Creating title //////////////////////////////////////////
    foutput << "DATE_TIME, SOURCE, V9, IPFIX, Other" << std::endl;

    bool isReceiveIpAndPort = true;

    while (true)
    {
        std::memset(&client, 0, clientLength); // Clear the client structure
        std::memset(&bufFWCopy, 0, sizeof(bufFWCopy)); // Clear the receive buffer

        // Receive data for file output
        int bytesIn = recvfrom(s, (char*)bufFWCopy, sizeof(bufFWCopy), 0, (struct sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cout << "Error receiving from client" << std::endl;
            continue;
        }

        if (!isReceiveIpAndPort)
        {
            foutput << bufFWCopy[0] << "-" << bufFWCopy[1] << "-" << bufFWCopy[2] << " "
                << bufFWCopy[3] << ":" << bufFWCopy[4] << ":" << bufFWCopy[5] << ", "
                << clientIp << ":" << IpAndPort[1] << ", "
                << bufFWCopy[6] - foutputTraffic[0] << ", " << bufFWCopy[7] - foutputTraffic[1] << ", " << bufFWCopy[8] - foutputTraffic[2] << std::endl;

            foutputTraffic[0] = bufFWCopy[6];
            foutputTraffic[1] = bufFWCopy[7];
            foutputTraffic[2] = bufFWCopy[8];

            std::cout << "Writing to file ..." << std::endl;
        }
        else
        {
            for (int i = 0; i < bufIpAndPortSize; ++i)
            {
                IpAndPort[i] = bufFWCopy[i];
            }

            // Convert from byte array to chars
            inet_ntop(AF_INET, &IpAndPort[0], clientIp, sizeof(clientIp));

            isReceiveIpAndPort = false;
        }
    }

    foutput.close();
}

FW::FileWriter::FileWriter(const char* newPath) : path(newPath) 
{
#ifdef PLATFORM_WINDOWS
    t = std::thread(&FileWriter::writeToFile, this);
#else
    int checkThreadCreate = pthread_create(&t, NULL, &writeToFile, (void*) path);
    if (checkThreadCreate != 0)
    {
        std::cout << "Cannnot create pthread for file writer" << std::endl;
    }
#endif // PLATFORM_WINDOWS
}

FW::FileWriter::~FileWriter() 
{
#ifdef PLATFORM_WINDOWS
    if (t.joinable())
        t.join();
#else
    int checkThreadJoin = pthread_join(t, NULL);
    if (checkThreadJoin != 0)
    {
        std::cout << "Cannnot join pthread for file writer" << std::endl;
    }
#endif // PLATFORM_WINDOWS
}
