
/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	qa_agent.h
*/

#ifndef __META_AGENT_H__
#define __META_AGENT_H__

extern CH_FREQ_MAP CH_HZ_ID_MAP[];
extern int CH_HZ_ID_MAP_NUM;
/*
 *	struct for command formats
 */
struct GNU_PACKED _META_CMD_HDR {
	UINT32		oid;
	UINT32		len_in;		/* For set */
	UINT32		len_out;	/* For query */
	UCHAR		data[2048];
};

struct GNU_PACKED _PARAM_MTK_WIFI_TEST {
	UINT32	idx;
	UINT32	data;
};

struct GNU_PACKED _PARAM_CUSTOM_EFUSE_RW {
	UINT32	offset;
	UINT32	data;
};

struct GNU_PACKED _PARAM_CUSTOM_EEPROM_RW {
	UINT8	method;
	UINT8	idx;
	UINT8	reserved;
	UINT16	data;
};

struct GNU_PACKED _PARAM_CUSTOM_MCR_RW {
	UINT32	offset;
	UINT32	data;
};

typedef INT32 (*META_CMD_HANDLER)(int ioctl_cmd, struct _RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT * wrq, META_CMD_HDR * cmd_hdr);

INT Set_Sdio_Bist_Write(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_Sdio_Bist_Read(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
/*
 *	define for command oid
 */
#define MT_META_WIFI_TEST_TABLE_VER 0x01180000

#define OID_CUSTOM_IF_VER				0xFFA0C000
#define OID_CUSTOM_MCR_RW				0xFFA0C801
#define OID_CUSTOM_EEPROM_RW			0xFFA0C803
#define OID_CUSTOM_EFUSE_RW				0xFFA0C805
#define	OID_CUSTOM_TEST_MODE			0xFFA0C901
#define OID_CUSTOM_ABORT_TEST_MODE		0xFFA0C906
#define OID_CUSTOM_MTK_WIFI_TEST		0xFFA0C911
/* NVRAM */
#define OID_CUSTOM_MTK_NVRAM_RW         0xFFA0C941
#define OID_CUSTOM_CFG_SRC_TYPE         0xFFA0C942
#define OID_CUSTOM_EEPROM_TYPE          0xFFA0C943

#define MT_META_WIFI_TEST_CMD_MASK		0x0000001F	/* 32 cmd for 1 set, 0x1F=31*/
#define MT_META_WIFI_TEST_SET_MASK		0x0EFFFFFF	/* 32 bits width, rest 27 bits*/

#define SLOT_TIME_SHORT 9
#define PARAM_EEPROM_READ_METHOD_READ           1
#define PARAM_EEPROM_READ_METHOD_GETSIZE        0

/*
 * MACRO
 */
#define FUNC_IDX_GET_IDX(__idx, __func_idx) \
	(__func_idx = __idx & 0x000000FF)
#define FUNC_IDX_GET_SUBFIELD(__idx, __sub) \
	(__sub = ((__idx & 0xff000000)>>24)&0x000000ff)
/* TODO: Need to put into include/rt_os.h when merge the file */
#define MTPRIV_IOCTL_META_SET (SIOCIWFIRSTPRIV + 0x08)
#define MTPRIV_IOCTL_META_QUERY (SIOCIWFIRSTPRIV + 0x09)
#endif /* __META_AGENT_H__ */
