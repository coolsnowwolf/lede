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


void printGponPasswd(MACRO_CMD_STRUCT(GponSystemInfo) *gponInfo )
{
	int i;
	if(gponInfo->hexFlag)	
	{	
		printf("\n PASSWD:    0x");
		for(i=0; i<(gponInfo->PasswdLength/2); i++)
		{
			printf("%02X",gponInfo->Password[i]);
		}
		if(gponInfo->PasswdLength%2==1)
		{
			printf("%X",gponInfo->Password[i]);
		}
		printf("\n");
	}
	else
		printf("\n PASSWD:    %s\n", gponInfo->Password) ;
	//return 0;

}
/*********************************************************************************************
MACRO_CMD_STRUCT(GponSystemInfo)
{
	MGR_U8 				OnuId ;
	MGR_GponState_t 	ActState ;
	MGR_U8 				Serial[CONST_GPON_SN_LENS] ;
	MGR_U8				PasswdLength;
	MGR_U8        		hexFlag;
	MGR_U8 				Password[CONST_GPON_PASSWD_LENS] ;
	MGR_U8				KeyIdx ;
	MGR_U8				Key[CONST_GPON_ENCRYPT_KEY_LENS] ;
	MGR_U32 			TO1Timer ;
	MGR_U32				TO2Timer ;
	MGR_U16				OMCIChannel ;
} ;
*********************************************************************************************/
void cmdGponGetInfo(const int argc, char **const argv)
{
	int i ;
	MACRO_CMD_STRUCT(GponSystemInfo) gponInfo ;
	
	if(PonApi_GponSystemInfo_Get(&gponInfo) != EXEC_OK) {
		printf("Exec. Failed: Get the 'GponSystemInfo' command failed\n") ;
		return ;
	}
	
	printf("ONU Info:\n") ;
	printf(" ONU ID:    %d\n", gponInfo.OnuId) ;
	printf(" ONU State: O%d\n", gponInfo.ActState) ;
	printf(" SN:        ") ;
	for(i=0 ; i<4 ; i++) {
		printf("%c", gponInfo.Serial[i]) ;
	}
	for(i=4 ; i<8 ; i++) {
		printf("%.2x", gponInfo.Serial[i]) ;
	}
	printGponPasswd(&gponInfo );
	printf(" Key Idx:   %d\n", gponInfo.KeyIdx) ;
	printf(" Key:       ") ;
	for(i=0 ; i<16 ; i++) {
		printf("%.2x", gponInfo.Key[i]) ;
	}
	printf("\n O1 Timer:  %d ms\n", gponInfo.TO1Timer) ;
	printf(" O2 Timer:  %d ms\n", gponInfo.TO2Timer) ;
	printf(" OMCC ID:   %d\n", gponInfo.OMCIChannel) ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponTodConfig)
{
	MGR_U32		SFCounter ;
	MGR_U32		Second ;
	MGR_U32		Nanosecond ;
} ;
*****************************************************************************/
void cmdGponGetCurrentTime(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponTodConfig) gponTodCfg ;

	if(PonApi_GponTodConfig_Get(&gponTodCfg) != EXEC_OK) {
		printf("Exec. Failed: Get the 'GponTodConfig' command failed\n") ;
		return ;
	}
	
	printf("Current ToD Information:\n") ;
	printf(" Second:        %d\n", gponTodCfg.Second) ;
	printf(" Nanosecond:    %d\n", gponTodCfg.Nanosecond) ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponDbaConfig)
{
	MGR_U16 				BlockSize ;
	MGR_Enable_t			GponTrtcmMode ;
	MGR_GponTrtcmScale_t	GponTrtcmScale ;
	struct {
		MGR_U8					Channel ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} GponTrtcm[CONFIG_GPON_MAX_TCONT] ;
} ;
MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;
typedef enum {
	ENUM_GPON_TRTCM_SCALE_1B		= XMCS_GPON_TRTCM_SCALE_1B, 
	ENUM_GPON_TRTCM_SCALE_2B		= XMCS_GPON_TRTCM_SCALE_2B,  
	ENUM_GPON_TRTCM_SCALE_4B		= XMCS_GPON_TRTCM_SCALE_4B,  
	ENUM_GPON_TRTCM_SCALE_8B		= XMCS_GPON_TRTCM_SCALE_8B,  
	ENUM_GPON_TRTCM_SCALE_16B		= XMCS_GPON_TRTCM_SCALE_16B, 
	ENUM_GPON_TRTCM_SCALE_32B		= XMCS_GPON_TRTCM_SCALE_32B, 
	ENUM_GPON_TRTCM_SCALE_64B		= XMCS_GPON_TRTCM_SCALE_64B, 
	ENUM_GPON_TRTCM_SCALE_128B		= XMCS_GPON_TRTCM_SCALE_128B,
	ENUM_GPON_TRTCM_SCALE_256B		= XMCS_GPON_TRTCM_SCALE_256B,
	ENUM_GPON_TRTCM_SCALE_512B		= XMCS_GPON_TRTCM_SCALE_512B,
	ENUM_GPON_TRTCM_SCALE_1K		= XMCS_GPON_TRTCM_SCALE_1K,  
	ENUM_GPON_TRTCM_SCALE_2K		= XMCS_GPON_TRTCM_SCALE_2K,  
	ENUM_GPON_TRTCM_SCALE_4K		= XMCS_GPON_TRTCM_SCALE_4K,  
	ENUM_GPON_TRTCM_SCALE_8K		= XMCS_GPON_TRTCM_SCALE_8K,  
	ENUM_GPON_TRTCM_SCALE_16K		= XMCS_GPON_TRTCM_SCALE_16K, 
	ENUM_GPON_TRTCM_SCALE_32K		= XMCS_GPON_TRTCM_SCALE_32K, 
	ENUM_GPON_TRTCM_SCALE_ITEMS     = XMCS_GPON_TRTCM_SCALE_ITEMS
} MGR_GponTrtcmScale_t ;
*****************************************************************************/
void cmdGponGetDbaConfig(const int argc, char **const argv)
{
	int i ;
	MACRO_CMD_STRUCT(GponDbaConfig) gponDbaConfig ;
	
	if(PonApi_GponDbaConfig_Get(&gponDbaConfig) != EXEC_OK) {
		printf("Exec. Failed: Get the 'PWanTrtcmConfig' command failed\n") ;
		return ;
	}
	
	printf(" GPON trTCM mode: %s\n", (gponDbaConfig.GponTrtcmMode)?"Enable":"Disable") ;
	printf(" GPON trTCM scale value: %d Byte\n", 1<<(gponDbaConfig.GponTrtcmScale)) ;
	printf(" GPON trTCM configuration:\n") ;
	printf("  TSID      CIR      CBS      PIR      PBS\n") ;
	for(i=0 ; i<CONFIG_GPON_MAX_TCONT ; i++) {
		if(gponDbaConfig.GponTrtcm[i].CIRValue!=0 || gponDbaConfig.GponTrtcm[i].CBSUnit!=0 || gponDbaConfig.GponTrtcm[i].PIRValue!=0 || gponDbaConfig.GponTrtcm[i].PBSUnit!=0) {
			printf("   %2d      %4d     %4d     %4d     %4d\n", 
										gponDbaConfig.GponTrtcm[i].Channel, 
										gponDbaConfig.GponTrtcm[i].CIRValue,
										gponDbaConfig.GponTrtcm[i].CBSUnit,
										gponDbaConfig.GponTrtcm[i].PIRValue,
										gponDbaConfig.GponTrtcm[i].PBSUnit) ;
		}
	}
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_U16			GemPortId ;
		MGR_U16			AllocId ;		//just for create command
		MGR_U8			IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t	Loopback ;		//enable/disable loopback
		MGR_Enable_t	Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponGetGemPortInfo(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemPortConfig) gponGemInfo ;
	int i ;
	
	memset(&gponGemInfo, 0, sizeof(MACRO_CMD_STRUCT(GponGemPortConfig))) ;
	if(PonApi_GponGemPortConfig_Get(&gponGemInfo) != EXEC_OK) {
		printf("Exec. Failed: Get the 'GponGemPortConfig' command failed\n") ;
		return ;
	}
	
	printf("GPON GEM Port information, Entries:%d:\n", gponGemInfo.EntryNum) ;
	for(i=0 ; i<gponGemInfo.EntryNum ; i++) {
		printf(" %.2d  %.9s GEM Port:%4d, TCONT:%4d,", i+1, (gponGemInfo.GemPort[i].GemType==ENUM_GPON_GEM_TYPE_MULTICAST)?"Multicast":"Unicast",
														gponGemInfo.GemPort[i].GemPortId, gponGemInfo.GemPort[i].AllocId) ;
		
		if(gponGemInfo.GemPort[i].IfIndex>=0 && gponGemInfo.GemPort[i].IfIndex<CONST_MAX_BRIDGE_PORT) {
			printf(" MAC If:pon%d,", gponGemInfo.GemPort[i].IfIndex) ;
		} else if(gponGemInfo.GemPort[i].IfIndex == CONST_GPON_OMCI_INTERFACE) {
			printf(" MAC If:omci,") ;
		} else {
			printf(" MAC If:drop,") ;	
		}
		
		printf(" Loopback:%s, Encryption:%s\n", (gponGemInfo.GemPort[i].Loopback==ENUM_ENABLE)?"Enable":"Disable", 
												(gponGemInfo.GemPort[i].Encryption==ENUM_ENABLE)?"Enable":"Disable") ; 
	}
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponTcontStatus)
{
	struct {
		MGR_U16			AllocId ;
		MGR_U8			Channel ;
	} Tcont[CONFIG_GPON_MAX_TCONT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponGetTcontInfo(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponTcontStatus) gponTcontInfo ;
	int i ;
	
	if(PonApi_GponTcontStatus_Get(&gponTcontInfo) != EXEC_OK) {
		printf("Exec. Failed: Get the 'GponTcontStatus' command failed\n") ;
		return ;
	}
	
	printf("GPON TCONT information:\n") ;
	for(i=0 ; i<gponTcontInfo.EntryNum ; i++) {
		printf(" %.2d   TCONT ID:%d, Channel:%d\n", i+1, gponTcontInfo.Tcont[i].AllocId, gponTcontInfo.Tcont[i].Channel) ; 
	}
}

/*******************************************************************************************
*******************************************************************************************/
/*****************************************************************************
MACRO_CMD_STRUCT(GponSnPasswd)
{
	MGR_U8 		Serial[CONST_GPON_SN_LENS] ;
	MGR_U8 		Password[CONST_GPON_PASSWD_LENS] ;
	MGR_U8      EmergencyState  ;
	MGR_U8       PasswordFlag;
	MGR_U8       PasswordFlag;
} ;
*****************************************************************************/
void cmdGponSetSnPasswd(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponSnPasswd) gponSnPasswd ;
	unsigned int value ;
	unsigned char tmp[8] ={0};
	int i=0;
        int db=0;
	memset(&gponSnPasswd, 0, sizeof(MACRO_CMD_STRUCT(GponSnPasswd))) ;
	if(strlen(argv[0]) != 4) {
		printf("Input Error: Vendir ID length must = 4 characters\n") ;
		return ;
	}
	memcpy(gponSnPasswd.Serial, argv[0], 4) ;
	
	value = strtoul(argv[1], NULL, 16) ;
	gponSnPasswd.Serial[4] = (unsigned char)(value>>24) ;
	gponSnPasswd.Serial[5] = (unsigned char)(value>>16) ;
	gponSnPasswd.Serial[6] = (unsigned char)(value>>8) ;
	gponSnPasswd.Serial[7] = (unsigned char)(value) ;
	
	sscanf(argv[2],"%2s",&tmp);
	if(strcmp("0x",tmp)==0 || strcmp("0X",tmp)==0)
		{	
			if(strlen(argv[2]) > CONST_GPON_HEX_PASSWD_LENS)
			{
				printf("Input Error: Password length must < %d characters\n", CONST_GPON_HEX_PASSWD_LENS) ;
				return ;
			}
			gponSnPasswd.PasswdLength = strlen(argv[2])-2;
			gponSnPasswd.hexFlag = 1;
			for(i=0; i<(strlen(argv[2])-2)/2; i++)
			{
				sscanf(argv[2]+2+2*i, "%2X",&db);
				gponSnPasswd.Password[i]=(unsigned char) db;
			}
			if(1==(gponSnPasswd.PasswdLength)%2)
			{
				sscanf(argv[2]+2+2*i, "%2X",&db);
				gponSnPasswd.Password[i]=(unsigned char) db;
			}
		}
	else
		{
			if(strlen(argv[2]) > CONST_GPON_PASSWD_LENS) 
			{
				printf("Input Error: Password length must < %d characters in %s\n", CONST_GPON_PASSWD_LENS,__func__) ;
				return ;
			}
			gponSnPasswd.hexFlag=0;
			strcpy((char *)gponSnPasswd.Password, argv[2]) ;
		}
	if(PonApi_GponSnPasswd_Set(0, &gponSnPasswd) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponSnPasswd' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponActTimer)
{
	MGR_U32 	TO1Timer ;  
	MGR_U32		TO2Timer ;	
} ;
*****************************************************************************/
void cmdGponSetActTimer(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponActTimer) gponActTimer ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	gponActTimer.TO1Timer = value ;
	
	value = strtoul(argv[1], NULL, 0) ;
	gponActTimer.TO2Timer = value ;
	
	if(PonApi_GponActTimer_Set(0, &gponActTimer) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponActTimer' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponDbaConfig)
{
	MGR_U16 				BlockSize ;
	MGR_Enable_t			GponTrtcmMode ;
	MGR_GponTrtcmScale_t	GponTrtcmScale ;
	struct {
		MGR_U8					Channel ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} GponTrtcm[CONFIG_GPON_MAX_TCONT] ;
} ;
MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;
typedef enum {
	ENUM_GPON_TRTCM_SCALE_1B		= XMCS_GPON_TRTCM_SCALE_1B, 
	ENUM_GPON_TRTCM_SCALE_2B		= XMCS_GPON_TRTCM_SCALE_2B,  
	ENUM_GPON_TRTCM_SCALE_4B		= XMCS_GPON_TRTCM_SCALE_4B,  
	ENUM_GPON_TRTCM_SCALE_8B		= XMCS_GPON_TRTCM_SCALE_8B,  
	ENUM_GPON_TRTCM_SCALE_16B		= XMCS_GPON_TRTCM_SCALE_16B, 
	ENUM_GPON_TRTCM_SCALE_32B		= XMCS_GPON_TRTCM_SCALE_32B, 
	ENUM_GPON_TRTCM_SCALE_64B		= XMCS_GPON_TRTCM_SCALE_64B, 
	ENUM_GPON_TRTCM_SCALE_128B		= XMCS_GPON_TRTCM_SCALE_128B,
	ENUM_GPON_TRTCM_SCALE_256B		= XMCS_GPON_TRTCM_SCALE_256B,
	ENUM_GPON_TRTCM_SCALE_512B		= XMCS_GPON_TRTCM_SCALE_512B,
	ENUM_GPON_TRTCM_SCALE_1K		= XMCS_GPON_TRTCM_SCALE_1K,  
	ENUM_GPON_TRTCM_SCALE_2K		= XMCS_GPON_TRTCM_SCALE_2K,  
	ENUM_GPON_TRTCM_SCALE_4K		= XMCS_GPON_TRTCM_SCALE_4K,  
	ENUM_GPON_TRTCM_SCALE_8K		= XMCS_GPON_TRTCM_SCALE_8K,  
	ENUM_GPON_TRTCM_SCALE_16K		= XMCS_GPON_TRTCM_SCALE_16K, 
	ENUM_GPON_TRTCM_SCALE_32K		= XMCS_GPON_TRTCM_SCALE_32K, 
	ENUM_GPON_TRTCM_SCALE_ITEMS     = XMCS_GPON_TRTCM_SCALE_ITEMS
} MGR_GponTrtcmScale_t ;
*****************************************************************************/
void cmdGponSetTrtcmScale(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponDbaConfig) gponDbaConfig ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value == 1) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_1B ;
	} else if(value == 2) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_2B ;
	} else if(value == 4) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_4B ;
	} else if(value == 8) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_8B ;
	} else if(value == 16) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_16B ;
	} else if(value == 32) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_32B ;
	} else if(value == 64) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_64B ;
	} else if(value == 128) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_128B ;
	} else if(value == 256) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_256B ;
	} else if(value == 512) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_512B ;
	} else if(value == 1024) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_1K ;
	} else if(value == 2048) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_2K ;
	} else if(value == 4096) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_4K ;
	} else if(value == 8192) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_8K ;
	} else if(value == 16384) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_16K ;
	} else if(value == 32768) {
		gponDbaConfig.GponTrtcmScale = ENUM_GPON_TRTCM_SCALE_32K ;
	} else {
		printf("Input Error: Trtcm scale error.\n") ;
		return ;
	}

	if(PonApi_GponDbaConfig_Set((MASK_GponTrtcmScale), &gponDbaConfig) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponDbaConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponDbaConfig)
{
	MGR_U16 				BlockSize ;
	MGR_Enable_t			GponTrtcmMode ;
	MGR_GponTrtcmScale_t	GponTrtcmScale ;
	struct {
		MGR_U8					Channel ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} GponTrtcm[CONFIG_GPON_MAX_TCONT] ;
} ;
MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;
typedef enum {
	ENUM_GPON_TRTCM_SCALE_1B		= XMCS_GPON_TRTCM_SCALE_1B, 
	ENUM_GPON_TRTCM_SCALE_2B		= XMCS_GPON_TRTCM_SCALE_2B,  
	ENUM_GPON_TRTCM_SCALE_4B		= XMCS_GPON_TRTCM_SCALE_4B,  
	ENUM_GPON_TRTCM_SCALE_8B		= XMCS_GPON_TRTCM_SCALE_8B,  
	ENUM_GPON_TRTCM_SCALE_16B		= XMCS_GPON_TRTCM_SCALE_16B, 
	ENUM_GPON_TRTCM_SCALE_32B		= XMCS_GPON_TRTCM_SCALE_32B, 
	ENUM_GPON_TRTCM_SCALE_64B		= XMCS_GPON_TRTCM_SCALE_64B, 
	ENUM_GPON_TRTCM_SCALE_128B		= XMCS_GPON_TRTCM_SCALE_128B,
	ENUM_GPON_TRTCM_SCALE_256B		= XMCS_GPON_TRTCM_SCALE_256B,
	ENUM_GPON_TRTCM_SCALE_512B		= XMCS_GPON_TRTCM_SCALE_512B,
	ENUM_GPON_TRTCM_SCALE_1K		= XMCS_GPON_TRTCM_SCALE_1K,  
	ENUM_GPON_TRTCM_SCALE_2K		= XMCS_GPON_TRTCM_SCALE_2K,  
	ENUM_GPON_TRTCM_SCALE_4K		= XMCS_GPON_TRTCM_SCALE_4K,  
	ENUM_GPON_TRTCM_SCALE_8K		= XMCS_GPON_TRTCM_SCALE_8K,  
	ENUM_GPON_TRTCM_SCALE_16K		= XMCS_GPON_TRTCM_SCALE_16K, 
	ENUM_GPON_TRTCM_SCALE_32K		= XMCS_GPON_TRTCM_SCALE_32K, 
	ENUM_GPON_TRTCM_SCALE_ITEMS     = XMCS_GPON_TRTCM_SCALE_ITEMS
} MGR_GponTrtcmScale_t ;
*****************************************************************************/
void cmdGponSetDbaBlockSize(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponDbaConfig) gponDbaConfig ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value > 65535) {
		printf("Input Error: block size error.\n") ;
		return ;
	}
	gponDbaConfig.BlockSize = value ;
	
	if(PonApi_GponDbaConfig_Set((MASK_GponBlockSize), &gponDbaConfig) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponDbaConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponDbaConfig)
{
	MGR_U16 				BlockSize ;
	MGR_Enable_t			GponTrtcmMode ;
	MGR_GponTrtcmScale_t	GponTrtcmScale ;
	struct {
		MGR_U8					Channel ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} GponTrtcm[CONFIG_GPON_MAX_TCONT] ;
} ;
MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;
typedef enum {
	ENUM_GPON_TRTCM_SCALE_1B		= XMCS_GPON_TRTCM_SCALE_1B, 
	ENUM_GPON_TRTCM_SCALE_2B		= XMCS_GPON_TRTCM_SCALE_2B,  
	ENUM_GPON_TRTCM_SCALE_4B		= XMCS_GPON_TRTCM_SCALE_4B,  
	ENUM_GPON_TRTCM_SCALE_8B		= XMCS_GPON_TRTCM_SCALE_8B,  
	ENUM_GPON_TRTCM_SCALE_16B		= XMCS_GPON_TRTCM_SCALE_16B, 
	ENUM_GPON_TRTCM_SCALE_32B		= XMCS_GPON_TRTCM_SCALE_32B, 
	ENUM_GPON_TRTCM_SCALE_64B		= XMCS_GPON_TRTCM_SCALE_64B, 
	ENUM_GPON_TRTCM_SCALE_128B		= XMCS_GPON_TRTCM_SCALE_128B,
	ENUM_GPON_TRTCM_SCALE_256B		= XMCS_GPON_TRTCM_SCALE_256B,
	ENUM_GPON_TRTCM_SCALE_512B		= XMCS_GPON_TRTCM_SCALE_512B,
	ENUM_GPON_TRTCM_SCALE_1K		= XMCS_GPON_TRTCM_SCALE_1K,  
	ENUM_GPON_TRTCM_SCALE_2K		= XMCS_GPON_TRTCM_SCALE_2K,  
	ENUM_GPON_TRTCM_SCALE_4K		= XMCS_GPON_TRTCM_SCALE_4K,  
	ENUM_GPON_TRTCM_SCALE_8K		= XMCS_GPON_TRTCM_SCALE_8K,  
	ENUM_GPON_TRTCM_SCALE_16K		= XMCS_GPON_TRTCM_SCALE_16K, 
	ENUM_GPON_TRTCM_SCALE_32K		= XMCS_GPON_TRTCM_SCALE_32K, 
	ENUM_GPON_TRTCM_SCALE_ITEMS     = XMCS_GPON_TRTCM_SCALE_ITEMS
} MGR_GponTrtcmScale_t ;
*****************************************************************************/
void cmdGponSetTrtcmMode(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponDbaConfig) gponDbaConfig ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value == 0) {
		gponDbaConfig.GponTrtcmMode = ENUM_DISABLE ;
	} else if(value == 1) {
		gponDbaConfig.GponTrtcmMode = ENUM_ENABLE ;
	} else {
		printf("Input Error: GPON trtcm mode error.\n") ;
		return ;
	}

	if(PonApi_GponDbaConfig_Set((MASK_GponTrtcmMode), &gponDbaConfig) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponDbaConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponDbaConfig)
{
	MGR_U16 				BlockSize ;
	MGR_Enable_t			GponTrtcmMode ;
	MGR_GponTrtcmScale_t	GponTrtcmScale ;
	struct {
		MGR_U8					Channel ;
		MGR_U16					CIRValue ;
		MGR_U16					CBSUnit ;
		MGR_U16					PIRValue ;
		MGR_U16					PBSUnit ;
	} GponTrtcm[CONFIG_GPON_MAX_TCONT] ;
} ;
MACRO_CMD_MASK(GponDbaConfig)
{
	MASK_GponBlockSize					= (1<<0) ,
	MASK_GponTrtcmMode					= (1<<1) ,
	MASK_GponTrtcmScale					= (1<<2) ,
	MASK_GponTrtcmParams				= (1<<3)
} ;
typedef enum {
	ENUM_GPON_TRTCM_SCALE_1B		= XMCS_GPON_TRTCM_SCALE_1B, 
	ENUM_GPON_TRTCM_SCALE_2B		= XMCS_GPON_TRTCM_SCALE_2B,  
	ENUM_GPON_TRTCM_SCALE_4B		= XMCS_GPON_TRTCM_SCALE_4B,  
	ENUM_GPON_TRTCM_SCALE_8B		= XMCS_GPON_TRTCM_SCALE_8B,  
	ENUM_GPON_TRTCM_SCALE_16B		= XMCS_GPON_TRTCM_SCALE_16B, 
	ENUM_GPON_TRTCM_SCALE_32B		= XMCS_GPON_TRTCM_SCALE_32B, 
	ENUM_GPON_TRTCM_SCALE_64B		= XMCS_GPON_TRTCM_SCALE_64B, 
	ENUM_GPON_TRTCM_SCALE_128B		= XMCS_GPON_TRTCM_SCALE_128B,
	ENUM_GPON_TRTCM_SCALE_256B		= XMCS_GPON_TRTCM_SCALE_256B,
	ENUM_GPON_TRTCM_SCALE_512B		= XMCS_GPON_TRTCM_SCALE_512B,
	ENUM_GPON_TRTCM_SCALE_1K		= XMCS_GPON_TRTCM_SCALE_1K,  
	ENUM_GPON_TRTCM_SCALE_2K		= XMCS_GPON_TRTCM_SCALE_2K,  
	ENUM_GPON_TRTCM_SCALE_4K		= XMCS_GPON_TRTCM_SCALE_4K,  
	ENUM_GPON_TRTCM_SCALE_8K		= XMCS_GPON_TRTCM_SCALE_8K,  
	ENUM_GPON_TRTCM_SCALE_16K		= XMCS_GPON_TRTCM_SCALE_16K, 
	ENUM_GPON_TRTCM_SCALE_32K		= XMCS_GPON_TRTCM_SCALE_32K, 
	ENUM_GPON_TRTCM_SCALE_ITEMS     = XMCS_GPON_TRTCM_SCALE_ITEMS
} MGR_GponTrtcmScale_t ;
*****************************************************************************/
void cmdGponSetTrtcmParameters(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponDbaConfig) gponDbaConfig ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value >= CONST_SHAPING_NUMBER) {
		printf("Input Error: TSID error.\n") ;
		return ;
	}
	gponDbaConfig.GponTrtcm[0].Channel = value ;
	
	value = strtoul(argv[1], NULL, 0) ;
	if(value > 16000) {
		printf("Input Error: CIR unit error.\n") ;
		return ;
	}
	gponDbaConfig.GponTrtcm[0].CIRValue = value ;

	value = strtoul(argv[2], NULL, 0) ;
	if(value > 65535) {
		printf("Input Error: CBS value error.\n") ;
		return ;
	}
	gponDbaConfig.GponTrtcm[0].CBSUnit = value ;

	value = strtoul(argv[3], NULL, 0) ;
	if(value>16000 || value<gponDbaConfig.GponTrtcm[0].CIRValue) {
		printf("Input Error: PIR unit error.\n") ;
		return ;
	}
	gponDbaConfig.GponTrtcm[0].PIRValue = value ;

	value = strtoul(argv[4], NULL, 0) ;
	if(value > 65535) {
		printf("Input Error: PBS value error.\n") ;
		return ;
	}
	gponDbaConfig.GponTrtcm[0].PBSUnit = value ;

	if(PonApi_GponDbaConfig_Set((MASK_GponTrtcmParams), &gponDbaConfig) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponDbaConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;	
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponTodConfig)
{
	MGR_U32		SFCounter ;
	MGR_U32		Second ;
	MGR_U32		Nanosecond ;
} ;
*****************************************************************************/
void cmdGponSetTodCfg(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponTodConfig) gponTodCfg ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	gponTodCfg.SFCounter = value ;
	
	value = strtoul(argv[1], NULL, 0) ;
	gponTodCfg.Second = value ;
	
	value = strtoul(argv[2], NULL, 0) ;
	gponTodCfg.Nanosecond = value ;
	
	if(PonApi_GponTodConfig_Set(0, &gponTodCfg) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponTodConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_U16			GemPortId ;
		MGR_U16			AllocId ;		//just for create command
		MGR_U8			IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t	Loopback ;		//enable/disable loopback
		MGR_Enable_t	Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponSetGemPortAssign(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemPortConfig) gponGemPort ;
	int value ;
	
	value = strtol(argv[0], NULL, 0) ;
	if(value<0 || value>CONST_MAX_GEMPORT_ID) {
		printf("Input Error: GEM port id error.\n") ;
		return ;
	}
	gponGemPort.GemPort[0].GemPortId = value ;
	
	value = strtol(argv[1], NULL, 0) ;
	if(value<0 || value>=CONST_MAX_BRIDGE_PORT) {
		printf("Input Error: MAC bridge port index error.\n") ;
		return ;
	}
	gponGemPort.GemPort[0].IfIndex = value ;
	
	gponGemPort.EntryNum = 1 ;
	if(PonApi_GponGemPortConfig_Set(MASK_AssignMacBridge, &gponGemPort) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponGemPortConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_U16			GemPortId ;
		MGR_U16			AllocId ;		//just for create command
		MGR_U8			IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t	Loopback ;		//enable/disable loopback
		MGR_Enable_t	Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponSetGemPortUnassign(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemPortConfig) gponGemPort ;
	int value ;
	
	value = strtol(argv[0], NULL, 0) ;
	if(value<0 || value>CONST_MAX_GEMPORT_ID) {
		printf("Input Error: GEM port id error.\n") ;
		return ;
	}
	gponGemPort.GemPort[0].GemPortId = value ;
		
	gponGemPort.EntryNum = 1 ;
	if(PonApi_GponGemPortConfig_Set(MASK_UnassignMacBridge, &gponGemPort) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponGemPortConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}

/*****************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_U16			GemPortId ;
		MGR_U16			AllocId ;		//just for create command
		MGR_U8			IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t	Loopback ;		//enable/disable loopback
		MGR_Enable_t	Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponSetGemPortLoopback(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemPortConfig) gponGemPort ;
	int value ;
	
	value = strtol(argv[0], NULL, 0) ;
	if(value<0 || value>CONST_MAX_GEMPORT_ID) {
		printf("Input Error: GEM port id error.\n") ;
		return ;
	}
	gponGemPort.GemPort[0].GemPortId = value ;
	
	value = strtol(argv[1], NULL, 0) ;
	if(value == 0) {
		gponGemPort.GemPort[0].Loopback = ENUM_DISABLE ;
	} else if(value == 1) {
		gponGemPort.GemPort[0].Loopback = ENUM_ENABLE ;
	} else {
		printf("Input Error: Loopback mode error.\n") ;
		return ;
	}
	
	gponGemPort.EntryNum = 1 ;
	if(PonApi_GponGemPortConfig_Set(MASK_Loopback, &gponGemPort) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponGemPortConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}
/*******************************************************************************************
*******************************************************************************************/
/*****************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_GponGemType_t	GemType ;
		MGR_U16				GemPortId ;
		MGR_U16				AllocId ;		//just for create command
		MGR_U8				IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t		Loopback ;		//enable/disable loopback
		MGR_Enable_t		Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponCreateGemPort(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemPortConfig) gponGemPort ;
	int value ;
	
	if(!strcmp(argv[0], "multicast")) {
		gponGemPort.GemPort[0].GemType = ENUM_GPON_GEM_TYPE_MULTICAST ;
		
		value = strtol(argv[1], NULL, 0) ;
		if(value<0 || value>CONST_MAX_GEMPORT_ID) {
			printf("Input Error: GEM port id error.\n") ;
			return ;
		}
		gponGemPort.GemPort[0].GemPortId = value ;
	} else if(!strcmp(argv[0], "unicast")) {
		gponGemPort.GemPort[0].GemType = ENUM_GPON_GEM_TYPE_UNICAST ;
		
		if(argc != 3) {
			printf("Input Error: Input arguments error.\n") ;
			return ;
		}
			
		value = strtol(argv[1], NULL, 0) ;
		if(value<0 || value>CONST_MAX_GEMPORT_ID) {
			printf("Input Error: GEM port id error.\n") ;
			return ;
		}
		gponGemPort.GemPort[0].GemPortId = value ;

		value = strtol(argv[2], NULL, 0) ;
		if(value<0 || value>CONST_MAX_TCONT_ID) {
			printf("Input Error: Alloc id error.\n") ;
			return ;
		}
		gponGemPort.GemPort[0].AllocId = value ;
	} else {
		printf("Input Error: GEM port type error.\n") ;
		return ;
	}

	gponGemPort.EntryNum = 1 ;
	if(PonApi_GponGemPortConfig_Create(&gponGemPort) != EXEC_OK) {
		printf("Exec. Failed: Create the 'GponGemPortConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}
/*******************************************************************************************
*******************************************************************************************/
/*****************************************************************************
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_U16			GemPortId ;
		MGR_U16			AllocId ;		//just for create command
		MGR_U8			IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t	Loopback ;		//enable/disable loopback
		MGR_Enable_t	Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;
*****************************************************************************/
void cmdGponDeleteGemPort(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemPortConfig) gponGemPort ;
	int value ;
	
	value = strtol(argv[0], NULL, 0) ;
	if(value<0 || value>CONST_MAX_GEMPORT_ID) {
		printf("Input Error: GEM port id error.\n") ;
		return ;
	}
	gponGemPort.GemPort[0].GemPortId = value ;

	gponGemPort.EntryNum = 1 ;
	if(PonApi_GponGemPortConfig_Delete(&gponGemPort) != EXEC_OK) {
		printf("Exec. Failed: Delete the 'GponGemPortConfig' command failed\n\n") ;
		return ;
	}
	
	printf("Exec. Successful\n") ;
}
/*******************************************************************************************
*******************************************************************************************/
/***********************************************************************************
MACRO_CMD_STRUCT(GponGemCounter)
{
	MGR_U16 			GemPortId ;
	MGR_U32				RxGemFrameH ;
	MGR_U32				RxGemFrameL ;
	MGR_U32				RxGemPayloadH ;
	MGR_U32				RxGemPayloadL ;
	MGR_U32				TxGemFrameH ;
	MGR_U32				TxGemFrameL ;	
	MGR_U32				TxGemPayloadH ;	
	MGR_U32				TxGemPayloadL ;		
} ;
***********************************************************************************/
void cmdGponGetGemCounter(const int argc, char **const argv)
{
	MACRO_CMD_STRUCT(GponGemCounter) gponGemCounter ;
	unsigned int value ;
	
	value = strtoul(argv[0], NULL, 0) ;
	if(value >= 4096) {
		printf("Input Error: gem port id error.\n") ;
		return ;
	}
	memset(&gponGemCounter, 0, sizeof(gponGemCounter)) ;
	gponGemCounter.GemPortId = value ;
	if(PonApi_GponGemCounter_Get(&gponGemCounter) != EXEC_OK) {
		printf("Exec. Failed: Set the 'GponGemCounter' command failed\n\n") ;
		return ;
	}
	
	printf("Gem Port:%d\n", gponGemCounter.GemPortId);
	printf("GemFrame Rx High:%u, GemFrame Rx Low:%u\n", gponGemCounter.RxGemFrameH, gponGemCounter.RxGemFrameL) ;
	printf("GemPayload Rx High:%u, GemPayload Rx Low:%u\n", gponGemCounter.RxGemPayloadH, gponGemCounter.RxGemPayloadL) ;
	printf("GemFrame Tx High:%u, GemFrame Tx Low:%u\n", gponGemCounter.TxGemFrameH, gponGemCounter.TxGemFrameL) ;
	printf("GemPayload Tx High:%u, GemPayload Tx Low:%u\n", gponGemCounter.TxGemPayloadH, gponGemCounter.TxGemPayloadL) ;
}

/********************************************************************************************
********************************************************************************************/
void ponmgrProcessGponCommand(int argc, char** argv) 
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

	for(i=0 ; i<gGponCmdNum ; i++) {
		if((gGponCmd[i].flags&flag) && !strcmp(argv[3], gGponCmd[i].name)) {
			if(argc>=(gGponCmd[i].minArgs+4) && argc<=(gGponCmd[i].maxArgs+4)) {
				gGponCmd[i].proc(argc-4, &argv[4]) ;
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

