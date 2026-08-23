#ifndef HAL_RAWSOCKET_H_
#define HAL_RAWSOCKET_H_

#include <linux/if_packet.h>

struct RawSocket
{
public:
     RawSocket();
     ~RawSocket();
	 
     bool send(char* buffer, int len);
     bool receive(char* buffer, int len, const int timeout_sec);
	 
private:
     bool isPacketReceived(const int timeout_sec)const;
     void setSrcMac(char* buffer)const;
     void logMsg(const char* buffer, int len)const;

     int socketId;
     struct sockaddr_ll socketAddr;
};

#endif

