/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	libeponmap.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/20	Create
*/

#ifndef LIBPONQOS_H_
#define LIBPONQOS_H_
#include <sys/types.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define ETH_ALEN 6

#define PORT_NUM  4
#define CLSFY_NUM_PER_PORT 64//8

#define EPONMAP_ENABLE  1
#define EPONMAP_DISABLE 0

#define OP_NEVER_MATCH  0x00
#define OP_EQUAL		 0x01
#define OP_NOT_EQUAL    0x02
#define OP_LESS_THAN    0x03
#define OP_GREATER_THAN 0x04
#define OP_EXISTS       0x05
#define OP_NOT_EXIST    0x06
#define OP_ALWAYS_MATCH 0x07
#define OP_NOT_SET      0x08

#define FIELD_DMAC      0x00
#define FIELD_SMAC      0x01
#define FIELD_PBIT      0x02
#define FIELD_VLANID    0x03
#define FIELD_ETHTYPE   0x04
#define FIELD_DIP4      0x05
#define FIELD_SIP4      0x06
#define FIELD_IPPROTO4  0x07
#define FIELD_IPDSCP4		0x08
#define FIELD_IPDSCP6		0x09
#define FIELD_SPORT     0x0A
#define FIELD_DPORT     0x0B
#define FIELD_IPVER     0x0C
#define FIELD_FLOWLABEL6	0x0D
#define FIELD_DIP6      0x0E
#define FIELD_SIP6      0x0F
#define FIELD_DIP6PREX		0x10
#define FIELD_SIP6PREX		0x11
#define FIELD_IPPROTO6  0x12
#define MAX_FIELD_NUM   19 // old is 14

#define XPONMAP_IOC_MAGIC 'a'
#define XPONMAP_MAJOR 210

#define EPONMAP_IOC_ENABLE     _IOW(XPONMAP_IOC_MAGIC, 10,  unsigned long)
#define EPONMAP_IOC_DISABLE    _IOW(XPONMAP_IOC_MAGIC, 11,  unsigned long)
#define EPONMAP_IOC_DELETE     _IOW(XPONMAP_IOC_MAGIC, 12,  unsigned long)
#define EPONMAP_IOC_ADD        _IOW(XPONMAP_IOC_MAGIC, 13,  unsigned long)
#define EPONMAP_IOC_CLEAR      _IOW(XPONMAP_IOC_MAGIC, 14,  unsigned long)
#define EPONMAP_IOC_SHOW       _IOR(XPONMAP_IOC_MAGIC, 15,  unsigned long)
#define EPONMAP_IOC_GETNUM     _IOR(XPONMAP_IOC_MAGIC, 16,  unsigned long)
#define EPONMAP_IOC_GETRULE    _IOR(XPONMAP_IOC_MAGIC, 17,  unsigned long)
#define EPONMAP_IOC_GETLLIDQ   _IOW(XPONMAP_IOC_MAGIC, 18,  unsigned long)
#define EPONMAP_IOC_SETLLIDQ   _IOW(XPONMAP_IOC_MAGIC, 19,  unsigned long)
#define EPONMAP_IOC_CLEARLLIDQ _IOW(XPONMAP_IOC_MAGIC, 20, unsigned long)
#define EPONMAP_IOC_SHOWLLIDQ  _IOW(XPONMAP_IOC_MAGIC, 21, unsigned long)
#define EPONMAP_IOC_DBG_LVL  _IOW(XPONMAP_IOC_MAGIC, 22, unsigned long)
#define EPONMAP_IOC_RESETALL      _IOW(XPONMAP_IOC_MAGIC, 23,  unsigned long)


#define XPONMAP_DEV "/dev/xponmap"
/* -----------------------Qos classification structure----------------------- */
typedef struct qosMatchRule_s
{
    u_char field;
    u_char op;
    union {
        u_char  v8;  // pbit / ipversion / ipproto /ipdscp4/ipdscp6
        u_short v16; // vid / ethertype / port
        u_int   ip4; // ipv4 / ipv6's flowlbl
        //struct in_addr ip4;
        u_char  mac[6];
        struct in6_addr ip6;
    };
}QosMatchRule_t, *QosMatchRule_Ptr;

/* -----------------Qos Object and Oam object translations----------------- */
typedef struct qosResult_s
{
	u_char precedence; // 1-255 the priority of the rule
	u_char queueMapped; // 0-7  the priority queue mapped to
	u_char priority;   // 0-7 or 0xFF new PBit of vlan tag, 0xFF use the original PBit 
}QosResult_t, *QosResult_Ptr;

typedef struct qosIOCtl_s
{
	u_char       portId;    // ether port index
	u_char       ruleIdx;   // the port's classification rules index
	u_char       matchNum;   // the classification rules's num
	QosResult_t result;	 // the classification result
	QosMatchRule_t matchs[MAX_FIELD_NUM]; // the rules
}QosIOCtl_t, *QosIOCtl_Ptr;

typedef struct qosClsfyIOCtl_s
{
	u_char  portId;
	u_char  clsfyNum; // the port's clsfy rule num
}QosClsfyIOCtl_t, *QosClsfyIOCtl_Ptr;

int eponmapIoctl(int cmd, void *data, int datalen);
/* -----------------------------OUT Interfaces----------------------------- */
int eponmapSetDbgLevel(u_char lvl);
int eponmapSetClsfyEnable(u_char port, int enable);
int eponmapDeleteClsfyRule(u_char portId, QosResult_Ptr pResult, u_char matchNum, QosMatchRule_Ptr pRules);
int eponmapInsertClsfyRule(u_char portId, QosResult_Ptr pResult, u_char matchNum, QosMatchRule_Ptr pRules);
int eponmapGetClsfyRule(u_char portId, u_char index, QosResult_Ptr pResult, u_char *pmatchNum, QosMatchRule_Ptr pRules);
int eponmapGetClsfyRulesNum(u_char portId);
void eponmapShowClsfyRules(u_char portId);
int eponmapClearClsfyRules(u_char portId);
void eponmapReset();

/* ----------------------LlidQueueMapping and weight---------------------- */
#define PRIORITY_QUEUE_NUM_MAX  64
#define LLID_NUM_MAX            8
#define LLID_QUEUE_NUM_MAX      8

#define QUEUEMAP_ENABLE  1
#define QUEUEMAP_DISABLE 0

typedef struct queueWeight_s
{
    u_char priQueue;
    u_char weight;
}QueueWeight_t, *QueueWeight_Ptr;

typedef struct qosQueueMappingIOCtl_s
{
	u_char llid;
	u_char num;
	QueueWeight_t queueWts[LLID_QUEUE_NUM_MAX];
}QosQueueMappingIOCtl_t, *QosQueueMappingIOCtl_Ptr;

int eponmapGetLlidQueueMapping(u_char llid, u_char *queueNum, QueueWeight_Ptr pQWeights);
int eponmapSetLlidQueueMapping(u_char llid, u_char queueNum, QueueWeight_Ptr pQWeights);
int eponmapClearLlidQueueMapping(u_char llid);
int eponmapShowLlidQueueMapping(u_char llid);

#endif
