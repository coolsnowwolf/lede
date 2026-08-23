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
	epon_ioctl.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/



#ifndef _EPON_IOCTL_H_
#define _EPON_IOCTL_H_

#include <linux/version.h>


#define EPON_MAC_IOC_MAGIC 'j'
#define EPON_MAC_MAJOR 221

#define EPON_IOCTL_SET_LLID_ENABLE_MASK     	_IOW(EPON_MAC_IOC_MAGIC, 0,  unsigned long)
#define EPON_IOCTL_LLID_DEREGISTER			_IOW(EPON_MAC_IOC_MAGIC, 1,  unsigned long) 
#define EPON_IOCTL_LLID_DISCV_RGSTACK		_IOW(EPON_MAC_IOC_MAGIC, 2,  unsigned long) 
#define EPON_IOCTL_SET_LLID_KEY				_IOW(EPON_MAC_IOC_MAGIC, 3,  unsigned long) 
#define EPON_IOCTL_GET_LLID_KEY				_IOW(EPON_MAC_IOC_MAGIC, 4,  unsigned long) 
#define EPON_IOCTL_SET_LLID_FEC				_IOW(EPON_MAC_IOC_MAGIC, 5,  unsigned long) 
#define EPON_IOCTL_GET_LLID_FEC				_IOR(EPON_MAC_IOC_MAGIC, 6,  unsigned long) 
#define EPON_IOCTL_MAC_RST					_IOR(EPON_MAC_IOC_MAGIC, 7,  unsigned long) 

#define EPON_IOCTL_SET_LLID_DBA_THRSHLD_NUM		_IOW(EPON_MAC_IOC_MAGIC, 8,  unsigned long) 
#define EPON_IOCTL_GET_LLID_DBA_THRSHLD_NUM		_IOR(EPON_MAC_IOC_MAGIC, 9,  unsigned long) 
#define EPON_IOCTL_SET_LLID_DBA_THROD		_IOW(EPON_MAC_IOC_MAGIC, 10,  unsigned long) 
#define EPON_IOCTL_GET_LLID_DBA_THROD		_IOR(EPON_MAC_IOC_MAGIC, 11,  unsigned long) 
#define EPON_IOCTL_SET_MPCP_TIME		_IOW(EPON_MAC_IOC_MAGIC, 12,  unsigned long) 
#define EPON_IOCTL_GET_MPCP_TIME		_IOR(EPON_MAC_IOC_MAGIC, 13,  unsigned long) 
#define EPON_IOCTL_SET_HOLDOVER_CFG		_IOW(EPON_MAC_IOC_MAGIC, 14,  unsigned long) 
#define EPON_IOCTL_GET_HOLDOVER_CFG		_IOR(EPON_MAC_IOC_MAGIC, 15,  unsigned long) 
#define EPON_IOCTL_GET_TX_ETH_MIB		_IOR(EPON_MAC_IOC_MAGIC, 16,  unsigned long) 
#define EPON_IOCTL_GET_RX_ETH_MIB		_IOR(EPON_MAC_IOC_MAGIC, 17,  unsigned long) 
#define EPON_IOCTL_CLR_TX_RX_MIB		_IOW(EPON_MAC_IOC_MAGIC, 18,  unsigned long) 
#define EPON_IOCTL_POWER_CTL               _IOW(EPON_MAC_IOC_MAGIC, 19,  unsigned long)
#define EPON_IOCTL_CTL_PS 				_IOW(EPON_MAC_IOC_MAGIC, 20,  unsigned long)  //ltm
#define EPON_IOCTL_AUTH_FAIL_CTL            _IOW(EPON_MAC_IOC_MAGIC, 21,  unsigned long)
#define EPON_IOCTL_SET_LLID_RX_FEC			_IOW(EPON_MAC_IOC_MAGIC, 22,  unsigned long) 
#define EPON_IOCTL_GET_LLID_RX_FEC			_IOR(EPON_MAC_IOC_MAGIC, 23,  unsigned long)
#define EPON_IOCTL_SET_LLID_MAC			    _IOW(EPON_MAC_IOC_MAGIC, 24,  unsigned long)
#define EPON_IOCTL_GET_LLID_KEY_INUSE		_IOR(EPON_MAC_IOC_MAGIC, 25,  unsigned long) 
#define EPON_IOCTL_SET_SILENCETIME          _IOW(EPON_MAC_IOC_MAGIC, 26,  unsigned long)
#define EPON_IOCTL_SET_DEBUG_LEVEL     	_IOW(EPON_MAC_IOC_MAGIC, 27,  unsigned char)
#define EPON_IOCTL_SET_PHY_BURST_EN     	_IOW(EPON_MAC_IOC_MAGIC, 28,  unsigned int)
#define EPON_IOCTL_SET_DYING_GASP_MODE     	_IOW(EPON_MAC_IOC_MAGIC, 29,  unsigned char)
#define EPON_IOCTL_GET_DYING_GASP_MODE     	_IOW(EPON_MAC_IOC_MAGIC, 30,  unsigned char)
#define EPON_IOCTL_SET_DYING_GASP_NUM     	_IOW(EPON_MAC_IOC_MAGIC, 31,  unsigned int)
#define EPON_IOCTL_GET_DYING_GASP_NUM     	_IOW(EPON_MAC_IOC_MAGIC, 32,  unsigned int)
#define EPON_IOCTL_SET_EARLY_WAKEUP     	_IOW(EPON_MAC_IOC_MAGIC, 33,  unsigned int)
#define EPON_IOCTL_SET_SNIFFER_MODE     	_IOW(EPON_MAC_IOC_MAGIC, 34,  unsigned int)


#define EPON_MAC_DEV "/dev/epon_mac"




typedef struct eponTxCnt_s{
	__u32 txFrameCnt;
	__u32 txFrameLen;
	__u32 txDropCnt;
	__u32 txBroadcastCnt;
	__u32 txMulticastCnt;
	__u32 txLess64Cnt;
	__u32 txMore1518Cnt;
	__u32 txEq64Cnt;
	__u32 txFrom65To127Cnt;
	__u32 txFrom128To255Cnt;
	__u32 txFrom256To511Cnt;
	__u32 txFrom512To1023Cnt;
	__u32 txFrom1024To1518Cnt;
}eponTxCnt_t, *eponTxCnt_p;


typedef struct eponRxCnt_s{
	__u32 rxFrameCnt;
	__u32 rxFrameLen;
	__u32 rxDropCnt;
	__u32 rxBroadcastCnt;
	__u32 rxMulticastCnt;
	__u32 rxCrcCnt;
	__u32 rxFragFameCnt;
	__u32 rxJabberFameCnt;
	__u32 rxLess64Cnt;
	__u32 rxMore1518Cnt;
	__u32 rxEq64Cnt;
	__u32 rxFrom65To127Cnt;
	__u32 rxFrom128To255Cnt;
	__u32 rxFrom256To511Cnt;
	__u32 rxFrom512To1023Cnt;
	__u32 rxFrom1024To1518Cnt;
}eponRxCnt_t, *eponRxCnt_p;


typedef struct {
	__u8 llidIndex;
	__u8 param0;
	__u16 param1;
	__u32 param2;
	__u8 info[128];
} eponMacIoctl_t ;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long eponMacIoctl(struct file *file, unsigned int cmd,
	                unsigned long arg);
#else
int eponMacIoctl (struct inode *inode, struct file *filp,
                  unsigned int cmd, unsigned long arg);
#endif
#endif//_EPON_IOCTL_H_
