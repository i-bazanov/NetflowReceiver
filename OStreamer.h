#pragma once

#include "FileWriter.h"

#include <mutex>

class OStreamer {
public:
    OStreamer(const char* newPath);

    ~OStreamer();

private:
    // Methods
    void writeToStream();

    // Members
    bool m_isWriteToFile;
    std::mutex m_mutex;

    FileWriter m_fileWriter;

    uint8_t m_bufOutputStreamer[1500];
    unsigned int m_bufFileWriter[11];  
};
