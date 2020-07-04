#ifdef CONFIG_OWE_SUPPORT
#include "rtmp.h"
#endif

struct _RTMP_ADAPTER;

#define RALINK_IE_LEN   0x9
#define MEDIATEK_IE_LEN 0x9

#ifdef CONFIG_OWE_SUPPORT
#define OUI_LEN 3
#define MAX_VENDOR_IE_LEN 255
#endif

#define RALINK_AGG_CAP      (1 << 0)
#define RALINK_PIGGY_CAP    (1 << 1)
#define RALINK_RDG_CAP      (1 << 2)
#define RALINK_256QAM_CAP   (1 << 3)

#define MEDIATEK_256QAM_CAP (1 << 3)
#ifdef WH_EZ_SETUP
#define MEDIATEK_EASY_SETUP (1 << 6)
#endif
#ifdef MWDS
#define MEDIATEK_MWDS_CAP   (1 << 7)
#endif
#define BROADCOM_256QAM_CAP (1 << 0)


#ifdef CONFIG_OWE_SUPPORT
#define VIE_BEACON_BITMAP (1 << VIE_BEACON)
#define VIE_PROBE_REQ_BITMAP (1 << VIE_PROBE_REQ)
#define VIE_PROBE_RESP_BITMAP (1 << VIE_PROBE_RESP)
#define VIE_ASSOC_REQ_BITMAP (1 << VIE_ASSOC_REQ)
#define VIE_ASSOC_RESP_BITMAP (1 << VIE_ASSOC_RESP)
#define VIE_AUTH_REQ_BITMAP (1 << VIE_AUTH_REQ)
#define VIE_AUTH_RESP_BITMAP (1 << VIE_AUTH_RESP)
#define VIE_FRM_TYPE_MAX_BITMAP (1 << VIE_FRM_TYPE_MAX) /*for sanity check purpose*/

#define GET_VIE_FRM_BITMAP(_frm_type)	(1 << _frm_type)



typedef enum vendor_ie_oper {
	VIE_ADD = 1,
	VIE_UPDATE,
	VIE_REMOVE,
	VIE_SHOW,
	VIE_OPER_MAX,
} VIE_OPERATION;



INT32 add_vie(struct _RTMP_ADAPTER *pAd,
	      struct wifi_dev *wdev,
	      UINT32 frm_type_map,
	      UINT32 oui_oitype,
	      ULONG ie_length,
	      UCHAR *frame_buffer);

INT32 remove_vie(struct _RTMP_ADAPTER *pAd,
		 struct wifi_dev *wdev,
		 UINT32 frm_type_map,
		 UINT32 oui_oitype,
		 ULONG ie_length,
		 UCHAR *frame_buffer);

VOID print_vie(struct wifi_dev *wdev, UINT32 frm_map);

VOID init_vie_ctrl(struct wifi_dev *wdev);
VOID deinit_vie_ctrl(struct wifi_dev *wdev);
INT vie_oper_proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif

#ifdef STA_FORCE_ROAM_SUPPORT
#define MEDIATEK_CLI_ENTRY (1 << 4)
#endif

typedef struct GNU_PACKED _ie_hdr {
    UCHAR eid;
    UINT8 len;
} IE_HEADER;

#ifdef CONFIG_OWE_SUPPORT
struct GNU_PACKED _generic_vie_t {
	IE_HEADER ie_hdr;
	UCHAR vie_ctnt[MAX_VENDOR_IE_LEN];
};
#endif

struct GNU_PACKED _ralink_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR cap0;
    UCHAR cap1;
    UCHAR cap2;
    UCHAR cap3;
};


typedef struct GNU_PACKED _vht_cap_ie {
    IE_HEADER ie_hdr;
    UCHAR vht_cap_info[4];
    UCHAR support_vht_mcs_nss[8];
} VHT_CAP;


typedef struct GNU_PACKED _vht_op_ie {
    IE_HEADER ie_hdr;
    UCHAR vht_op_info[3];
    UCHAR basic_vht_mcs_nss[2];
} VHT_OP;


typedef struct GNU_PACKED _vht_tx_pwr_env_ie {
    IE_HEADER ie_hdr;
    UCHAR tx_pwr_info;
    UCHAR local_max_txpwr_20Mhz;
    UCHAR local_max_txpwr_40Mhz;
} VHT_TX_PWR_ENV;


struct GNU_PACKED _mediatek_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR cap0;
    UCHAR cap1;
    UCHAR cap2;
    UCHAR cap3;
};


struct GNU_PACKED _mediatek_vht_ie {
    VHT_CAP vht_cap;
    VHT_OP vht_op;
    VHT_TX_PWR_ENV vht_txpwr_env;
};


struct GNU_PACKED _broadcom_ie {
    IE_HEADER ie_hdr;
    UCHAR oui[3];
    UCHAR fixed_pattern[2];
    VHT_CAP vht_cap;
    VHT_OP vht_op;
    VHT_TX_PWR_ENV vht_txpwr_env;
};


ULONG build_vendor_ie(struct _RTMP_ADAPTER *pAd,
		      struct wifi_dev *wdev,
		      UCHAR *frame_buffer
#ifdef CONFIG_OWE_SUPPORT
			  , VIE_FRM_TYPE vie_frm_type
#endif
#ifdef WH_EZ_SETUP
		, UCHAR SubType
#endif
		);

#if defined(MWDS) || defined(MAP_SUPPORT)
VOID check_vendor_ie(struct _RTMP_ADAPTER *pAd,
        UCHAR *ie_buffer, struct _vendor_ie_cap *vendor_ie);
#endif


