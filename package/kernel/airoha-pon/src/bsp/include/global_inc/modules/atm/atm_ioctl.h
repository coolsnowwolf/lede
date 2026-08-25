

#ifndef _ATM_IOCTL_H_
#define _ATM_IOCTL_H_

#define IOCTL_PATH "/dev/atm"

#define ATMMGR_IOC_MAGIC 'A'

#define ATM_MODE_BR2684_BRIDGED	(0x00)
#define ATM_MODE_BR2684_ROUTED	(0x02)
#define ATM_MODE_PPPOA_ROUTED	(0x03)
#define ATM_MODE_NONE			(0x0F)


#define ATM_ECNAP_VC_MUX		(0x00)
#define ATM_ECNAP_LLC			(0x01)
#define ATM_ECNAP_NONE			(0x0F)

#define	ATM_QOS_UBR				(0x01)
#define ATM_QOS_CBR     		(0x02)
#define ATM_QOS_RTVBR   		(0x03)
#define ATM_QOS_NRTVBR  		(0x06)
#define ATM_QOS_NONE			(0x0F)

#define	ATM_OAM_F4				(0x00)
#define ATM_OAM_F5     			(0x01)

#define ATM_OAM_SEGMENT  		(0x00)
#define ATM_OAM_ENDTOEND   		(0x01)

#define ATM_OAM_RESET   		(0x00)
#define ATM_OAM_ACTIVE  		(0x01)

#define ATM_MAX_VC				10

typedef enum
{
	ATMMGR_FUNC_CREATE_VCC = 0,
	ATMMGR_FUNC_DELETE_VCC,
	ATMMGR_FUNC_SHOW_VCC,
	ATMMGR_FUNC_START_OAM_LOOPBACK,
	ATMMGR_FUNC_RESET_OAM_LOOPBACK,
	ATMMGR_FUNC_SHOW_OAM_LOOPBACK,
	ATMMGR_FUNC_SHOW_WAN_COUNTER,
	ATMMGR_FUNC_SHOW_MIB_COUNTER,
	
	ATMMGR_FUNC_DEF_NO
} ATMMGR_FUNC_ID;

typedef enum
{
	ECNT_ATMMGR_SUCCESS = 0,
	ECNT_ATMMGR_FAIL,
	ECNT_ATMMGR_NOT_BSPMODE,
	ECNT_ATMMGR_VCC_NOT_CLOSED,
	ECNT_ATMMGR_VCC_NOT_FOUND,
	ECNT_ATMMGR_VCC_IS_EXIT,
	ECNT_ATMMGR_VCC_IS_FULL,
	ECNT_ATMMGR_VCC_PARAM_ERR,

	ECNT_ATMMGR_DEF_NO
} ECNT_ATMMGR_RET;

typedef enum
{
	ATM_OAM_TEST_COMPLETED = 0,
	ATM_OAM_TEST_ERROR_INTERNAL,
	ATM_OAM_TEST_IN_PROCESS,
	ATM_OAM_TEST_ERROR_OTHER,

	ATM_OAM_TEST_DEF_NO
} ATM_OAM_TEST_STATUS;

typedef struct atm_vcc_param
{
	unsigned char mode;
	unsigned char encap;
	unsigned char qos_type;
	unsigned int pcr;
	unsigned int scr;
	unsigned int mbs;
} ATM_VCC_PARAM;

typedef struct atm_oam_param
{
	unsigned char oam_type;
	unsigned char test_type;
} ATM_OAM_PARAM;

typedef struct atm_oam_status
{
	ATM_OAM_TEST_STATUS diag_state;
	unsigned int diag_time;
} ATM_OAM_STATUS;

typedef struct atm_wan_cnt
{
	unsigned int rxPkts;
	unsigned int txPkts;
	unsigned int rxDropPkts;
	unsigned int txDropPkts;
	unsigned int rxBytes;
	unsigned int txBytes;
} ATM_WAN_CNT;

typedef struct atm_mib_cnt
{
	unsigned char vc_max;
/*****IN PACKET COUNTER*****/
	unsigned int rxDataPkts;		/* Transmit Data Packets */
    unsigned int rxF4Pkts;			/* Receive F4 Packets */
    unsigned int rxF5Pkts;			/* Receive F5 Packets */

/*****IN ERROR COUNTER*****/
    unsigned int rxDiscards;			/* Receive Discard Packets */
    unsigned int rxCrcErr;
	unsigned int rxCrc32Err;
	unsigned int rxCrc10Err;
	unsigned int rxL4CsErr;
	unsigned int rxIp4CsErr;
	unsigned int rxActErr;
    unsigned int rxBufLenErr;
	unsigned int rxMpoaErr;

/*****OUT PACKET COUNTER*****/
	unsigned int txDataPkts;		/* Receive Data Packets */
    unsigned int txF4Pkts;			/* Transmit F4 Packets */
    unsigned int txF5Pkts;			/* Transmit F5 Packets */
	unsigned int txVcNum[ATM_MAX_VC];

/*****OUT DROP COUNTER*****/
	unsigned int txHardDropVcNum[ATM_MAX_VC];
} ATM_MIB_CNT;

typedef struct atmmgr_ioctl
{
    ATMMGR_FUNC_ID func_id;
	ECNT_ATMMGR_RET rtn_val;
	unsigned char vpi;
	unsigned int vci;
	union
	{
		ATM_VCC_PARAM vcc_param;
		ATM_OAM_PARAM oam_param;
		ATM_OAM_STATUS oam_status;
		ATM_WAN_CNT wan_cnt;
		ATM_MIB_CNT mib_cnt;
	} atmmgr_data;
} ATMMGR_IOCTL;


#define ATMMGR_IOC_OPT	_IOWR(ATMMGR_IOC_MAGIC, 0, ATMMGR_IOCTL)

#endif

