#pragma once

#include "CommonSys.h"

#if PLATFORM_WINDOWS
#include <thread>
#else
#include <pthread.h>

void* writeToFile(void* args);
#endif


namespace FW
{
class FileWriter 
{
#if PLATFORM_WINDOWS
    void writeToFile();
    std::thread t;
#else
    pthread_t t;
#endif

    const char* path;

public:

    FileWriter(const char* newPath);

    ~FileWriter();
};

}// namespace FW
