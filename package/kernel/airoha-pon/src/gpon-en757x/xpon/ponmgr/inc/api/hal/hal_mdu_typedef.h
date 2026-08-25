/*
 * hal_typedef.h
 *
 *  Created on: Jan 3, 2014
 *      Author: root
 */

#ifndef HAL_TYPEDEF_H_
#define HAL_TYPEDEF_H_


enum MduMsgType
{
	SYNC_CONFIRM =0x0001,
	GPON_REGISTER_STATUS = 0x0002,
	LOID_AUTH_STATUS = 0x0003,
	RE_RANGE = 0x0004,
	SLID_REQUEST = 0x0005,
	LOID_REQUEST = 0x0006,
	OAM_GLOBAL = 0x0009,
	OAM_SNMP = 0x000a,
	
	SYNC_CONFIRM_ACK = 0x0011,
	GPON_REGISTER_STATUS_ACK = 0x0012,
	LOID_AUTH_STATUS_ACK = 0x0013,
	RE_RANGE_ACK = 0x0014,
	SLID_RESPONSE = 0x0015,
	LOID_RESPONSE = 0x0016,
	OAM_GLOBAL_ACK = 0x0019,
	OAM_SNMP_ACK = 0x001a
};
const char RCV_SOCKET_QUEUE_FILE[] = "/configs/mdu/stateinfo.cfg";
const unsigned short MDU_ETHER_TYPE = 0x8100;
const unsigned short MDU_VLANID = 4093;
const unsigned short MDU_ETYPE = 0x8809;
const unsigned char MDU_PRI_BIT = 7;
const unsigned short MDU_PRI_MASK = 0xE000;

const int mac_addr_length = 6;

typedef struct {
    unsigned char dstMac[mac_addr_length];
    unsigned char srcMac[mac_addr_length];
    unsigned short tpid;
    unsigned short tci;
    unsigned short ethType;
    unsigned short transId;
    unsigned short msgType;
} __attribute__((packed)) PrivateMsgHeader;

struct PrivateMsgResponse
{
    unsigned short transId;
    unsigned short msgType;
    unsigned char msgLen;
    unsigned char response;
}__attribute__((packed));

#define MDU_MSG_BUFFER_MAX_SIZE 512
// IPC message structure used in MDU inter-process communication
typedef struct mdu_msg
{
    long msgid;
    char mbuff[MDU_MSG_BUFFER_MAX_SIZE];
}MDU_MSG;


#endif /* HAL_TYPEDEF_H_ */
