#ifndef _CMGR_GPON_STRUCT_H_
#define _CMGR_GPON_STRUCT_H_

#include "core/cmgr/cmgr_gpon_enum.h"
#include "core/cmgr/cmgr_gpon_const.h"

MACRO_CMD_STRUCT(GponSnPasswd)
{
	MGR_U8 		Serial[CONST_GPON_SN_LENS] ;
	MGR_U8 		Password[CONST_GPON_PASSWD_LENS] ;
    MGR_U8      EmergencyState;
	MGR_U8		PasswdLength;    /*for hex format password's length*/
	MGR_U8        hexFlag;
} ;


MACRO_CMD_STRUCT(GponActTimer)
{
	MGR_U32 	TO1Timer ;  	/* millisecond */
	MGR_U32		TO2Timer ;		/* millisecond */
} ;


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
	MGR_U32 			TO1Timer ;  	/* millisecond */
	MGR_U32				TO2Timer ;		/* millisecond */
	MGR_U16				OMCIChannel ;
} ;

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

MACRO_CMD_STRUCT(GponTodConfig)
{
	MGR_U32		SFCounter ;
	MGR_U32		Second ;
	MGR_U32		Nanosecond ;
} ;

/**********************************************************************
**********************************************************************/
MACRO_CMD_STRUCT(GponGemPortConfig)
{
	struct {
		MGR_GponGemType_t	GemType ;
		MGR_U16				GemPortId ;
		MGR_U16				AllocId ;		//just for create command
		MGR_U16				IfIndex ;		//assign/unassign MAC bridge port
		MGR_Enable_t		Loopback ;		//enable/disable loopback
		MGR_Enable_t		Encryption ;	//just for get command
	} GemPort[CONFIG_GPON_MAX_GEMPORT] ;
	MGR_U16		EntryNum ;
} ;

MACRO_CMD_STRUCT(GponTcontStatus)
{
	struct {
		MGR_U16			AllocId ;
		MGR_U8			Channel ;
	} Tcont[CONFIG_GPON_MAX_TCONT] ;
	MGR_U16		EntryNum ;
} ;

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

#endif /* _CMGR_GPON_STRUCT_H_ */
