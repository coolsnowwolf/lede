#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "api/mgr_api.h"
#include "cmd_cmds.h"
#include "cmd_enum.h"
#include "cmd_const.h"


/*******************************************************************************************
*******************************************************************************************/
/*****************************************************************************
typedef enum {
	ENUM_WEIGHT_TYPE_PACKET 	= XMCS_IF_WEIGHT_TYPE_PACKET,
	ENUM_WEIGHT_TYPE_BYTE		= XMCS_IF_WEIGHT_TYPE_BYTE
} MGR_QoSWeightType_t ;

typedef enum {
	ENUM_WEIGHT_SCALE_64B 		= XMCS_IF_WEIGHT_SCALE_64B,
	ENUM_WEIGHT_SCALE_16B		= XMCS_IF_WEIGHT_SCALE_16B
} MGR_QoSWeightScale_t ;

typedef enum {
	ENUM_QOS_TYPE_WRR 			= XMCS_IF_QOS_TYPE_WRR,
	ENUM_QOS_TYPE_SP			= XMCS_IF_QOS_TYPE_SP,
	ENUM_QOS_TYPE_SPWRR7		= XMCS_IF_QOS_TYPE_SPWRR7, 
	ENUM_QOS_TYPE_SPWRR6		= XMCS_IF_QOS_TYPE_SPWRR6, 
	ENUM_QOS_TYPE_SPWRR5		= XMCS_IF_QOS_TYPE_SPWRR5, 
	ENUM_QOS_TYPE_SPWRR4		= XMCS_IF_QOS_TYPE_SPWRR4, 
	ENUM_QOS_TYPE_SPWRR3		= XMCS_IF_QOS_TYPE_SPWRR3, 
	ENUM_QOS_TYPE_SPWRR2		= XMCS_IF_QOS_TYPE_SPWRR2
} MGR_QosType_t ;

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
*****************************************************************************/
void cmdPWanGetChannelQos(const int argc, char **const argv)
{
	int i ;
	MACRO_CMD_STRUCT(PWanChannelQoS) pwanQosInfo ;
	
	if(PonApi_PWanChannelQoS_Get(&pwanQosInfo) != EXEC_OK) {
		printf("Exec. Failed: Get the 'PWanChannelQoS' command failed\n") ;
		return ;
	}
	
	printf(" QoS Weigth Type: %s\n", (pwanQosInfo.WeightType==ENUM_WEIGHT_TYPE_PACKET)?"packet":"byte") ;
	if(pwanQosInfo.WeightType == ENUM_WEIGHT_TYPE_BYTE) {
		printf(" QoS Weigth Scale: %s\n", (pwanQosInfo.WeightScale==ENUM_WEIGHT_SCALE_64B)?"64 Byte":"16 Byte") ;
	}
	for(i=0 ; i<CONST_CHANNEL_NUMBER ; i++) {
		if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_WRR) {
			printf("%2d    WRR", i) ;	
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SP) {
			printf("%2d     SP", i) ;	
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SPWRR7) {
			printf("%2d SPWRR7", i) ;
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SPWRR6) {
			printf("%2d SPWRR6", i) ;
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SPWRR5) {
			printf("%2d SPWRR5", i) ;
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SPWRR4) {
			printf("%2d SPWRR4", i) ;
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SPWRR3) {
			printf("%2d SPWRR3", i) ;
		} else if(pwanQosInfo.Channel[i].QosType == ENUM_QOS_TYPE_SPWRR2) {
			printf("%2d SPWRR2", i) ;
		} 
		
		printf(" %3d %3d %3d %3d %3d %3d %3d %3d\n", pwanQosInfo.Channel[i].Queue[0].Weight, 
													pwanQosInfo.Channel[i].Queue[1].Weight, 
													pwanQosInfo.Channel[i].Queue[2].Weight, 
													pwanQosInfo.Channel[i].Queue[3].Weight, 
													pwanQosInfo.Channel[i].Queue[4].Weight, 
													pwanQosInfo.Channel[i].Queue[5].Weight, 
													pwanQosInfo.Channel[i].Queue[6].Weight, 
													pwanQosInfo.Channel[i].Queue[7].Weight) ;
	}
}

/*****************************************************************************
typedef enum {
	ENUM_DISABLE = 0,
	ENUM_ENABLE 
} MGR_Enable_t ;

typedef enum {
	ENUM_CONGESTIOM_SCALE_2		= XMCS_IF_CONGESTIOM_SCALE_2,
	ENUM_CONGESTIOM_SCALE_4		= XMCS_IF_CONGESTIOM_SCALE_4, 
	ENUM_CONGESTIOM_SCALE_8		= XMCS_IF_CONGESTIOM_SCALE_8, 
	ENUM_CONGESTIOM_SCALE_16	= XMCS_IF_CONGESTIOM_SCALE_16
} MGR_CongestionScale_t ;

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
*****************************************************************************/
void cmdPWanGetCongestion(const int argc, char **const argv)
{
	int i ;
	MACRO_CMD_STRUCT(PWanCongestConfig) pwanCongestCfg ;
	
	if(PonApi_PWanCongestConfig_Get(&pwanCongestCfg) != EXEC_OK) {
		printf("Exec. Failed: Get the 'PWanCongestConfig' command failed\n") ;
		return ;
	}

	printf(" trTCM/DEI/Threshold: %s %s %s\n", (pwanCongestCfg.TrtcmMode==ENUM_ENABLE)?"enable":"disable", \
		(pwanCongestCfg.DeiDropMode==ENUM_ENABLE)?"enable":"disable" \
		, (pwanCongestCfg.ThresholdMode==ENUM_ENABLE)?"enable":"disable");

	printf(" Max scale value: %d DSCPs\n", (1<<(pwanCongestCfg.MaxScale+1))) ;
	printf(" Min scale value: %d DSCPs\n", (1<<(pwanCongestCfg.MinScale+1))) ;
	printf(" Gerrn dropped probability: %d (percent)\n", (pwanCongestCfg.GreenDropProbability*100)/255) ;
	printf(" Yellow dropped probability: %d (percent)\n", (pwanCongestCfg.YellowDropProbability*100)/255) ;
	printf(" Congestion threshold:\n") ;
	printf("  Queue   Green Max   Green Min   Yellow Max   Yellow Min\n") ;
	for(i=0 ; i<CONST_QUEUE_NUMBER ; i++) {
		printf("    %d        %3d         %3d         %3d           %3d\n", 
										pwanCongestCfg.Thrshld[i].QueueIdx, 
										pwanCongestCfg.Thrshld[i].GreenMaxThreshold,
										pwanCongestCfg.Thrshld[i].GreenMinThreshold,
										pwanCongestCfg.Thrshld[i].YellowMaxThreshold,
										pwanCongestCfg.Thrshld[i].YellowMinThreshold) ;
	}
}

/*****************************************************************************
typedef enum {
	ENUM_IF_TRTCM_SCALE_1B			= XMCS_IF_TRTCM_SCALE_1B, 
	ENUM_IF_TRTCM_SCALE_2B			= XMCS_IF_TRTCM_SCALE_2B,  
	ENUM_IF_TRTCM_SCALE_4B			= XMCS_IF_TRTCM_SCALE_4B,  
	ENUM_IF_TRTCM_SCALE_8B			= XMCS_IF_TRTCM_SCALE_8B,  
	ENUM_IF_TRTCM_SCALE_16B			= XMCS_IF_TRTCM_SCALE_16B, 
	ENUM_IF_TRTCM_SCALE_32B			= XMCS_IF_TRTCM_SCALE_32B, 
	ENUM_IF_TRTCM_SCALE_64B			= XMCS_IF_TRTCM_SCALE_64B, 
	ENUM_IF_TRTCM_SCALE_128B		= XMCS_IF_TRTCM_SCALE_128B,
	ENUM_IF_TRTCM_SCALE_256B		= XMCS_IF_TRTCM_SCALE_256B,
	ENUM_IF_TRTCM_SCALE_512B		= XMCS_IF_TRTCM_SCALE_512B,
	ENUM_IF_TRTCM_SCALE_1K			= XMCS_IF_TRTCM_SCALE_1K,  
	ENUM_IF_TRTCM_SCALE_2K			= XMCS_IF_TRTCM_SCALE_2K,  
	ENUM_IF_TRTCM_SCALE_4K			= XMCS_IF_TRTCM_SCALE_4K,  
	ENUM_IF_TRTCM_SCALE_8K			= XMCS_IF_TRTCM_SCALE_8K,  
	ENUM_IF_TRTCM_SCALE_16K			= XMCS_IF_TRTCM_SCALE_16K, 
	ENUM_IF_TRTCM_SCALE_32K			= XMCS_IF_TRTCM_SCALE_32K, 
	ENUM_IF_TRTCM_SCALE_ITEMS       = XMCS_IF_TRTCM_SCALE_ITEMS
} MGR_TrtcmScale_t ;

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
*****************************************************************************/
void cmdPWanGetTrtcm(const int argc, char **const argv)
{
	int i ;
	MACRO_CMD_STRUCT(PWanTrtcmConfig) pwanTrtcmCfg ;
	
	if(PonApi_PWanTrtcmConfig_Get(&pwanTrtcmCfg) != EXEC_OK) {
		printf("Exec. Failed: Get the 'PWanTrtcmConfig' command failed\n") ;
		return ;
	}
	
	printf(" trTCM scale value: %d Byte\n", 1<<(pwanTrtcmCfg.TrtcmScale)) ;
	printf(" trTCM configuration:\n") ;
	printf("  TSID      CIR      CBS      PIR      PBS\n") ;
	for(i=0 ; i<CONST_SHAPING_NUMBER ; i++) {
		if(pwanTrtcmCfg.Trtcm[i].CIRValue!=0 || pwanTrtcmCfg.Trtcm[i].CBSUnit!=0 || pwanTrtcmCfg.Trtcm[i].PIRValue!=0 || pwanTrtcmCfg.Trtcm[i].PBSUnit!=0) {
			printf("   %2d      %4d     %4d     %4d     %4d\n", 
										pwanTrtcmCfg.Trtcm[i].TSIdx, 
										pwanTrtcmCfg.Trtcm[i].CIRValue,
										pwanTrtcmCfg.Trtcm[i].CBSUnit,
										pwanTrtcmCfg.Trtcm[i].PIRValue,
										pwanTrtcmCfg.Trtcm[i].PBSUnit) ;
		}
	}
}

/*******************************************************************************************
*******************************************************************************************/
/*****************************************************************************
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
*****************************************************************************/
void cmdPWanSetQosWeight(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanChannelQoS) pwanQosCfg ;
	unsigned int value ;
	
	if(!strcmp(argv[0], "packet")) {
		pwanQosCfg.WeightType = ENUM_WEIGHT_TYPE_PACKET ;
	} else if(!strcmp(argv[0], "byte")) {
		pwanQosCfg.WeightType = ENUM_WEIGHT_TYPE_BYTE ;
	} else {
		printf("Input Error: QoS weight type error.\n") ;
		return ;
	}
	
	if(pwanQosCfg.WeightType == ENUM_WEIGHT_TYPE_BYTE) {
		if(argc != 2) {
			printf("Input Error: QoS weight scale error.\n") ;
			return ;
		}
		
		value = strtoul(argv[1], NULL, 0) ;
		if(value == 64) {
			pwanQosCfg.WeightScale = ENUM_WEIGHT_SCALE_64B ;
		} else if(value == 16) {
			pwanQosCfg.WeightScale = ENUM_WEIGHT_SCALE_16B ;
		} else {
			printf("Input Error: QoS weight scale error.\n") ;
			return ;
		}
	}
	
	if(PonApi_PWanChannelQoS_Set((MASK_ChannelQosWeightParameters), &pwanQosCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanChannelQoS' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
*****************************************************************************/
void cmdPWanSetChannelQos(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanChannelQoS) pwanQosCfg ;
	unsigned int value ;
	int i ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value > CONST_CHANNEL_NUMBER) {
		printf("Input Error: Channel error.\n") ;
		return ;
	}
	pwanQosCfg.Channel[0].Channel = value ;
	
	if(!strcmp(argv[1], "sp")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SP ;
	} else if(!strcmp(argv[1], "wrr")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_WRR ;
	} else if(!strcmp(argv[1], "spwrr7")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SPWRR7 ;
	} else if(!strcmp(argv[1], "spwrr6")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SPWRR6 ;
	} else if(!strcmp(argv[1], "spwrr5")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SPWRR5 ;
	} else if(!strcmp(argv[1], "spwrr4")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SPWRR4 ;
	} else if(!strcmp(argv[1], "spwrr3")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SPWRR3 ;
	} else if(!strcmp(argv[1], "spwrr2")) {
		pwanQosCfg.Channel[0].QosType = ENUM_QOS_TYPE_SPWRR2 ;
	} else {
		printf("Input Error: QoS type error.\n") ;
		return ;
	}
	
	for(i=0 ; i<CONST_QUEUE_NUMBER ; i++) {
		value = strtoul(argv[2+i], NULL, 0) ;
		if((value<0 || value>100) && value!=255) {
			printf("Input Error: Queue %d weight error (0~100, 255 means don't care).\n", i) ;
			return ;
		} 
		pwanQosCfg.Channel[0].Queue[i].Weight = value ;
	}

	if(PonApi_PWanChannelQoS_Set((MASK_ChannelQosConfig), &pwanQosCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanChannelQoS' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
*****************************************************************************/
void cmdPWanSetCongestMode(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanCongestConfig) pwanCongestCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value == 0) {
		pwanCongestCfg.TrtcmMode = ENUM_DISABLE ;
	} else if(value == 1) {
		pwanCongestCfg.TrtcmMode = ENUM_ENABLE ;
	} else {
		printf("Input Error: trTCM congestion mode error.\n") ;
		return ;
	}

	value = strtoul(argv[1], NULL, 0) ;
	if(value == 0) {
		pwanCongestCfg.DeiDropMode = ENUM_DISABLE ;
	} else if(value == 1) {
		pwanCongestCfg.DeiDropMode = ENUM_ENABLE ;
	} else {
		printf("Input Error: DEI dropped congestion mode error.\n") ;
		return ;
	}
	
	value = strtoul(argv[2], NULL, 0) ;
	if(value == 0) {
		pwanCongestCfg.ThresholdMode = ENUM_DISABLE ;
	} else if(value == 1) {
		pwanCongestCfg.ThresholdMode = ENUM_ENABLE ;
	} else {
		printf("Input Error: Threshold congestion mode error.\n") ;
		return ;
	}
	
	if(PonApi_PWanCongestConfig_Set((MASK_CongestTrtcmMode|MASK_CongestDeiDropMode|MASK_CongestThresholdMode), &pwanCongestCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanCongestConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
*****************************************************************************/
void cmdPWanSetCongestScale(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanCongestConfig) pwanCongestCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value == 2) {
		pwanCongestCfg.MaxScale = ENUM_CONGESTIOM_SCALE_2 ;
	} else if(value == 4) {
		pwanCongestCfg.MaxScale = ENUM_CONGESTIOM_SCALE_4 ;
	} else if(value == 8) {
		pwanCongestCfg.MaxScale = ENUM_CONGESTIOM_SCALE_8 ;
	} else if(value == 16) {
		pwanCongestCfg.MaxScale = ENUM_CONGESTIOM_SCALE_16 ;
	} else {		
		printf("Input Error: Max scale error.\n") ;
		return ;
	}
	
	value = strtoul(argv[1], NULL, 0) ;
	if(value == 2) {
		pwanCongestCfg.MinScale = ENUM_CONGESTIOM_SCALE_2 ;
	} else if(value == 4) {
		pwanCongestCfg.MinScale = ENUM_CONGESTIOM_SCALE_4 ;
	} else if(value == 8) {
		pwanCongestCfg.MinScale = ENUM_CONGESTIOM_SCALE_8 ;
	} else if(value == 16) {
		pwanCongestCfg.MinScale = ENUM_CONGESTIOM_SCALE_16 ;
	} else {		
		printf("Input Error: Min scale error.\n") ;
		return ;
	}
		
	if(PonApi_PWanCongestConfig_Set((MASK_CongestScale), &pwanCongestCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanCongestConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
*****************************************************************************/
void cmdPWanSetCongestDropProbability(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanCongestConfig) pwanCongestCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value > 100) {
		printf("Input Error: Green dropped probability error.\n") ;
		return ;
	}
	pwanCongestCfg.GreenDropProbability = ((value*255)/100) + (((value*255)%100)?1:0) ;
	
	value = strtoul(argv[1], NULL, 0) ;
	if(value > 100) {
		printf("Input Error: Yellow dropped probability error.\n") ;
		return ;
	}
	pwanCongestCfg.YellowDropProbability = ((value*255)/100) + (((value*255)%100)?1:0) ;
		
	if(PonApi_PWanCongestConfig_Set((MASK_CongestDropProbability), &pwanCongestCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanCongestConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
*****************************************************************************/
void cmdPWanSetCongestThreshold(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanCongestConfig) pwanCongestCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value >= CONST_QUEUE_NUMBER) {
		printf("Input Error: Queue index error.\n") ;
		return ;
	}
	pwanCongestCfg.Thrshld[0].QueueIdx = value ;
	
	value = strtoul(argv[1], NULL, 0) ;
	if(value > 15) {
		printf("Input Error: Green max threshold error.\n") ;
		return ;
	}
	pwanCongestCfg.Thrshld[0].GreenMaxThreshold = value ;

	value = strtoul(argv[2], NULL, 0) ;
	if(value > 15) {
		printf("Input Error: Green min threshold error.\n") ;
		return ;
	}
	pwanCongestCfg.Thrshld[0].GreenMinThreshold = value ;

	value = strtoul(argv[3], NULL, 0) ;
	if(value > 15) {
		printf("Input Error: Yellow max threshold error.\n") ;
		return ;
	}
	pwanCongestCfg.Thrshld[0].YellowMaxThreshold = value ;
		
	value = strtoul(argv[4], NULL, 0) ;
	if(value > 15) {
		printf("Input Error: Yellow min threshold error.\n") ;
		return ;
	}
	pwanCongestCfg.Thrshld[0].YellowMinThreshold = value ;

	if(PonApi_PWanCongestConfig_Set((MASK_CongestThreshold), &pwanCongestCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanCongestConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
MACRO_CMD_MASK(PWanTrtcmConfig)
{
	MASK_TrtcmScale						= (1<<0) ,
	MASK_TrtcmParameters				= (1<<1) 
} ;
typedef enum {
	ENUM_IF_TRTCM_SCALE_1B			= XMCS_IF_TRTCM_SCALE_1B, 
	ENUM_IF_TRTCM_SCALE_2B			= XMCS_IF_TRTCM_SCALE_2B,  
	ENUM_IF_TRTCM_SCALE_4B			= XMCS_IF_TRTCM_SCALE_4B,  
	ENUM_IF_TRTCM_SCALE_8B			= XMCS_IF_TRTCM_SCALE_8B,  
	ENUM_IF_TRTCM_SCALE_16B			= XMCS_IF_TRTCM_SCALE_16B, 
	ENUM_IF_TRTCM_SCALE_32B			= XMCS_IF_TRTCM_SCALE_32B, 
	ENUM_IF_TRTCM_SCALE_64B			= XMCS_IF_TRTCM_SCALE_64B, 
	ENUM_IF_TRTCM_SCALE_128B		= XMCS_IF_TRTCM_SCALE_128B,
	ENUM_IF_TRTCM_SCALE_256B		= XMCS_IF_TRTCM_SCALE_256B,
	ENUM_IF_TRTCM_SCALE_512B		= XMCS_IF_TRTCM_SCALE_512B,
	ENUM_IF_TRTCM_SCALE_1K			= XMCS_IF_TRTCM_SCALE_1K,  
	ENUM_IF_TRTCM_SCALE_2K			= XMCS_IF_TRTCM_SCALE_2K,  
	ENUM_IF_TRTCM_SCALE_4K			= XMCS_IF_TRTCM_SCALE_4K,  
	ENUM_IF_TRTCM_SCALE_8K			= XMCS_IF_TRTCM_SCALE_8K,  
	ENUM_IF_TRTCM_SCALE_16K			= XMCS_IF_TRTCM_SCALE_16K, 
	ENUM_IF_TRTCM_SCALE_32K			= XMCS_IF_TRTCM_SCALE_32K, 
	ENUM_IF_TRTCM_SCALE_ITEMS      	= XMCS_IF_TRTCM_SCALE_ITEMS
} MGR_TrtcmScale_t ;
*****************************************************************************/
void cmdPWanSetTrtcmScale(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanTrtcmConfig) pwanTrtcmCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value == 1) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_1B ;
	} else if(value == 2) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_2B ;
	} else if(value == 4) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_4B ;
	} else if(value == 8) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_8B ;
	} else if(value == 16) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_16B ;
	} else if(value == 32) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_32B ;
	} else if(value == 64) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_64B ;
	} else if(value == 128) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_128B ;
	} else if(value == 256) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_256B ;
	} else if(value == 512) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_512B ;
	} else if(value == 1024) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_1K ;
	} else if(value == 2048) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_2K ;
	} else if(value == 4096) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_4K ;
	} else if(value == 8192) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_8K ;
	} else if(value == 16384) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_16K ;
	} else if(value == 32768) {
		pwanTrtcmCfg.TrtcmScale = ENUM_IF_TRTCM_SCALE_32K ;
	} else {
		printf("Input Error: Trtcm scale error.\n") ;
		return ;
	}

	if(PonApi_PWanTrtcmConfig_Set((MASK_TrtcmScale), &pwanTrtcmCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanTrtcmConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
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
MACRO_CMD_MASK(PWanTrtcmConfig)
{
	MASK_TrtcmScale						= (1<<0) ,
	MASK_TrtcmParameters				= (1<<1) 
} ;
*****************************************************************************/
void cmdPWanSetTrtcmParameters(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(PWanTrtcmConfig) pwanTrtcmCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value >= CONST_SHAPING_NUMBER) {
		printf("Input Error: TSID error.\n") ;
		return ;
	}
	pwanTrtcmCfg.Trtcm[0].TSIdx = value ;
	
	value = strtoul(argv[1], NULL, 0) ;
	if(value > 16000) {
		printf("Input Error: CIR unit error.\n") ;
		return ;
	}
	pwanTrtcmCfg.Trtcm[0].CIRValue = value ;

	value = strtoul(argv[2], NULL, 0) ;
	if(value > 65535) {
		printf("Input Error: CBS value error.\n") ;
		return ;
	}
	pwanTrtcmCfg.Trtcm[0].CBSUnit = value ;

	value = strtoul(argv[3], NULL, 0) ;
	if(value>16000 || value<pwanTrtcmCfg.Trtcm[0].CIRValue) {
		printf("Input Error: PIR unit error.\n") ;
		return ;
	}
	pwanTrtcmCfg.Trtcm[0].PIRValue = value ;

	value = strtoul(argv[4], NULL, 0) ;
	if(value > 65535) {
		printf("Input Error: PBS value error.\n") ;
		return ;
	}
	pwanTrtcmCfg.Trtcm[0].PBSUnit = value ;

	if(PonApi_PWanTrtcmConfig_Set((MASK_TrtcmParameters), &pwanTrtcmCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'PWanTrtcmConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}


void cmdPWanSetStormCtrlConfig(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(StormCtrlConfig) stormCtrlConfig ;
	unsigned int value ;

	printf("cmdPWanSetStormCtrlConfig:---argv[0] = %s\n",argv[0]) ;	
	value = strtoul(argv[0], NULL, 0) ;
	if(value > 128) {
		printf("Input Error: value error.\n") ;
		return ;
	}

	stormCtrlConfig.threld = value ;

	printf("cmdPWanSetStormCtrlConfig:---argv[1] = %s\n",argv[1]) ;	
	value = strtoul(argv[1], NULL, 0) ;
	stormCtrlConfig.timer = value ;

	if(PonApi_StormCtrlConfig_Set((MASK_StormThreld | MASK_StormTimer), &stormCtrlConfig) != EXEC_OK) {
		printf("Exec. Failed: Set the 'stormCtrlConfig' command failed\n\n") ;
		return ;
	}	
	printf("Exec. Successful\n") ;	
	return;
}	
void cmdPWanGetStormCtrlConfig(const int argc, char **const argv)
{
	int i ;
	MACRO_CMD_STRUCT(StormCtrlConfig) stormCtrlConfig ;
	
	if(PonApi_StormCtrlConfig_Get(&stormCtrlConfig) != EXEC_OK) {
		printf("Exec. Failed: Get the 'StormCtrlConfig' command failed\n") ;
		return ;
	}


	printf(" strom control threld: %d\n", stormCtrlConfig.threld) ;
	printf(" strom control timer: %d (ms)\n", stormCtrlConfig.timer) ;

	return;
}


/********************************************************************************************
********************************************************************************************/
void ponmgrProcessPWanCommand(int argc, char** argv) 
{
	int i, flag ;
	
	if(!strcmp(argv[2], "set")) {
		flag = kSysCmdSet ;
	} else if(!strcmp(argv[2], "get")) {
		flag = kSysCmdGet ;
	} else if(!strcmp(argv[2], "create")) {
		flag = kSysCmdCreate ;
	} else if(!strcmp(argv[2], "delete")) {
		flag = kSysCmdDelete ;
	} else {
		printf("CMD Failed: input command '%s' failed.\n", argv[2]) ;	
		return ;
	}

	for(i=0 ; i<gPWanCmdNum ; i++) {
		if((gPWanCmd[i].flags&flag) && !strcmp(argv[3], gPWanCmd[i].name)) {
			if(argc>=(gPWanCmd[i].minArgs+4) && argc<=(gPWanCmd[i].maxArgs+4)) {
				gPWanCmd[i].proc(argc-4, &argv[4]) ;
				return ;
			} else {
				printf("CMD Failed: input arguments failed.\n") ;	
				return ;
			}
		}
	}

	printf("CMD Failed: no such command, '%s'.\n", argv[3]) ;	
	return ;
}

