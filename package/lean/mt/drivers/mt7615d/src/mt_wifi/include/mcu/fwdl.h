
#ifndef _FWDL_H
#define _FWDL_H

/*
 * FW feature set
 * bit(0)  : encrypt or not.
 * bit(1,2): encrypt key index.
 * bit(3)  : compressed image or not. (added in CONNAC)
 * bit(5)  : replace RAM code starting address with image destination address or not. (added in CONNAC)
 * bit(7)  : download to EMI or not. (added in CONNAC)
 */
#define FW_FEATURE_SET_ENCRY (1 << 0)
#define FW_FEATURE_SET_KEY_MASK (0x3 << 1)
#define GET_FW_FEATURE_SET_KEY(p) (((p) & FW_FEATURE_SET_KEY_MASK) >> 1)
#define FW_FEATURE_COMPRESS_IMG (1 << 3)
#define FW_FEATURE_OVERRIDE_RAM_ADDR (1 << 5)
#define FW_FEATURE_DL_TO_EMI (1 << 7)

#define WAIT_LOOP 1500

#define PATCH_V1_INFO_SIZE 30
#define FW_V2_INFO_SIZE 36
/* CONNAC */
#define FW_V3_COMMON_TAILER_SIZE 36
#define FW_V3_REGION_TAILER_SIZE 40

#define FWDL_PRINT_CHAR(src, cnt, info)	\
	do {	\
		UINT32 loop;	\
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, info);	\
		for (loop = 0; loop < cnt; loop++)	\
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%c", *(src + loop)));	\
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));	\
	} while (0)

#define FWDL_PRINT_HEX(src, cnt, info)	\
			do {	\
				UINT32 loop;	\
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, info);	\
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("0x"));	\
				for (loop = 0; loop < cnt; loop++)	\
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", *(src + loop)));	\
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));	\
			} while (0)

enum fwdl_stage {
	FWDL_STAGE_FW_NOT_DL,
	FWDL_STAGE_CMD_EVENT,
	FWDL_STAGE_SCATTER,
	FWDL_STAGE_FW_RUNNING
};

/* define FW SYNC CR, same as FW's definition */
enum wifi_task_state {
	WIFI_TASK_STATE_INITIAL          = 0,
	WIFI_TASK_STATE_FW_DOWNLOAD      = 1,
	WIFI_TASK_STATE_NORMAL_OPERATION = 2,
	WIFI_TASK_STATE_NORMAL_TRX       = 3,
	WIFI_TASK_STATE_WACPU_RDY        = 7
};

/* use BIT(n) macro to convert to bitmap */
enum target_cpu {
	WM_CPU,
	WA_CPU,
	MAX_CPU
};

/* use BIT(n) macro to convert to bitmap */
enum load_code_method {
	HEADER_METHOD,
	BIN_METHOD
};

/*
 * fw download cmd/event:
 * FW_FLOW_V1 support 7603, 7628, 7636, 7637, 7615, 7622, CONNAC (p18, 7663)
 */
enum load_fw_flow {
	FW_FLOW_V1,
	FW_FLOW_V2_COMPRESS_IMG
};

/*
 * patch download cmd/event:
 * PATCH_FLOW_V1 support 7636, 7637, 7615, 7622, CONNAC (p18, 7663)
 */
enum load_patch_flow {
	PATCH_FLOW_V1
};

/*
 * fw format:
 * FW_FORMAT_V1 support 7603, 7628 (all von-neumann architecture)
 * FW_FORMAT_V2 support 7636, 7637, 7615, 7622 (all harvard architecture)
 * FW_FORMAT_V3 support CONNAC architecture (p18, mt7663 and so on.)
 */
enum fw_format {
	FW_FORMAT_V1,
	FW_FORMAT_V2,
	FW_FORMAT_V3
};

/*
 * patch format:
 * PATCH_FORMAT_V1 support 7636, 7637, 7615, 7622, CONNAC (p18, 7663)
 */
enum patch_format {
	PATCH_FORMAT_V1
};

enum sem_status {
	PATCH_NOT_DL_SEM_FAIL    = 0,
	PATCH_IS_DL              = 1,
	PATCH_NOT_DL_SEM_SUCCESS = 2,
	SEM_RELEASE              = 3
};

struct img_source {
	enum load_code_method applied_method;
	UCHAR *header_ptr;
	UINT32 header_len;
	UCHAR *bin_name;
	UCHAR *img_ptr;
	UINT32 img_len;
};

struct fw_info {
	UINT8 chip_id;
	UINT8 eco_ver;
	UINT8 num_of_region;
	UINT8 format_ver;
	UINT8 ram_ver[10];
	UINT8 ram_built_date[15];
	UINT32 crc;
};

struct patch_info {
	UINT8 built_date[16];
	UINT8 platform[4];
	UINT8 hw_sw_version[4];
	UINT8 patch_version[4];
	UINT16 checksum;
};

struct img_profile_fw {
	struct img_source source;
	struct fw_info fw_info;
};

struct img_profile_patch {
	struct img_source source;
	struct patch_info patch_info;
};

struct fw_dl_buf {
	UINT8 *img_ptr;
	UINT32 img_dest_addr;
	UINT32 img_size;
	UINT8 feature_set;
	/* decompress features are supported in CONNAC */
	UINT32 decomp_crc;
	UINT32 decomp_img_size;
	UINT32 decomp_block_size;
};

struct fw_dl_target {
	struct fw_dl_buf *fw_region;
	UINT8 num_of_region;
};

struct patch_dl_buf {
	UINT8 *img_ptr;
	UINT32 img_dest_addr;
	UINT32 img_size;
	BOOLEAN check_crc;
};

struct patch_dl_target {
	struct patch_dl_buf *patch_region;
	UINT8 num_of_region;
};

struct fwdl_op {
	NDIS_STATUS (*load_fw)(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target);
	NDIS_STATUS (*load_patch)(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct patch_dl_target *target);
	NDIS_STATUS (*parse_fw)(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target);
	NDIS_STATUS (*parse_patch)(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct patch_dl_target *target);
	NDIS_STATUS (*ctrl_fw_state)(struct _RTMP_ADAPTER *pAd, enum fwdl_stage target_stage);
};

struct fwdl_ctrl {
	struct fwdl_op fwdl_op;
	struct img_profile_fw fw_profile[MAX_CPU];
	struct img_profile_patch patch_profile[MAX_CPU];
	enum sem_status sem_status;
	enum fwdl_stage stage;
};

INT NICLoadRomPatch(struct _RTMP_ADAPTER *ad);
INT NICLoadFirmware(struct _RTMP_ADAPTER *pAd);
INT FwdlHookInit(struct _RTMP_ADAPTER *pAd);
VOID NICRestartFirmware(struct _RTMP_ADAPTER *pAd);
NDIS_STATUS mt_load_patch(struct _RTMP_ADAPTER *pAd);
NDIS_STATUS mt_load_fw(struct _RTMP_ADAPTER *pAd);
NDIS_STATUS mt_fwdl_hook_init(struct _RTMP_ADAPTER *pAd);
NDIS_STATUS mt_restart_fw(struct _RTMP_ADAPTER *pAd);
VOID show_patch_info(struct _RTMP_ADAPTER *pAd);
VOID show_fw_info(struct _RTMP_ADAPTER *pAd);

#endif
