

#ifndef _PTM_IOCTL_H_
#define _PTM_IOCTL_H_

#define IOCTL_PATH "/dev/ptm"

#define PTMMGR_IOC_MAGIC 'P'

#define PTM_MAX_BEARER	2

typedef enum
{
	PTMMGR_FUNC_SHOW_WAN_COUNTER = 0,
	PTMMGR_FUNC_SHOW_MIB_COUNTER,
	
	PTMMGR_FUNC_DEF_NO
} PTMMGR_FUNC_ID;

typedef enum
{
	ECNT_PTMMGR_SUCCESS = 0,
	ECNT_PTMMGR_FAIL,

	ECNT_PTMMGR_DEF_NO
} ECNT_PTMMGR_RET;

typedef struct ptm_wan_cnt
{
	unsigned int rxPkts;
	unsigned int txPkts;
	unsigned int rxDropPkts;
	unsigned int txDropPkts;
	unsigned int rxBytes;
	unsigned int txBytes;
} PTM_WAN_CNT;

typedef struct ptm_mib_cnt
{
	unsigned char bearer_max;
	unsigned int rxBytes[PTM_MAX_BEARER];
	unsigned int rxPkts[PTM_MAX_BEARER];
	unsigned int rxDiscards[PTM_MAX_BEARER];
	unsigned int rxErrors[PTM_MAX_BEARER];
	unsigned int txBytes[PTM_MAX_BEARER];
	unsigned int txPkts[PTM_MAX_BEARER];
	unsigned int txDiscards[PTM_MAX_BEARER];
	unsigned int rxErrCrc[PTM_MAX_BEARER];
	unsigned int rxErrLong[PTM_MAX_BEARER];
	unsigned int rxErrRunt[PTM_MAX_BEARER];
	unsigned int rxErrIp4Cs[PTM_MAX_BEARER];
	unsigned int rxErrL4Cs[PTM_MAX_BEARER];
} PTM_MIB_CNT;

typedef struct ptmmgr_ioctl
{
    PTMMGR_FUNC_ID func_id;
	ECNT_PTMMGR_RET rtn_val;
	union
	{
		PTM_WAN_CNT wan_cnt;
		PTM_MIB_CNT mib_cnt;
	} ptmmgr_data;
} PTMMGR_IOCTL;


#define PTMMGR_IOC_OPT	_IOWR(PTMMGR_IOC_MAGIC, 0, PTMMGR_IOCTL)

#endif

