/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_QDMA_TYPE_H_
#define _ECNT_HOOK_QDMA_TYPE_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
	
#ifdef QDMA_LAN
#define ECNT_QDMA ECNT_QDMA_LAN
#define storm_ctrl_shrehold storm_ctrl_shrehold_lan
#define qdma_fwd_timer qdma_lan_fwd_timer
#else
#define ECNT_QDMA ECNT_QDMA_WAN
#define storm_ctrl_shrehold storm_ctrl_shrehold_wan
#define qdma_fwd_timer qdma_wan_fwd_timer
#endif


/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define CONFIG_QDMA_QUEUE	8
#define ECNT_DRIVER_API  0

#if defined(TCSUPPORT_CPU_EN7581)
#define TX_RING_NUM		(32)
#elif defined(TCSUPPORT_CPU_EN7580)
#define TX_RING_NUM		(8)
#else /*TCSUPPORT_CPU_EN7527 || TCSUPPORT_CPU_EN7516 and before */
#define TX_RING_NUM		(2)
#endif

#if defined(TCSUPPORT_CPU_EN7581)
#define RX_RING_NUM		(32)
#elif defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define RX_RING_NUM		(16)
#else
#define RX_RING_NUM		(2)
#endif

#define MAX_NAME_LENGTH             (16)
#if defined(TCSUPPORT_CPU_EN7581)
#define LRO_RX_RING_START		(23)
#define LRO_RX_RING_END			(30)
#else
#define LRO_RX_RING_START		(11)
#define LRO_RX_RING_END			(14)
#endif
#define LRO_RXMSG_AGGCOUNT_MASK		(0xFF)
#define LRO_RXMSG_AGGCOUNT_SHIFT	(0x18)

#ifdef TCSUPPORT_LRO_ENABLE
#define INT2_RX_RING_START		LRO_RX_RING_START
#define INT2_RX_RING_END		LRO_RX_RING_END
#else
#define INT2_RX_RING_START		(0)
#define INT2_RX_RING_END		(RX_RING_NUM-1)
#endif

#define RING_IDX_0		(0)
#define RING_IDX_1		(1)
#define RING_IDX_2		(2)
#define RING_IDX_3		(3)
#define RING_IDX_4		(4)
#define RING_IDX_5		(5)
#define RING_IDX_6		(6)
#define RING_IDX_7		(7)
#define RING_IDX_8		(8)
#define RING_IDX_9		(9)
#define RING_IDX_10		(10)
#define RING_IDX_11		(11)
#define RING_IDX_12		(12)
#define RING_IDX_13		(13)
#define RING_IDX_14		(14)
#define RING_IDX_15		(15)

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#define QDMA_INT_IDX_1		(1)
#define QDMA_INT_IDX_2		(2)
#define QDMA_INT_IDX_3		(3)
#define QDMA_INT_IDX_4		(4)
#endif

#define FTP_PASV_PORT_RING	(RING_IDX_3)
#define QDMA_BUSY_TIMER_MAX 60 /* 60s */
#define QDMA_REG_POLLING_MAX 60 /* 60s */

#define PER_CHNL_TICKSEL_NUM (2)
#define TX_WRED_THR_NUM      (5)
#define TX_WRED_PROBABILITY_NUM      (4)

#define QDMA_INGRESS_INDEX_MAX		(127) /*EN7580:128 ratelimit or 64 trtcm*/
#if defined(QDMA_LAN) // qdma1
#if defined(TCSUPPORT_CPU_AN7583)
	#define CONFIG_QDMA_CHANNEL 48
#elif defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
    #define CONFIG_QDMA_CHANNEL		32
#elif defined(TCSUPPORT_CPU_EN7580)
    #define CONFIG_QDMA_CHANNEL		13
#else
    #define CONFIG_QDMA_CHANNEL		8
#endif
#else // qdma2
#if defined(TCSUPPORT_CPU_EN7517)
    #define CONFIG_QDMA_CHANNEL		12
#else
    #define CONFIG_QDMA_CHANNEL		32
#endif
#endif

#define QDMA_MAGIC					(0xAB)
#define IO_QDMA_CMD_OPT				_IOW(QDMA_MAGIC, 0, struct ECNT_QDMA_Data *)
#if 0
/* Interface IO Command */
#define IO_QDMA_SET_QOS_CONFIG				_IOW(QDMA_MAGIC, 0x01, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_QOS_VALUE				_IOW(QDMA_MAGIC, 0x02, struct ECNT_QDMA_Data *)
#define IO_QDMA_GET_QOS						_IOR(QDMA_MAGIC, 0x02, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_TX_RATELIMIT_VALUE		_IOW(QDMA_MAGIC, 0x10, struct ECNT_QDMA_Data *)
#define IO_QDMA_GET_TX_RATELIMIT_VALUE		_IOR(QDMA_MAGIC, 0x10, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_RX_RATELIMIT_VALUE		_IOW(QDMA_MAGIC, 0x18, struct ECNT_QDMA_Data *)
#define IO_QDMA_GET_RX_RATELIMIT_VALUE		_IOR(QDMA_MAGIC, 0x18, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_RX_RATELIMIT_EN			_IOW(QDMA_MAGIC, 0x19, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_RX_RATELIMIT_PKT_MODE	_IOW(QDMA_MAGIC, 0x1A, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_VIRTUAL_CHANNEL_CONFIG	_IOW(QDMA_MAGIC, 0x20, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_VIRTUAL_CHANNEL_VALUE	_IOW(QDMA_MAGIC, 0x21, struct ECNT_QDMA_Data *)
#define IO_QDMA_GET_VIRTUAL_CHANNEL			_IOR(QDMA_MAGIC, 0x21, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_RX_LOW_THRESHOLD		_IOW(QDMA_MAGIC, 0x24, struct ECNT_QDMA_Data *)
#define IO_QDMA_GET_RX_LOW_THRESHOLD		_IOR(QDMA_MAGIC, 0x24, struct ECNT_QDMA_Data *)
#define IO_QDMA_SET_DEI_DROP_EN				_IOW(QDMA_MAGIC, 0x25, struct ECNT_QDMA_Data *)
#define IO_QDMA_GET_DEI_DROP_EN				_IOR(QDMA_MAGIC, 0x25, struct ECNT_QDMA_Data *)

#define IO_QDMA_CLEAR_CPU_COUNTER			_IOW(QDMA_MAGIC, 0xC1, struct ECNT_QDMA_Data *)
#define IO_QDMA_DUMP_CPU_COUNTER			_IOW(QDMA_MAGIC, 0xC2, struct ECNT_QDMA_Data *)
#define IO_QDMA_DUMP_REGISTER				_IOW(QDMA_MAGIC, 0xC3, struct ECNT_QDMA_Data *)
#define IO_QDMA_DUMP_DESCRIPTOR				_IOW(QDMA_MAGIC, 0xC4, struct ECNT_QDMA_Data *)
#define IO_QDMA_DUMP_IRQ					_IOW(QDMA_MAGIC, 0xC5, struct ECNT_QDMA_Data *)
#define IO_QDMA_DUMP_HWFWD					_IOW(QDMA_MAGIC, 0xC6, struct ECNT_QDMA_Data *)
#endif


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
	
struct port_info;

/***************************************
 enum definition
***************************************/
typedef enum {
	DBG_ERR ,
	DBG_ST ,
	DBG_WARN ,
	DBG_MSG ,
	DBG_LEVEL_MAX
} QDMA_DebugLevel_t ;

typedef enum {
	QDMA_CALLBACK_RX_PACKET,
#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
    QDMA_CALLBACK_RX_PACKET_PTM_LAN,
#endif
	QDMA_CALLBACK_EVENT_HANDLER,
	QDMA_CALLBACK_RECYCLE_PACKET,
	QDMA_CALLBACK_GPON_MAC_HANDLER,
	QDMA_CALLBACK_EPON_MAC_HANDLER,
	QDMA_CALLBACK_XPON_PHY_HANDLER,
	/*multi-INTs extend*/
	QDMA_CALLBACK_INT2_RX_PACKET,
	QDMA_CALLBACK_INT2_EVENT_HANDLER,
	QDMA_CALLBACK_INT3_RX_PACKET,
	QDMA_CALLBACK_INT3_EVENT_HANDLER,
	QDMA_CALLBACK_INT4_RX_PACKET,
	QDMA_CALLBACK_INT4_EVENT_HANDLER,
} QDMA_CbType_t ;

typedef enum {
	QDMA_EVENT_RECV_PKTS = 0 ,
	QDMA_EVENT_NO_RX_BUFFER ,
	QDMA_EVENT_TX_CROWDED
} QDMA_EventType_t ;

typedef enum {
	QDMA_LOOPBACK_DISABLE = 0 ,
	QDMA_LOOPBACK_QDMA ,
	QDMA_LOOPBACK_UMAC
} QDMA_LoopbackMode_t ;

typedef enum {
	QDMA_TX_POLLING = 0 ,
	QDMA_TX_INTERRUPT ,
} QDMA_RecycleMode_t ;

typedef enum {
	QDMA_RX_POLLING = 0 ,
	QDMA_RX_INTERRUPT ,
	QDMA_RX_NAPI
} QDMA_RecvMode_t ;

typedef enum {
	QDMA_DISABLE = 0 ,
	QDMA_ENABLE
} QDMA_Mode_t ;

typedef enum {
	QDMA_WAN_TYPE_GPON = 0,
	QDMA_WAN_TYPE_EPON,
	QDMA_WAN_TYPE_PTM,
	QDMA_WAN_TYPE_SAR
} QDMA_WanType_t ;

typedef enum {
	QDMA_TXQOS_WEIGHT_BY_PACKET = 0,
	QDMA_TXQOS_WEIGHT_BY_BYTE,
	QDMA_TXQOS_WEIGHT_MAX,
} QDMA_TxQosWeightType_t ;

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
typedef enum {
	QDMA_TXQOS_WEIGHT_SCALE_1B = 0,
	QDMA_TXQOS_WEIGHT_SCALE_16B,
	QDMA_TXQOS_WEIGHT_SCALE_MAX,
} QDMA_TxQosWeightScale_t ;
#else
typedef enum {
	QDMA_TXQOS_WEIGHT_SCALE_64B = 0,
	QDMA_TXQOS_WEIGHT_SCALE_16B,
	QDMA_TXQOS_WEIGHT_SCALE_MAX,
} QDMA_TxQosWeightScale_t ;
#endif

typedef enum {
	QDMA_TXQOS_TYPE_WRR = 0,
	QDMA_TXQOS_TYPE_SP,
	QDMA_TXQOS_TYPE_SPWRR7, 
	QDMA_TXQOS_TYPE_SPWRR6, 
	QDMA_TXQOS_TYPE_SPWRR5, 
	QDMA_TXQOS_TYPE_SPWRR4, 
	QDMA_TXQOS_TYPE_SPWRR3, 
	QDMA_TXQOS_TYPE_SPWRR2, 
	QDMA_TXQOS_TYPE_NUMS
} QDMA_TxQosType_t ;

typedef enum {
	QDMA_VCHNL_TXQOS_TYPE_WRR = 0,
	QDMA_VCHNL_TXQOS_TYPE_SP,
	QDMA_VCHNL_TXQOS_TYPE_SPWRR3, 
	QDMA_VCHNL_TXQOS_TYPE_SPWRR2, 
	QDMA_VCHNL_TXQOS_TYPE_NUMS
} QDMA_VirChnlQosType_t ;

typedef enum {
	QDMA_TRTCM_SCALE_1BYTE = 0,
	QDMA_TRTCM_SCALE_2BYTE,
	QDMA_TRTCM_SCALE_4BYTE, 
	QDMA_TRTCM_SCALE_8BYTE,
	QDMA_TRTCM_SCALE_16BYTE,
	QDMA_TRTCM_SCALE_32BYTE,
	QDMA_TRTCM_SCALE_64BYTE,
	QDMA_TRTCM_SCALE_128BYTE,
	QDMA_TRTCM_SCALE_256BYTE,
	QDMA_TRTCM_SCALE_512BYTE,
	QDMA_TRTCM_SCALE_1KBYTE,
	QDMA_TRTCM_SCALE_2KBYTE,
	QDMA_TRTCM_SCALE_4KBYTE,
	QDMA_TRTCM_SCALE_8KBYTE,
	QDMA_TRTCM_SCALE_16KBYTE,
	QDMA_TRTCM_SCALE_32KBYTE,
	QDMA_TRTCM_SCALE_MAX_ITEMS
} QDMA_TrtcmScale_t ;

typedef enum {
	QDMA_TRTCM_PARAM_CIR = 0,
	QDMA_TRTCM_PARAM_CBS,
	QDMA_TRTCM_PARAM_PIR,
	QDMA_TRTCM_PARAM_PBS
} QDMA_TrtcmParamType_t ;

typedef enum {
	QDMA_EPON_REPORT_WO_THRESHOLD = 0,
	QDMA_EPON_REPORT_ONE_THRESHOLD,
	QDMA_EPON_REPORT_TWO_THRESHOLD,
	QDMA_EPON_REPORT_THREE_THRESHOLD
} QDMA_EponReportMode_t ;

typedef enum {
	QDMA_TXQUEUE_SCALE_2_DSCP = 0,
	QDMA_TXQUEUE_SCALE_4_DSCP,
	QDMA_TXQUEUE_SCALE_8_DSCP,
	QDMA_TXQUEUE_SCALE_16_DSCP,
	QDMA_TXQUEUE_SCALE_ITEMS
} QDMA_TxQueueThresholdScale_t ;

typedef enum {
	PSE_PCP_TYPE_CDM_TX = 0,
	PSE_PCP_TYPE_CDM_RX, 
	PSE_PCP_TYPE_GDM_RX
} PSE_PcpType_t ;

typedef enum {
	PSE_PCP_MODE_DISABLE = 0,
	PSE_PCP_MODE_8B0D = 1,
	PSE_PCP_MODE_7B1D = 2, 
	PSE_PCP_MODE_6B2D = 4,
	PSE_PCP_MODE_5B3D = 8
} PSE_PcpMode_t ;

typedef enum {
	QDMA_RX_RATE_LIMIT_BY_BYTE = 0,
	QDMA_RX_RATE_LIMIT_BY_PACKET,
} QDMA_RxPktMode_t ;

#ifndef TCSUPPORT_CPU_EN7580
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
typedef enum {
	QDMA_Rx_Rate_Limit_PIR = 2,
	QDMA_Rx_Rate_Limit_PBS
} QDMA_RxRateLimitType_t ;
#else
typedef enum {
	QDMA_Rx_Rate_Limit_PIR = 0,
	QDMA_Rx_Rate_Limit_PBS
} QDMA_RxRateLimitType_t ;
#endif
#endif

typedef enum {
	QDMA_Tx_Rate_Limit_CIR = 0,
	QDMA_Tx_Rate_Limit_CBS,
	QDMA_Tx_Rate_Limit_PIR,
	QDMA_Tx_Rate_Limit_PBS
} QDMA_TxRateLimitType_t ;

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
typedef enum {
	QDMA_INT1 = 1,
	QDMA_INT2 ,
	QDMA_INT3 ,
	QDMA_INT4 ,
	QDMA_INT_NUM,
} QDMA_InterruptNo_t ;

typedef enum {
	QDMA_IRQ1 = 1,
	QDMA_IRQ2 ,
	QDMA_IRQ_MAX_NUM,
} QDMA_IrqNo_t ;

typedef enum {
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	QDMA_INT_ENABLE0 = 0 ,
#endif
	QDMA_INT_ENABLE1 = 1 ,
	QDMA_INT_ENABLE2 = 2 ,
#if defined(TCSUPPORT_CPU_EN7581)
	QDMA_INT_ENABLE3 = 3 ,
	QDMA_INT_ENABLE4 = 4 ,
	QDMA_INT_ENABLE5 = 5 ,
#endif

#if defined(TCSUPPORT_CPU_EN7581)
	QDMA_INT_ENABLE_NUM = 5 ,
#elif defined(TCSUPPORT_CPU_EN7580)
	QDMA_INT_ENABLE_NUM = 2 ,
#else
	QDMA_INT_ENABLE_NUM = 3 ,
#endif
} QDMA_IntEnable_t ;
#endif

/*-----TCSUPPORT_CPU_EN7580 start------*/
typedef enum{
	QDMA_DRAM_MODE=0,
	QDMA_SRAM_MODE=1,
}QDMA_RamMode_T;

/*Ingress Ratelimit or Egress Ratelimit Module*/
typedef enum{
	INGRESS_TRTCM=0,
	SLA_TRTCM,
	EGRESS_QUEUE_RATELIMIT,
	EGRESS_TRTCM,
	GLB_RATECTL,
	TRTCM_MODE_MAX,
} GENERAL_TrtcmModuleType_T;

/*ratelimit module is enable nor not*/
typedef enum{
	TRTCM_DISABLE=0,
	TRTCM_ENABLE
} GENERAL_Trtcm_T;

/*ratelimit module works as trtcm mode or ratelimit mode*/
typedef enum{
	TRTCM_RATELIMIT_MODE=0,
	TRTCM_MODE
} GENERAL_TrtcmMode_T;

/*each meter enable or not*/
typedef enum{
	GENERAL_METER_DISABLE=0,
	GENERAL_METER_ENABLE
} GENERAL_TrtcmMeter_T;

typedef enum{
	TRTCM_FAST_TICK = 0,
	TRTCM_SLOW_TICK
} GENERAL_TrtcmTickSel_T;

typedef enum{
	TRTCM_MISC = 0,  //include meter_en, ppsmode, ticksel
	TRTCM_TOKEN_RATE,
	TRTCM_BUCKETSIZE_SHIFT,
	TRTCM_BUCKET_COUNTER
} GENERAL_TrtcmParaType_T;

typedef enum{
	TRTCM_COMMIT_MODE = 0,
	TRTCM_PEAK_MODE,
	TRTCM_RATE_TYPE_MAX
} GENERAL_TrtcmRateType_T ;

typedef enum{
	TRTCM_BYTE_MODE = 0,
	TRTCM_PACKET_MODE
}GENERAL_TrtcmPktMode_T;

typedef enum{
	FLOWCNT_CLEAR_BYTE=0,
	FLOWCNT_CLEAR_PKT,
	FLOWCNT_CLEAR_ALL,
}QDMA_FlowCnt_Clear_Mode_T;

typedef enum{
	NORMAL_PACKET=0,
	DEI_PACKET,
	PACKET_TYPE_ITEMS
}QDMA_PacketType_T;

typedef enum{
	ACNT0_TYPE=0,
	ACNT1_TYPE,
	ACNT2_TYPE,
}QDMA_AcntType_T;
/*-----TCSUPPORT_CPU_EN7580 end------*/

/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 start------*/
typedef enum{
	METER0_TYPE=0,
	METER1_TYPE,
	METER2_TYPE,
}QDMA_MeterType_T;

typedef enum{
	QDMA_QOS_AGING_THRESHOLD=0, /* when used dscp num greater than total_max_threshold */
	QDMA_QOS_AGING_ALWAYS=1,    /* always */
}QDMA_AgingMethod_T;
/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 end------*/

typedef enum {
	QDMA_TX_RATE_METER_TIME_DIVISOR_1 = 0,
	QDMA_TX_RATE_METER_TIME_DIVISOR_2,
	QDMA_TX_RATE_METER_TIME_DIVISOR_4,
	QDMA_TX_RATE_METER_TIME_DIVISOR_8,
	QDMA_TX_RATE_METER_TIME_DIVISOR_ITEMS
} QDMA_TxRateMeterTimeDivisor_t ;

typedef enum {
	QDMA_DYNCNGST_DEI_THRH_SCALE_1_2 = 0,
	QDMA_DYNCNGST_DEI_THRH_SCALE_1_4,
	QDMA_DYNCNGST_DEI_THRH_SCALE_1_8,
	QDMA_DYNCNGST_DEI_THRH_SCALE_1_16,
	QDMA_DYNCNGST_DEI_THRH_SCALE_ITEMS
} QDMA_TxDynCngstDeiThrhScale_t ;

typedef enum {
	QDMA_QUEUE_NONBLOCKING = 0 ,
	QDMA_QUEUE_NORMAL
} QDMA_TxQCngstQueueMode_t ;

typedef enum {
	QDMA_CHANNEL_NONBLOCKING = 0 ,
	QDMA_CHANNEL_NORMAL
} QDMA_TxQCngstChannelMode_t ;

typedef enum {
	QDMA_DBG_CNTR_SRC_CPU_TX = 0,
	QDMA_DBG_CNTR_SRC_FWD_TX,
	QDMA_DBG_CNTR_SRC_CPU_RX,
	QDMA_DBG_CNTR_SRC_FWD_RX,
	QDMA_DBG_CNTR_CPU_TX_DROP,
	QDMA_DBG_CNTR_FWD_RX_DROP,	
	QDMA_DBG_CNTR_CPU_RX_DROP,
	QDMA_DBG_CNTR_MULTICAST_DROP,	
	QDMA_DBG_CNTR_MULTICAST_ALL_DROP,
	QDMA_DBG_CNTR_AGE_OUT_DROP,
	QDMA_DBG_CNTR_SRC_ITEMS,
} QDMA_DbgCntrSrc_t ;

typedef enum {
	QDMA_TXCNGST_DYNAMIC_NORMAL = 0,
	QDMA_TXCNGST_DYNAMIC_PEAKRATE_MARGIN,
	QDMA_TXCNGST_STATIC,
} QDMA_TxCngstMode_t ;

typedef enum {
	QDMA_TXCNGST_PEEKRATE_MARGIN_0 = 0,
	QDMA_TXCNGST_PEEKRATE_MARGIN_25,
	QDMA_TXCNGST_PEEKRATE_MARGIN_50,
	QDMA_TXCNGST_PEEKRATE_MARGIN_100,
} QDMA_PeekRateMargin_t ;


typedef enum {
	QDMA_DBG_MEM_XS_MEMSEL_LOCAL_DSCP_INFO = 0,
	QDMA_DBG_MEM_XS_MEMSEL_LOCAL_QUEUE_INFO,
	QDMA_DBG_MEM_XS_MEMSEL_QOS_WEIGHT_COUNTER,
	QDMA_DBG_MEM_XS_MEMSEL_LOCAL_DMA_INDEX,
	QDMA_DBG_MEM_XS_MEMSEL_BUFFER_MONITOR,
	QDMA_DBG_MEM_XS_MEMSEL_RATELIMIT_PARAM,
	QDMA_DBG_MEM_XS_MEMSEL_VCHNL_QOS_WEIGHT,
	QDMA_DBG_MEM_XS_MEMSEL_MAX,
} QDMA_DbgMemXsMemSel_t ;

typedef enum {
	MAC_TYPE_ETHER = 0,
	MAC_TYPE_XPON,
	MAC_TYPE_PTM,
	MAC_TYPE_ATM,
	MAC_TYPE_LRO,
} MAC_Type_t ;

typedef enum {
	QDMA_VIRTUAL_CHANNEL_16Queue = 0,
	QDMA_VIRTUAL_CHANNEL_32Queue,
} QDMA_VirChnlMapMode_t ;

typedef enum {
	QDMA_DBG_CNTR_CHNL_TXCPU = 0,
	QDMA_DBG_CNTR_CHNL_TXFWD1,
	QDMA_DBG_CNTR_CHNL_TXFWD2,
	QDMA_DBG_CNTR_QUEUE,
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	QDMA_DBG_CNTR_RING,
#endif
	QDMA_DBG_CNTR_MAX,
} QDMA_DbgCntrChnlGroup_t ;

typedef enum {
	QDMA_DBG_CNTR_MEM_CTX = 0,
	QDMA_DBG_CNTR_MEM_FTX,
	QDMA_DBG_CNTR_MEM_FRX,
	QDMA_DBG_CNTR_MEM_AGEOUT_DROP,
	QDMA_DBG_CNTR_MEM_MAX,
} QDMA_DbgCntrMemGroup_t ;

typedef enum {
	QDMA_DUMP_TX_DSCP = 0,
	QDMA_DUMP_RX_DSCP,
} QDMA_DscpDumpMode_t ;

typedef enum{
    NULLQOS = -1,
    QOS_SW_PQ =	0,	/* will use hw at the same time */
    QOS_SW_WRR = 1,
    QOS_SW_CAR = 2,
    QOS_HW_WRR = 3,
    QOS_HW_PQ = 4,
    QOS_HW_CAR = 5,
}QDMA_QoSClassfication_t ;

typedef enum{
    TXMSG_FIELD_INIT = 0,
    TXMSG_FIELD_WHOLE_MSG0,
    TXMSG_FIELD_WHOLE_MSG1,
    TXMSG_FIELD_WHOLE_MSG2,
    TXMSG_FIELD_MIC_IDX,
    TXMSG_FIELD_SPTAG,
    TXMSG_FIELD_GEMPORT,
    TXMSG_FIELD_ICO,
    TXMSG_FIELD_UCO,
    TXMSG_FIELD_TCO,
    TXMSG_FIELD_TSO,
    TXMSG_FIELD_FAST,
    TXMSG_FIELD_OAM,
    TXMSG_FIELD_CNH,
    TXMSG_FIELD_QID,
    TXMSG_FIELD_NODROP,
    TXMSG_FIELD_MTR0,
    TXMSG_FIELD_FPORT,
    TXMSG_FIELD_NBQ,
    TXMSG_FIELD_HWF,
    TXMSG_FIELD_HOP,
    TXMSG_FIELD_PTP,
    TXMSG_FIELD_ACNT0,
    TXMSG_FIELD_ACNT1,
    TXMSG_FIELD_SW_UDF,
    TXMSG_FIELD_MTR1,
    TXMSG_FIELD_MTR2,
    TXMSG_FIELD_ACNT2,
    TXMSG_FIELD_MAX_NUM
}QDMA_SET_TXMSG_FIELD_t ;

typedef enum{
    RXMSG_FIELD_WHOLE_MSG0 = 0,
    RXMSG_FIELD_WHOLE_MSG1,
    RXMSG_FIELD_WHOLE_MSG2,
    RXMSG_FIELD_WHOLE_MSG3,
    RXMSG_FIELD_SPTAG,
    RXMSG_FIELD_FOE_INDEX,
    RXMSG_FIELD_CRSN,
    RXMSG_FIELD_SPORT,
    RXMSG_FIELD_IP4F,
    RXMSG_FIELD_L4F,
    RXMSG_FIELD_L4VLD,
    RXMSG_FIELD_IP4,
    RXMSG_FIELD_IP6,
    RXMSG_FIELD_L2_LEN,
    RXMSG_FIELD_IFC_HIT,
    RXMSG_FIELD_IFC_ID,
    RXMSG_FIELD_MAX_NUM
}QDMA_GET_RXMSG_FIELD_t ;

/* Warning: same sequence with function array 'qdma_operation' */
typedef enum {
	/* init */
	/* 0x00 */
	QDMA_FUNCTION_INIT = 0,
	QDMA_FUNCTION_DEINIT ,
	QDMA_FUNCTION_TX_DMA_MODE ,
	QDMA_FUNCTION_RX_DMA_MODE ,
	QDMA_FUNCTION_LOOPBACK_MODE ,
	QDMA_FUNCTION_REGISTER ,
	QDMA_FUNCTION_UNREGISTER ,
	QDMA_FUNCTION_ENABLE_RXPKT_INT ,
	QDMA_FUNCTION_DISABLE_RXPKT_INT ,
	QDMA_FUNCTION_RECEIVE_PACKETS ,
	QDMA_FUNCTION_TRANSMIT_PACKETS ,
	QDMA_FUNCTION_SET_TX_QOS_WEIGHT ,
	QDMA_FUNCTION_GET_TX_QOS_WEIGHT ,
	QDMA_FUNCTION_SET_TX_QOS ,
	QDMA_FUNCTION_GET_TX_QOS ,
	QDMA_FUNCTION_SET_MAC_LIMIT_THRESHOLD ,
	QDMA_FUNCTION_GET_MAC_LIMIT_THRESHOLD ,
	QDMA_FUNCTION_TRANSMIT_PACKET_WIFI_FAST ,

	/* other */
	/* 0x12 */
	QDMA_FUNCTION_SET_TXBUF_THRESHOLD ,
	QDMA_FUNCTION_GET_TXBUF_THRESHOLD ,
	QDMA_FUNCTION_SET_PREFETCH_MODE ,
	QDMA_FUNCTION_SET_PKTSIZE_OVERHEAD_EN ,
	QDMA_FUNCTION_GET_PKTSIZE_OVERHEAD_EN ,
	QDMA_FUNCTION_SET_PKTSIZE_OVERHEAD_VALUE ,
	QDMA_FUNCTION_GET_PKTSIZE_OVERHEAD_VALUE ,
	QDMA_FUNCTION_SET_LMGR_LOW_THRESHOLD ,
	QDMA_FUNCTION_GET_LMGR_LOW_THRESHOLD ,
	QDMA_FUNCTION_GET_LMGR_STATUS ,

	/* test */
	/* 0x1C */
	QDMA_FUNCTION_SET_DEBUG_LEVEL,
	QDMA_FUNCTION_DUMP_DMA_BUSY,
	QDMA_FUNCTION_DUMP_REG_POLLING,
	QDMA_FUNCTION_SET_FORCE_RX_RING1,

	/* tx rate limit */
	/* 0x20 */
	QDMA_FUNCTION_SET_TX_DROP_EN,	//default enable
	QDMA_FUNCTION_GET_TX_DROP_EN,	//default enable
	QDMA_FUNCTION_SET_TX_RATEMETER,
	QDMA_FUNCTION_GET_TX_RATEMETER,
	QDMA_FUNCTION_ENABLE_TX_RATELIMIT,
	QDMA_FUNCTION_SET_TX_RATELIMIT_CFG,
	QDMA_FUNCTION_GET_TX_RATELIMIT_CFG,
	QDMA_FUNCTION_SET_TX_RATELIMIT,
	QDMA_FUNCTION_GET_TX_RATELIMIT,
	QDMA_FUNCTION_SET_TX_DBAREPORT,
	QDMA_FUNCTION_GET_TX_DBAREPORT,

	/* rx rate limit */
	/* 0x2B */
	QDMA_FUNCTION_SET_RX_PROTECT_EN,	//default enable
	QDMA_FUNCTION_GET_RX_PROTECT_EN,	//default enable
	QDMA_FUNCTION_SET_RX_LOW_THRESHOLD,
	QDMA_FUNCTION_GET_RX_LOW_THRESHOLD,
	QDMA_FUNCTION_SET_RX_RATELIMIT_EN,
	QDMA_FUNCTION_SET_RX_RATELIMIT_PKT_MODE,
	QDMA_FUNCTION_GET_RX_RATELIMIT_CFG,
	QDMA_FUNCTION_SET_RX_RATELIMIT,
	QDMA_FUNCTION_GET_RX_RATELIMIT,

	/* txq cngst */
	/* 0x34 */
	QDMA_FUNCTION_SET_TXQ_DEI_DROP_MODE,
	QDMA_FUNCTION_GET_TXQ_DEI_DROP_MODE,
	QDMA_FUNCTION_SET_TXQ_CNGST_MODE,
	QDMA_FUNCTION_GET_TXQ_CNGST_MODE,
	QDMA_FUNCTION_SET_TXQ_DEI_THRH_SCALE,
	QDMA_FUNCTION_GET_TXQ_DEI_THRH_SCALE,
	QDMA_FUNCTION_SET_TXQ_CNGST_AUTO_CONFIG,
	QDMA_FUNCTION_GET_TXQ_CNGST_AUTO_CONFIG,
	QDMA_FUNCTION_SET_TXQ_CNGST_DYNAMIC_THRESHOLD,
	QDMA_FUNCTION_GET_TXQ_CNGST_DYNAMIC_THRESHOLD,
	QDMA_FUNCTION_SET_TXQ_CNGST_TOTAL_THRESHOLD,
	QDMA_FUNCTION_GET_TXQ_CNGST_TOTAL_THRESHOLD,
	QDMA_FUNCTION_SET_TXQ_CNGST_CHANNEL_THRESHOLD,
	QDMA_FUNCTION_GET_TXQ_CNGST_CHANNEL_THRESHOLD,
	QDMA_FUNCTION_SET_TXQ_CNGST_QUEUE_THRESHOLD,
	QDMA_FUNCTION_GET_TXQ_CNGST_QUEUE_THRESHOLD,
	QDMA_FUNCTION_SET_PEEKRATE_PARAMS,
	QDMA_FUNCTION_GET_PEEKRATE_PARAMS,
	QDMA_FUNCTION_SET_TXQ_STATIC_QUEUE_NORMAL_THRESHOLD,
	QDMA_FUNCTION_SET_TXQ_STATIC_QUEUE_DEI_THRESHOLD,
	QDMA_FUNCTION_GET_TXQ_CNGST_DYNAMIC_INFO,
	QDMA_FUNCTION_GET_TXQ_CNGST_STATIC_INFO,
	QDMA_FUNCTION_SET_TXQ_CNGST_QUEUE_NONBLOCKING,
	QDMA_FUNCTION_GET_TXQ_CNGST_QUEUE_NONBLOCKING,
	QDMA_FUNCTION_SET_TXQ_CNGST_CHANNEL_NONBLOCKING,
	QDMA_FUNCTION_GET_TXQ_CNGST_CHANNEL_NONBLOCKING,

	/* virtual channel */
	/* 0x4E */
	QDMA_FUNCTION_SET_VIRTUAL_CHANNEL_MODE,
	QDMA_FUNCTION_GET_VIRTUAL_CHANNEL_MODE,
	QDMA_FUNCTION_SET_VIRTUAL_CHANNEL_QOS,
	QDMA_FUNCTION_GET_VIRTUAL_CHANNEL_QOS,

	/* dbg cntr */
	/* 0x52 */
	QDMA_FUNCTION_SET_DBGCNTR_CHANNEL,
	QDMA_FUNCTION_SET_DBGCNTR_QUEUE,
	QDMA_FUNCTION_CLEAR_DBGCNTR,
	QDMA_FUNCTION_DUMP_DBGCNTR,
	QDMA_FUNCTION_SET_CHANNEL,
	QDMA_FUNCTION_DUMP_CHANNEL,
	QDMA_FUNCTION_GET_ALL_QUEUE_DBGCNTR,
	QDMA_FUNCTION_GET_RX_RING_CNT,

	/* dump */
	/* 0x5A */
	QDMA_FUNCTION_DUMP_TX_QOS,
	QDMA_FUNCTION_DUMP_VIRTUAL_CHANNEL_QOS,
	QDMA_FUNCTION_DUMP_TX_RATELIMIT,
	QDMA_FUNCTION_DUMP_RX_RATELIMIT,
	QDMA_FUNCTION_DUMP_TX_DBA_REPORT,
	QDMA_FUNCTION_DUMP_TXQ_CNGST,
	/* 0x60 */
	QDMA_FUNCTION_CLEAR_CPU_COUNTER,
	QDMA_FUNCTION_DUMP_CPU_COUNTER,
	QDMA_FUNCTION_DUMP_REGISTER_INFO,
	QDMA_FUNCTION_DUMP_DESCRIPTOR_INFO,
	QDMA_FUNCTION_DUMP_IRQ_INFO,
	QDMA_FUNCTION_DUMP_HWFWD_INFO,
	QDMA_FUNCTION_DUMP_INFO_ALL,
	
	/*other function*/
	/* 0x67 */
	QDMA_FUNCTION_READ_VIP_INFO,
	QDMA_FUNCTION_SET_MAC_QOS_CONFIG,
	/* 0x69 */
	QDMA_FUNCTION_GENERAL_SET_TRTCM_CFG,
	QDMA_FUNCTION_GENERAL_GET_TRTCM_CFG,
	QDMA_FUNCTION_GENERAL_SET_RATELIMIT_MODE_CFG,
	QDMA_FUNCTION_GENERAL_GET_RATELIMIT_MODE_CFG,
	QDMA_FUNCTION_GENERAL_SET_RATELIMIT_MODE_VALUE,
	QDMA_FUNCTION_GENERAL_GET_RATELIMIT_MODE_VALUE,
	QDMA_FUNCTION_GENERAL_SET_RATELIMIT_BUCKET_SIZE,

	/*EN7527/16 new add APIs*/
	/* 0x70 */
	QDMA_FUNCTION_SET_DBGCNTR_RING,

	/*EN7580 new add APIs*/
	/* 0x71 */
	QDMA_FUNCTION_ENABLE_RXPKT_INT2 ,
	QDMA_FUNCTION_DISABLE_RXPKT_INT2 , 
	QDMA_FUNCTION_RECEIVE_PACKETS_INT2 ,
	/* 0x74 */
	QDMA_FUNCTION_GENERAL_SET_TRTCM_MODE_CFG,
	QDMA_FUNCTION_GENERAL_GET_TRTCM_MODE_CFG,
	QDMA_FUNCTION_GENERAL_SET_TRTCM_MODE_VALUE,
	QDMA_FUNCTION_GENERAL_GET_TRTCM_MODE_VALUE,
	QDMA_FUNCTION_GENERAL_SET_TRTCM_BUCKET_SIZE,
	/* 0x79 */
	QDMA_FUNCTION_SET_FLOW_CNTR_CFG,
	QDMA_FUNCTION_GET_FLOW_CNTR_CFG,
	QDMA_FUNCTION_GET_FLOW_CNTR_VALUE,
	QDMA_FUNCTION_CLEAR_FLOW_CNTR_VALUE,
	QDMA_FUNCTION_SET_TX_WRED_MODE,
	QDMA_FUNCTION_GET_TX_WRED_MODE,
	QDMA_FUNCTION_SET_TX_WRED_THRESHOLD,
	QDMA_FUNCTION_SET_TX_WRED_PROBABILITY,
	QDMA_FUNCTION_GET_TX_WRED_CFG,
	QDMA_FUNCTION_SET_CPU_RX_RED_PROBABILITY,
	QDMA_FUNCTION_GET_CPU_RX_RED_PROBABILITY,
	/* 0x84 */
	QDMA_FUNCTION_SET_CHANNEL_CLOSE_STATUS,
	QDMA_FUNCTION_GET_CHANNEL_CLOSE_STATUS,
	QDMA_FUNCTION_GET_CHANNEL_EMPTY_STATUS,
	/* 0x87 */
	QDMA_FUNCTION_SET_OAM_MODIFY_FP_EN,
	QDMA_FUNCTION_GET_OAM_MODIFY_FP_EN,
	QDMA_FUNCTION_SET_MULTICAST_EN,
	QDMA_FUNCTION_GET_MULTICAST_EN,
	/* 0x8B */
	QDMA_FUNCTION_ALLOCATE_METER,
	QDMA_FUNCTION_FREE_METER,
	QDMA_FUNCTION_GET_QOS_FLAG,
	QDMA_FUNCTION_ALLOCATE_ACNT,
	QDMA_FUNCTION_FREE_ACNT,

	/*EN7581 / EN7523: new APIs*/
	/* 0x90 */
	QDMA_FUNCTION_SET_SLA_CHANNEL_CFG,
	QDMA_FUNCTION_GET_SLA_CHANNEL_CFG,
	QDMA_FUNCTION_SET_QOS_AGING_CFG,
	QDMA_FUNCTION_GET_QOS_AGING_CFG,
	QDMA_FUNCTION_SET_PER_QUEUE_AGING_CFG,
	QDMA_FUNCTION_GET_PER_QUEUE_AGING_CFG,
	QDMA_FUNCTION_SET_MULTICAST_SPTAG_CFG,
	QDMA_FUNCTION_GET_MULTICAST_SPTAG_CFG,
	QDMA_FUNCTION_SET_MULTICAST_FPORT_CFG,
	QDMA_FUNCTION_GET_MULTICAST_FPORT_CFG,
	QDMA_FUNCTION_SET_TXQ_STATIC_CHANNEL_EN,
	QDMA_FUNCTION_SET_TXQ_STATIC_QUEUE_RATIO,
	QDMA_FUNCTION_GET_HQOS_EN,

	/* 0x9D */
	QDMA_FUNCTION_RESET,
	QDMA_FUNCTION_DRAM_TEST_DMA_CONFIG,
	QDMA_FUNCTION_DRAM_TEST_DMA_ENABLE,
	QDMA_FUNCTION_DRAM_TEST_RX_DONE,
	QDMA_FUNCTION_DRAM_TEST_DST_GET,
	QDMA_FUNCTION_GET_DSCP_CHNL_INFO ,
	QDMA_FUNCTION_SET_MULTICAST_1TON_CFG,
	QDMA_FUNCTION_GET_CHANNEL_CFG,
	QDMA_FUNCTION_SET_TXMSG ,
	QDMA_FUNCTION_GET_RXMSG ,
	QDMA_FUNCTION_GET_RXMSG_ETH ,
	QDMA_FUNCTION_CHECK_DSCP_IS_FREE ,
	QDMA_FUNCTION_SET_DOWNSTREAM_QOS ,
	QDMA_FUNCTION_GET_DOWNSTREAM_QOS ,
	/* 0xAB */
	QDMA_FUNCTION_MAX_NUM ,
} QDMA_HookFunction_t ;

typedef struct {
	int ring_idx;
	uint tcp_ts_rply;
	uint tcp_ack_seq;
} QDMA_RxInfo_T ;

typedef int (*qdma_callback_recv_packet_with_info_t)(void *, uint, struct sk_buff *, uint, QDMA_RxInfo_T *) ;
typedef int (*qdma_callback_recv_packet_t)(void *, uint, struct sk_buff *, uint) ;
typedef int (*qdma_callback_event_handler_t)(QDMA_EventType_t) ;
typedef void (*qdma_callback_int_handler_t)(void) ;
typedef int (*qdma_callback_recycle_packet_t)(struct sk_buff *, uint) ;

typedef struct {
    MAC_Type_t macType ;
    unchar txRecycleThrh ;
    qdma_callback_recv_packet_t		cbRecvPkts ;
#if defined(TCSUPPORT_FTTDP_V2) && defined(QDMA_LAN)
    qdma_callback_recv_packet_t		cbRecvPktsPtmLan ;
#endif
    qdma_callback_event_handler_t	cbEventHandler ;
    qdma_callback_recycle_packet_t	cbRecyclePkts ;
    qdma_callback_int_handler_t		cbGponMacHandler ;
    qdma_callback_int_handler_t		cbEponMacHandler ;
    qdma_callback_int_handler_t		cbXponPhyHandler ;

    qdma_callback_recv_packet_with_info_t	cbInt2RecvPkts ;
    qdma_callback_event_handler_t               cbInt2EventHandler ;
    qdma_callback_recv_packet_with_info_t	cbInt3RecvPkts ;
    qdma_callback_event_handler_t               cbInt3EventHandler ;
    qdma_callback_recv_packet_with_info_t	cbInt4RecvPkts ;
    qdma_callback_event_handler_t               cbInt4EventHandler ;
} QDMA_InitCfg_t ;

typedef struct {
	QDMA_Mode_t	mode ;
	ushort		chnThreshold ;
	ushort		totalThreshold ;
} QDMA_TxBufCtrl_T ;

typedef struct {
	unchar					channel ;
	QDMA_TxQosType_t		qosType ;
	struct {
		ushort				weight ;		//0 for don't care
	} queue[CONFIG_QDMA_QUEUE] ;
} QDMA_TxQosScheduler_T ;

typedef struct {
	unchar	channel ;
	ushort	cir ;
	ushort	cbs ;
	ushort	pir ;
	ushort	pbs ;
} QDMA_TrtcmParam_T ;

typedef struct {
	QDMA_TxQueueThresholdScale_t	maxScale ;
	QDMA_TxQueueThresholdScale_t	minScale ;
} QDMA_TxQueueCongestScale_T ;

typedef struct {
	unchar deiScale;
	struct {
		unchar			staticDeiThreshold ;
		unchar			staticNormalThreshold ;
	} queue[CONFIG_QDMA_QUEUE] ;
} QDMA_TxQueueCongestCfg_T ;

typedef struct {
	uint	normalThrh[CONFIG_QDMA_QUEUE] ;	// unit is kByte.
} QDMA_TxQStaticNormalCfg_T ;

typedef struct {
	uint	deiThrh[CONFIG_QDMA_QUEUE] ;	// unit is kByte.
} QDMA_TxQStaticDeiCfg_T ;

/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 start------*/
typedef struct {  
    unchar				chnlIdx; 
    QDMA_Mode_t			staticThrEn;
}QDMA_TxQStaticCfg_T ;

typedef struct {  
    unchar				queueThrRatio[CONFIG_QDMA_QUEUE] ;
}QDMA_TxQStaticThrRatio_T ;

typedef struct {  
    unchar						slaIdx; /* support 4 SLA channels at most */
    QDMA_Mode_t                 slaEn;
    unchar  		            chnlIdx;
}QDMA_SlaChnlCfg_T ;

typedef struct {  
    QDMA_Mode_t					agingEn;
    QDMA_AgingMethod_T          agingMethod;
    QDMA_Mode_t  		        agingFastReplaceEn;
	uint						agingTime; /*us*/
}QDMA_QoSAgingCfg_T ;

typedef struct {  
    unchar				chnlIdx;
	unchar				queueIdx;
    QDMA_Mode_t			queueAgingEn;
}QDMA_TxQAgingCfg_T ;

typedef struct {
	unchar chnlIdx;
	unchar sptagKeepHiEn;
	ushort sptag;
}QDMA_MulticastSptagCfg_T ;

typedef struct {
	unchar chnlIdx;
	ushort fport;
}QDMA_MulticastFportCfg_T ;
/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 end------*/

typedef struct {
	unchar				cntrIdx ;
	unchar				cntrEn ;
	QDMA_DbgCntrSrc_t	cntrSrc ;
	unchar				isChnlAll ;
	unchar				isQueueAll ;
	unchar				isDscpRingAll ;
	unchar				chnlIdx ;
	unchar				queueIdx ;
	unchar				dscpRingIdx ;
	uint				cntrVal ;
} QDMA_DBG_CNTR_T ;

/*-----TCSUPPORT_CPU_EN7580 start------*/
typedef struct {
	GENERAL_TrtcmModuleType_T 	trtcmModule;		/*set Ingress or Egress*/
	GENERAL_Trtcm_T 			trtcmEn;			/*enable trtcm function or not*/
	GENERAL_TrtcmMode_T 		trtcmMode;			/*work as trtcm mode or ratelimit mode*/
	uint						trtcmSlowTickRatio;	/*slow_tick = slow_tickratio X fast_tick*/
	uint						trtcmFastTick;		/*unit: us*/
}GENERAL_TrtcmCfg_T ;

/*for set ratelimit mode cfg*/
typedef struct {
	GENERAL_TrtcmModuleType_T 	trtcmModule;
	ushort  					Index;				/*Egress: means channel ; Ingress: means ring or meter; SLA: SLA_idx+queue*/
	GENERAL_TrtcmMeter_T		MeterEn;
	GENERAL_TrtcmPktMode_T 		PktMode;			/*Packet mode or Byte mode*/
	GENERAL_TrtcmTickSel_T 		TickSel;			/*slow tick or fast tick*/
}GENERAL_TrtcmRatelimitCfg_T ;

/*for set ratelimit mode value*/
typedef struct {
	GENERAL_TrtcmModuleType_T 	trtcmModule;
	ushort  					Index;				/*Egress: means channel ; Ingress: means ring or meter; SLA: SLA_idx+queue*/
	uint						RateLimitValue; 	/* for ratelimit mode */
	uint						BucketSize; 
}GENERAL_TrtcmRatelimitSet_T ;

/*for set trtcm modet cfg*/
typedef struct {
	GENERAL_TrtcmModuleType_T 	trtcmModule;
	ushort  					Index;				/*Egress: means channel ; Ingress: means ring or meter; SLA: SLA_idx+queue*/
	GENERAL_TrtcmMeter_T		CbsMeterEn;
	GENERAL_TrtcmPktMode_T 		CbsPktMode;			/*Packet mode or Byte mode*/
	GENERAL_TrtcmTickSel_T 		CbsTickSel;			/*slow tick or fast tick*/
	GENERAL_TrtcmMeter_T		PbsMeterEn;
	GENERAL_TrtcmPktMode_T 		PbsPktMode;			/*Packet mode or Byte mode*/
	GENERAL_TrtcmTickSel_T 		PbsTickSel;			/*slow tick or fast tick*/
}GENERAL_TrtcmCbsPbsCfg_T ;

/*for set trtcm modet value*/
typedef struct {
	GENERAL_TrtcmModuleType_T 	trtcmModule;
	ushort  					Index;				/*Egress: means channel ; Ingress: means ring or meter; SLA: SLA_idx+queue*/
	uint						CbsParamValue;	 	/* for trtcm mode, CIR value */
	uint						PbsParamValue;	 	/* for trtcm mode, PIR value */
	uint						CbsBucketSize;
	uint						PbsBucketSize;
}GENERAL_TrtcmCbsPbsSet_T ;

/*for flow cnt config inc drop or not*/
typedef struct {
    unchar  					grpIdx;
    unchar  					enable;
}QDMA_FlowCntIncDropCfg_T ;

typedef struct {
    unchar  					grpIdx;
    unchar  					cntIdx;
    uint                        byteLow;
    uint                        byteHigh;
    uint                        pktCnt;
	uint                        pktHigh;
}QDMA_FlowCntValueGet_T ;

typedef struct {
    unchar  					grpIdx;
    unchar  					cntIdx;
    QDMA_FlowCnt_Clear_Mode_T   clearMode;
}QDMA_FlowCntValueClear_T ;

typedef struct {
    unchar  					chnlIdx;
    /*bit0~7, means queue0~7*/
    unchar  					chnlStatus;
}QDMA_ChannelStatus_T ;

typedef struct {
    char bind_func_name[MAX_NAME_LENGTH];
    int meter_id;
    QDMA_MeterType_T meter_type;
    int Qos_flag;
}QDMA_MeterManage_T ;

typedef struct {
    char bind_func_name[MAX_NAME_LENGTH];
    int acnt_id;
    QDMA_AcntType_T acnt_type; 
}QDMA_AcntManage_T ;

typedef struct {
    QDMA_Mode_t                 txWredEn;
    QDMA_PacketType_T           txWredPacketType;
    unchar  		            txWredThrProbability[TX_WRED_PROBABILITY_NUM];
    unchar  		            txWredThrValue[TX_WRED_THR_NUM];
}QDMA_TxWredCfg_T ;

typedef struct {  
    QDMA_PacketType_T           rxRedPacketType;
    unchar  		            rxRedThrProbability;
    unchar  		            rxRedThrValue[RX_RING_NUM];
}QDMA_RxRedCfg_T ;
/*-----TCSUPPORT_CPU_EN7580 end------*/

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
typedef struct {
	ushort				txRateMeterWindowSize ;
	ushort				txRateMeterTimeSlice ;
} QDMA_TxRateMeter_T ;
#else
typedef struct {
	ushort				txRateMeterTimeSlice ;
	QDMA_TxRateMeterTimeDivisor_t	txRateMeterTimeDivisor ;
} QDMA_TxRateMeter_T ;
#endif

typedef struct {
	ushort				txRateLimitUnit[PER_CHNL_TICKSEL_NUM] ; //unit is kbps, scope is (1~65)
	QDMA_TrtcmScale_t	txRateLimitBucketScale ;
} QDMA_TxRateLimitCfg_T ;

typedef struct {
	unchar				chnlIdx ;
	unchar				chnlRateLimitEn ;
	uint				rateLimitValue ; //unit is kbps
} QDMA_TxRateLimitSet_T ;

typedef struct {
	unchar				chnlIdx ;
	unchar				chnlRateLimitEn ;
	uint				rateLimitValue ; //unit is kbps
	uint				pbsParamValue ;	
} QDMA_TxRateLimitGet_T ;

typedef struct {
	unchar				tsid ;
	ushort				cirParamValue ; //The unit of CIR, PIR is 64Kbps
	ushort				cbsParamValue ; //The default trtcm scale of CBS,PBS is 128 Byte
	ushort				pirParamValue ; 
	ushort				pbsParamValue ;
} QDMA_TxQueueTrtcm_T ;

typedef struct {
	unchar 			channel ;
	unchar			queue ;
	unchar			thrIdx ;
	ushort			value ;
} QDMA_EponQueueThreshold_T ;

typedef struct {
	unchar				channel ;
	uint				cirParamValue ; // unit  is Kbps
	uint				cbsParamValue ; // unit is Byte
	uint				pirParamValue ; // unit is Kbps
	uint				pbsParamValue ; // unit is Byte
} QDMA_TxDbaReport_T ;

#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
typedef struct {
	uint				rxRingLowThrh[RX_RING_NUM] ;

} QDMA_RxLowThreshold_T ;
#else

typedef struct {
	uint				rxRing0LowThrh ;
	uint				rxRing1LowThrh ;
} QDMA_RxLowThreshold_T ;
#endif
typedef struct {
	unchar				rxRateLimitEn ;
	QDMA_RxPktMode_t	rxRateLimitPktMode ;
	ushort				rxRateLimitUnit ; //unit is kbps, scope is (1~65), or pps, scope is (16~1000)
	QDMA_TrtcmScale_t	rxRateLimitBucketScale ;
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	ushort				rxRateLimitUnit0;
#endif
} QDMA_RxRateLimitCfg_T ;

typedef struct {
	unchar				ringIdx ;
	uint				rateLimitValue ; //unit is kbps or pps
} QDMA_RxRateLimitSet_T ;

typedef struct {
	unchar				ringIdx ;
	uint				rateLimitValue ; //unit is kbps or pps
	uint				pbsParamValue ;	
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
	unchar				rxBindTicker;
#endif
} QDMA_RxRateLimitGet_T ;

typedef struct {
	unchar				cngstDropEn ;
	unchar				cngstDeiDropEn ;
	unchar				dynCngstEn ;
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7516)
#if defined(TCSUPPORT_CPU_EN7580)
	unchar				cngstWredEn ;
#endif
	unchar				dynCngstMaxThrhTxEn[TX_RING_NUM] ;
	unchar				dynCngstMinThrhTxEn[TX_RING_NUM];
#else
	unchar				dynCngstMaxThrhTx1En ;
	unchar				dynCngstMinThrhTx1En ;
	unchar				dynCngstMaxThrhTx0En ;
	unchar				dynCngstMinThrhTx0En ;
#endif
	unchar				dynCngstModeConfigTrigEn ;
	unchar				dynCngstModePacketTrigEn ;
	unchar				dynCngstModeTimeTrigEn ;
	ushort				dynCngstTicksel ;
	QDMA_TxDynCngstDeiThrhScale_t	dynCngstDeiThrhScale ;
} QDMA_TxQCngstCfg_T ;

typedef struct {
	uint				dynCngstTotalMaxThrh ;	// unit is kByte. EN7581 || EN7523: DSCP number
	uint				dynCngstTotalMinThrh ;	// unit is kByte. EN7581 || EN7523:: DSCP number
} QDMA_TxQDynCngstTotalThrh_T ;

typedef struct {
	uint				dynCngstChnlMaxThrh ;	// unit is kByte. EN7581 || EN7523:: DSCP number
	uint				dynCngstChnlMinThrh ;	// unit is kByte. EN7581 || EN7523:: DSCP number
} QDMA_TxQDynCngstChnlThrh_T ;

typedef struct {
	uint				dynCngstQueueMaxThrh ;	// unit is kByte. EN7581 || EN7523:: DSCP number
	uint				dynCngstQueueMinThrh ;	// unit is kByte. EN7581 || EN7523:: DSCP number
} QDMA_TxQDynCngstQueueThrh_T ;

typedef struct {
	ushort				dynCngstTotalMaxThrh ;
	ushort				dynCngstTotalMinThrh ;
	ushort				dynCngstChnlMaxThrh ;
	ushort				dynCngstChnlMinThrh ;
	ushort				dynCngstQueueMaxThrh ;
	ushort				dynCngstQueueMinThrh ;
#if defined(TCSUPPORT_CPU_EN7580)
	ushort				dynCngstMinDscpThrh ;
#endif
} QDMA_TxQDynCngstThrh_T ;

typedef struct {
	unchar				queueIdx ;
	ushort				staticDeiThrh ;
	ushort				staticNormalThrh ;
} QDMA_TxQStaticCngstThrh_T ;

typedef struct {
	QDMA_TxCngstMode_t				txCngstMode ;
	QDMA_PeekRateMargin_t			peekRateMargin ;	// only used in peek rate mode
	uint							peekRateDuration ;	// only used in peek rate mode, unit is ms
} QDMA_txCngstCfg_t ;

typedef struct {
	QDMA_Mode_t						peekRateEn ;
	QDMA_PeekRateMargin_t			peekRateMargin ;	// only used in peek rate mode
	uint							peekRateDuration ;	//only used in peek rate mode, unit is ms
} QDMA_PeekRateCfg_t ;

typedef struct {	
	QDMA_TxQDynCngstTotalThrh_T *totalThrhPtr;
	QDMA_TxQDynCngstChnlThrh_T *chnlThrhPtr;
	QDMA_TxQDynCngstQueueThrh_T *queueThrhPtr;
	QDMA_PeekRateCfg_t *peekrateCfgPtr;
} QDMA_TxQDynamicCngstInfo_T ;

typedef struct {	
	QDMA_TxQDynCngstTotalThrh_T *totalThrhPtr;
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
	QDMA_TxQStaticCfg_T *staticThrhCfgPtr;
	QDMA_TxQStaticThrRatio_T *staticThrhRatioPtr;
#else
	QDMA_TxQStaticNormalCfg_T *normThrhPtr;
	QDMA_TxQStaticDeiCfg_T *deiThrhPtr;
#endif
} QDMA_TxQStaticCngstInfo_T ;

typedef struct {	
	QDMA_TxQCngstQueueMode_t queueMode;
	uint queue;
} QDMA_TxQCngstQueueCfg_T ;

typedef struct {	
	QDMA_TxQCngstChannelMode_t channelMode;
	uint channel;
} QDMA_TxQCngstChannelCfg_T ;

typedef struct {
	uint queueAllCount[CONFIG_QDMA_QUEUE];
} QDMA_TxQCngstAllQueueValue_T ;

typedef struct {	
	QDMA_Mode_t virChnlEn;
	QDMA_VirChnlMapMode_t virChnlMode;
} QDMA_VirtualChannelMode_T ;

typedef struct {
	unchar					phyChnl ;
	QDMA_VirChnlQosType_t	qosType ;
	struct {
		unchar				weight ;		//0 for don't care
	} queue[8] ;
} QDMA_VirtualChannelQoS_T ;

typedef struct {
	QDMA_CbType_t type;
	void *pCbFun;
} QDMA_RegisterCallBack_T ;

typedef struct {
    uint txmsg0;
    uint txmsg1;
    uint txmsg2;
    uint txmsg3;
}QDMA_TxMsg_Ex_T;

typedef struct {
	struct sk_buff *skb;
	QDMA_TxMsg_Ex_T *pTxMsg;
	struct port_info *pMacInfo;
} QDMA_Transmit_T ;

typedef struct {
    unchar hopflags:3;
    unchar ptp:1;
    ushort sptag;
    ushort foe_index;
    unchar crsn;
    unchar sport;
    unchar l4f:1;
    unchar l4vld:1;
    unchar ip4f:1;
    unchar ip4:1;
    unchar ip6:1;
    unchar rsv:3;
    unchar swudf:7;
    uint rxmsg0;
    uint rxmsg1;
    uint rxmsg2;
}QDMA_RxMsg_Ether_T;

typedef struct {
	struct sk_buff *skb;
	int ringIdx;
} QDMA_Transmit_Wifi_Fast_T ;

typedef struct {
	QDMA_TxQosWeightType_t weightBase;
	QDMA_TxQosWeightScale_t weightScale;
	QDMA_TxQosScheduler_T *pTxQos;
} QDMA_TxQos_T;

typedef struct {
	QDMA_Mode_t prefecthMode;
	QDMA_Mode_t overDragMode;
	uint overDragCnt;
} QDMA_PrefetchMode_T;

typedef struct {
	uint freeLmgr;
	uint usedLmgr;
	uint usedBuf;
} QDMA_LmgrStatus_T;

typedef struct {
	uint dbgLevel;
	uint busyDuration; /* unit is second */
	uint regOffset;
	uint pollingDuration; /* unit is second */
	uint forceEn;
} QDMA_OldProc_T;

typedef struct {
	uint chnlThrh;
	uint queueThrh;
} QDMA_MacLimitThrh_T;

typedef struct {
	QDMA_DscpDumpMode_t dscpDumpMode;
	unchar              ringIdx;
} QDMA_DscpDumpInfo_T;

typedef struct {
   QDMA_QoSClassfication_t qosClsfy;
   int queueMask;
   int qosWrrInfo[4];
}QDMA_MacQoSCfg_T;

typedef struct {
   unchar *dataP; 	/* data pointer for transmit */
   int dataLen;		/* data length */
   struct sk_buff *skb; /* for DMA dst get */
   unchar rxDone;		/* if has rx packet */
}QDMA_DramTestCfg_T;

typedef struct {
	uint chnlNum;
	unsigned char channel[CONFIG_QDMA_CHANNEL];
} QDMA_DscpChnlInfo_T;

typedef struct {
	uint rx_ring[RX_RING_NUM];
} QDMA_RX_Ring_Cnt_T;

typedef struct {
    QDMA_TxMsg_Ex_T* pTxMsg;
    QDMA_SET_TXMSG_FIELD_t field;
    uint value;
}QDMA_TxMsg_Cfg_T;

typedef struct {
    void* pRxMsg;
    QDMA_GET_RXMSG_FIELD_t field;
    uint* pValue;
}QDMA_RxMsg_Cfg_T;

typedef struct {
    void* pRxMsg;
    QDMA_RxMsg_Ether_T* p_rxmsg_eth;
}QDMA_RxMsg_Cfg_Eth_T;

struct ECNT_QDMA_Data {
	QDMA_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	
	union {
		QDMA_InitCfg_t *pInitCfg;
		QDMA_Mode_t mode;
		QDMA_LoopbackMode_t lbMode;
		QDMA_DbgCntrChnlGroup_t cntrSrc;
		int cnt;
		int channel;
		uint threshold;
		
		QDMA_RegisterCallBack_T qdma_reg_cb;
		QDMA_Transmit_T qdma_transmit;
		QDMA_Transmit_Wifi_Fast_T qdma_transmit_wifi_fast;
		QDMA_TxQos_T qdma_tx_qos;
		QDMA_TxBufCtrl_T *pQdmaTxBufCtrl;
		QDMA_PrefetchMode_T *qdma_prefetch;
		QDMA_LmgrStatus_T *qdma_lmgr_status;
		QDMA_OldProc_T qdma_old_proc;
		QDMA_MacLimitThrh_T *qdma_mac_limit_thrh;
		QDMA_DscpChnlInfo_T *qdma_dscp_chnl_info;

		/*-----TCSUPPORT_CPU_EN7580 start------*/
		GENERAL_TrtcmCfg_T *generalTrtcmCfgPtr;
		GENERAL_TrtcmRatelimitCfg_T *generalTrtcmRatelimitCfgPtr;
		GENERAL_TrtcmRatelimitSet_T *generalTrtcmRatelimitSetPtr;
		GENERAL_TrtcmCbsPbsCfg_T *generalTrtcmCbsPbsCfgPtr;
		GENERAL_TrtcmCbsPbsSet_T *generalTrtcmCbsPbsSetPtr;

		QDMA_FlowCntIncDropCfg_T *flowCntIncDropCfg;
		QDMA_FlowCntValueGet_T *flowCntValueGet;
		QDMA_FlowCntValueClear_T *flowCntValueClear;
		QDMA_TxWredCfg_T    *txWredCfgPtr;
		QDMA_RxRedCfg_T     *rxRedCfgPtr;
        
		QDMA_ChannelStatus_T *chnlCloseStatusSet;
		QDMA_ChannelStatus_T *chnlCloseStatusGet;
		QDMA_ChannelStatus_T *chnlEmptyStatusGet;

        QDMA_MeterManage_T *meterManage;
        QDMA_AcntManage_T  *acntManage;
		/*-----TCSUPPORT_CPU_EN7580 end------*/

		/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 start------*/
		QDMA_SlaChnlCfg_T *slaChnlCfgPtr;
		QDMA_QoSAgingCfg_T *qosAgingCfgPtr;
		QDMA_TxQAgingCfg_T *txqAgingCfgPtr;
		QDMA_MulticastSptagCfg_T *multicastSptagCfgPtr;
		QDMA_MulticastFportCfg_T *multicastFportCfgPtr;
		QDMA_RX_Ring_Cnt_T  *rxRingCntPtr;
		/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 end------*/
		
		QDMA_TxRateMeter_T *txRateMeterPtr;
		QDMA_TxRateLimitCfg_T *txRateLimitCfgPtr;
		QDMA_TxRateLimitSet_T *txRateLimitSetPtr;
		QDMA_TxRateLimitGet_T *txRateLimitGetPtr;
		QDMA_TxDbaReport_T *txDbaReportPtr;
		
		QDMA_RxLowThreshold_T *rxLowThresholdPtr;
		QDMA_RxPktMode_t pktMode;
		QDMA_RxRateLimitCfg_T *rxRateLimitCfgPtr;
		QDMA_RxRateLimitSet_T *rxRateLimitSetPtr;
		QDMA_RxRateLimitGet_T *rxRateLimitGetPtr;
		
		QDMA_txCngstCfg_t *pTxCngstCfg;
		QDMA_TxQDynCngstThrh_T *dynCngstThrhPtr;
		QDMA_TxQDynCngstTotalThrh_T *totalThrhPtr;
		QDMA_TxQDynCngstChnlThrh_T *chnlThrhPtr;
		QDMA_TxQDynCngstQueueThrh_T *queueThrhPtr;
		QDMA_PeekRateCfg_t *peekrateCfgPtr;

		QDMA_TxQStaticDeiCfg_T *deiThrhPtr;
		QDMA_TxQStaticNormalCfg_T *normThrhPtr;
		/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 start------*/
		QDMA_TxQStaticCfg_T *staticThrhCfgPtr;
		QDMA_TxQStaticThrRatio_T *staticThrhRatioPtr;
		/*-----TCSUPPORT_CPU_EN7581 || TCSUPPORT_CPU_EN7523 end------*/

		QDMA_TxQDynamicCngstInfo_T *dynCfgPtr;
		QDMA_TxQStaticCngstInfo_T *staticCfgPtr;

		QDMA_VirtualChannelMode_T *virChnlModePtr;
		QDMA_VirtualChannelQoS_T *virChnlQoSPtr;
		QDMA_TxQCngstQueueCfg_T *txqCngstQueueCfgPtr;
		QDMA_TxQCngstChannelCfg_T *txqCngstChannelCfgPtr;

        QDMA_DscpDumpInfo_T *dscpDumpInfoPtr;
        QDMA_MacQoSCfg_T *macQosCfg;
		QDMA_TxQCngstAllQueueValue_T *txqCngstAllQueueValuePtr;

		QDMA_DramTestCfg_T *dramTestCfgPtr;
		QDMA_TxMsg_Cfg_T qdmaSetTxMsgCfg;
		QDMA_RxMsg_Cfg_T qdmaGetRxMsgCfg;
		QDMA_RxMsg_Cfg_Eth_T qdmaGetRxMsgCfgEth;
	} qdma_private;
};

typedef union
{
    struct
    {
#ifdef __BIG_ENDIAN
		uint wifi		: 1 ;
		uint mic_idx	: 1 ;
		uint gem_port_id: 16 ;
		uint ico		: 1 ;
		uint uco		: 1 ;
		uint tco		: 1 ;
		uint tso		: 1 ;
		uint fast		: 1 ;
		uint oam		: 1 ;
		uint channel	: 5 ;
		uint queue		: 3 ;
#else
		uint queue		: 3 ;
		uint channel	: 5 ;
		uint oam		: 1 ;
		uint fast		: 1 ;
		uint tso		: 1 ;
		uint tco		: 1 ;
		uint uco		: 1 ;
		uint ico		: 1 ;
		uint gem_port_id: 16 ;
		uint mic_idx	: 1 ;
		uint wifi		: 1 ;
#endif
    }raw0;	/*format for en7580*/
    struct
    {
#ifdef __BIG_ENDIAN
		uint              : 4;
		uint sp_tag       : 16;
		uint oam          : 1;
		uint channel      : 8;
		uint queue        : 3;
#else
		uint queue        : 3;
		uint channel      : 8;
		uint oam          : 1;
		uint sp_tag       : 16;
		uint              : 4;
#endif
    }raw1;/*format for en7528*/
	struct
	{
		uint reserve1     : 7;
		uint dei          : 1;
		uint gemport_id     : 12;
		uint oam          : 1;
		uint channel      : 8;
		uint queue        : 3;
	}raw2;/*format for en7526*/
    uint word;
}QDMA_txMsgWord0_t;

typedef union
{
    struct
    {
#ifdef __BIG_ENDIAN
		uint no_drop	: 1;
		uint mtr_g		: 7;	/*0x7f means not use meter*/
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
		uint fport		: 4;
		uint nboq 		: 5;
		uint hwf		:1;
		uint			: 3;
		uint acnt_g1	: 5;    /*0x1F means no count*/
		uint acnt_g0	: 6;    /*0x3F means no count*/
#else
		uint fport		: 3;
		uint nboq 		: 5;
		uint 			: 6;
		uint acnt_g1	: 5;    /*bit4=1 means no count*/
		uint acnt_g0	: 5;    /*0x1F means no count*/
#endif
#else
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
		uint acnt_g0	: 6;    /*0x3F means no count*/
		uint acnt_g1	: 5;    /*0x1F means no count*/
		uint			: 3;		
		uint hwf		:1;
		uint nboq 		: 5;
		uint fport		: 4;
#else
		uint acnt_g0	: 5;    /*0x1F means no count*/
		uint acnt_g1	: 5;    /*bit4=1 means no count*/
		uint 			: 6;
		uint nboq 		: 5;
		uint fport		: 3;
#endif
		uint mtr_g		: 7;	/*0x7f means not use meter*/
		uint no_drop	: 1;
#endif /* __BIG_ENDIAN */
    }raw0;/*format for en7580*/
    struct
    {
#ifdef __BIG_ENDIAN
		uint ico		: 1;
		uint uco		: 1;
		uint tco		: 1;
		uint tso		: 1;
		uint            : 6;
		uint fport		: 3;
		uint 			: 19;
#else
		uint            : 19;
		uint fport		: 3;
		uint 			: 6;
		uint tso		: 1;
		uint tco		: 1;
		uint uco		: 1;
		uint ico		: 1;
#endif /* __BIG_ENDIAN */
    }raw1;
	struct
	{
		uint ico:1;
		uint uco:1;
		uint tco:1;
		uint tso:1;
		uint rsv1:6;
		uint fport:3;
		uint rsv2:19;
	}raw2;/*format for en7526*/
    uint word;
} QDMA_txMsgWord1_t;

typedef union
{
    struct
    {
#if defined(TCSUPPORT_CPU_EN7581)
#ifdef __BIG_ENDIAN
		uint sw_udf     : 8 ;
		uint    		: 8 ;
		uint mtr_g2     : 4 ;
		uint mtr_g1		: 5 ;
		uint acnt_g2    : 7 ;
#else
		uint acnt_g2    : 7 ;
		uint mtr_g1		: 5 ;
		uint mtr_g2     : 4 ;
		uint    		: 8 ;
		uint sw_udf     : 8 ;
#endif
#else
		uint    		: 32 ;
#endif
    };
    uint word;
} QDMA_txMsgWord2_t;;


typedef struct QDMA_txMsgWord_s{
	QDMA_txMsgWord0_t *pTxMsgW0;
	QDMA_txMsgWord1_t *pTxMsgW1;
	QDMA_txMsgWord2_t *pTxMsgW2;
} QDMA_txMsgWord_t;

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#endif /* _ECNT_HOOK_QDMA_TYPE_H_ */

