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
	lib_pon_mac_filter.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Reid.Ma		2013/2/4	Create
*/


#ifndef _LIB_PON_MAC_FILTER_H
#define _LIB_PON_MAC_FILTER_H

#include <xpon_global/private/xpon_if.h>

#define MAC_FILTER_UNICAST_RULE_LIMIT		16
#define MAC_FILTER_MULTICAST_RULE_LIMIT	16

#define MAC_FILTER_ANI_PORT_NUM	GPON_MAX_ANI_INTERFACE

#define MODE_HGU 0
#define MODE_SFU 1

#define G988_936 0
#define G988_937 1

#define DISABLE 0
#define ENABLE 1

#define OPT_GET 0
#define OPT_SET 1
#define OPT_DEL 2
#define OPT_CLEAN 3
#define OPT_CLEAN_ALL 4
#define OPT_SHOW 5

#define RULE_FORWARD 0
#define RULE_DISCARD 1

#define DES_MAC 0
#define SRC_MAC 1

#define PONMACFILTER_IOC_MAGIC 'k'
#define PONMACFILTER_MAJOR 216
#define PONMACFILTER_IOC_SWITCH_OPT _IOR(PONMACFILTER_IOC_MAGIC,0,unsigned long)
#define PONMACFILTER_IOC_RULE_OPT _IOR(PONMACFILTER_IOC_MAGIC,1,unsigned long)
#define PONMACFILTER_IOC_DBG_LEVEL_OPT _IOR(PONMACFILTER_IOC_MAGIC,2,unsigned long)

#define UPSTREAM 0
#define DOWNSTREAM 1

#define PORT_INDEX_LEN 10

#define isUniPort(x) (((x) > 10 && (x) < 15) \
	|| ((x) > 20 && (x) < 25) || (x) == 30)

#define isEtherType(x) ((x) >= 0 && (x) <= 0xFFFF)

#define isZeroToTwo(x) ((x) >= 0 && (x) < 3)

#define isZeroToThree(x) ((x) >= 0 && (x) < 4)

#define isZeroToEight(x) ((x) >= 0 && (x) < 9)

#define isZeroToFifteen(x) ((x) >= 0 && (x) < 16)

#define isAniPort(x) ((x) >= 0 && (x) < 32)

typedef struct pon_mac_filter_rule_s
{
	__u8 filter_type;//0:forward 1:discard
	__u8 mac_type;//0:des mac 1:src mac
	__u16 ethertype;//0:don't care
	__u8 start_mac[6];
	__u8 end_mac[6];
	
}pon_mac_filter_rule, *pon_mac_filter_rule_p;

typedef struct pon_mac_filter_ioctl_data_s
{
	__u8 option_flag;//0:get 1:set 2:del 3: clean 4:clean all 5:show

	__u8 direction;//0:upstream,set rule on ANI port. 1:downstream,set rule on UNI port
	__u8 ani_index;//0~31:ani port index
	__u8 uni_index;//11~14:lan1~4  21~24:wlan 1~4  31:usb
	__u8 rule_type;//0:unicast rule 1:multicast
	__u8 rule_index;
	__u8 enable_flag;	
	__u8 dbg_level;
	
	pon_mac_filter_rule rule;
}pon_mac_filter_ioctl_data, *pon_mac_filter_ioctl_data_p;

extern int pon_add_mac_filter_rule(__u8 ifindex,__u8 type,pon_mac_filter_rule * rule);//type: 0->9.3.6  1->9.3.7
extern int pon_get_mac_filter_rule(__u8 ifindex,__u8 type,pon_mac_filter_rule * rule);
extern int pon_del_mac_filter_rule(__u8 ifindex, __u8 type,__u8 index);
extern int pon_clean_mac_filter_rule(__u8 ifindex,__u8 type);
extern int pon_clean_all_mac_filter_rule(void);

#endif

