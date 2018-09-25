#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#ifndef PCAP
    #define PCAP
#endif // PCAP

#ifndef HAVE_REMOTE
    #define HAVE_REMOTE
#endif // HAVE_REMOTE

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#ifndef WIN32
    #include <sys/types.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <net/if.h>
    #include <unistd.h>
#else
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <Windows.h>
    #include <Iphlpapi.h>
    #pragma comment(lib, "iphlpapi.lib")
    /* WINDOWS SDKS file C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\IPHlpApi.Lib */
#endif

#include "pcap.h"
//#include "encrypt.h"
//#include "platform.h"

//#define DEBUG 1

#ifdef DEBUG
    #include "adapter.h"
#endif

#endif // GLOBAL_H_INCLUDED
