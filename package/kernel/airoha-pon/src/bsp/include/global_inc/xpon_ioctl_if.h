#ifndef _XPON_IOCTL_IF_H
#define _XPON_IOCTL_IF_H  

/**
* \file  xpon_ioctl_if.h
* \brief This file is xpon ioctl header file that will be exported for others to use.
* \author jun.wu
* \date     2020-09-22
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

/************************************************************************
*		   I N C L U D E S
*************************************************************************
*/
#include "xpon_const.h"
#include "xpon_event_global.h"


/************************************************************************
*		   D E F I N E S	&	C O N S T A N T S
*************************************************************************
*/

/************************************************************************
* 		   M A C R O S
*************************************************************************
*/

/************************************************************************
*		   D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*		   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/******************************************************************************************************
*                               MACRO DATA
*******************************************************************************************************/
typedef enum {
    GPON_STATE_O1 = 1, 
    GPON_STATE_O2, 
    GPON_STATE_O3, 
    GPON_STATE_O4, 
    GPON_STATE_O5, 
    GPON_STATE_O6, 
    GPON_STATE_O7 
} ENUM_GponState_t;

typedef enum {
    GPON_10G_STATE_O1 = 1, 
    GPON_10G_STATE_O2_3, 
    GPON_10G_STATE_O4 = 4, 
    GPON_10G_STATE_O5, 
    GPON_10G_STATE_O6, 
    GPON_10G_STATE_O7,
    GPON_10G_STATE_O8, 
    GPON_10G_STATE_O9,
    GPON_10G_STATE_O5_2 = 10,
} ENUM_Gpon_10G_State_t;

typedef enum {
	GPON_SW = 0,
	GPON_HW,
	GPON_SW_HW,
} GPON_SW_HW_SELECT_T, GPON_SW_HW_SELECT_t ;

typedef enum {
	EPON_RX_FORWARDING = 0,
	EPON_RX_DISCARD,
	EPON_RX_LOOPBACK
} EPON_RxMode_t ;

typedef enum {
	EPON_TX_FORWARDING = 0,
	EPON_TX_DISCARD,
} EPON_TxMode_t ;

typedef enum {
	XMCS_IF_WEIGHT_TYPE_PACKET = 0,
	XMCS_IF_WEIGHT_TYPE_BYTE
} XMCSIF_QoSWeightType_t ;

typedef enum {
	XMCS_IF_WEIGHT_SCALE_64B = 0,
	XMCS_IF_WEIGHT_SCALE_16B,
	XMCS_IF_WEIGHT_SCALE_1B
} XMCSIF_QoSWeightScale_t ;


typedef enum {
	XGPON_SW = 0,
	XGPON_HW,
} XGPON_SW_HW_SELECT_T ;

typedef enum {
    GPON_DBA_BACKDOOR_NOT_MODIFY = 0,
    GPON_DBA_BACKDOOR_FIX_MODE ,
    GPON_DBA_BACKDOOR_ADD_MODE ,
    GPON_DBA_BACKDOOR_SHIFT_MODE ,
} GPON_10G_DEV_DBA_MODIFY_MODE_T ;

typedef enum {
    GPON_DBA_BACKDOOR_ADD = 0,
    GPON_DBA_BACKDOOR_MINUS ,
} GPON_10G_DEV_DBA_ADD_MODE_T ;

typedef enum {
    GPON_DBA_BACKDOOR_LEFT_SHIFT = 0,
    GPON_DBA_BACKDOOR_RIGHT_SHIFT ,
} GPON_10G_DEV_DBA_SHIFT_MODE_T ;

#if defined(TCSUPPORT_CPU_AN7583)
typedef enum {
    GPON_DBA_BACKDOOR_FOR_TCONT_IDX0 = 0,
    GPON_DBA_BACKDOOR_FOR_TCONT_IDX1 ,
} GPON_10G_DEV_DBA_TCONT_SELECT_T ;
#endif

typedef enum {
    GPON_10G_COUNTER_TYPE_GEM = 0,
    GPON_10G_COUNTER_TYPE_ETHERNET,
} GPON_10G_COUNTER_TYPE_t ;

typedef enum {
	GPON_10G_USE_DEFAULT_SET_AS_SEPC = 0,
	GPON_10G_USE_OCBODY_FIELD,
	GPON_10G_DS_FEC_FORCE_OFF,
	GPON_10G_DS_FEC_FORCE_ON
}GPON_10G_DEV_DS_FEC_MODE_T;

typedef enum {
    GPON_10G_COUNTER_TYPE_ALL = 0,
    GPON_10G_COUNTER_TYPE_SPECIFIC,
} GPON_10G_CLEAR_COUNTER_TYPE_t ;

typedef enum {
    GPON_10G_SNIFFER_MODE_LAN0 = 0x0001,
    GPON_10G_SNIFFER_MODE_LAN1 = 0x0002,
    GPON_10G_SNIFFER_MODE_LAN2 = 0x0004,
    GPON_10G_SNIFFER_MODE_LAN3 = 0x0008,
} GPON_10G_DEV_SNIFFER_MODE_LAN_PORT_T ;

typedef enum {
	XGMCS_IF_PHY_LOSS = 0,
	XGMCS_IF_PHY_READY, 
} XGMCSIF_PhyMode_t;

typedef enum {
	XMCS_IF_QOS_TYPE_WRR = 0,
	XMCS_IF_QOS_TYPE_SP,
	XMCS_IF_QOS_TYPE_SPWRR7, 
	XMCS_IF_QOS_TYPE_SPWRR6, 
	XMCS_IF_QOS_TYPE_SPWRR5, 
	XMCS_IF_QOS_TYPE_SPWRR4, 
	XMCS_IF_QOS_TYPE_SPWRR3, 
	XMCS_IF_QOS_TYPE_SPWRR2, 
} XMCSIF_QosType_t ;

typedef enum {
	XMCS_IF_CONGESTIOM_SCALE_2 = 0,
	XMCS_IF_CONGESTIOM_SCALE_4,
	XMCS_IF_CONGESTIOM_SCALE_8,
	XMCS_IF_CONGESTIOM_SCALE_16,
} XMCSIF_CongestionScale_t ;

typedef enum {
	XMCS_IF_TRTCM_SCALE_1B = 0,
	XMCS_IF_TRTCM_SCALE_2B,
	XMCS_IF_TRTCM_SCALE_4B, 
	XMCS_IF_TRTCM_SCALE_8B,
	XMCS_IF_TRTCM_SCALE_16B,
	XMCS_IF_TRTCM_SCALE_32B,
	XMCS_IF_TRTCM_SCALE_64B,
	XMCS_IF_TRTCM_SCALE_128B,
	XMCS_IF_TRTCM_SCALE_256B,
	XMCS_IF_TRTCM_SCALE_512B,
	XMCS_IF_TRTCM_SCALE_1K,
	XMCS_IF_TRTCM_SCALE_2K,
	XMCS_IF_TRTCM_SCALE_4K,
	XMCS_IF_TRTCM_SCALE_8K,
	XMCS_IF_TRTCM_SCALE_16K,
	XMCS_IF_TRTCM_SCALE_32K,
	XMCS_IF_TRTCM_SCALE_ITEMS
} XMCSIF_TrtcmScale_t ;

typedef enum {
	XMCS_IF_ONU_TYPE_UNKNOWN = 0,
	XMCS_IF_ONU_TYPE_SFU,
	XMCS_IF_ONU_TYPE_HGU
} XMCSIF_OnuType_t;

typedef enum {
    XMCS_GPON_TRTCM_SCALE_1B = 0,
    XMCS_GPON_TRTCM_SCALE_2B,
    XMCS_GPON_TRTCM_SCALE_4B, 
    XMCS_GPON_TRTCM_SCALE_8B,
    XMCS_GPON_TRTCM_SCALE_16B,
    XMCS_GPON_TRTCM_SCALE_32B,
    XMCS_GPON_TRTCM_SCALE_64B,
    XMCS_GPON_TRTCM_SCALE_128B,
    XMCS_GPON_TRTCM_SCALE_256B,
    XMCS_GPON_TRTCM_SCALE_512B,
    XMCS_GPON_TRTCM_SCALE_1K,
    XMCS_GPON_TRTCM_SCALE_2K,
    XMCS_GPON_TRTCM_SCALE_4K,
    XMCS_GPON_TRTCM_SCALE_8K,
    XMCS_GPON_TRTCM_SCALE_16K,
    XMCS_GPON_TRTCM_SCALE_32K,
    XMCS_GPON_TRTCM_SCALE_ITEMS
} XMCSGPON_TrtcmScale_t;


typedef enum {
    GPON_COUNTER_TYPE_GEM = 0,
    GPON_COUNTER_TYPE_ETHERNET,
} GPON_COUNTER_TYPE_t ;

typedef enum {
    GPON_BURST_MODE_OVERHEAD_LEN_DEFAULT = 0,
    GPON_BURST_MODE_OVERHEAD_LEN_MORE_THAN_128,
} GPON_BURST_MODE_OVERHEAD_LEN_T ;

typedef enum {
    Doze = 0,
    Sleep,
    WSleep,
} GPON_PLOAMu_SLEEP_MODE_t ;

typedef enum {
    SNIFFER_MODE_LAN0 = 0x8001,
    SNIFFER_MODE_LAN1 = 0x8002,
    SNIFFER_MODE_LAN2 = 0x8004,
    SNIFFER_MODE_LAN3 = 0x8008,
} GPON_DEV_SNIFFER_MODE_LAN_PORT_T ;


typedef enum {
	SEND_PLOAMU_BEFORE = 0,
	SEND_PLOAMU_AFTER,
} GPON_DEV_SEND_PLOAMU_WAIT_MODE_T ;


typedef enum{
	SC_PLOAM_POWER_LEVEL_NORMAL,
	SC_PLOAM_POWER_LEVEL_MIN_3DB,
	SC_PLOAM_POWER_LEVEL_MIN_6DB,
	SC_PLOAM_POWER_LEVEL_OTHER
}PLOAM_POWER_LEVEL_T;

typedef enum {
    WAN_MODE_XGPON = 0,
    WAN_MODE_XGSPON,
    WAN_MODE_NGPON2,
} GPON_10G_DEV_WAN_MODE_T ;

typedef enum {
    NO_ENCRYPTION = 0,
    GPON_UNICAST_ENCRYPTION,
    GPON_BROADCAST_ENCRYPTION,
	GPON_UNICAST_ENCRYPTION_ONLY_DOWN,	
} GPON_GemEncryption_t;

typedef enum {
	XMCS_IF_PCP_TYPE_CDM_TX = 0,
	XMCS_IF_PCP_TYPE_CDM_RX, 
	XMCS_IF_PCP_TYPE_GDM_RX
} XMCSIF_PcpType_t ;

typedef enum {
	XMCS_IF_PCP_MODE_DISABLE = 0,
	XMCS_IF_PCP_MODE_8B0D,
	XMCS_IF_PCP_MODE_7B1D, 
	XMCS_IF_PCP_MODE_6B2D,
	XMCS_IF_PCP_MODE_5B3D
} XMCSIF_PcpMode_t ;


typedef enum {
	XMCS_IF_CLEAR_TYPE_XPON = 0,
} XMCSIF_ClearType_t;

typedef enum {
	XMCS_PHY_BURST_MODE = 0,
	XMCS_PHY_CONTINUOUS_MODE
} XMCSPHY_TxBurstMode_t ;

typedef enum {
	MSG_ERR		= 0x0001,
	MSG_WARN	= 0x0002,
	MSG_INT		= 0x0004,
	MSG_ACT		= 0x0008,
	MSG_OAM		= 0x0010,
	MSG_OMCI	= 0x0020,
	MSG_TRACE	= 0x0040,
	MSG_CONTENT	= 0x0080,
	MSG_DBG		= 0x0100,
	MSG_EQD		= 0x0200,
	MSG_XMCS	= 0x0400,
	MSG_SECUR	= 0x0800,
	MSG_TYPEB   = 0x1000,
} xPON_DebugMsg_t ;

typedef enum {
    PHY_LOS_STATUS = 0,
	PHY_READY_STATUS,
} PHY_STATUS_t ;

/******************************************************************************************************
*                               STRUCT DATA
*******************************************************************************************************/

typedef struct XMCS_GponOnuInfo_S {
    uint8_t  onuId ;
    uint8_t  state ;
    uint8_t  sn[GPON_SN_LENS] ;
    uint8_t  PasswdLength;
	uint8_t  RegidLength;
    uint8_t  hexFlag;
    uint8_t  passwd[GPON_PASSWD_LENS] ;
	uint8_t  regid[GPON_REG_ID_LENS] ;
    uint8_t  keyIdx ;
    uint8_t  key[GPON_ENCRYPT_KEY_LENS] ;
    uint32_t    actTo1Timer ;
    uint32_t    actTo2Timer ;
    uint16_t  omcc ;
    uint8_t  EmergencyState ;
} GPON_ONU_INFO_t;

typedef struct XMCS_GponGemCounter_S {
    uint64_t rxGemFrame ;
    uint64_t rxGemPayload ;
    uint64_t txGemFrame ;
    uint64_t txGemPayload ;
} GEM_STATISTIC_t;

typedef struct XMCS_CounterCfg_S {
    uint8_t    tcontId;
    uint16_t   allocId;
    uint16_t   gemPortId;
    GEM_STATISTIC_t sta;
} STATISTIC_CFG_t;

typedef struct XMCS_GponTodCfg_S {
    uint32_t    superframe ;
	uint32_t    sec ;
    uint32_t    nanosec ;
	uint16_t    sec_H16 ;
}XMCS_GponTodCfg_t;

typedef struct{
	uint8_t numGuardBits;
	uint8_t numType1PreambleBits;
	uint8_t numType2PreambleBits;
	uint8_t numType3PreambleBits;
	uint8_t type3PreamblePattern;
	uint8_t delimiterData[SC_PLOAM_NUM_DELIMITER_BYTES];
	PLOAM_POWER_LEVEL_T powerLevel;
	uint32_t eqd;
}PLOAM_GTC_INFO_T;

typedef struct{
    uint16_t tcont;
	uint16_t alloc_id;
	uint32_t tx_counter;
}TCONT_COUNTERS_T;

typedef  struct{
	TCONT_COUNTERS_T counters[GPON_TCONT_MAX_NUM];
	uint8_t tcont_num;
}ALL_TCONT_COUNTERS_T;


/**************************************************
*    10G STRUCT DATA START
****************************************************/
typedef struct {
   	GPON_10G_DEV_DBA_MODIFY_MODE_T mode;
    GPON_10G_DEV_DBA_ADD_MODE_T    addMode;
    GPON_10G_DEV_DBA_SHIFT_MODE_T  shiftMode;
    unsigned int                   value;
} GPON_10G_DEV_DBA_BACKDOOR_T;

#if defined(TCSUPPORT_CPU_AN7583)
typedef struct {
   	GPON_10G_DEV_DBA_MODIFY_MODE_T mode;
    GPON_10G_DEV_DBA_ADD_MODE_T    addMode;
    GPON_10G_DEV_DBA_SHIFT_MODE_T  shiftMode;
    unsigned int                   value;
	GPON_10G_DEV_DBA_TCONT_SELECT_T tcontSelect;
	unsigned int                   tcontId;
	XPON_Mode_t                    dba_backdoor_enable;
} GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T;
#endif

typedef struct {
   	XGPON_SW_HW_SELECT_T     usOmciMicMode;
    XGPON_SW_HW_SELECT_T     dsOmciMicMode;
} GPON_10G_DEV_OMCI_MIC_CTRL_T;

typedef struct {
   	GPON_10G_CLEAR_COUNTER_TYPE_t  clearType;
    unsigned int                   gemportId;
} GPON_10G_DEV_COUNTER_CLEAR_T;


struct XGMCS_EqdOffset_S {
    unsigned char O4 ;
    unsigned char O5 ;
    unsigned char eqdOffsetFlag;
} ;

typedef struct {
    GPON_10G_DEV_SNIFFER_MODE_LAN_PORT_T lan_port;
    uint16_t          tx_da;
    uint16_t          tx_sa;
    uint16_t          rx_da;
    uint16_t          rx_sa;
    uint16_t          ethertype;
    uint16_t          tpid;
    uint16_t          gemPortId;
	XPON_Mode_t     ds_enable;
    XPON_Mode_t     us_enable;
} GPON_10G_DEV_SNIFFER_MODE_T;

typedef struct {
    XGPON_SW_HW_SELECT_T dyingGaspCtrl;
	unsigned int        dyingGaspNum;
    unsigned char        hwDGStatus;
} GPON_10G_DYING_GASP_MODE_T;


typedef struct {
   	XPON_Mode_t     ploamd_filter; /*exclude profile ploam*/
    XPON_Mode_t     profile_filter;
} GPON_10G_DEV_PLOAMD_FILTER_MODE_T;

typedef struct XGMCS_XgponOnuInfo_S {
    uint8_t  onuId ;
    uint8_t  state ;
    uint8_t  sn[GPON_SN_LENS] ;
    uint32_t    actTo1Timer ;
    uint32_t    actTo2Timer ;
    uint16_t  omcc ;
    uint8_t  EmergencyState ;
    uint16_t onuRespTime;
    uint8_t  regid[GPON_REG_ID_LENS] ;
} XGPON_ONU_INFO_t;

typedef struct {
   	uint32_t PSBdHECErrCount ;
    uint32_t XGTCHECErrCount ;
    uint32_t UnknownProfCount ;
    unsigned long long TransmitXGEMFrames ;
    unsigned long long FragmentXGEMFrames ;
    uint32_t XGEMHECLostWordCount ;
    uint32_t XGEMKeyErrors ;  
    uint32_t XGEMHECErrCount ;    
    unsigned long long TransmitByteNoidleXGEMFrames ;
	unsigned long long ReceiveByteNoidleXGEMFrames;
    uint32_t LODSEventCount ;    
    uint32_t LODSEventRestoreCount ;
	uint32_t ONUReactivLODSEvents ;
} GPON_10G_TC_COUNTER_T;
/**************************************************
*    10G STRUCT DATA END
****************************************************/

/**************************************************
*    XPON PHY  STRUCT DATA START
****************************************************/
typedef struct XMCS_PhyTxRxFecStatus_S{
	uint32_t rx_status;
	uint32_t tx_status;
} PHY_FECSTATUS_t;
/**************************************************
*    XPON PHY  STRUCT DATA END
****************************************************/
typedef struct XMCS_EponRxConfig_S {
	uint8_t 			idx ;
	EPON_RxMode_t	rxMode ;
}EponRxConfig_t ;

typedef struct XMCS_EponTxConfig_S {
	uint8_t			idx ;
	EPON_TxMode_t	txMode ;
}EponTxConfig_t;

struct XMCS_QoSWeightConfig_S {
	XMCSIF_QoSWeightType_t		weightType ;
	XMCSIF_QoSWeightScale_t		weightScale ;
};


typedef struct XMCS_ChannelQoS_S {
	uint8_t						channel ;
	XMCSIF_QosType_t			qosType ;
	struct {
		uint8_t					weight ;
	} queue[XPON_QUEUE_NUMBER];
}XMCS_ChannelQoS_t;

struct XMCS_TxQueueCongestion_S {
	struct {
		XPON_Mode_t				trtcm ;
		XPON_Mode_t				dei ;
		XPON_Mode_t				threshold ;
	} congestMode ;
	struct {
		XMCSIF_CongestionScale_t	max ;
		XMCSIF_CongestionScale_t	min ;
	} scale ;
	struct {
		uint8_t					green ;
		uint8_t					yellow ;
	} dropProbability ;
	struct {
		uint8_t					queueIdx ;
		uint8_t					greenMax ;
		uint8_t					greenMin ;
		uint8_t					yellowMax ;
		uint8_t					yellowMin ;
	} queueThreshold[XPON_QUEUE_NUMBER] ;
};

struct XMCS_TxTrtcmScale_S {
	XMCSIF_TrtcmScale_t			trtcmScale ;
};

struct XMCS_TxQueueTrtcm_S {
	uint8_t						tsIdx ;
	uint16_t						cirValue ;
	uint16_t						cbsUnit ;
	uint16_t						pirValue ;
	uint16_t						pbsUnit ;
};

typedef struct {
	unsigned char mac[6];
} XMCSIF_OnuMac_t;


typedef struct{
    uint32_t report_init_O1:1;
    uint32_t reserve:31;
}Event_ctrlFlag_t;

struct XMCS_GponTrtcmConfig_S {
    XPON_Mode_t             trtcmMode ;
    XMCSGPON_TrtcmScale_t   trtcmScale ;
} ;

/* struct definition for GPON common command */
struct XMCS_GponSnPasswd_S {
    uint8_t  sn[GPON_SN_LENS] ;
    uint8_t  passwd[GPON_PASSWD_LENS] ;
	uint8_t  regid[GPON_REG_ID_LENS] ;
    uint8_t  EmergencyState  ;
    uint8_t  PasswdLength;
	uint8_t  RegidLength;
    uint8_t  hexFlag;
} ;

struct XMCS_GponActTimer_S {
    uint32_t    to1Timer ;
    uint32_t    to2Timer ;
} ;

struct XMCS_GponTcontInfo_S{
	uint16_t allocId;
	uint8_t tcontIdx;
};

struct XMCS_GponTrtcmParams_S {
    uint8_t   channel ;
    uint16_t  cirValue ;
    uint16_t  cbsUnit ;
    uint16_t  pirValue ;
    uint16_t  pbsUnit ;
} ;

struct XMCS_EqdOffset_S {
    unsigned char O4 ;
    unsigned char O5 ;
    unsigned char eqdOffsetFlag;
} ;
struct XMCS_GponGetCounter_S {
    uint32_t gponGetCounter_table[16];
};

typedef struct {
    uint32_t aesSpf;
    uint32_t activeKey[4];
    uint32_t shadowKey[4];
} GPON_DEV_ENCRYPT_KEY_INFO_T;


typedef struct {
    XPON_Mode_t     omci;
    XPON_Mode_t     data;
} GPON_DEV_UP_TRAFFIC_T;

typedef struct {
    GPON_DEV_SNIFFER_MODE_LAN_PORT_T lan_port;
    uint16_t          tx_da;
    uint16_t          tx_sa;
    uint16_t          tx_ethertype;
    uint16_t          tx_vid;
    uint16_t          tx_tpid;
    uint16_t          rx_da;
    uint16_t          rx_sa;
    uint16_t          rx_ethertype;
    uint16_t          rx_vid;
    uint16_t          rx_tpid;
    XPON_Mode_t     packet_padding;
    XPON_Mode_t     enable;
    XPON_Mode_t     ds_filter_enable;
	XPON_Mode_t     ds_enable;
    XPON_Mode_t     us_enable;
} GPON_DEV_SNIFFER_MODE_T;

typedef struct {
    uint32_t            dba_backdoor_total_buf;
    uint32_t            dba_backdoor_green_buf;
    uint32_t            dba_backdoor_yellow_buf;
    XPON_Mode_t     enable;
} GPON_DEV_DBA_BACKDOOR_T;

typedef struct {
    uint16_t          dba_slight_modify_total_buf;
    uint16_t          dba_slight_modify_green_buf;
    uint16_t          dba_slight_modify_yellow_buf;
    XPON_Mode_t     enable;
} GPON_DEV_SLIGHT_MODIFY_T;

struct XMCS_DBAShiftMod_S {
    uint32_t   size ;
	uint32_t   direction ;
} ;

typedef struct GPON_DEV_SD_SF_THLD_S{
    uint8_t          sd; //x
    uint8_t          sf; //y
} GPON_DEV_SD_SF_THLD_T;

typedef struct GPON_DEV_SD_SF_CNT_S{
    uint32_t          sd;
    uint32_t          sf;
    uint8_t           cl_en;
} GPON_DEV_SD_SF_CNT_T;

struct XMCS_OMCI_BROADCAST_KEY_S{
	uint8_t			row_ctl;
	uint8_t			key_index;
	uint8_t			fragment[16];
};

struct XMCS_XgponSn_S {
    uint8_t  sn[GPON_SN_LENS] ;
    uint8_t  EmergencyState  ;
} ;

struct XMCS_XgponActTimer_S {
    uint32_t    to1Timer ;
    uint32_t    to2Timer ;
    uint32_t    to3Timer ;
    uint32_t    to4Timer ;
    uint32_t    to5Timer ;
    //uint32_t    to6Timer ;
    uint32_t    toZTimer ;
} ;

struct XGMCS_XgponMsk_S {
    uint8_t  msk[GPON_MSK_LENS];
} ;

struct XGMCS_XgponBroadcast_Key_S {
	uint8_t  keyIndex;
	uint8_t  keyFragmentNum;
	uint8_t  keyFragmentIndex;
    uint8_t  key[16];
} ;

struct XMCS_EponLlidInfo_S {
	struct {
		uint8_t 				idx ;
		uint16_t 				llid ;
		uint8_t				channel ;
		EPON_RxMode_t		rxMode ;
		EPON_TxMode_t		txMode ;
	} info[EPON_LLID_MAX_NUM] ;
	uint16_t entryNum ; 
} ;

struct XMCS_DebugLevel_S {
	xPON_DebugMsg_t mask;
	XPON_Mode_t enable;
};

struct XMCS_TxPCPConfig_S {
	XMCSIF_PcpType_t			pcpType ;
	XMCSIF_PcpMode_t			pcpMode ;
};

struct XMCS_ClearConfig_S  {
	XMCSIF_ClearType_t			clearType ;
};


struct XMCS_StormCtrlConfig_S {
	uint32_t				mask;
	uint32_t 				threld ;
	uint32_t 				timer ;
};


/* struct definition for GPON */
struct XMCS_GemPortAssign_S {
	struct {
		uint16_t id ;
		uint16_t ani ;
	} gemPort[GPON_GEMPORT_MAX_NUM] ;
	uint16_t entryNum ;
};

typedef struct XMCS_GemPortCreate_S {
	GPON_GemType_t		gemType ;
	uint16_t 				gemPortId ;
	uint16_t 				allocId ;
    GPON_GemEncryption_t gemEncrypt;
}XMCS_GemPortCreate_t;

struct XMCS_GemPortLoopback_S {
	uint16_t 		gemPortId ;
	XPON_Mode_t	loopback ;
};

typedef struct XMCS_TcontInfo_S {
	struct {
		uint16_t 			allocId ;
		uint8_t 			channel ;
	} info[GPON_TCONT_MAX_NUM] ;
	uint16_t entryNum ; 
}XMCS_TcontInfo_t ;

struct XMCS_TcontCfg_S {
	uint16_t 			allocId ;	
	uint8_t 			tcontId ;
	uint8_t 			tcontPolicy ; 
} ;

struct XMCS_OMCC_Info_S {
    uint16_t  allocId;
    uint32_t    gemportid;
} ;

struct XMCS_TcontTrtcmScale {
	XMCSIF_TrtcmScale_t			trtcmScale ;
} ;

struct XMCS_TcontTrtcm_S {
	struct {
		uint8_t					channel ;
		uint32_t					cirValue ;
		uint32_t					cbsUnit ;
		uint32_t					pirValue ;
		uint32_t					pbsUnit ;
	} trtcmParam[GPON_TCONT_MAX_NUM] ;
	uint8_t entryNum ;
} ;

struct XMCS_PhyFrameCount_S {
	int		frameCntReset ;
	struct {
		uint32_t		low ;
		uint32_t		high ;
		uint32_t		lof ;
	} frameCounter ;
} ;

struct XMCS_PhyTransSetting_S {
	XPON_Mode_t		txSdInverse ;
	XPON_Mode_t		txFaultInverse ;
	XPON_Mode_t		txBurstEnInverse ;
	XPON_Mode_t		rxSdInverse ;
} ;

struct XMCS_PhyTxBurstCfg_S {
	XMCSPHY_TxBurstMode_t	burstMode ;
} ;
		
struct XMCS_PHY_I2cCtrl_S
{
	uint8_t  u1CHannelID;
	uint16_t u2ClkDiv;
	uint8_t  u1DevAddr;
	uint8_t  u1WordAddrNum;
	uint32_t u4WordAddr;
	uint8_t  * pu1Buf;
	uint16_t u2ByteCnt;
};

typedef struct GPON_Alarm_S{
#ifdef __LITTLE_ENDIAN
	uint32_t				rdi_alarm	:	1;
	uint32_t				pee_alarm	:	1;
	uint32_t				los_alarm	:	1;
	uint32_t				lof_alarm	:	1;
	uint32_t				dis_alarm	:	1;
	uint32_t				dact_alarm	:	1;
	uint32_t				mis_alarm	:	1;
	uint32_t				mem_alarm	:	1;
	uint32_t				suf_alarm	:	1;
	uint32_t				sf_alarm	:	1;
	uint32_t				sd_alarm	:	1;
	uint32_t				lcdg_alarm	:	1;	
	uint32_t				tf_alarm	:	1;
	uint32_t				rogue_alarm	:	1;
	uint32_t				lods_alarm	:	1;
#else
	uint32_t				lods_alarm	:	1;
	uint32_t				rogue_alarm	:	1;
	uint32_t				tf_alarm	:	1;
	uint32_t				lcdg_alarm	:	1;
	uint32_t				sd_alarm	:	1;
	uint32_t				sf_alarm	:	1;
	uint32_t				suf_alarm	:	1;
	uint32_t				mem_alarm	:	1;
	uint32_t				mis_alarm	:	1;
	uint32_t				dact_alarm	:	1;
	uint32_t				dis_alarm	:	1;
	uint32_t				lof_alarm	:	1;
	uint32_t				los_alarm	:	1;
	uint32_t				pee_alarm	:	1;
	uint32_t				rdi_alarm	:	1;
#endif
} GPON_Alarm_T;

typedef struct GPON_PLOAM_CNT_S{
	uint32_t				rxPloamMsgCnt;
	uint32_t				txPloamMsgCnt;
}GPON_PLOAM_CNT_T;

typedef struct GPON_OMCI_CNT_S{
	uint32_t				rxOmciMsgCnt;
	uint32_t				txOmciMsgCnt;
}GPON_OMCI_CNT_T;

typedef struct {
    uint32_t    PloamMicErrCnt;
    uint32_t    DsPloamMsgCnt; 
    uint32_t    ProfileMsgRx;
    uint32_t    RangeTimeMsgRx;
    uint32_t    DeactOnuIdMsgRx;
    uint32_t    DisableSnMsgRx;
    uint32_t    ReqRegMsgRx;
    uint32_t    AssignAllocIdMsgRx;
    uint32_t    KeyCtrlMsgRx;
    uint32_t    SleepAllowMsgRx;
    uint32_t    BaseOmciMsgRx;
    uint32_t    ExtOmciMsgRx;
    uint32_t    AssignOnuIdMsgRx;
    uint32_t    OmciMicErrCnt; 
}GPON_10G_DS_MGNT_COUNTER_T;

typedef struct {
    uint32_t    UsPloamMsgCnt; 
    uint32_t    SnOnuMsgCnt;
    uint32_t    RegMsgCnt;
    uint32_t    KeyRptMsgCnt;
    uint32_t    AckMsgCnt;
    uint32_t    SleepReqMsgCnt;
}GPON_10G_US_MGNT_COUNTER_T;


#if defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
struct XMCS_QosPolicerCreat_S {
	uint32_t                policer_id;
	uint32_t                cir;
};

struct XMCS_QosPolicerDelete_S {
    uint32_t                policer_id;
};

struct XMCS_OverallRatelimitConfig_S {
    uint32_t                bandwidth;
};
#endif

struct XMCS_GponTxRateLimit_S{
	unsigned char chnlIdx ;
	unsigned char chnlRateLimitEn ;
	uint32_t		  rateLimitValue ; //unit is kbps
};

typedef struct eponTxCnt_s{
	uint32_t txFrameCnt;
	uint32_t txFrameLen;
	uint32_t txDropCnt;
	uint32_t txBroadcastCnt;
	uint32_t txMulticastCnt;
	uint32_t txLess64Cnt;
	uint32_t txMore1518Cnt;
	uint32_t txEq64Cnt;
	uint32_t txFrom65To127Cnt;
	uint32_t txFrom128To255Cnt;
	uint32_t txFrom256To511Cnt;
	uint32_t txFrom512To1023Cnt;
	uint32_t txFrom1024To1518Cnt;
}eponTxCnt_t, *eponTxCnt_p;


typedef struct eponRxCnt_s{
	uint32_t rxFrameCnt;
	uint32_t rxFrameLen;
	uint32_t rxDropCnt;
	uint32_t rxBroadcastCnt;
	uint32_t rxMulticastCnt;
	uint32_t rxCrcCnt;
	uint32_t rxFragFameCnt;
	uint32_t rxJabberFameCnt;
	uint32_t rxLess64Cnt;
	uint32_t rxMore1518Cnt;
	uint32_t rxEq64Cnt;
	uint32_t rxFrom65To127Cnt;
	uint32_t rxFrom128To255Cnt;
	uint32_t rxFrom256To511Cnt;
	uint32_t rxFrom512To1023Cnt;
	uint32_t rxFrom1024To1518Cnt;
}eponRxCnt_t, *eponRxCnt_p;

typedef struct eponMpcpCnt_s{
	uint16_t rxHecErrorCnt;
	uint16_t txRegQeqCnt;
	uint8_t rxRegCnt;
	uint16_t txRegAckCnt;
	uint32_t rxNormalGateCnt;
	uint16_t rxDiscGateCnt;
	uint16_t txReportCnt;
}eponMpcpCnt_t, *eponMpcpCnt_p;

typedef struct onuDDM_s{
	uint16_t temperature;
	uint16_t supply_voltage;
	uint16_t tx_current;
	uint16_t tx_power;
	uint16_t rx_power;
}onuDDM_t, *onuDDM_p;


typedef struct {
	uint8_t llidIndex;
	uint8_t param0;
	uint16_t param1;
	uint32_t param2;
	uint8_t info[128];
} eponMacIoctl_t ;

typedef enum
{
	RATE_1G_1G,
    RATE_10G_1G_ASYM,
    RATE_10G_10G_SYM
}EPON_RATE_MODE;

typedef enum
{
    REG_MODE_10G_ASYM = 6,
    REG_MODE_10G_SYM,
    REG_MODE_1G_EPON
}EPON_REG_RATE_MODE;


typedef struct {
    uint8_t an;
    uint32_t pn;
    uint8_t sci[8];
    uint8_t confid_offset;
    int key_len;
    uint8_t sak[16];
}eponMacSec_t;


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/

#endif   //_XPON_IOCTL_IF_H



