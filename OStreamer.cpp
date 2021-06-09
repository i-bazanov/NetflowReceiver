#include "CommonLibsAndFuncs.h"

#include "OStreamer.h"

#include <iostream>
#include <ctime>

#ifdef PLATFORM_WINDOWS
#else
#include <cstring>
#endif // PLATFORM_WINDOWS


uint8_t bufOS[1500];
unsigned int bufFW[9];

void OS::OStreamer::writeToStream()
{
    init_sockets();

    socktype_t s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == SOCKET_ERROR)
    {
        std::cout << "Error creating socket" << std::endl;
        return;
    }

    struct sockaddr_in addr;
    int addr_size = sizeof(addr);
    addr.sin_family = AF_INET;

#ifdef PLATFORM_WINDOWS
    addr.sin_addr.S_un.S_addr = ADDR_ANY;
#else
    addr.sin_addr.s_addr = INADDR_ANY;
#endif // PLATFORM_WINDOWS

    addr.sin_port = htons(9995);

    if (bind(s, (sockaddr*)&addr, addr_size) == SOCKET_ERROR)
    {
        std::cout << "Can't bind socket!" << std::endl;
        return;
    }

    sockaddr_in client; // Use to hold the client information (port / ip address)
#ifdef PLATFORM_WINDOWS
    int clientLength = sizeof(client); // The size of the client information
#else
    unsigned int clientLength = sizeof(client); // The size of the client information
#endif // PLATFORM_WINDOWS

    ////////////////////////////// [0] -> V9, [1] -> IPFIX, [2] -> other //////////////////////////////
    int counter[3] = { 0, 0, 0 };
    int traffic[3] = { 0, 0, 0 };

    ////////////////////////////// Creating new way for translation data //////////////////////////////
    struct sockaddr_in addrFW;
    int addrFW_size = sizeof(addrFW);
    addrFW.sin_family = AF_INET;
    inet_pton(AF_INET, (const char*)"127.0.0.2", &(addrFW.sin_addr));
    addrFW.sin_port = htons(9996);
    
    bool isSendIpAndPort = true;
    bool isFirstTwenty = true;

    while (true)
    {
        std::cout << "Receiving ..." << std::endl;

        std::memset(&client, 0, clientLength); // Clear the client structure
        std::memset(&bufOS, 0, sizeof(bufOS)); // Clear the receive buffer
        std::memset(&bufFW, 0, sizeof(bufFW)); // Clear the file writer buffer

        // Receive data
        int bytesIn = recvfrom(s, (char*)bufOS, sizeof(bufOS), 0, (struct sockaddr*)&client, &clientLength);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cout << "Error receiving from client" << std::endl;
            continue;
        }

        ////////////////////////////////// IP address //////////////////////////////////
        char clientIp[15];
        std::memset(clientIp, 0, sizeof(clientIp));

        // Convert from byte array to chars
        inet_ntop(AF_INET, &client.sin_addr, clientIp, sizeof(clientIp));

        ///////////////////////////////////// Port /////////////////////////////////////
        int clientPort = ntohs(addr.sin_port);

        /////////////////////////////// Send Ip and Port ///////////////////////////////
        if (isSendIpAndPort)
        {
#ifdef PLATFORM_WINDOWS
            bufFW[0] = client.sin_addr.S_un.S_addr;        // ip
#else
            bufFW[0] = client.sin_addr.s_addr;             // ip
#endif // PLATFORM_WINDOWS

            bufFW[1] = clientPort;                         // port

            int checkSendTo = sendto(s, (char*)bufFW, sizeof(bufFW), 0, (struct sockaddr*)&addrFW, addrFW_size);
            if (-1 == checkSendTo)
            {
                std::cout << "Error sendto Ip and Port for File Writer" << std::endl;
                int x; std::cin >> x;
                return;
            }

            isSendIpAndPort = false;
            std::memset(&bufFW, 0, sizeof(bufFW)); // Clear the file writer buffer
        }

        ///////////////// Number of each received protocol and traffic /////////////////
        if (9 == bufOS[1])
        {
            ++counter[0];
            traffic[0] += (bufOS[2] << 8) + bufOS[3];
        }
        else if (10 == bufOS[1])
        {
            ++counter[1];
            traffic[1] += (bufOS[2] << 8) + bufOS[3];
        }
        else
        {
            ++counter[2];
            traffic[2] += (bufOS[2] << 8) + bufOS[3];
        }

        time_t rawtime;
        struct tm timeinfo;
        time(&rawtime);
        
#ifdef PLATFORM_WINDOWS
        localtime_s(&timeinfo, &rawtime);
#else
        localtime_r(&rawtime, &timeinfo);
#endif // PLATFORM_WINDOWS

        std::cout << timeinfo.tm_year + 1900 << "-" << timeinfo.tm_mon + 1 << "-" << timeinfo.tm_mday << " "
            << timeinfo.tm_hour << ":" << timeinfo.tm_min << ":" << timeinfo.tm_sec << ", "
            << clientIp << ":" << clientPort << ";\n   Counters: "
            << counter[0] << ", " << counter[1] << ", " << counter[2] << ";\n   Traffics: "
            << traffic[0] << ", " << traffic[1] << ", " << traffic[2] << ";\n";

        if (timeinfo.tm_sec % 20 != 0)
        {
            isFirstTwenty = true;
        }
        else if (0 == timeinfo.tm_sec % 20 && isFirstTwenty)
        {
            bufFW[0] = timeinfo.tm_year + 1900;           // year
            bufFW[1] = timeinfo.tm_mon + 1;               // month
            bufFW[2] = timeinfo.tm_mday;                  // day
            bufFW[3] = timeinfo.tm_hour;                  // hour
            bufFW[4] = timeinfo.tm_min;                   // minute
            bufFW[5] = timeinfo.tm_sec;                   // second

            bufFW[6] = traffic[0];                         // traffic V9
            bufFW[7] = traffic[1];                         // traffic IPFIX (v10)
            bufFW[8] = traffic[2];                         // traffic from others protocols

            int checkSendTo = sendto(s, (char*)bufFW, sizeof(bufFW), 0, (struct sockaddr*)&addrFW, addrFW_size);
            if (-1 == checkSendTo)
            {
                std::cout << "Error sendto for File Writer" << std::endl;
                int x; std::cin >> x;
                return;
            }

            isFirstTwenty = false;
        }
    }
}

OS::OStreamer::OStreamer() 
{
    writeToStream();
}

OS::OStreamer::~OStreamer() {};
