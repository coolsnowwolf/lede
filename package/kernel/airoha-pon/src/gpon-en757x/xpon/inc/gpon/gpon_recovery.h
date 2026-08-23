/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±)
are protected under relevant copyright laws. The information contained herein
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or
its licensors. Without the prior written permission of EcoNet and/or its licensors,
any reproduction, modification, use or disclosure of EcoNet Software, and
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡±
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE.
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH
ECONET SOFTWARE.
***************************************************************/

/*********************************************************************************************************************
Description:
	Recover gpon service
Date:
	2016/01/06
Author:
	yu.he
*********************************************************************************************************************/
#ifndef _GPON_RECOVERY_H_
#define _GPON_RECOVERY_H_

#include "pwan/xpon_netif.h"
#include "xmcs/xmcs_if.h"
#include "common/drv_types.h"

#define HERE_INFO(flag)	do{\
						    if(flag){\
							    printk("%s [%d]\n", __FUNCTION__, __LINE__);\
						    }\
					    }while(0)

#define DBG_INFO(flag, fmt, ...)    do{\
									    if(flag){\
										    printk("%s [%d]: ", __FUNCTION__, __LINE__);\
										    printk(fmt, ##__VA_ARGS__);\
									    }\
								    }while(0)

extern struct Gpon_Recovery_S gponRecovery;

struct Gem_Recovery_Info{
	ushort 							gemPortId ;
	ushort 							allocId ;
	unchar 							channel;
	unchar 							encryption;
	/*not use*/
	ushort							ani;
};

struct Gpon_Recovery_S{
	int allocId[CONFIG_GPON_MAX_TCONT];
	struct Gem_Recovery_Info gemPort[CONFIG_GPON_MAX_GEMPORT];
	int gemPortNum;
	int allocIdNum;
	unchar dbgPrint;
};

static inline int clear_gemport_info(struct Gem_Recovery_Info* gemPortClear)
{
	gemPortClear->allocId = GPON_UNASSIGN_ALLOC_ID;
	gemPortClear->gemPortId = GPON_UNASSIGN_GEM_ID;

	DBG_INFO(gponRecovery.dbgPrint, "Delete gemport success\n");

	return XPON_SUCCESS;
}

static inline int * find_backup_allocId(ushort allocId)
{
	int i = 0;
	int * tmp = NULL;

	for(i = 0; i < gponRecovery.allocIdNum; i++)
	{
		if(gponRecovery.allocId[i] == allocId)
		{
			tmp = &(gponRecovery.allocId[i]);
			break;
		}
	}

	return tmp;
}

static inline struct Gem_Recovery_Info * find_backup_gemport(ushort gemPort)
{
	int i = 0;
	struct Gem_Recovery_Info * tmp = NULL;

	for(i = 0; i < gponRecovery.gemPortNum; i++)
	{
		if(gponRecovery.gemPort[i].gemPortId == gemPort)
		{
			tmp = &(gponRecovery.gemPort[i]);
			break;
		}
	}

	return tmp;
}

#define GPON_RECOVER_DBG_PRINT_GEM(pos) do{\
	DBG_INFO(gponRecovery.dbgPrint, "gemPort[%d].allocId = %d\n", pos, gponRecovery.gemPort[pos].allocId);\
	DBG_INFO(gponRecovery.dbgPrint, "gemPort[%d].gemPortId = %d\n", pos, gponRecovery.gemPort[pos].gemPortId);\
	DBG_INFO(gponRecovery.dbgPrint, "gemPort[%d].channel = %d\n", pos, gponRecovery.gemPort[pos].channel);\
	DBG_INFO(gponRecovery.dbgPrint, "gemPort[%d].encryption = %d\n\n", pos, gponRecovery.gemPort[pos].encryption);\
}while(0)

int gpon_recovery_init(void);

int gpon_recover_backup_allocId(unsigned short allocId);
int gpon_recover_create_allocId(void);
int gpon_recover_delete_allocId(unsigned short allocId);

int gpon_recover_backup_gemport(struct Gem_Recovery_Info * bakGemportId);
int gpon_recover_create_gemport(void);
int gpon_recover_delete_gemport(unsigned short gemPortId);

int gpon_recover_get_real_channel(unsigned short allocId);

#endif /* _GPON_RECOVERY_H_ */

