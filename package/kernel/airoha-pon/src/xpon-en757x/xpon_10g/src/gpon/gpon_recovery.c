/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#include "gpon/gpon_recovery.h"

struct Gpon_Recovery_S gponRecovery;
extern PWAN_GlbPriv_T  * gpWanPriv;

/*********************************************************************************************************************
Description:
	init
Input Args:
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
void gpon_recovery_init(void)
{
	int i;
	gponRecovery.allocIdNum = 0;
	gponRecovery.gemPortNum = 0;

	for(i = 0; i < CONFIG_GPON_MAX_TCONT; i++){
		gponRecovery.allocId[i] = GPON_10G_UNASSIGN_ALLOC_ID;
    }

	gponRecovery.dbgPrint = 0;
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
	gpon_recovery_set_ani
Input Args:
	*bakGemport: back gemport struct pointer
Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
int gpon_recovery_set_ani(unsigned short gemPortId, unsigned int ani)
{
    struct Gem_Recovery_Info * tmp = NULL;
    
    tmp = find_backup_gemport(gemPortId);

    DBG_INFO(gponRecovery.dbgPrint, "gemPort = %d, ani = %d\n",gemPortId,ani);

    if(NULL != tmp){
        tmp->ani = ani;
    }else{
        DBG_INFO(gponRecovery.dbgPrint, "no gemPort in backup list\n");
    }
    
    return XPON_SUCCESS;
}

/*********************************************************************************************************************
Description:
	set channel
Input Args:
	allocId: alloc id
Ret Value:
	if success, return channel
	if fali, return non-zero error code
*********************************************************************************************************************/

int gpon_recovery_set_channel(unsigned short gemPortId, unsigned int channel)
{
    struct Gem_Recovery_Info * tmp = NULL;
    
    tmp = find_backup_gemport(gemPortId);

    DBG_INFO(gponRecovery.dbgPrint, "gemPort = %d, channel = %d\n",gemPortId,channel);

    if(NULL != tmp){
        tmp->channel = channel;
    }else{
        DBG_INFO(gponRecovery.dbgPrint, "no gemPort in backup list\n");
    }
    
    return XPON_SUCCESS;
}

int gpon_check_gem_exist(ushort gemPortId)
{
	int i = 0;

        if(gpWanPriv->gpon.gemNumbers > CONFIG_GPON_MAX_GEMPORT) {
		return -ENOSPC ;
	}
  
	for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) 
	{
		if(gpWanPriv->gpon.gemPort[i].info.portId == gemPortId){
			return 1;
		}
	}
    	return 0;
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

	if(GPON_10G_UNASSIGN_ALLOC_ID == allocId){
		return GPON_MULTICAST_CHANNEL;
	}

	for(channel = 0; channel < CONFIG_GPON_10G_MAX_TCONT; channel++)
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
	int i = 0 ; 
	uint gemType = 0;
	struct Gem_Recovery_Info * pBakGemPort = NULL;
	int channel = 0;

	for(i = 0; i < gponRecovery.gemPortNum; i++)
	{
		pBakGemPort = &gponRecovery.gemPort[i]; 
        
		if(gpon_check_gem_exist(pBakGemPort->gemPortId) != 1){
		
			if(GPON_MULTICAST_CHANNEL == pBakGemPort->channel || GPON_UNKNOWN_CHANNEL == pBakGemPort->channel){
			    gemType = GPON_MULTICAST_GEM;
		    }
		    else{
				gemType = GPON_UNICAST_GEM;
		    }

			channel = gpon_recover_get_real_channel(pBakGemPort->allocId);
			if((-1 == channel)  && (GPON_MULTICAST_CHANNEL != pBakGemPort->channel) && (GPON_UNKNOWN_CHANNEL != pBakGemPort->channel)){
				DBG_INFO(gponRecovery.dbgPrint, "channel err\n");
				continue;
			}
			if((GPON_MULTICAST_CHANNEL != pBakGemPort->channel) && (GPON_UNKNOWN_CHANNEL != pBakGemPort->channel))
				pBakGemPort->channel = channel;
			
			if(0 != gwan_create_new_gemport(pBakGemPort->gemPortId, pBakGemPort->channel, gemType,pBakGemPort->allocId)){
				DBG_INFO(gponRecovery.dbgPrint, "create_gemport err\n");
				continue;
			}
		}
		if(0 != gwan_config_gemport(pBakGemPort->gemPortId, ENUM_CFG_NETIDX, pBakGemPort->ani)){
			DBG_INFO(gponRecovery.dbgPrint, "config_gemport ENUM_CFG_NETIDX err\n");
			continue;
		}
		
		if(0 != gwan_config_gemport(pBakGemPort->gemPortId, ENUM_CFG_ENCRYPTION, pBakGemPort->encryption)){
			DBG_INFO(gponRecovery.dbgPrint, "config_gemport ENUM_CFG_ENCRYPTION err\n");
			continue;
		}
		DBG_INFO(gponRecovery.dbgPrint, "gpon_recover_create_gemport\n");
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
	struct Gem_Recovery_Info * bakGemport = gponRecovery.gemPort;
	struct Gem_Recovery_Info * tmp = NULL;

	tmp = find_backup_gemport(gemPortId);

	if(NULL == tmp){
		return XPON_FAIL;
	}

	if((tmp != &(gponRecovery.gemPort[gponRecovery.gemPortNum-1]))&&(gponRecovery.gemPortNum != 1) )
    {
        DBG_INFO(gponRecovery.dbgPrint, "gemPort = %d\n", tmp->gemPortId);
		memcpy(tmp, &bakGemport[gponRecovery.gemPortNum-1], sizeof(struct Gem_Recovery_Info));
	}
	clear_gemport_info(&bakGemport[gponRecovery.gemPortNum-1]);

	gponRecovery.gemPortNum--;

	return XPON_SUCCESS;
}
/*********************************************************************************************************************
Description:
	recover gemport inof
Input Args:
	none
Ret Value:
	none
*********************************************************************************************************************/
void gpon_exec_gem_recover(void)
{
	DBG_INFO(gponRecovery.dbgPrint, "gem recover, mibFlag:%d\n",gponRecovery.mibFlag);
	if(!gponRecovery.mibFlag)
	{	
		gpon_recover_create_gemport();
	}
	gponRecovery.mibFlag = 0;
}

