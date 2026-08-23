#ifndef _XPON_PUBLIC_CONST_H_
#define _XPON_PUBLIC_CONST_H_

typedef unsigned int uint;

#define XPON_MODE_FLASH_ADDR             (0xff9c)
#define TRANS_VENDOR_NAME_SIZE           16
#define TRANS_VENDOR_PN_SIZE             16
#define TRANS_VENDOR_SN_SIZE             16
#define GPON_SN_LENS                     8
#define GPON_VENDOR_ID_LENS              4
#define GPON_PASSWD_LENS                 10
#define GPON_REG_ID_LENS                 36
#define GPON_ENCRYPT_KEY_LENS            24
#define GPON_MAX_GEM_ID                  4096
#define GPON_UNASSIGN_ALLOC_ID           0xff
#define GPON_MAX_ALLOC_ID                4096
#define GPON_10G_MAX_GEM_ID              65536
#define GPON_10G_MAX_VALID_GEM_ID        65534
#define GPON_10G_UNASSIGN_ALLOC_ID       0x3ff
#define GPON_10G_MAX_ALLOC_ID            16384
#define GPON_MSK_LENS                    16
#define GPON_GEMPORT_MAX_NUM             256
#define GPON_TCONT_MAX_NUM               32
#define NG2_CHANNEL_MAX_NUM              16
#define GPON_MODULE_VENDOR_LEN           16
#define GPON_MODULE_NAME_LEN             16
#define GPON_MODULE_VERSION_LEN          16
#define GPON_MODULE_FW_VERSION_LEN       16
#define GPON_MODULE_STATUS_LEN           16

typedef enum {
	XPON_LED_OFF_MODE = 0,
	XPON_LED_FLICKER_MODE,
	XPON_LED_ON_MODE,
	XPON_LED_SW_UPGRADE_MODE,
} XponLedMode_t;

typedef enum {
	XMCS_IF_WAN_DETECT_MODE_AUTO = 0,
	XMCS_IF_WAN_DETECT_MODE_GPON,
	XMCS_IF_WAN_DETECT_MODE_EPON,
	XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON,
	XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON,
	XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON,
	XMCS_IF_WAN_DETECT_MODE_XGPON,
	XMCS_IF_WAN_DETECT_MODE_XGSPON,
	XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G,
	XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G,
	XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G,
	XMCS_IF_WAN_DETECT_MODE_GPON_SYM,
	XMCS_IF_WAN_DETECT_MODE_TURBO_EPON,
	XMCS_IF_WAN_DETECT_MODE_MAX,
} XMCSIF_WanDetectionMode_t;

typedef enum {
	AUTOMODE_COMBO_OFF = 0,
	AUTOMODE_XEPON1G_TURBOEPON,
} AutoMode_Combo_Type_t;

typedef enum {
	SYS_GPON_MODE = 0,
	SYS_EPON_MODE,
	SYS_PTM_MODE,
	SYS_ATM_MODE,
	SYS_reserved_MODE,
	SYS_GPON_P2P_MODE,
	SYS_10G_1G_EPON_MODE,
	SYS_10G_10G_EPON_MODE,
	SYS_1G_1G_EPON_MODE,
	SYS_XGPON_MODE,
	SYS_XGSPON_MODE,
	SYS_NGPON2_10G_10G_MODE,
	SYS_NGPON2_10G_2G_MODE,
	SYS_NGPON2_2G_2G_MODE,
	SYS_reserved_MODE2,
	SYS_reserved_MODE3,
	SYS_SGMII_MODE,
	SYS_HSGMI_MODE,
	SYS_BASER_MODE,
	SYS_GPON_t_MODE,
	SYS_GPON_SYM_MODE,
	SYS_TURBO_EPON_MODE = 21,
	SYS_WAN_MODE_MAX,
} SCU_WanMode_t;

typedef enum {
	GPON_UNICAST_GEM = 0,
	GPON_MULTICAST_GEM,
} GPON_GemType_t;

typedef enum {
	XPON_DISABLE = 0,
	XPON_ENABLE,
	XPON_POWER_DOWN,
	XPON_OTHER,
} XPON_Mode_t;

typedef enum {
	XMCS_IF_WAN_LINK_OFF = 0,
	XMCS_IF_WAN_LINK_GPON,
	XMCS_IF_WAN_LINK_EPON,
} XMCSIF_WanLinkStatus_t;

typedef struct XMCS_PhyTransInfo_S {
	uint16_t txWaveLen;
	uint8_t vendorName[TRANS_VENDOR_NAME_SIZE];
	uint8_t vendorPN[TRANS_VENDOR_PN_SIZE];
	uint8_t vendorSN[TRANS_VENDOR_SN_SIZE];
} PHY_TRANSINFO_T;

typedef enum {
	UPAES_MODE_14BIT = 0,
	UPAES_MODE_128BIT,
	UPAES_MODE_14BIT_NO_DATA,
	UPAES_MODE_128BIT_NO_DATA,
	UPAES_MODE_NONE,
	UPAES_MODE_MAX,
} UPAES_Mode_t;

typedef struct XMCS_PhyTransParams_S {
	uint16_t temperature;
	uint16_t voltage;
	uint16_t txCurrent;
	uint16_t txPower;
	uint16_t rxPower;
	PHY_TRANSINFO_T transInfo;
} PHY_PARAMS_t;

typedef struct XMCS_WanLinkConfig_S {
	XPON_Mode_t linkStart;
	XMCSIF_WanDetectionMode_t detectMode;
	XMCSIF_WanLinkStatus_t linkStatus;
} WAN_LINKCFG_t;

struct XMCS_GemPortInfo_S {
	struct {
		GPON_GemType_t gemType;
		uint16_t gemPortId;
		uint16_t allocId;
		uint16_t aniIdx;
		XPON_Mode_t lbMode;
		XPON_Mode_t enMode;
		XPON_Mode_t txEncrypt;
	} info[GPON_GEMPORT_MAX_NUM];
	uint16_t entryNum;
};

typedef struct XMCS_PhyRxFecConfig_S {
	XPON_Mode_t fecMode;
	XPON_Mode_t fecStatus;
	XPON_Mode_t fecCntReset;
	struct {
		uint32_t corrBytes;
		uint32_t corrCodeWords;
		uint32_t unCorrCodeWords;
		uint32_t totalRxCodeWords;
		uint32_t fecSeconds;
	} fecCounter;
} PHY_FECCONFIG_t;

typedef struct XMCS_WanCntStats_S {
	uint32_t mask;
	uint32_t txFrameCnt;
	unsigned long long txFrameLen;
	uint32_t txDropCnt;
	uint32_t txBroadcastCnt;
	uint32_t txMulticastCnt;
	uint32_t txLess64Cnt;
	uint32_t txMore1518Cnt;
	uint32_t tx64Cnt;
	uint32_t tx65To127Cnt;
	uint32_t tx128To255Cnt;
	uint32_t tx256To511Cnt;
	uint32_t tx512To1023Cnt;
	uint32_t tx1024To1518Cnt;
	uint32_t rxFrameCnt;
	unsigned long long rxFrameLen;
	uint32_t rxDropCnt;
	uint32_t rxBroadcastCnt;
	uint32_t rxMulticastCnt;
	uint32_t rxCrcCnt;
	uint32_t rxFragFameCnt;
	uint32_t rxJabberFameCnt;
	uint32_t rxLess64Cnt;
	uint32_t rxMore1518Cnt;
	uint32_t rx64Cnt;
	uint32_t rx65To127Cnt;
	uint32_t rx128To255Cnt;
	uint32_t rx256To511Cnt;
	uint32_t rx512To1023Cnt;
	uint32_t rx1024To1518Cnt;
	uint32_t rxHecErrorCnt;
	uint32_t rxFecErrorCnt;
	uint32_t rxFecCerrorCnt;
	uint32_t FecSeconds;
	uint32_t BipError;
	unsigned long long rxFCDropCnt;
	unsigned long long rxRCDropCnt;
	unsigned long long rxOVDropCnt;
	unsigned long long rxERRDropCnt;
	unsigned long long rxOKPktCnt;
	unsigned long long rxOKByteCnt;
	unsigned long long rxOversizeCnt;
	unsigned long long rxUnderSizeCnt;
} WAN_STATISTIC_t;

typedef struct {
	char cage[8];
	char moduleVendor[GPON_MODULE_VENDOR_LEN];
	char moduleName[GPON_MODULE_NAME_LEN];
	char moduleVersion[GPON_MODULE_VERSION_LEN];
	char moduleFwVersion[GPON_MODULE_FW_VERSION_LEN];
	char connector[24];
	char status[GPON_MODULE_STATUS_LEN];
} phyMedModule_t;

#endif
