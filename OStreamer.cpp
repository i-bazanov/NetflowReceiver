#include "CommonLibsAndFuncs.h"
#include "OStreamer.h"

#include <iostream>
#include <ctime>

#ifdef PLATFORM_WINDOWS
#else
#include <cstring>
#endif // PLATFORM_WINDOWS

struct header_t
{
    uint16_t type;
    uint16_t size;
};

void OStreamer::writeToStream()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////// Init socket ///////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    init_socket();

    socktype_t s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == SOCKET_ERROR)
    {
        std::cout << "Error creating socket" << std::endl;
        return;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// Init server address ///////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    struct sockaddr_in addrServer;

#ifdef PLATFORM_WINDOWS
    // The size of the client information
    int addrServerSize = sizeof(addrServer);
#else
    // The size of the client information
    unsigned int addrServerSize = sizeof(addrServer);
#endif // PLATFORM_WINDOWS

    addrServer.sin_family = AF_INET;

#ifdef PLATFORM_WINDOWS
    addrServer.sin_addr.S_un.S_addr = ADDR_ANY;
#else
    addrServer.sin_addr.s_addr = INADDR_ANY;
#endif // PLATFORM_WINDOWS

    addrServer.sin_port = htons(9995);

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////// Bind socket and address /////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    if (bind(s, (sockaddr*)&addrServer, addrServerSize) == SOCKET_ERROR)
    {
        std::cout << "Can't bind socket!" << std::endl;
        return;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////// Init variables //////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // [0] -> V9, [1] -> IPFIX, [2] -> other
    int counter[3] = { 0, 0, 0 };
    int traffic[3] = { 0, 0, 0 };

    bool isFirstTwenty = true;

    uint16_t bytesIn = SOCKET_ERROR;

    struct sockaddr_in addrClient;

#ifdef PLATFORM_WINDOWS
    // The size of the client information
    int addrClientSize = sizeof(addrClient);
#else
    // The size of the client information
    unsigned int addrClientSize = sizeof(addrClient);
#endif // PLATFORM_WINDOWS

    char clientIp[15];
    std::memset(clientIp, 0, sizeof(clientIp));
    int clientPort = -1;

    time_t rawtime;
    struct tm timeinfo;

    header_t* header = nullptr;

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// Traffic processing ////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    while (true)
    {
        std::cout << "Receiving ..." << std::endl;

        // Clear the receive buffer
        std::memset(&m_bufOutputStreamer, 0, sizeof(m_bufOutputStreamer));

        // Receive data
        bytesIn = recvfrom(s, (char*)m_bufOutputStreamer, sizeof(m_bufOutputStreamer), 0, (struct sockaddr*)&addrClient, &addrClientSize);
        if (bytesIn == SOCKET_ERROR)
        {
            std::cout << "Error receiving from client" << std::endl;
            continue;
        }

        ////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////// Get current time ////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        time(&rawtime);

#ifdef PLATFORM_WINDOWS
        localtime_s(&timeinfo, &rawtime);
#else
        localtime_r(&rawtime, &timeinfo);
#endif // PLATFORM_WINDOWS

        ////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////// Convert Ip and Port //////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////
        // Convert IP address from byte array to chars
        inet_ntop(AF_INET, &addrClient.sin_addr, clientIp, sizeof(clientIp));

        clientPort = ntohs(addrClient.sin_port);

        ////////////////////////////////////////////////////////////////////////////////
        ///////////////// Number of each received protocol and traffic /////////////////
        ////////////////////////////////////////////////////////////////////////////////
        header = (struct header_t*)m_bufOutputStreamer;

        if (9 == ntohs(header->type))
        {
            ++counter[0];
            traffic[0] += ntohs(header->size);
        }
        else if (10 == ntohs(header->type))
        {
            ++counter[1];
            traffic[1] += ntohs(header->size);
        }
        else
        {
            ++counter[2];
            traffic[2] += bytesIn;
        }

        ////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////// Output data to stream ////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////

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
            // Clear the file writer buffer
            //std::memset(&m_bufFileWriter, 0, sizeof(m_bufFileWriter));
            {
                std::lock_guard<std::mutex> guard(m_mutex);
                m_isWriteToFile = true;

#ifdef PLATFORM_WINDOWS
                m_bufFileWriter[0] = addrClient.sin_addr.S_un.S_addr;   // ip
#else
                m_bufFileWriter[0] = addrClient.sin_addr.s_addr;        // ip
#endif // PLATFORM_WINDOWS
                m_bufFileWriter[1] = addrClient.sin_port;               // port
                m_bufFileWriter[2] = timeinfo.tm_year + 1900;           // year
                m_bufFileWriter[3] = timeinfo.tm_mon + 1;               // month
                m_bufFileWriter[4] = timeinfo.tm_mday;                  // day
                m_bufFileWriter[5] = timeinfo.tm_hour;                  // hour
                m_bufFileWriter[6] = timeinfo.tm_min;                   // minute
                m_bufFileWriter[7] = timeinfo.tm_sec;                   // second

                m_bufFileWriter[8] = traffic[0];                        // traffic V9
                m_bufFileWriter[9] = traffic[1];                        // traffic IPFIX (v10)
                m_bufFileWriter[10] = traffic[2];                       // traffic from others protocols
            }

            isFirstTwenty = false;
        }
    }
}

OStreamer::OStreamer(const char* newPath) : 
    m_isWriteToFile(false),
    m_fileWriter(newPath, m_bufFileWriter, m_isWriteToFile, m_mutex)
{
    // Clear the receive buffer
    std::memset(&m_bufOutputStreamer, 0, sizeof(m_bufOutputStreamer));
    // Clear the file writer buffer
    std::memset(&m_bufFileWriter, 0, sizeof(m_bufFileWriter));

    writeToStream();
}

OStreamer::~OStreamer() 
{
    free(m_bufOutputStreamer);
    free(m_bufFileWriter);
};
