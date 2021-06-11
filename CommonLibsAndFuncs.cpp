#include "CommonLibsAndFuncs.h"

#include <iostream>


#ifdef PLATFORM_WINDOWS
void init_socket()
{
    WSADATA wsaData;
    int checkWS = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (checkWS != 0)
    {
        // Not ok! Get out quickly
        std::cout << "Can't start Winsock! " << checkWS;
        return;
    }
}

void return_error()
{
    return;
}

#else
void init_socket() {}

void return_error() {}

#endif // PLATFORM_WINDOWS
