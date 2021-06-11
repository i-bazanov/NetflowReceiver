#include "CommonLibsAndFuncs.h"
#include "FileWriter.h"

#include <fstream>
#include <iostream>
#include <ctime>

#ifdef PLATFORM_WINDOWS
#include <thread>
#else
#include <thread>
//#include <pthread.h>
#include <cstring>
#endif // PLATFORM_WINDOWS


//#ifdef PLATFORM_WINDOWS
void FileWriter::writeToFile()
//#else
//void* FileWriter::writeToFile(void* args)
//#endif // PLATFORM_WINDOWS
{
    /////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////// Check file stream ////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    if (!m_foutput)
    {
        std::cout << "Error: file isn't able to open" << std::endl;
        return_error();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////// Creating title /////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    m_foutput << "DATE_TIME, SOURCE, V9, IPFIX, Other" << std::endl;

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////// Init data and check when output to file /////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////
    char clientIp[15];
    std::memset(clientIp, 0, sizeof(clientIp));
    int clientPort = -1;

    // [0] -> V9, [1] -> IPFIX, [2] -> other
    int foutputTraffic[3] = { 0, 0, 0 };

    while (true)
    {
        if (m_isWriteToFile)
        {
            std::cout << "Writing to file ..." << std::endl;
            {
                std::lock_guard<std::mutex> guard(m_mutex);

                // Convert IP address from byte array to chars
                inet_ntop(AF_INET, m_bufFileWriter, clientIp, sizeof(clientIp));
                clientPort = ntohs(m_bufFileWriter[1]);

                m_foutput << m_bufFileWriter[2] << "-" << m_bufFileWriter[3] << "-" << m_bufFileWriter[4] << " "
                    << m_bufFileWriter[5] << ":" << m_bufFileWriter[6] << ":" << m_bufFileWriter[7] << ", "
                    << clientIp << ":" << clientPort << ", "
                    << m_bufFileWriter[8] - foutputTraffic[0] << ", "
                    << m_bufFileWriter[9] - foutputTraffic[1] << ", "
                    << m_bufFileWriter[10] - foutputTraffic[2] << std::endl;

                foutputTraffic[0] = m_bufFileWriter[8];
                foutputTraffic[1] = m_bufFileWriter[9];
                foutputTraffic[2] = m_bufFileWriter[10];
            }
            m_isWriteToFile = false;
        }
    }
}

FileWriter::FileWriter(const char* newPath, const unsigned int* bufFileWriter, bool& isWriteToFile, std::mutex& mutex) :
    m_path(newPath),
    m_bufFileWriter(bufFileWriter),
    m_isWriteToFile(isWriteToFile),
    m_mutex(mutex),
    m_foutput(m_path)
{
//#ifdef PLATFORM_WINDOWS
    t = std::thread(&FileWriter::writeToFile, this);
//#else
    /*int checkThreadCreate = pthread_create(&t, NULL, &FileWriter::writeToFile, NULL);
    if (checkThreadCreate != 0)
    {
        std::cout << "Cannnot create pthread for file writer" << std::endl;
    }*/
//#endif // PLATFORM_WINDOWS
}

FileWriter::~FileWriter() 
{
//#ifdef PLATFORM_WINDOWS
    if (t.joinable())
        t.join();
//#else
    /*int checkThreadJoin = pthread_join(t, NULL);
    if (checkThreadJoin != 0)
    {
        std::cout << "Cannnot join pthread for file writer" << std::endl;
    }*/
//#endif // PLATFORM_WINDOWS

    m_path = nullptr;
    m_bufFileWriter = nullptr;

    m_foutput.close();
}
