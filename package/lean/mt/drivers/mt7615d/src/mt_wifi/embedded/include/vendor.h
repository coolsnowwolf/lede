
struct _RTMP_ADAPTER;

#define RALINK_IE_LEN   0x9
#define MEDIATEK_IE_LEN 0x9

#define OUI_LEN 3
#define MAX_VENDOR_IE_LEN 255

#define RALINK_AGG_CAP      (1 << 0)
#define RALINK_PIGGY_CAP    (1 << 1)
#define RALINK_RDG_CAP      (1 << 2)
#define RALINK_256QAM_CAP   (1 << 3)

#define MEDIATEK_256QAM_CAP (1 << 3)
#define BROADCOM_256QAM_CAP (1 << 0)

typedef enum vendor_ie_in_frame_type {
	VIE_BEACON = 0, /*make sure it starts from 0, for initialization.*/
	VIE_PROBE_REQ,
	VIE_PROBE_RESP,
	VIE_ASSOC_REQ,
	VIE_ASSOC_RESP,
	/*due to there is less opportunity to add vendor ie in Auth. arranage them in the bottom seq.*/
	VIE_AUTH_REQ,
	VIE_AUTH_RESP,
	VIE_FRM_TYPE_MAX,
} VIE_FRM_TYPE;

#define VIE_BEACON_BITMAP (1 << VIE_BEACON)
#define VIE_PROBE_REQ_BITMAP (1 << VIE_PROBE_REQ)
#define VIE_PROBE_RESP_BITMAP (1 << VIE_PROBE_RESP)
#define VIE_ASSOC_REQ_BITMAP (1 << VIE_ASSOC_REQ)
#define VIE_ASSOC_RESP_BITMAP (1 << VIE_ASSOC_RESP)
#define VIE_AUTH_REQ_BITMAP (1 << VIE_AUTH_REQ)
#define VIE_AUTH_RESP_BITMAP (1 << VIE_AUTH_RESP)
#define VIE_FRM_TYPE_MAX_BITMAP (1 << VIE_FRM_TYPE_MAX) /*for sanity check purpose*/

#define GET_VIE_FRM_BITMAP(_frm_type)	(1 << _frm_type)

typedef struct vie_struct {
	ULONG vie_length;/*the total length which starts from oui until the tail.*/
	UCHAR oui_oitype[4];/*used to comparism*/
	UCHAR *ie_ctnt;
	struct vie_struct *next_vie;
} VIE_STRUCT;

typedef enum vendor_ie_oper {
	VIE_ADD = 1,
	VIE_UPDATE,
	VIE_REMOVE,
	VIE_SHOW,
	VIE_OPER_MAX,
} VIE_OPERATION;

typedef struct vie_ctrl_struct {
	VIE_FRM_TYPE type;
	UCHAR vie_num;
	struct vie_struct *vie_in_frm;
} VIE_CTRL;

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

typedef struct GNU_PACKED _ie_hdr {
	UCHAR eid;
	UINT8 len;
} IE_HEADER;

struct GNU_PACKED _generic_vie_t {
	IE_HEADER ie_hdr;
	UCHAR vie_ctnt[MAX_VENDOR_IE_LEN];
};

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
		      UCHAR *frame_buffer,
		      VIE_FRM_TYPE vie_frm_type
		      );

VOID check_vendor_ie(struct _RTMP_ADAPTER *pAd,
					 UCHAR *ie_buffer, struct _vendor_ie_cap *vendor_ie);


