#pragma once

#include "CommonSys.h"

#ifdef PLATFORM_WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>

typedef SOCKET socktype_t;
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET_ERROR -1
typedef int socktype_t;
#endif // PLATFORM_WINDOWS

void init_socket();

void return_error();
