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

#include "gpon/gpon_recovery.h"

struct Gpon_Recovery_S gponRecovery={0};

/*********************************************************************************************************************
Description:
	init
Input Args:
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recovery_init(void)
{
	int i;
	gponRecovery.allocIdNum = 0;
	gponRecovery.gemPortNum = 0;

	for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++){
		gponRecovery.allocId[i] = GPON_UNASSIGN_ALLOC_ID;
    }

	gponRecovery.dbgPrint = 0;
}

/*********************************************************************************************************************
Description:
	backup allocid
Input Args:
	allocId: alloc id
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recover_backup_allocId(unsigned short allocId)
{
	if(NULL != find_backup_allocId(allocId)){
		return XPON_FAIL;
	}

	gponRecovery.allocId[gponRecovery.allocIdNum] = allocId;

	DBG_INFO(gponRecovery.dbgPrint, "allocId[%d] = %d\n", gponRecovery.allocIdNum, allocId);

	gponRecovery.allocIdNum++;

	return XPON_SUCCESS;
}

/*********************************************************************************************************************
Description:
	recover allocid
Input Args:
	allocId: alloc id
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recover_create_allocId(void)
{
	int i = 0;
	int ret = XPON_SUCCESS;

	for(i = 0; i < gponRecovery.allocIdNum; i++)
	{
		if(0 != gwan_create_new_tcont(gponRecovery.allocId[i])){
			ret = XPON_FAIL;
			continue;
		}

		DBG_INFO(gponRecovery.dbgPrint, "allocId[%d] = %d\n", i, gponRecovery.allocId[i]);
	}

	return ret;
}

/*********************************************************************************************************************
Description:
	delete allocid
Input Args:
	allocId: alloc id
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recover_delete_allocId(unsigned short allocId)
{
	int i = 0;
	struct Gem_Recovery_Info * bakGemport = gponRecovery.gemPort;
	int * tmp = NULL;

	tmp = find_backup_allocId(allocId);

	if(NULL == tmp){
		return XPON_FAIL;
	}

	for(i = 0; i < gponRecovery.gemPortNum; i++)
	{
		if(allocId == bakGemport[i].allocId)
		{
			if(0 == gwan_remove_gemport(bakGemport[i].gemPortId))
			{
				memcpy(&bakGemport[i],
					   &bakGemport[gponRecovery.gemPortNum-1],
					   sizeof(struct Gem_Recovery_Info) );

				clear_gemport_info(&bakGemport[gponRecovery.gemPortNum-1]);

				gponRecovery.gemPortNum--;
				i--;
			}
		}
	}

	DBG_INFO(gponRecovery.dbgPrint, "allocId = %d\n", tmp);

	*tmp = gponRecovery.allocId[gponRecovery.allocIdNum-1];
	gponRecovery.allocId[gponRecovery.allocIdNum-1] = GPON_UNASSIGN_ALLOC_ID;
	gponRecovery.allocIdNum--;
	return XPON_SUCCESS;
}

/*********************************************************************************************************************
Description:
	backup gemport
Input Args:
	*bakGemport: back gemport struct pointer
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recover_backup_gemport(struct Gem_Recovery_Info * bakGemportId)
{
	if(NULL != find_backup_gemport(bakGemportId->gemPortId)){
		return XPON_FAIL;
	}

	gponRecovery.gemPort[gponRecovery.gemPortNum] = *bakGemportId;
	GPON_RECOVER_DBG_PRINT_GEM(gponRecovery.gemPortNum);
	gponRecovery.gemPortNum++;

	return XPON_SUCCESS;
}

/*********************************************************************************************************************
Description:
	get channel
Input Args:
	allocId: alloc id
Ret Value:
	if success, return channel
	if fali, return non-zero error code
*********************************************************************************************************************/
extern PWAN_GlbPriv_T  * gpWanPriv;
int gpon_recover_get_real_channel(ushort allocId)
{
	int channel;

	if(GPON_UNASSIGN_ALLOC_ID == allocId){
		return GPON_MULTICAST_CHANNEL;
	}

	for(channel = 0; channel < CONFIG_GPON_MAX_TCONT; channel++)
	{
		if(gpWanPriv->gpon.allocId[channel] == allocId){
			return channel;
		}
	}

	return XPON_FAIL;
}

/*********************************************************************************************************************
Description:
	recover gemport
Input Args:
	*recGemportCre: recovery gemport struct pointer
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recover_create_gemport(void)
{
	int i = 0;
	struct Gem_Recovery_Info * pBakGemPort = NULL;

	for(i = 0; i < gponRecovery.gemPortNum; i++)
	{
		pBakGemPort = &gponRecovery.gemPort[i];

		pBakGemPort->channel = gpon_recover_get_real_channel(pBakGemPort->allocId);

		if(-1 == pBakGemPort->channel){
			DBG_INFO(gponRecovery.dbgPrint, "channel err\n");
			continue;
		}

		if(0 != gwan_create_new_gemport(pBakGemPort->gemPortId, pBakGemPort->channel, pBakGemPort->encryption)){
			DBG_INFO(gponRecovery.dbgPrint, "create_gemport err\n");
			continue;
		}

		if(0 != gwan_config_gemport(pBakGemPort->gemPortId, ENUM_CFG_NETIDX, 1)){
			DBG_INFO(gponRecovery.dbgPrint, "config_gemport err\n");
			continue;
		}

		GPON_RECOVER_DBG_PRINT_GEM(i);
	}

	return XPON_SUCCESS;
}

/*********************************************************************************************************************
Description:
	delete gemport
Input Args:
	gemPortId: gemport id
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recover_delete_gemport(unsigned short gemPortId)
{
	int i = 0;
	struct Gem_Recovery_Info * bakGemport = gponRecovery.gemPort;
	struct Gem_Recovery_Info * tmp = NULL;

	tmp = find_backup_gemport(gemPortId);

	if(NULL == tmp){
		return XPON_FAIL;
	}

	if((tmp - gponRecovery.gemPort) != (gponRecovery.gemPortNum - 1) )
    {
        DBG_INFO(gponRecovery.dbgPrint, "gemPort = %d\n", tmp->gemPortId);
		memcpy(tmp, &bakGemport[gponRecovery.gemPortNum-1], sizeof(struct Gem_Recovery_Info));
	}
	clear_gemport_info(&bakGemport[gponRecovery.gemPortNum-1]);

	gponRecovery.gemPortNum--;

	return XPON_SUCCESS;
}

