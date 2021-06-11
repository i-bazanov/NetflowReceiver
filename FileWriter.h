#pragma once

#include "CommonSys.h"

#include <fstream>
#include <mutex>

//#if PLATFORM_WINDOWS
#include <thread>
//#else
//#include <pthread.h>
//#endif

class FileWriter 
{
public:

    FileWriter(const char* newPath, const unsigned int* bufFileWriter, bool& isWriteToFile, std::mutex& mutex);

    ~FileWriter();

private:
//#if PLATFORM_WINDOWS
    void writeToFile();

    std::thread t;
//#else
    /*void* writeToFile(void* args);

    pthread_t t;*/
//#endif

    const char* m_path;
    const unsigned int* m_bufFileWriter;
    bool& m_isWriteToFile;
    std::mutex& m_mutex;

    std::ofstream m_foutput;
};
