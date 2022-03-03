
#ifndef __MAP_H__
#define __MAP_H__

#include "rtmp_type.h"

#define ASSOC_REQ_LEN       154
#define BLOCK_LIST_NUM      16
#define VENDOR_SPECIFIC_LEN 128
#define MAX_BH_PROFILE_CNT    4

/* For rssi steering*/
#define RCPI_TOLLENACE       8 /* 2dB */

#define IS_MAP_ENABLE(pAd) \
		(pAd->bMAPEnable == TRUE)

#define IS_MAP_TURNKEY_ENABLE(pAd) \
		((pAd->bMAPTurnKeyEnable == TRUE) && (pAd->bMAPEnable == TRUE))

typedef enum {
	BELOW_THRESHOLD = 0,
	ABOVE_THRESHOLD,
} RSSI_STATUS;

typedef enum {
	AGENT_INIT_STEER_DISALLOW = 0,
	AGENT_INIT_RSSI_STEER_MANDATE,
	AGENT_INIT_RSSI_STEER_ALLOW,
} STEERING_POLICY;

struct GNU_PACKED map_policy_setting {
	unsigned char steer_policy;
	unsigned char cu_thr;
	unsigned char rcpi_thr;
};

#define MAX_PROFILE_CNT 4
struct scan_SSID {
	char ssid[32 + 1];
	unsigned char SsidLen;
};

struct GNU_PACKED scan_BH_ssids
{
	unsigned long scan_cookie;
	unsigned char scan_channel_count;
	unsigned char scan_channel_list[32];
	unsigned char profile_cnt;
	struct scan_SSID scan_SSID_val[MAX_PROFILE_CNT];
};

typedef struct _MAP_CONFIG {
	/*Support Unassociated STA link metric report on current operating Bss*/
	BOOLEAN bUnAssocStaLinkMetricRptOpBss;
	/*Support Unassociated STA link metric report on currently non operating Bss */
	BOOLEAN bUnAssocStaLinkMetricRptNonOpBss;
	/*Support Agent-initiated Rssi-based steering */
	BOOLEAN bAgentInitRssiSteering;
	UCHAR DevOwnRole;
	UCHAR vendor_ie_buf[VENDOR_SPECIFIC_LEN];
	UCHAR vendor_ie_len;
	struct scan_BH_ssids scan_bh_ssids;
	BOOLEAN FireProbe_on_DFS;
} MAP_CONFIG, *PMAP_CONFIG;

/* spec v171027 */
enum MAPRole {
	MAP_ROLE_TEARDOWN = 4,
	MAP_ROLE_FRONTHAUL_BSS = 5,
	MAP_ROLE_BACKHAUL_BSS = 6,
	MAP_ROLE_BACKHAUL_STA = 7,
};

#define NON_PREF 0
#define PREF_SCORE_1    BIT(4)
#define PREF_SCORE_2    BIT(5)
#define PREF_SCORE_3    (BIT(4)|BIT(5))
#define PREF_SCORE_4    BIT(6)
#define PREF_SCORE_5    (BIT(4)|BIT(6))
#define PREF_SCORE_6    (BIT(5)|BIT(6))
#define PREF_SCORE_7    (BIT(4)|BIT(5)|BIT(6))
#define PREF_SCORE_8    (BIT(7))
#define PREF_SCORE_9    (BIT(4)|BIT(7))
#define PREF_SCORE_10   (BIT(5)|BIT(7))
#define PREF_SCORE_11   (BIT(4)|BIT(5)|BIT(6))
#define PREF_SCORE_12   (BIT(6)|BIT(7))
#define PREF_SCORE_13   (BIT(4)|BIT(6)|BIT(7))
#define PREF_SCORE_14   (BIT(5)|BIT(6)|BIT(7))

#define UNSPECIFICIED 0
#define NON80211_INTERFERER_IN_LOCAL_ENV            BIT(0)
#define INTRA_NETWORK_80211_OBSS_INTERFERENCE       BIT(1)
#define ENTERNAL_NETWORK_80211_OBSS_INTERFERENCE    (BIT(0)|BIT(1))
#define REDUCED_COVERAGE                            BIT(2)
#define REDUCED_TP                                  (BIT(0)|BIT(2))
#define INDEVICE_INTERFERER                         (BIT(1)|BIT(2))
#define OP_DISALLOWED_DUE_TO_DFS                    (BIT(0)|BIT(1)|BIT(2))
#define OP_PREVENT_BACKHAUL_OP                      (BIT(3))
#define IMMEDIATE_OP_POSSIBLE_ON_DFS_CHN            (BIT(0)|BIT(3))
#define DFS_CHN_STATE_UNKNOWN                       (BIT(1)|BIT(3))

extern UCHAR multicast_mac_1905[MAC_ADDR_LEN];

VOID MAP_Init(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN IN UCHAR wdev_type
);

VOID MAP_InsertMapCapIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen
);

INT MAP_InsertMapWscAttr(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf
);

BOOLEAN map_check_cap_ie(
	IN PEID_STRUCT   eid,
	OUT  unsigned char *cap
);

UCHAR getNonOpChnNum(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR op_class
);

VOID setNonOpChnList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PCHAR nonOpChnList,
	IN UCHAR op_class,
	IN UCHAR nonOpChnNum
);

INT map_send_bh_sta_wps_done_event(
	IN PRTMP_ADAPTER adapter,
	IN struct _MAC_TABLE_ENTRY *mac_entry,
	IN BOOLEAN is_ap
);

VOID map_rssi_status_check(
	IN PRTMP_ADAPTER pAd);

INT ReadMapParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);

#ifdef A4_CONN
BOOLEAN map_a4_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BOOLEAN is_ap
);

BOOLEAN map_a4_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN struct _MAC_TABLE_ENTRY *entry,
	IN BOOLEAN is_ap
);

BOOLEAN map_a4_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
);

BOOLEAN map_a4_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
);
#endif /*A4_CONN*/
BOOLEAN MapNotRequestedChannel(struct wifi_dev *wdev, unsigned char channel);
#endif

