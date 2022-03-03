#ifndef __MIX_MODE_H__
#define __MIX_MODE_H__

#define MixModeDefualtListenTime 200

#define RMAC_SMESH		(WF_RMAC_BASE + 0x048)
#define RMAC_SMESH_B1	(WF_RMAC_BASE + 0x148)


#define SMESH_RX_CTL							BIT(20)
#define SMESH_RX_CTL_OFFSET						20
#define SMESH_RX_MGT							BIT(19)
#define SMESH_RX_MGT_OFFSET						19
#define SMESH_RX_DATA							BIT(18)
#define SMESH_RX_DATA_OFFSET					18
#define SMESH_RX_A1								BIT(17)
#define SMESH_RX_A2								BIT(16)

#define MAR1_MAR_GROUP_MASK                    	BITS(30,31)
#define MAR1_MAR_GROUP_OFFSET                  	30
#define MAR1_MAR_HASH_MODE_BSSID1              	BIT(30)
#define MAR1_MAR_HASH_MODE_BSSID2              	BIT(31)
#define MAR1_MAR_HASH_MODE_BSSID3              	BITS(30,31)

#define MAR1_ADDR_INDEX_MASK                   	BITS(24,29)
#define MAR1_ADDR_INDEX_OFFSET                 	24
#define MAR1_READ                              	0
#define MAR1_WRITE                             	BIT(17)
#define MAR1_ACCESS_START_STATUS               	BIT(16)


#define DEFAULT_MNTR_RULE	(SMESH_RX_CTL |SMESH_RX_MGT| SMESH_RX_DATA| SMESH_RX_A1| SMESH_RX_A2)

#define MAX_NUM_OF_MONITOR_STA		8 /*0, 1~4=4, 5~8=8, 9~12=12, 13~16=16 */
#define SMESH_OFFSET			(MAX_NUM_OF_MONITOR_STA / 4)
#define MAX_NUM_OF_MONITOR_GROUP    	(MAX_NUM_OF_MONITOR_STA / 2)
#define MAX_NUM_PER_GROUP           	2

#define MONITOR_MUAR_BASE_INDEX1 0x20
#define MONITOR_MUAR_BASE_INDEX2 0x28

#define ENTRY_CAT_MONITOR		0x010
#define ENTRY_MONITOR		ENTRY_CAT_MONITOR

#define IS_ENTRY_MONITOR(_x)    ((_x)->EntryType == ENTRY_CAT_MONITOR)
#define SET_ENTRY_MONITOR(_x)   ((_x)->EntryType = ENTRY_MONITOR)


#define MixMode_On(_p) (((_p)->MixModeCtrl.MixModeOn) == TRUE)

#define MIX_MODE_OFF  0
#define MIX_MODE_FULL 2

typedef enum _MNT_BAND_TYPE {
	MNT_BAND0 = 1,
	MNT_BAND1 = 2
} MNT_BAND_TYPE;


enum MIX_MODE_STATE {
	MIX_MODE_STATE_BASE = 0,
	MIX_MODE_STATE_INIT = MIX_MODE_STATE_BASE,
	MIX_MODE_STATE_RUNNING,
	MIX_MODE_STATE_MAX
};

enum MIX_MODE_MSG {
	MIX_MODE_MSG_LISTEN = 0,
	MIX_MODE_MSG_CANCLE,
	MIX_MODE_MSG_TIMEOUT,
	MIX_MODE_MSG_MAX
};

#define MIX_MODE_FUNC_SIZE MIX_MODE_STATE_MAX * MIX_MODE_MSG_MAX

typedef struct	_MNT_MUAR_GROUP {
	BOOLEAN bValid;
	UCHAR	Count;
    UCHAR   Band;
    UCHAR	MuarGroupBase;
} MNT_MUAR_GROUP, *PMNT_MUAR_GROUP;

struct INFO_STA {
	LONG cnt;
	LONG  rssi;
};

struct MNT_STA_INFO {
	LONG Count;
	struct INFO_STA frm[FC_TYPE_RSVED];
};

typedef struct	_MNT_STA_ENTRY {
	NDIS_SPIN_LOCK MntStaLock;
	BOOLEAN bValid;
    	UCHAR Band;
    	UCHAR muar_idx;
    	UCHAR muar_group_idx;
	struct MNT_STA_INFO mnt_sta;
	UCHAR addr[MAC_ADDR_LEN];
	RSSI_SAMPLE RssiSample;
	VOID *pMacEntry;
} MNT_STA_ENTRY, *PMNT_STA_ENTRY;

struct mix_peer_parameter {
	UCHAR channel; /*listen channel*/
	UCHAR center_channel;
	UCHAR bw;
	UCHAR ch_offset;
	UCHAR mac_addr[MAC_ADDR_LEN];
	USHORT duration; /*listen time*/
//	UINT32 duration;
};
struct peer_info {
	UCHAR channel; /*listen channel*/
	UCHAR center_channel;
	UCHAR bw;
	UCHAR ch_offset;
	UCHAR mac_addr[MAC_ADDR_LEN];
	USHORT duration; /*listen time*/
//	UINT32 duration;
	struct MNT_STA_INFO mnt_sta;
	RSSI_SAMPLE RssiSample;
	NDIS_SPIN_LOCK MntStaLock;
};

typedef struct _MIX_MODE_CTRL {
	struct peer_info sta_info[MAX_NUM_OF_MONITOR_STA];
	RALINK_TIMER_STRUCT	MixModeTimer;
	STATE_MACHINE MixModeStatMachine;
	STATE_MACHINE_FUNC MixModeStateFunc[MIX_MODE_FUNC_SIZE];
	struct wifi_dev *pMixModewdev;
	INT ioctl_if;
	BOOLEAN MixModeOn;
	MNT_STA_ENTRY MntTable[MAX_NUM_OF_MONITOR_STA];
	MNT_MUAR_GROUP MntGroupTable[MAX_NUM_OF_MONITOR_GROUP];
	UCHAR	MntIdx;
	UCHAR current_monitor_mode;
} MIX_MODE_CTRL;

VOID MixModeTimeout(IN PVOID SystemSpecific1,IN PVOID FunctionContext,
						IN PVOID SystemSpecific2, IN PVOID SystemSpecific3);
VOID MixModeDebugInfo(IN PRTMP_ADAPTER pAd, IN int idx);
INT MixModeSet(IN PRTMP_ADAPTER pAd, struct mix_peer_parameter *sta_info, IN UCHAR idx);
INT RTMPIoctlQueryMixModeRssi(IN PRTMP_ADAPTER pAd, IN RTMP_IOCTL_INPUT_STRUCT *wrq);
VOID MixModeCancel(IN PRTMP_ADAPTER pAd);
VOID MixModeInit(IN PRTMP_ADAPTER pAd);
VOID MixModeDel(IN PRTMP_ADAPTER pAd);
VOID MixModeProcessData(RTMP_ADAPTER *pAd, void *pRxBlk);
BOOLEAN MixModeCliIdxCheck(IN void *pAd, IN UCHAR CliIdx);
#endif
