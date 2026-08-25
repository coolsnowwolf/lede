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
	libgponmap.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/20	Create
*/
#ifndef _LIBGPONMAP_H_
#define _LIBGPONMAP_H_
#include <sys/types.h>
#include <asm/ioctl.h>



#ifndef NULL
#define NULL ((void*)0)
#endif


#define XPONMAP_IOC_MAGIC 'a'
#define XPONMAP_MAJOR 210


#define GEMPORT_MAPPING_ADD_ENTRY     		_IOW(XPONMAP_IOC_MAGIC, 0,  unsigned long)
#define GEMPORT_MAPPING_DEL_ENTRY    		_IOW(XPONMAP_IOC_MAGIC, 1,  unsigned long)
#define QUEUE_MAPPING_ADD_ENTRY     		_IOW(XPONMAP_IOC_MAGIC, 2,  unsigned long)
#define QUEUE_MAPPING_DEL_ENTRY       		_IOW(XPONMAP_IOC_MAGIC, 3,  unsigned long)
#define GEMPORT_MAPPING_DUMP_ALL_ENTRY    	_IOW(XPONMAP_IOC_MAGIC, 4,  unsigned long)
#define QUEUE_MAPPING_DUMP_ALL_ENTRY    		_IOW(XPONMAP_IOC_MAGIC, 5,  unsigned long)
#define DOWNSTREAM_SWITCH_OPT   		 	_IOW(XPONMAP_IOC_MAGIC, 6,  unsigned long)
#define DOWNSTREAM_MAPPING_RULE_OPT    		_IOW(XPONMAP_IOC_MAGIC, 7,  unsigned long)

#define MAX_GEM_PORT_NUM 256

#define OPT_GET 0
#define OPT_SET 1
#define OPT_DEL 2
#define OPT_CLEAN 3
#define OPT_SHOW 4

#define DISABLE 0
#define ENABLE 1
#define DS_TRTCM_ENABLE 2

typedef struct gpon_downstream_mapping_ioctl_s{
	u_short option_flag;
	u_short ds_pq_enable;
	u_short ds_trtcm_enable;
	u_short downstream_mapping_switch;
	u_short gem_port_num;//0~4095:index of gem port,4096 mean not a rule
	u_int if_mask;
	u_char queue;//0~7 mean the queue index,8 mean don't specify queue
	u_char trtcmId;
}gpon_downstream_mapping_ioctl, *gpon_downstream_mapping_ioctl_p;


#define UPSTREAM_VLAN_POLICER_ADD_RULE   _IOW(XPONMAP_IOC_MAGIC, 8,  unsigned long)
#define UPSTREAM_VLAN_POLICER_DEL_RULE   _IOW(XPONMAP_IOC_MAGIC, 9,  unsigned long)

typedef struct gpon_upstream_vlan_policer_ioctl_s{
        u_char  ethID;    // corresponding to eth lan port
        u_char  entryID;
        u_char  chanID;
        u_short vid;
}gpon_upstream_vlan_policer_ioctl, * gpon_upstream_vlan_policer_ioctl_p;


#define XPONMAP_DEV "/dev/xponmap"


enum gponmap_status {
	GPONMAP_SUCCESS=0,
	GPONMAP_FAIL=1,
	GPONMAP_ENTRY_NOT_FOUND=2,
	GPONMAP_ENTRY_EXIST=3
};
/*
**********************************************************************************
gpon traffic class to GEM port mapping entry ioctl data structure.
**********************************************************************************
*/
typedef struct gemPortMappingIoctl_s{
	u_short tagCtl;
	u_char tagFlag;//0:untagged,1:tagged
	u_char userPort;//0xff: this value indicates all user ports.
	u_short aniPort;	
	u_short vid;
	u_char dscp;
	u_char pbit;
	u_short gemPort;	
}gemPortMappingIoctl_t, *gemPortMappingIoctl_ptr;


/*
**********************************************************************************
GEM port to priority queue mapping entry ioctl data structure.
**********************************************************************************
*/
#define TRAFFIC_SCHEDULER_SPECIFIC_QUEUE_MODE 	0
#define TRAFFIC_SCHEDULER_MULTI_QUEUE_MODE 		1

#define GEM_TYPE_UNICAST							0
#define GEM_TYPE_MULTICAST						1

#define TRAFFIC_SHAPING_DISABLE					0
#define TRAFFIC_SHAPING_ENABLE					1

typedef struct gponQueueMappingIoctl_s{
	u_short gemPort;
	u_char gemType;		/*gem type unicast, multicast*/
	u_char pqMode; 		/*0--traffic scheduler, gemport-->specfic PQ; 1--traffic scheduler, gemport-->t-cont. via p-bit mapping PQ. */
	u_char tsEnable; 		/*traffic shaping enable flag; 0--disable; 1--enable*/
	u_char tsChannelId;	/*traffic shaping channel id(0-31)*/
	u_char queue;
	u_short allocId;
}gponQueueMappingIoctl_t, *gponQueueMappingIoctl_ptr;

int gponmap_ioctl(int cmd, void *data, int datalen);


/* -----------------------------OUT Interfaces----------------------------- */
int addGemPortMappingRule(gemPortMappingIoctl_t *tableEntry);
int delGemPortMappingRule(gemPortMappingIoctl_t *tableEntry);
int addQueueMappingRule(gponQueueMappingIoctl_t *tableEntry);
int delQueueMappingRule(gponQueueMappingIoctl_t *tableEntry);
int displayAllGemPortMappingRule(void);
int displayAllQueueMappingRule(void);

int enableDownMapping(u_char modeSwitch);
int disableDownstreamMapping();
int addGemDownstreamMappingRule(gpon_downstream_mapping_ioctl data);

#endif
