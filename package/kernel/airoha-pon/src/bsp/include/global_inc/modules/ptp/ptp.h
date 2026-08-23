#ifndef _PTP_H_
#define _PTP_H_

#define PTP_ADDR_BASE       			0x00000000

#define PTP_GLB_CFG  					(PTP_ADDR_BASE + 0x0000)
#define SYNC_RX_RATE_LIMIT_SET 			(PTP_ADDR_BASE + 0x0004)
#define PTP_INT_STS			   			(PTP_ADDR_BASE + 0x0008)
#define PTP_INT_EN			   			(PTP_ADDR_BASE + 0x000c)
#define VLAN_TAG_SET		   			(PTP_ADDR_BASE + 0x0010)
#define PTP_RX_TS_HIGH_16   			(PTP_ADDR_BASE + 0x0014)
#define PTP_SPTAG_CFG                   (PTP_ADDR_BASE + 0x0018)
#define PTP_TX_TO_SW_INFO_RD   			(PTP_ADDR_BASE + 0x0024)
#define PTP_TX_TO_SW_TS_H   			(PTP_ADDR_BASE + 0x0028)
#define PTP_TX_TO_SW_TS_L   			(PTP_ADDR_BASE + 0x002c)
#define PTP_TX_TO_SW_TYPE_SEQID   		(PTP_ADDR_BASE + 0x0030)
#define PTP_RX_TS_COMP   				(PTP_ADDR_BASE + 0x0034)
#define PTP_TX_TS_COMP   				(PTP_ADDR_BASE + 0x0038)
#define PTP_TX_SYNC_FOLLOW_INTERVAL   	(PTP_ADDR_BASE + 0x003c)
#define SYNC_DELAY_REQ_TX_FREQ   		(PTP_ADDR_BASE + 0x0040)
#define PDELAY_REQ_TX_FREQ   			(PTP_ADDR_BASE + 0x0044)
#define ANNOUNCE_TX_FREQ   				(PTP_ADDR_BASE + 0x0048)
#define PTP_PKT_SEQID_SET0   			(PTP_ADDR_BASE + 0x004c)
#define PTP_PKT_SEQID_SET1   			(PTP_ADDR_BASE + 0x0050)
#define PTP_AUTOGEN_HEADER_OFFSET   	(PTP_ADDR_BASE + 0x0054)
#define PTP_AUTOGEN_LENGTH   			(PTP_ADDR_BASE + 0x0058)
#define PTP_AUTOGEN_TYPE_SET   			(PTP_ADDR_BASE + 0x005c)
#define PTP_AUTOGEN_PKT_SET   			(PTP_ADDR_BASE + 0x0060)
#define PTP_AUTOGEN_PKT_DATA   			(PTP_ADDR_BASE + 0x0064)
//#define PTP_PRB_SEL   				(PTP_ADDR_BASE + 0x0070)
//#define PTP_PRB_OUTH   				(PTP_ADDR_BASE + 0x0074)
//#define PTP_PRB_OUTL   				(PTP_ADDR_BASE + 0x0078)
#define PTP_CNT_CLR   					(PTP_ADDR_BASE + 0x0080)
#define PTP_RX_NOT_PTP_CNT   			(PTP_ADDR_BASE + 0x0084)
#define PTP_RX_PTP_CNT   				(PTP_ADDR_BASE + 0x0088)
#define PTP_RX_PTP_BYTE_CNT   			(PTP_ADDR_BASE + 0x008c)
#define PTP_RXMBI_CNT   				(PTP_ADDR_BASE + 0x0090)
#define PTP_RXMBI_BYTE_CNT  			(PTP_ADDR_BASE + 0x0094)
#define PTP_RX_SYNC_DROP_CNT			(PTP_ADDR_BASE + 0x0098)
#define PTP_RX_DROP_CNT   				(PTP_ADDR_BASE + 0x009c)
#define PTP_TX_CNT		 				(PTP_ADDR_BASE + 0x00b0)
#define PTP_TX_AUTOG_SYNC_CNT			(PTP_ADDR_BASE + 0x00b4)
#define PTP_TX_AUTOG_DELAY_REQ_CNT		(PTP_ADDR_BASE + 0x00b8)
#define PTP_TX_AUTOG_PDELAY_REQ_CNT		(PTP_ADDR_BASE + 0x00bc)
#define PTP_TX_AUTOG_ANNOUNCE_CNT		(PTP_ADDR_BASE + 0x00c0)
#define PTP_TX_AUTOG_FOLLOW_UP_CNT		(PTP_ADDR_BASE + 0x00c4)
#define PTP_TX_MBI_CNT					(PTP_ADDR_BASE + 0x00c8)



#define XFI_MAC_ADDR_BASE       		0x1FA09000
#define XFI_PON_MAC_ADDR_OFFSET			0x00001000
#define XFI_MAC_PTP_ADDR_BASE			0x1FA09500

#define XFI_GLB_CFG                     (XFI_MAC_ADDR_BASE + 0x0000)
#define XFI_INT_STS                     (XFI_MAC_ADDR_BASE + 0x0004)
#define XFI_INT_EN                      (XFI_MAC_ADDR_BASE + 0x0008)

#define XFI_CNT_CLR                     (XFI_MAC_ADDR_BASE + 0x0100)
#define XFI_TX_OCTETS_CNT               (XFI_MAC_ADDR_BASE + 0x0104)
#define XFI_TX_PKT_CNT                  (XFI_MAC_ADDR_BASE + 0x0108)
#define XFI_TXMBI_ETH_CNT               (XFI_MAC_ADDR_BASE + 0x0114)
#define XFI_TXMBI_UCETH_CNT             (XFI_MAC_ADDR_BASE + 0x0118)
#define XFI_TXMBI_MCETH_CNT             (XFI_MAC_ADDR_BASE + 0x011c)
#define XFI_TXMBI_PAUSE_CNT             (XFI_MAC_ADDR_BASE + 0x0120)

#define XFI_XGMII_TX_SOF_EOF_CNT        (XFI_MAC_ADDR_BASE + 0x0130)
#define XFI_XGMII_TX_BYTES_CNT          (XFI_MAC_ADDR_BASE + 0x0134)
#define XFI_TX_NORMAL_PKT_BYTES_CNT     (XFI_MAC_ADDR_BASE + 0x0138)
#define XFI_TX_DEQ_CHECK_CNT1           (XFI_MAC_ADDR_BASE + 0x013c)
#define XFI_TX_DEQ_CHECK_CNT2           (XFI_MAC_ADDR_BASE + 0x0140)

#define XFI_RX_FRAME_CNT                (XFI_MAC_ADDR_BASE + 0x0180)
#define XFI_RX_OCTETS_CNT               (XFI_MAC_ADDR_BASE + 0x0184)
#define XFI_RX_PKT_CNT                  (XFI_MAC_ADDR_BASE + 0x0188)
#define XFI_RX_ETH_CNT                  (XFI_MAC_ADDR_BASE + 0x018c)
#define XFI_RX_PAUSE_CNT                (XFI_MAC_ADDR_BASE + 0x0190)
#define XFI_RX_LENERR_CNT               (XFI_MAC_ADDR_BASE + 0x0194)
#define XFI_RX_CRCERR_CNT               (XFI_MAC_ADDR_BASE + 0x0198)
#define XFI_RXMBI_PKT_CNT               (XFI_MAC_ADDR_BASE + 0x019c)
#define XFI_RXMBI_DROP_CNT              (XFI_MAC_ADDR_BASE + 0x01a0)

#define XFI_XGMII_RX_SOF_EOF_CNT        (XFI_MAC_ADDR_BASE + 0x01a4)
#define XFI_RX_MPI_SOP_EOP_CNT          (XFI_MAC_ADDR_BASE + 0x01a8)
#define XFI_RX_NORMAL_PKT_BYTES_CNT     (XFI_MAC_ADDR_BASE + 0x01ac)
#define XFI_ENQ_CHECK_CNT1              (XFI_MAC_ADDR_BASE + 0x01b0)
#define XFI_ENQ_CHECK_CNT2              (XFI_MAC_ADDR_BASE + 0x01b4)
#define XFI_RX_MBI_SOP_EOP_CNT          (XFI_MAC_ADDR_BASE + 0x01b8)

#define XFI_PTP_GLB_CFG  				(XFI_MAC_PTP_ADDR_BASE + 0x0000)
#define XFI_SYNC_RX_RATE_LIMIT_SET 		(XFI_MAC_PTP_ADDR_BASE + 0x0004)
#define XFI_PTP_INT_STS			   		(XFI_MAC_PTP_ADDR_BASE + 0x0008)
#define XFI_PTP_INT_EN			   		(XFI_MAC_PTP_ADDR_BASE + 0x000c)
#define XFI_PTP_TX_TO_SW_INFO_RD   		(XFI_MAC_PTP_ADDR_BASE + 0x0010)
#define XFI_PTP_TX_TO_SW_TS_H   		(XFI_MAC_PTP_ADDR_BASE + 0x0018)
#define XFI_PTP_TX_TO_SW_TS_L   		(XFI_MAC_PTP_ADDR_BASE + 0x001c)
#define XFI_PTP_TX_TO_SW_TYPE_SEQID   	(XFI_MAC_PTP_ADDR_BASE + 0x0020)
#define XFI_PTP_RX_TS_COMP   			(XFI_MAC_PTP_ADDR_BASE + 0x0028)
#define XFI_PTP_TX_TS_COMP   			(XFI_MAC_PTP_ADDR_BASE + 0x002c)
#define XFI_PTP_TX_SYNC_FOLLOW_INTERVAL (XFI_MAC_PTP_ADDR_BASE + 0x0030)
#define XFI_PDELAY_REQ_TX_FREQ   		(XFI_MAC_PTP_ADDR_BASE + 0x0034)  //ptp_pkt_freq_set0
#define XFI_SYNC_DELAY_REQ_TX_FREQ   	(XFI_MAC_PTP_ADDR_BASE + 0x0038)  //ptp_pkt_freq_set1
#define XFI_ANNOUNCE_TX_FREQ   			(XFI_MAC_PTP_ADDR_BASE + 0x003c)  //ptp_pkt_freq_set2
#define XFI_PTP_PKT_SEQID_SET0   		(XFI_MAC_PTP_ADDR_BASE + 0x0040)
#define XFI_PTP_PKT_SEQID_SET1   		(XFI_MAC_PTP_ADDR_BASE + 0x0044)
#define XFI_PTP_AUTOGEN_HEADER_OFFSET   (XFI_MAC_PTP_ADDR_BASE + 0x0048)
#define XFI_PTP_AUTOGEN_LENGTH   		(XFI_MAC_PTP_ADDR_BASE + 0x004c)
#define XFI_PTP_AUTOGEN_PKT_SET   		(XFI_MAC_PTP_ADDR_BASE + 0x0050)
#define XFI_PTP_AUTOGEN_PKT_DATA   		(XFI_MAC_PTP_ADDR_BASE + 0x0054)
#define XFI_VLAN_TAG_SET		   		(XFI_MAC_PTP_ADDR_BASE + 0x0058)
#define XFI_PTP_RX_TS_HIGH_16   		(XFI_MAC_PTP_ADDR_BASE + 0x005c)

#define XFI_PTP_TX_MBI_CNT              (XFI_MAC_PTP_ADDR_BASE + 0x0100)
#define XFI_PTP_RXMBI_CNT               (XFI_MAC_PTP_ADDR_BASE + 0x0104)
#define XFI_PTP_TX_AUTOG_SYNC_CNT		(XFI_MAC_PTP_ADDR_BASE + 0x0108)
#define XFI_PTP_TX_AUTOG_DELAY_REQ_CNT	(XFI_MAC_PTP_ADDR_BASE + 0x0110)
#define XFI_PTP_TX_AUTOG_PDELAY_REQ_CNT	(XFI_MAC_PTP_ADDR_BASE + 0x0114)
#define XFI_PTP_TX_AUTOG_ANNOUNCE_CNT	(XFI_MAC_PTP_ADDR_BASE + 0x0118)
#define XFI_PTP_TX_AUTOG_FOLLOW_UP_CNT	(XFI_MAC_PTP_ADDR_BASE + 0x011c)

#define XFI_RX_PARSER_ETH_NUM           (XFI_MAC_PTP_ADDR_BASE + 0x0120)
#define XFI_RX_PRRSER_IPV4_NUM          (XFI_MAC_PTP_ADDR_BASE + 0x0124)
#define XFI_RX_PARSER_IPV6_NUM          (XFI_MAC_PTP_ADDR_BASE + 0x0128)
#define XFI_RX_PARSER_DROP_NUM          (XFI_MAC_PTP_ADDR_BASE + 0x012c)

/* ptp message types */
#define PTP_SYNC					0x0
#define PTP_DELAY_REQ				0x1
#define PTP_PDELAY_REQ				0x2
#define PTP_PDELAY_RESP				0x3
#define PTP_FOLLOW_UP				0x8
#define PTP_DELAY_RESP				0x9
#define PTP_PDELAY_RESP_FOLLOW_UP	0xA
#define PTP_ANNOUNCE				0xB
#define PTP_SIGNALING				0xC
#define PTP_MANAGEMENT				0xD

#define DISABLE    0
#define ENABLE     1

#define PDEV_GSW_PTP_ID         0
#define PDEV_XFI_PTP_ID         1
#define PDEV_XFI_PON_PTP_ID     2

typedef struct ptp_tx_feedback_sw_info_s
{
	uint32_t tx_timestamp_byte_7_4; 
	uint32_t tx_timestamp_byte_3_0;
	uint32_t tx_seqId_msgType_infoCnt;
	
} ptp_tx_feedback_sw_info_t;

typedef struct hw_autogen_basic_config_s 
{
	uint8_t msg_type;	
	uint8_t ts_insert;
	uint32_t tx_freq;
	uint32_t tx_comp;
	uint16_t seqid;
	uint8_t header_offset;
	uint8_t pkt_len;
	uint8_t pkt_type;
	uint8_t autogen_msg_type;
	uint8_t load_seqid;
	
} hw_autogen_basic_config_t;

//0x18
typedef union
{
	struct
	{
		unsigned int ptp_rx_sptag_en    : 1;
		unsigned int ptp_tx_sptag_en    : 1;
		unsigned int ptp_rx_sptag_mode  : 1;
		unsigned int ptp_tx_sptag_mode  : 1;
		unsigned int resv               : 28;
	} bits;
	unsigned int word;
} ptp_sptag_cfg_t;

/* [GSW PTP] PTP autogen message type */
enum{
	PTP_AUTOGEN_SYNC_DELAYREQ_MSG_TYPE = 0,
	PTP_AUTOGEN_PDELAYREQ_MSG_TYPE     = 1,
	PTP_AUTOGEN_ANNOUNCE_MSG_TYPE      = 2,
	PTP_AUTOGEN_FOLLOWUP_MSG_TYPE      = 3
};

/* [GSW PTP] sync filter bits */
enum {
	SYNC_RX_FILTER_V1    = 10,
	SYNC_RX_FILTER_V2    = 11,
	SYNC_RX_FILTER_ETHER = 12,
	SYNC_RX_FILTER_IPV4  = 13,
	SYNC_RX_FILTER_IPV6  = 14,	
	SYNC_RX_FILTER_MULTI = 15,
	SYNC_RX_FILTER_UNI   = 16
};

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#if 0
enum {
	FALSE = 0,
	TRUE
};
#endif

/* [GSW PTP] Packet Type index */
enum {
	PTP_OVER_ETHERNET      = 0,
	PTP_OVER_UDP_OVER_IPv4 = 1,
	PTP_OVER_UDP_OVER_IPv6 = 2,
	NORMAL_UDP_OVER_IPV4   = 3,
};

/* [GSW PTP] interrupt bits */
enum {
	RX_DROP_PKT = 0,
	TX_TO_SW_INFO,
	TX_TO_SW_INFO_CNT_FULL,
	TX_AUTO_GEN_SYNC,
	TX_AUTO_GEN_DELAY_REQ,
	TX_AUTO_GEN_PDELAY_REQ,
	TX_AUTO_GEN_ANNOUNCE,
	TX_AUTO_GEN_FOLLOW_UP
};

/* [ToD] interrupt bits */
enum {
	TOD_LOAD_DONE_INT = 0,
	TOD_1PPS_INT,
	FREQ_ADJ_DONE_INT
};

/* [XFI PTP] Loopback mode */
enum {
	XFI_PTP_LPBK_DISABLE = 0,
	XFI_PTP_LPBK_ENABLE
};

/* RX MSG definition is in tcswithc.h of ether driver*/
#define PTP_PKT_BIT_RX_MSG (1<<9)
#define PTP_1588_PKT_RX_CHECK(x) unlikely((x).ptp)

#endif
