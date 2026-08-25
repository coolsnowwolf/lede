#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <utime.h>
#include <dirent.h>

#include "common/mgr_pthread.h"
#include "common/mgr_cmd.h"
#include "common/mgr_util.h"
#include "common/mgr_sdi.h"

#include "core/fmgr/fmgr.h"
#include "core/cmgr/cmgr.h"
#include "core/dbmgr/dbmgr.h"

#if 0
/***********************************************************************************
MACRO_CMD_STRUCT(GponSnPasswd)
{
	MGR_U8 		sn[CONST_GPON_SN_LENS] ;
	MGR_U8 		passwd[CONST_GPON_PASSWD_LENS] ;
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(GponSnPasswd)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(GponSnPasswd) *pObj = &pMsg->u.oGponSnPasswd ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_CREATE(GponSnPasswd) :		
			break ;
		case MACRO_CMDID_DELETE(GponSnPasswd) :
			break ;
		case MACRO_CMDID_SET(GponSnPasswd) :
			break ;
		case MACRO_CMDID_GET(GponSnPasswd) :
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(GponSnPasswd, ret) ;
}
#endif /* 0 */

/***********************************************************************************
 MACRO_CMD_STRUCT(PWanChannelQoS)
 {
 	MGR_QoSWeightType_t			WeightType ;
 	MGR_QoSWeightScale_t		WeightScale ;
 	struct {
 		MGR_U8						Channel ;
 		MGR_QosType_t				QosType ;
 		struct {
 			MGR_U8					Weight ;
 		} Queue[CONST_QUEUE_NUMBER] ;
 	} Channel[CONST_CHANNEL_NUMBER] ;
 } ;
MACRO_CMD_MASK(PWanChannelQoS)
{
	MASK_ChannelQosWeightParameters		= (1<<0) ,
	MASK_ChannelQosConfig				= (1<<1)
} ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PWanChannelQoS)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(PWanChannelQoS) *pObj = &pMsg->u.oPWanChannelQoS ;
	int i, j ;
	struct XMCS_QoSWeightConfig_S pwanQosWeightCfg ;
	struct XMCS_ChannelQoS_S pwanChannelQos ;

	switch(pMsg->CmdId) {
		case MACRO_CMDID_SET(PWanChannelQoS) :
			if(pMsg->Mask & MASK_ChannelQosWeightParameters) {
				if(pObj->WeightType!=ENUM_WEIGHT_TYPE_PACKET && pObj->WeightType!=ENUM_WEIGHT_TYPE_BYTE) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				} 
				pwanQosWeightCfg.weightType = pObj->WeightType ;
								
				if(pObj->WeightType == ENUM_WEIGHT_TYPE_BYTE) {
					if(pObj->WeightScale!=ENUM_WEIGHT_SCALE_64B && pObj->WeightScale!=ENUM_WEIGHT_SCALE_16B) {
						ret = EXEC_ERR_PARAM_INVAL ;
						break ;
					}
					pwanQosWeightCfg.weightScale = pObj->WeightScale ;
				}

				if((ret = XMCS_IOCTL_SDI(IF_IOS_QOS_WEIGHT_CONFIG, &pwanQosWeightCfg)) != 0) {
					break ;
				}
			} 
			
			if(pMsg->Mask & MASK_ChannelQosConfig) {
				if(pObj->Channel[0].Channel >= CONST_CHANNEL_NUMBER) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanChannelQos.channel = pObj->Channel[0].Channel ;
				
				if(pObj->Channel[0].QosType!=ENUM_QOS_TYPE_WRR && 
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SP &&
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SPWRR7 &&
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SPWRR6 &&
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SPWRR5 &&
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SPWRR4 &&
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SPWRR3 &&
				   pObj->Channel[0].QosType!=ENUM_QOS_TYPE_SPWRR2) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanChannelQos.qosType = pObj->Channel[0].QosType ;
				
				for(i=0 ; i<CONST_QUEUE_NUMBER ; i++) {
					if((pObj->Channel[0].Queue[i].Weight<0 || pObj->Channel[0].Queue[i].Weight>100) && pObj->Channel[0].Queue[i].Weight!=255) {
						ret = EXEC_ERR_PARAM_INVAL ;
						break ;
					}	
					pwanChannelQos.queue[i].weight = pObj->Channel[0].Queue[i].Weight ;
				}
				
				if((ret = XMCS_IOCTL_SDI(IF_IOS_QOS_SCHEDULER, &pwanChannelQos)) != 0) {
					break ;
				}
			}
			break ;
		case MACRO_CMDID_GET(PWanChannelQoS) :
			ret = XMCS_IOCTL_SDI(IF_IOG_QOS_WEIGHT_CONFIG, &pwanQosWeightCfg) ;
			if(ret != 0) {
				break ;
			}
			pObj->WeightType = pwanQosWeightCfg.weightType ;
			pObj->WeightScale = pwanQosWeightCfg.weightScale ;
			for(i=0 ; i<CONST_CHANNEL_NUMBER ; i++) {
				pwanChannelQos.channel = i ;
				ret = XMCS_IOCTL_SDI(IF_IOG_QOS_SCHEDULER, &pwanChannelQos) ;
				if(ret != 0) {
					break ;
				}
				pObj->Channel[i].Channel = i ;
				pObj->Channel[i].QosType = pwanChannelQos.qosType;
				for(j=0 ; j<CONST_QUEUE_NUMBER ; j++) {
					pObj->Channel[i].Queue[j].Weight = pwanChannelQos.queue[j].weight ;
				}
			}
			break ;
		case MACRO_CMDID_CREATE(PWanChannelQoS) :	
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PWanChannelQoS) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PWanChannelQoS, ret) ;
}

/***********************************************************************************
 MACRO_CMD_STRUCT(PWanCongestConfig)
 {
 	MGR_Enable_t			TrtcmMode ;
 	MGR_Enable_t			DeiDropMode ;
	MGR_Enable_t			ThresholdMode ;
 	MGR_CongestionScale_t	MaxScale ;
 	MGR_CongestionScale_t	MinScale ;
 	MGR_U8					GreenDropProbability ;
 	MGR_U8					YellowDropProbability ;
 	struct {
 		MGR_U8				QueueIdx ;
 		MGR_U8				GreenMaxThreshold ;
 		MGR_U8				GreenMinThreshold ;
 		MGR_U8				YellowMaxThreshold ;
 		MGR_U8				YellowMinThreshold ;
 	} Thrshld[CONST_QUEUE_NUMBER] ;
 } ;
 MACRO_CMD_MASK(PWanCongestConfig)
 {
 	MASK_CongestTrtcmMode				= (1<<0) ,
 	MASK_CongestDeiDropMode				= (1<<1) ,
 	MASK_CongestThresholdMode			= (1<<2) ,
 	MASK_CongestScale					= (1<<3) ,
 	MASK_CongestDropProbability			= (1<<4) ,
 	MASK_CongestThreshold				= (1<<5)
 } ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PWanCongestConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(PWanCongestConfig) *pObj = &pMsg->u.oPWanCongestConfig ;
	int i ;
	struct XMCS_TxQueueCongestion_S pwanCongest ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_SET(PWanCongestConfig) :
			if(pMsg->Mask & MASK_CongestTrtcmMode) {
				if(pObj->TrtcmMode!=ENUM_DISABLE && pObj->TrtcmMode!=ENUM_ENABLE) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanCongest.congestMode.trtcm = (pObj->TrtcmMode==ENUM_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
				if((ret = XMCS_IOCTL_SDI(IF_IOS_TRTCM_CONGEST_MODE, &pwanCongest)) != 0) {
					break ;
				}
			}
			
			if(pMsg->Mask & MASK_CongestDeiDropMode) {
				if(pObj->DeiDropMode!=ENUM_DISABLE && pObj->DeiDropMode!=ENUM_ENABLE) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanCongest.congestMode.dei = (pObj->DeiDropMode==ENUM_ENABLE) ? XPON_ENABLE : XPON_DISABLE ;
				if((ret = XMCS_IOCTL_SDI(IF_IOS_DEI_CONGEST_MODE, &pwanCongest)) != 0) {
					break ;
				}
			} 
			
			if(pMsg->Mask & MASK_CongestThresholdMode) {
				if(pObj->ThresholdMode!=ENUM_DISABLE && pObj->ThresholdMode!=ENUM_ENABLE && pObj->ThresholdMode!=ENUM_OTHER) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanCongest.congestMode.threshold = pObj->ThresholdMode;
				if((ret = XMCS_IOCTL_SDI(IF_IOS_THRESHOLD_CONGEST_MODE, &pwanCongest)) != 0) {
					break ;
				}
			} 
			
			if(pMsg->Mask & MASK_CongestScale) {
				if((pObj->MaxScale!=ENUM_CONGESTIOM_SCALE_2 && pObj->MaxScale!=ENUM_CONGESTIOM_SCALE_4 && pObj->MaxScale!=ENUM_CONGESTIOM_SCALE_8 && pObj->MaxScale!=ENUM_CONGESTIOM_SCALE_16) ||
				   (pObj->MinScale!=ENUM_CONGESTIOM_SCALE_2 && pObj->MinScale!=ENUM_CONGESTIOM_SCALE_4 && pObj->MinScale!=ENUM_CONGESTIOM_SCALE_8 && pObj->MinScale!=ENUM_CONGESTIOM_SCALE_16)) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanCongest.scale.max = pObj->MaxScale ;
				pwanCongest.scale.min = pObj->MinScale ;
				if((ret = XMCS_IOCTL_SDI(IF_IOS_CONGEST_SCALE, &pwanCongest)) != 0) {
					break ;
				}
			}
			
			if(pMsg->Mask & MASK_CongestDropProbability) {
				if(pObj->GreenDropProbability>255 || pObj->YellowDropProbability>255) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanCongest.dropProbability.green = pObj->GreenDropProbability ;
				pwanCongest.dropProbability.yellow = pObj->YellowDropProbability ;
				if((ret = XMCS_IOCTL_SDI(IF_IOS_CONGEST_DROP_PROBABILITY, &pwanCongest)) != 0) {
					break ;
				}
			}

			if(pMsg->Mask & MASK_CongestThreshold) {
				if(pObj->Thrshld[0].QueueIdx >= CONST_QUEUE_NUMBER) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				if((pObj->Thrshld[0].GreenMaxThreshold < pObj->Thrshld[0].GreenMinThreshold) ||
				   (pObj->Thrshld[0].YellowMaxThreshold < pObj->Thrshld[0].YellowMinThreshold)) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanCongest.queueThreshold[0].queueIdx = pObj->Thrshld[0].QueueIdx ;
				pwanCongest.queueThreshold[0].greenMax = pObj->Thrshld[0].GreenMaxThreshold ;
				pwanCongest.queueThreshold[0].greenMin = pObj->Thrshld[0].GreenMinThreshold ;
				pwanCongest.queueThreshold[0].yellowMax = pObj->Thrshld[0].YellowMaxThreshold ;
				pwanCongest.queueThreshold[0].yellowMin = pObj->Thrshld[0].YellowMinThreshold ;
				if((ret = XMCS_IOCTL_SDI(IF_IOS_CONGEST_THRESHOLD, &pwanCongest)) != 0) {
					break ;
				}
			}
			break ;
		case MACRO_CMDID_GET(PWanCongestConfig) :
			ret = XMCS_IOCTL_SDI(IO_IOG_CONGEST_INFO, &pwanCongest) ;
			if(ret == 0) {
				pObj->TrtcmMode = pwanCongest.congestMode.trtcm ;
				pObj->DeiDropMode = pwanCongest.congestMode.dei ;
				pObj->ThresholdMode = pwanCongest.congestMode.threshold ;
				pObj->MaxScale = pwanCongest.scale.max ;
				pObj->MinScale = pwanCongest.scale.min ;
				pObj->GreenDropProbability = pwanCongest.dropProbability.green ;
				pObj->YellowDropProbability = pwanCongest.dropProbability.yellow ;
				for(i=0 ; i<CONST_QUEUE_NUMBER ; i++) {
					pObj->Thrshld[i].QueueIdx = i ;
					pObj->Thrshld[i].GreenMaxThreshold = pwanCongest.queueThreshold[i].greenMax ;
					pObj->Thrshld[i].GreenMinThreshold = pwanCongest.queueThreshold[i].greenMin ;
					pObj->Thrshld[i].YellowMaxThreshold = pwanCongest.queueThreshold[i].yellowMax ;
					pObj->Thrshld[i].YellowMinThreshold = pwanCongest.queueThreshold[i].yellowMin ;
				}
			}
			break ;
		case MACRO_CMDID_CREATE(PWanCongestConfig) :		
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PWanCongestConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PWanCongestConfig, ret) ;
}

/***********************************************************************************
 MACRO_CMD_STRUCT(PWanTrtcmConfig)
 {
 	MGR_TrtcmScale_t			TrtcmScale ;
 	struct {
 		MGR_U8					TSIdx ;
 		MGR_U16					CIRValue ;
 		MGR_U16					CBSUnit ;
 		MGR_U16					PIRValue ;
 		MGR_U16					PBSUnit ;
 	} Trtcm[CONST_SHAPING_NUMBER] ;
 } ;
 MACRO_CMD_MASK(PWanCongestConfig)
 {
 	MASK_TrtcmScale						= (1<<0) ,
 	MASK_TrtcmParameters				= (1<<1) 
 } ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PWanTrtcmConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(PWanTrtcmConfig) *pObj = &pMsg->u.oPWanTrtcmConfig ;
	int i ;
	struct XMCS_TxTrtcmScale_S pwanTrtcmScale ;
	struct XMCS_TxQueueTrtcm_S pwanTrtcmParameters ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_SET(PWanTrtcmConfig) :
			if(pMsg->Mask & MASK_TrtcmScale) {
				if(pObj->TrtcmScale<0 || pObj->TrtcmScale>=ENUM_IF_TRTCM_SCALE_ITEMS) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanTrtcmScale.trtcmScale = pObj->TrtcmScale ;
				if((ret = XMCS_IOCTL_SDI(IO_IOS_TX_TRTCM_SCALE, &pwanTrtcmScale)) != 0) {
					break ;
				}
				
				for(i=0 ; i<CONST_SHAPING_NUMBER ; i++) {
					pwanTrtcmParameters.tsIdx = i ;
					pwanTrtcmParameters.cirValue = 0 ;
					pwanTrtcmParameters.cbsUnit = 0 ;
					pwanTrtcmParameters.pirValue = 0 ;
					pwanTrtcmParameters.pbsUnit = 0 ;
					XMCS_IOCTL_SDI(IO_IOS_TX_TRTCM_PARAMS, &pwanTrtcmParameters) ;
				}
			} 
			
			if(pMsg->Mask & MASK_TrtcmParameters) {
				if(pObj->Trtcm[0].TSIdx >= CONST_SHAPING_NUMBER) {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				if(pObj->Trtcm[0].CIRValue > pObj->Trtcm[0].PIRValue) {	
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanTrtcmParameters.tsIdx = pObj->Trtcm[0].TSIdx ;
				pwanTrtcmParameters.cirValue = pObj->Trtcm[0].CIRValue ;
				pwanTrtcmParameters.cbsUnit = pObj->Trtcm[0].CBSUnit ;
				pwanTrtcmParameters.pirValue = pObj->Trtcm[0].PIRValue ;
				pwanTrtcmParameters.pbsUnit = pObj->Trtcm[0].PBSUnit ;
				if((ret = XMCS_IOCTL_SDI(IO_IOS_TX_TRTCM_PARAMS, &pwanTrtcmParameters)) != 0) {
					break ;
				}
			} 
			break ;
		case MACRO_CMDID_GET(PWanTrtcmConfig) :
			ret = XMCS_IOCTL_SDI(IO_IOG_TX_TRTCM_SCALE, &pwanTrtcmScale) ;
			if(ret != 0) {
				break ;
			}
			pObj->TrtcmScale = pwanTrtcmScale.trtcmScale ;
		
			for(i=0 ; i<CONST_SHAPING_NUMBER ; i++) {
				pwanTrtcmParameters.tsIdx = i ;
				ret = XMCS_IOCTL_SDI(IO_IOG_TX_TRTCM_PARAMS, &pwanTrtcmParameters) ;
				if(ret != 0) {
					break ;
				}
				pObj->Trtcm[i].TSIdx = i ;
				pObj->Trtcm[i].CIRValue = pwanTrtcmParameters.cirValue;
				pObj->Trtcm[i].CBSUnit = pwanTrtcmParameters.cbsUnit ;
				pObj->Trtcm[i].PIRValue = pwanTrtcmParameters.pirValue ;
				pObj->Trtcm[i].PBSUnit = pwanTrtcmParameters.pbsUnit ;
			}
			break ;
		case MACRO_CMDID_CREATE(PWanTrtcmConfig) :		
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PWanTrtcmConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PWanTrtcmConfig, ret) ;
}

/***********************************************************************************
 MACRO_CMD_STRUCT(PWanPcpConfig)
 {
	MGR_PcpMode_t				CdmTxEncode ;
	MGR_PcpMode_t				CdmRxDecode ;
	MGR_PcpMode_t				GdmRxDecode ;
 } ;
 MACRO_CMD_MASK(PWanPcpConfig)
 {
	MASK_PcpCdmTxEncoding				= (1<<0) ,
	MASK_PcpCdmRxDecoding				= (1<<1) ,
	MASK_PcpGdmRxDecoding				= (1<<2) 
 } ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PWanPcpConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(PWanPcpConfig) *pObj = &pMsg->u.oPWanPcpConfig ;
	struct XMCS_TxPCPConfig_S pwanPcpCfg ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_SET(PWanPcpConfig) :
			if(pMsg->Mask & MASK_PcpCdmTxEncoding) {
				if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_DISABLE) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_DISABLE ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_8B0D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_8B0D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_7B1D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_7B1D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_6B2D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_6B2D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_5B3D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_5B3D ;			
				} else {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanPcpCfg.pcpType = XMCS_IF_PCP_TYPE_CDM_TX ;
				if((ret = XMCS_IOCTL_SDI(IO_IOS_PCP_CONFIG, &pwanPcpCfg)) != 0) {
					break ;
				}
			}
			if(pMsg->Mask & MASK_PcpCdmRxDecoding) {
				if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_DISABLE) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_DISABLE ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_8B0D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_8B0D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_7B1D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_7B1D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_6B2D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_6B2D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_5B3D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_5B3D ;			
				} else {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanPcpCfg.pcpType = XMCS_IF_PCP_TYPE_CDM_RX ;
				if((ret = XMCS_IOCTL_SDI(IO_IOS_PCP_CONFIG, &pwanPcpCfg)) != 0) {
					break ;
				}
			}
			if(pMsg->Mask & MASK_PcpGdmRxDecoding) {
				if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_DISABLE) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_DISABLE ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_8B0D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_8B0D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_7B1D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_7B1D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_6B2D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_6B2D ;
				} else if(pObj->CdmTxEncode == ENUM_IF_PCP_MODE_5B3D) {
					pwanPcpCfg.pcpMode = XMCS_IF_PCP_MODE_5B3D ;			
				} else {
					ret = EXEC_ERR_PARAM_INVAL ;
					break ;
				}
				pwanPcpCfg.pcpType = XMCS_IF_PCP_TYPE_GDM_RX ;
				if((ret = XMCS_IOCTL_SDI(IO_IOS_PCP_CONFIG, &pwanPcpCfg)) != 0) {
					break ;
				}
			}
			break ;
		case MACRO_CMDID_GET(PWanPcpConfig) :
			pwanPcpCfg.pcpType = XMCS_IF_PCP_TYPE_CDM_TX ;
			if((ret = XMCS_IOCTL_SDI(IO_IOG_PCP_CONFIG, &pwanPcpCfg)) != 0) {
				break ;
			}
			pObj->CdmTxEncode = pwanPcpCfg.pcpMode ;

			pwanPcpCfg.pcpType = XMCS_IF_PCP_TYPE_CDM_RX ;
			ret = XMCS_IOCTL_SDI(IO_IOG_PCP_CONFIG, &pwanPcpCfg) ;
			if(ret != 0) {
				break ;
			}
			pObj->CdmRxDecode = pwanPcpCfg.pcpMode ;

			pwanPcpCfg.pcpType = XMCS_IF_PCP_TYPE_GDM_RX ;
			ret = XMCS_IOCTL_SDI(IO_IOG_PCP_CONFIG, &pwanPcpCfg) ;
			if(ret != 0) {
				break ;
			}
			pObj->GdmRxDecode = pwanPcpCfg.pcpMode ;
			break ;
		case MACRO_CMDID_CREATE(PWanPcpConfig) :		
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
		case MACRO_CMDID_DELETE(PWanPcpConfig) :
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PWanPcpConfig, ret) ;
}

/***********************************************************************************
 MACRO_CMD_STRUCT(PWanCntStats)
 {
	MGR_PcpMode_t				CdmTxEncode ;
	MGR_PcpMode_t				CdmRxDecode ;
	MGR_PcpMode_t				GdmRxDecode ;
 } ;
***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(PWanCntStats)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(PWanCntStats) *pObj = &pMsg->u.oPWanCntStats ;
	struct XMCS_WanCntStats_S pWanCntStats ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_SET(PWanCntStats) :
			ret = XMCS_IOCTL_SDI(IO_IOS_WAN_CNT_STATS, pObj->mask);
			break ;
		case MACRO_CMDID_GET(PWanCntStats) :
			if((ret = XMCS_IOCTL_SDI(IO_IOG_WAN_CNT_STATS, &pWanCntStats)) != 0) {
				break ;
			}
			memcpy(pObj, &pWanCntStats, sizeof(struct XMCS_WanCntStats_S));
			break ;
		//case MACRO_CMDID_CREATE(PWanCntStats) :		
		//	ret = EXEC_ERR_CMD_NOT_SUPPORT ;
		//	break ;
		//case MACRO_CMDID_DELETE(PWanCntStats) :
		//	ret = EXEC_ERR_CMD_NOT_SUPPORT ;
		//	break ;
     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(PWanCntStats, ret) ;
}


/***********************************************************************************

***********************************************************************************/
MACRO_CMD_PROC_FUNCTION(StormCtrlConfig)
{
	MGR_Ret ret = EXEC_OK ;
	MACRO_CMD_STRUCT(StormCtrlConfig) *pObj = &pMsg->u.oStormCtrlConfig ;
	struct XMCS_StormCtrlConfig_S stormCtrlConfig ;
	
	switch(pMsg->CmdId) {
		case MACRO_CMDID_SET(StormCtrlConfig) :
			printf("stormCtrlConfig set: ---stormCtrlConfig.mask = %d\r\n", pObj->mask);
			printf("stormCtrlConfig set: ---stormCtrlConfig.threld = %d\r\n", pObj->threld);
			printf("stormCtrlConfig set: ---stormCtrlConfig.timer = %d\r\n", pObj->timer);
			stormCtrlConfig.mask= pObj->mask;
			stormCtrlConfig.threld = pObj->threld;
			stormCtrlConfig.timer = pObj->timer;
			
			ret = XMCS_IOCTL_SDI(IO_IOS_STORM_CTL_CONFIG, &stormCtrlConfig);
			break ;
		case MACRO_CMDID_GET(StormCtrlConfig) :
			if((ret = XMCS_IOCTL_SDI(IO_IOG_STORM_CTL_CONFIG, &stormCtrlConfig)) != 0) {
				break ;
			}
			printf("stormCtrlConfig get: ---stormCtrlConfig.threld = %d\r\n", stormCtrlConfig.threld);
			printf("stormCtrlConfig get: ---stormCtrlConfig.threld = %d\r\n", stormCtrlConfig.timer);
			
			memcpy(pObj, &stormCtrlConfig, sizeof(struct XMCS_StormCtrlConfig_S));
			break ;

     	default:
			ret = EXEC_ERR_CMD_NOT_SUPPORT ;
			break ;
	}     

	MACRO_PROC_RETURN(StormCtrlConfig, ret) ;
}




