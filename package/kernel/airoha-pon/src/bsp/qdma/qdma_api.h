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
#ifndef _QDMA_API_H_
#define _QDMA_API_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook_qdma.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*                  M A C R O S
*************************************************************************
*/
	
#define atoi(x)  simple_strtoul(x, NULL,10)
#define QDMA_STOP_TIMER(timer)			{ del_timer_sync(&timer) ; }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define QDMA_START_TIMER(timer)			{ mod_timer(&timer, (jiffies + ((timer.data*HZ)/1000))) ; }
#else
#define QDMA_START_TIMER(timer)			{ mod_timer(&timer, (jiffies + ((timer.expires*HZ)/1000))) ; }
#endif
/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/


/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

/******************************************************************************
 Descriptor:	It's used to init the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				use the QDMA to send/receive packets.
 Input Args:	The pointer of the QDMA_InitCfg_t
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_init(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to deinit the QDMA software driver and hardware device.
 				This function must be called if the upper layer application wanna
 				transfer to another application.
 Input Args:	None
 Ret Value:		0: init successful otherwise failed.
******************************************************************************/
int qdma_deinit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TXDMA, RXDMA and HWFWD mode
 Input Args:	arg1: TX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
int qdma_tx_dma_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the TXDMA, RXDMA and HWFWD mode
 Input Args:	arg1: RX DMA mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value: 	No
******************************************************************************/
int qdma_rx_dma_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable the QDMA loopback mode
 Input Args:	arg1: loopback mode (QDMA_LOOPBACK_DISABLE/QDMA_LOOPBACK_QDMA/QDMA_LOOPBACK_UMAC)
 Ret Value: 	No
******************************************************************************/
int qdma_loopback_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to register the QDMA callback functions. The QDMA 
 				driver support several callback function type that is define 
 				in QDMA_CbType_t enum.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 				arg2: the pointer of the callback function.
 Ret Value:		0: register successful otherwise failed.
******************************************************************************/
int qdma_register_callback_function(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Description:	It's used to unregister the QDMA callback functions.
 Input Args:	arg1: callback function type that is define in QDMA_CbType_t enum.
 Ret Value:		0: unregister successful otherwise failed.
******************************************************************************/
int qdma_unregister_callback_function(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int2(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int2(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int3(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int3(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_enable_rxpkt_int4(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_disable_rxpkt_int4(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets_int2(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets_int3(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_receive_packets_int4(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_transmit_packet(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:
 Input Args:
 Ret Value:	
******************************************************************************/
int qdma_bm_transmit_packet_wifi_fast(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to configure the TXQOS weight type and scale.
 Input Args:	arg1: setting the WRR weighting value is base on packet or byte
 					  (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: setting the byte weighting scale(QDMA_TXQOS_WEIGHT_SCALE_64B
 				      /QDMA_TXQOS_WEIGHT_SCALE_16B). when the weigthing value 
 				      is base on packet, these argument is don't care.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos_weight(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the TXQOS weight type and scale.
 Input Args:	arg1: the pointer of the weight base value 
 				      (QDMA_TXQOS_WEIGHT_BY_PACKET/QDMA_TXQOS_WEIGHT_BY_BYTE)
 				arg2: the pointer of the weight scale value
 					  (QDMA_TXQOS_WEIGHT_SCALE_64B/QDMA_TXQOS_WEIGHT_SCALE_16B)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos_weight(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to configure the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_qos(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue scheduler and queue weigth
 				for specific channel.
 Input Args:	The pointer of the tx qos scheduler struct. It includes:
 				- channel: specific the channel ID (0~15)
 				- qosType: The QoS type is define in QDMA_TxQosType_t enum.
 				- weight: The unit of WRR weight is packets.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_qos(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set channel/queue limit threshold for mac driver.
 Input Args:	arg1: channel limit threshold
                     arg2: queue limit threshold
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_mac_limit_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get channel/queue limit threshold for mac driver.
 Input Args:	arg1: channel limit threshold
                     arg2: queue limit threshold
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_mac_limit_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get channel of all current DSCPs for mac driver.
 Input Args:	arg1: channel number
                     arg2: the id list of all channels
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_using_tx_dscp_channel(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to configure the tx buffer threshold. For the buffer 
 				management, the total available on-chip buffer is 64Kbyte (256 
 				Blocks, 256 bytes per block). It is shared among WAN and LAN 
 				Tx/Rx interface. If the buffer usage exceeds the threshold, the
 				Tx DMA will stop retrieving packets.
 Input Args:	The pointer of the tx buffer control struct. It includes:
 				- mode: Eanble/Disable tx buffer usage control
 				- chnThreshold: Per tx per channel block usage threshold.
 				- totalThreshold: Total tx block usage threshold.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txbuf_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the tx buffer threshold.
 Input Args:	The pointer of the tx buffer control struct. It includes:
 				- mode: Eanble/Disable tx buffer usage control
 				- chnThreshold: Per tx per channel block usage threshold.
 				- totalThreshold: Total tx block usage threshold.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txbuf_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable QDMA pre-fetch function. Since the
                on-chip buffer resource is limited, to prevent from Head-Of-Line
                blocking issue, 'when' to retrieving packet from External DRAM 
                to on-chip buffer will be application dependent.
 Input Args:	arg1: Enable/disable TXDMA pre-fetch function (QDMA_ENABLE/QDMA_DISABLE)
 			arg2: Enable/disable TXDMA pre-fetch overdrag function (QDMA_ENABLE/QDMA_DISABLE)
 			arg3: set TXDMA pre-fetch overdrag count value
 Ret Value:	No return value
******************************************************************************/
int qdma_set_prefetch_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable or disable pktsize overhead function.
 Input Args:	arg1: mode means enable or disable.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_pktsize_overhead_en(struct ECNT_QDMA_Data *qdma_data) ;


/******************************************************************************
 Descriptor:	It's used to get pktsize overhead function is enable or disable.
 Input Args:	None
 Ret Value:	0: Disable, 1: Enable.
******************************************************************************/
int qdma_get_pktsize_overhead_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of pktsize overhead.
 Input Args:	arg1: pktsize overhead value
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_pktsize_overhead_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of pktsize overhead.
 Input Args:	None
 Ret Value:	pktsize overhead value.
******************************************************************************/
int qdma_get_pktsize_overhead_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of lmgr low threshold.
 Input Args:	arg1: lmgr low threshold value
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_lmgr_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of lmgr low threshold.
 Input Args:	None
 Ret Value:	lmgr low threshold value.
******************************************************************************/
int qdma_get_lmgr_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get lmgr status: free lmgr dscp, used lmgr dscp, 
 			and used total buffer usage.
 Input Args:	arg1: to get free lmgr dscp counter
 			arg2: to get used lmgr dscp counter
 			arg3: to get used total buffer usage
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_lmgr_status(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to clear all the qdma cpu counters.
 Input Args:	None
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_clear_cpu_counters(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all the qdma cpu counters.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_cpu_counters(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all qdma related register values.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_register_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all cpu tx/rx dscp information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_descriptor_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all cpu tx/rx dscp cnt info.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_descriptor_cnt(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to dump all IRQ information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_irq_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all IRQ2 information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_irq2_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump all hwfwd dscp information.
 Input Args:	None
 Ret Value:	No return value
******************************************************************************/
int qdma_dump_hwfwd_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set dbg level for qdma debug.
 Input Args:	arg1: dbgLevel: 0 for ERROR; 1 for ST; 2 for WARN; 3 for MSG
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_dbg_level(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump tx/rx dma busy bit in several seconds.
 Input Args:	arg1: busyDuration means the seconds of dma busy bit print out. 
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_dump_dma_busy(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to update dynamic threshold per ms.
 Input Args:	NULL. 
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_config_trigger_test(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump the value of specific qdma register in several seconds.
 Input Args:	arg1: regOffset means the offset of specific qdma register. 
			arg2: pollingDuration means the seconds of polling time. 
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_dump_reg_polling(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set receive rx ring1 packets only: rx1:rx0 from 5:2 to 5:0.
 Input Args:	arg1: forceEn means enable/disable force receive rx1 function.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_force_receive_rx_ring1(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable or disable TX_DROP_EN.
 Input Args:	arg1: mode means enable or disable.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_drop_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of TX_DROP_EN.
 Ret Value:		0: Disable, 1: Enable.
******************************************************************************/
int qdma_get_tx_drop_en(struct ECNT_QDMA_Data *qdma_data) ;

/*--------------EN7580 New APIs start-------------------*/
int qdma_general_set_trtcm_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_trtcm_cfg(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_general_set_ratelimit_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_ratelimit_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_ratelimit_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_ratelimit_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_ratelimit_bucket_size(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_general_set_trtcm_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_trtcm_mode_cfg(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_get_trtcm_mode_value(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_general_set_trtcm_bucket_size(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_set_flow_cntr_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_flow_cntr_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_flow_cntr_value(struct ECNT_QDMA_Data *qdma_data);
int qdma_clear_flow_cntr_value(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_tx_wred_mode(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_mode(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_tx_wred_threshold(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_threshold(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_tx_wred_probability(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_probability(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_wred_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_cpu_rx_red_probability(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_cpu_rx_red_probability(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_channel_close_status(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_channel_close_status(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_channel_empty_status(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_oam_modify_fp_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_oam_modify_fp_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_multicast_en(struct ECNT_QDMA_Data *qdma_data);

int qdma_allocate_meter(struct ECNT_QDMA_Data *qdma_data);
int qdma_free_meter(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_qos_flag(struct ECNT_QDMA_Data *qdma_data);
int qdma_allocate_acnt(struct ECNT_QDMA_Data *qdma_data);
int qdma_free_acnt(struct ECNT_QDMA_Data *qdma_data);

int qdma_dram_test_dma_config(struct ECNT_QDMA_Data *qdma_data);
int qdma_dram_test_dma_enable(struct ECNT_QDMA_Data *qdma_data);
int qdma_dram_test_is_rx_done(struct ECNT_QDMA_Data *qdma_data);
int qdma_dram_test_dma_skb_get(struct ECNT_QDMA_Data *qdma_data);

int qdma_set_sla_chnl_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_sla_chnl_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_qos_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_qos_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_per_queue_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_per_queue_aging_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_sptag_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_multicast_sptag_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_fport_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_multicast_fport_cfg(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_txq_cngst_static_channel_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_txq_cngst_static_queue_ratio(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_hqos_en(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_dbg_cntr_rx_ring(struct ECNT_QDMA_Data *qdma_data);
int qdma_set_multicast_1toN_cfg(struct ECNT_QDMA_Data *qdma_data);

/*--------------EN7580 New APIs end-------------------*/

int qdma_get_channel_cfg(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to set the value of tx ratemeter config. It includes
 			timeDivisor and timeSlice, while tx_rateMeter_En always enable.
 Input Args:	arg1: The pointer of the  Tx RateMeter Config struct.
                     tx ratemeter default enable.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratemeter(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx ratemeter config. It includes
 			timeDivisor and timeSlice, while tx_rateMeter_En always enable.
 Input Args:	arg1: The pointer of the  Tx RateMeter Config struct. It includes
 				- timeDivisor: the divisor of timeSlice, which will affect datarate.
 				- timeSlice: ratemeter calculate duration.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_ratemeter(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx channel datarate.
 Input Args:	arg1: channel index
 Ret Value:		return tx datarate by channel, unit is bps
******************************************************************************/
int qdma_get_tx_channel_datarate(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable tx global ratelimit function. 
 Input Args:	No input arguments
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_enable_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx rate limit config. It includes bucketScale and ticksel.
 Input Args:	arg1: The pointer of the  Tx Rate Limit Config struct.It includes
 			- txRateLimitUnit: unit is 1kbps, scope is (1kbps~65kbps). 0 means not change.
 			- txRateLimitBucketScale: ratemeter calculate duration.
 			- txRateLimitEn: default enable, ratelimit enable or not decide by chnlRateLimitEn.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx rate limit config. It includes
 			tx_rateLimit_En, bucketScale and ticksel.
 Input Args:	arg1: The pointer of the  Tx Rate Limit Config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx rate limit parameter. It includes chnlRateLimitEn and PIR.
 Input Args:	arg1: The pointer of the TX RateLimit Parameter struct.It includes
 			- chnlRateLimitEn: means current channel ratelimit enable or disable.
 			- PIR: unit is kbps, scope is [1kbps~1Gbps]
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx dba report parameter. It includes CIR, CBS, PIR and PBS.
 Input Args:	arg1: The pointer of the TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  TX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable rx protect mode. 
 Input Args:	arg1: Enable/disable rx protect mode (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_protect_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get rx protect mode:enable/disable.
 Input Args:	No input arguments
 Ret Value:		return rx protect mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
int qdma_get_rx_protect_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx low threshold. 
 Input Args:	arg1: The pointer of the RX Low Threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of rx low threshold. 
 Input Args:	arg1: The pointer of the RX Low Threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_low_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit_en(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit_pkt_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of rx rate limit config. It includes
 			rx_rateLimit_En, bucketScale and tickSel.
 Input Args:	arg1: The pointer of the RX Rate Limit config struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_ratelimit_cfg(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of rx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the RX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of rx rate limit parameter. It includes PIR and PBS.
 Input Args:	arg1: The pointer of the  RX RateLimit Parameter struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue DEI dropped function. 
 Input Args:	arg1: Enable/disable tx queue DEI function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_dei_drop_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue DEI dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue DEI mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
int qdma_get_txq_dei_drop_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue dynamic cngst mode. 
 Input Args:	arg1: Enable/disable tx queue dynamic cngst (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue dynamic cngst mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue dynamic cngst mode.  (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
int qdma_get_txq_cngst_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set tx queue dei threshold scale, scope is (1/2 ~ 1/16) 
 Input Args:	arg1: set tx queue dei threshold scale  (1/2 ~ 1/16)
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_dei_threshold_scale(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue dei threshold scale, scope is (1/2 ~ 1/16) 
 Input Args:	No input arguments
 Ret Value:	return tx queue dei threshold scale  (1/2 ~ 1/16)
******************************************************************************/
int qdma_get_txq_dei_threshold_scale(struct ECNT_QDMA_Data *qdma_data) ;

/* need modify later */
int qdma_set_txq_cngst_auto_config(struct ECNT_QDMA_Data *qdma_data) ;

/* need modify later */
int qdma_get_txq_cngst_auto_config(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set the value of tx dynamic threshold. It includes 
 			total max threshold, total min Threshold, channel max threshold, 
 			channel min threshold, queue max threshold, queue min threshold.
 Input Args:	arg1: The pointer of the TX dynamic threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_dynamic_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx dynamic threshold. It includes 
 			total max threshold, total min Threshold, channel max threshold, 
 			channel min threshold, queue max threshold, queue min threshold.
 Input Args:	arg1: The pointer of the TX dynamic threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txq_cngst_dynamic_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ total max/min threshold, unit is byte.
 Input Args:	arg1: set txQ total max/min threshold,
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_total_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ total max/min threshold, unit is byte.
 Input Args:	No input arguments
 Ret Value:	return txQ total max/min threshold,
******************************************************************************/
int qdma_get_txq_cngst_total_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ channel  max/min threshold, unit is byte.
 Input Args:	arg1: set txQ channel max/min threshold,
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_channel_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ channel  max/min threshold, unit is byte.
 Input Args:	No input arguments
 Ret Value:	return txQ channel max/min threshold,
******************************************************************************/
int qdma_get_txq_cngst_channel_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ queue max/min threshold, unit is byte.
 Input Args:	arg1: set txQ queue max/min threshold,
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_queue_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ queue max/min threshold, unit is byte.
 Input Args:	No input arguments
 Ret Value:	return txQ queue max/min threshold,
******************************************************************************/
int qdma_get_txq_cngst_queue_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ peekrate parameters.
 Input Args:	arg1: set txQ peekrate parameters, it includes
 			- peekRateEn: peekrate enable or disable.
 			- peekRateMargin: scope is [0~3], means (0, 25%, 50%, 100%)
 			- peekRateDuration: unit is ms
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_peekrate_params(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ peekrate parameters.
 Input Args:	No input arguments
 Ret Value:	return txQ peekrate parameters.
******************************************************************************/
int qdma_get_txq_peekrate_params(struct ECNT_QDMA_Data *qdma_data) ;


/******************************************************************************
 Descriptor:	It's used to set txQ static queue normal threshold.
 Input Args:	arg1: set txQ static queue normal threshold, it includes
 			- normalThrh[8]: unit is byte, scope is 256Byte~16MByte.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_static_queue_normal_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ static queue dei threshold.
 Input Args:	arg1: set txQ static queue dei threshold, it includes
 			- deiThrh[8]: unit is byte, scope is 256Byte~16MByte.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_static_queue_dei_threshold(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ dynamic cngst infomation.
 Input Args:	No input arguments
 Ret Value:	return txQ dynamic cngst infomation.
******************************************************************************/
int qdma_get_txq_cngst_dynamic_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ static cngst infomation.
 Input Args:	No input arguments
 Ret Value:	return txQ static cngst infomation.
******************************************************************************/
int qdma_get_txq_cngst_static_info(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ queue nonblocking or not.
 Input Args:	arg1: The pointer of the TXQ Cngst Queue Cfg struct.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_queue_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ queue nonblocking or not.
 Input Args:	No input arguments
 Ret Value:	return txQ queue nonblocking or not.
******************************************************************************/
int qdma_get_txq_cngst_queue_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set txQ channel nonblocking or not.
 Input Args:	arg1: The pointer of the TXQ Cngst Chanel Cfg struct.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_channel_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get txQ channel nonblocking or not.
 Input Args:	No input arguments
 Ret Value:	return txQ channel nonblocking or not.
******************************************************************************/
int qdma_get_txq_cngst_channel_nonblocking(struct ECNT_QDMA_Data *qdma_data) ;


/******************************************************************************
 Descriptor:	It's used to set virtual channel mode.
 Input Args:	arg1: set virtual channel mode, it includes
 			- virChnlEn: virtual channel mapping mode enable or disable.
 			- virChnlMode: 16 Queue or 32 Queue.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_virtual_channel_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get virtual channel mode.
 Input Args:	No input arguments
 Ret Value:	return virtual channel mode.
******************************************************************************/
int qdma_get_virtual_channel_mode(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to set single physical channel qos value.
 Input Args:	arg1: set single physical channel qos value, it includes
 			- phyChnl: specific physical channel index, scope is (0~7) or (0~15).
 			- qosType: means SP / WRR / SPWRR3 / SPWRR2.
 			- queue[4]: means weight of 2 or 4 queue.
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to get single physical channel qos value.
 Input Args:	No input arguments
 Ret Value:	return single physical channel qos value.
******************************************************************************/
int qdma_get_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_clear_dbg_cntr_value_all(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_clear_and_set_dbg_cntr_channel_group(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_clear_and_set_dbg_cntr_queue_group(struct ECNT_QDMA_Data *qdma_data) ;
int qdma_clear_and_set_dbg_cntr_ring_group(struct ECNT_QDMA_Data *qdma_data) ;
//added tmp by czw
int qdma_set_cntr_channel(struct ECNT_QDMA_Data *qdma_data);

int qdma_dump_cntr_channel(struct ECNT_QDMA_Data *qdma_data);

int qdma_get_dbg_cntr_all_queue_value(struct ECNT_QDMA_Data *qdma_data);

int qdma_dump_dbg_cntr_value(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to dump the tx qos type and queue weigth for all channel.
 Input Args:		chnlIdx: specific the channel ID (0~31) and chnl
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_dump_tx_qos(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_virtual_channel_qos(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_tx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_rx_ratelimit(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_tx_dba_report(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_txq_cngst(struct ECNT_QDMA_Data *qdma_data) ;

int qdma_dump_info_all(struct ECNT_QDMA_Data *qdma_data) ;

/******************************************************************************
 Descriptor:	It's used to configure dbg counter, which has 32 groups to set.
 Input Args:		- cntrIdx:
 				- cntrEn:
 				- cntrSrc:
 				- chnlIdx:
 				- queueIdx:
 				- dscpRingIdx:
 				- isChnlAll:
 				- isQueueAll:
 				- isDscpRingAll:
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_dbg_cntr_info(QDMA_DBG_CNTR_T *dbgCntrPtr) ;

int qdma_clear_and_enable_dbg_cntr_info(QDMA_DBG_CNTR_T *dbgCntrPtr) ;

/******************************************************************************
 Descriptor:	It's used to configure dbg counter, which has 32 groups to set.
 Input Args:		- cntrIdx:
 Output Args:	- cntrEn:
 				- cntrSrc:
 				- chnlIdx:
 				- queueIdx:
 				- dscpRingIdx:
 				- isChnlAll:
 				- isQueueAll:
 				- isDscpRingAll:
 				- cntrVal:
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_dbg_cntr_info(QDMA_DBG_CNTR_T *dbgCntrPtr) ;

/******************************************************************************
 Descriptor:	It's used to set the value of TXQ configuration. It includes
 			txqDropEn, txqDeiDropEn, dynCngstEn, MaxThrhTx1En, MinThrhTx1En, 
 			MaxThrhTx0En, MinThrhTx0En, modeConfigTriggerEn, modePacketTriggerEn, 
 			modeTimeTriggerEn, deiThrhScale, dynCngstTicksel
 Input Args:	arg1: The pointer of the  TXQ configuration struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_cngst_cfg(QDMA_TxQCngstCfg_T *txqCngstCfgPtr) ;

/******************************************************************************
 Descriptor:	It's used to get the value of TXQ configuration. It includes
 			txqDropEn, txqDeiDropEn, dynCngstEn, MaxThrhTx1En, MinThrhTx1En, 
 			MaxThrhTx0En, MinThrhTx0En, modeConfigTriggerEn, modePacketTriggerEn, 
 			modeTimeTriggerEn, deiThrhScale, dynCngstTicksel
 Input Args:	arg1: The pointer of the  TXQ configuration struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txq_cngst_cfg(QDMA_TxQCngstCfg_T *txqCngstCfgPtr) ;

#if 0
/******************************************************************************
 Descriptor:	It's used to set the value of tx static threshold. It includes
 			dei packet threshold and Normal packet threshold.
 Input Args:	arg1: The pointer of the TX static threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_txq_static_queue_threshold(QDMA_TxQStaticCngstThrh_T *txqCngstThrhPtr) ;

/******************************************************************************
 Descriptor:	It's used to get the value of tx static threshold. It includes
 			dei packet threshold and Normal packet threshold.
 Input Args:	arg1: The pointer of the TX static threshold struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_txq_static_queue_threshold(QDMA_TxQStaticCngstThrh_T *txqCngstThrhPtr) ;
#endif

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue DEI dropped function. 
 Input Args:	arg1: Enable/disable tx queue trTCM function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_dei_mode(QDMA_Mode_t deiDropMode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue DEI dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue DEI mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_dei_mode(void) ;

/******************************************************************************
 Descriptor:	It's used to enable/disable tx queue threshold dropped function. 
 Input Args:	arg1: Enable/disable tx queue threshold function (QDMA_ENABLE/QDMA_DISABLE)
 Ret Value:		No return value
******************************************************************************/
void qdma_set_txqueue_threshold_mode(QDMA_Mode_t thrsldDropMode) ;

/******************************************************************************
 Descriptor:	It's used to get tx queue threshold dropped mode. 
 Input Args:	No input arguments
 Ret Value:		return tx queue threshold mode (QDMA_ENABLE/QDMA_DISABLE)
******************************************************************************/
QDMA_Mode_t qdma_get_txqueue_threshold_mode(void) ;

/******************************************************************************
 Descriptor:	It's used to set the tx queue congestion threshold. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestThreshold_T struct.
 				It includes five parameters.
 				1. queueIdx: tx queue index (0~7).
 				2. grnMaxThreshold: The green max threshold for specific tx queue
 				3. grnMinThreshold: The green min threshold for specific tx queue
 				4. ylwMaxThreshold: The yellow max threshold for specific tx queue
 				5. ylwMinThreshold: The yellow min threshold for specific tx queue
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_set_congest_threshold(QDMA_TxQStaticCngstThrh_T *pThreshold) ;

/******************************************************************************
 Descriptor:	It's used to get the tx queue congestion configuration. 
 Input args:	arg1: The pointer of the QDMA_TxQueueCongestCfg_T struct.
 Ret Value:		0: successful, otherwise failed.
******************************************************************************/
int qdma_get_congest_config(QDMA_TxQueueCongestCfg_T *pCongest) ;


void qdmaSetDbgCntrCfg_init(void);
void qdmaGetDbgCntrCfg_Val(uint *cpuTxCnt, uint *cpuRxCnt, uint *fwdTxCnt, uint *fwdRxCnt);
int qdma_set_tx_cngst_mode(struct ECNT_QDMA_Data *qdma_data);
int qdma_get_tx_cngst_mode(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to read VIP info from fe. 
 Input Args:	None
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_read_vip_info(void);
/******************************************************************************
 Descriptor:	It's used to set mac qos config ,include qos_flag, queuemask qos_wrr_info etc. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_set_mac_qos_config(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to set tx msg. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_api_set_txmsg(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to get rx msg. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_api_get_rxmsg(struct ECNT_QDMA_Data *qdma_data);

/******************************************************************************
 Descriptor:	It's used to get rx msg for eth driver. 
 Input Args:	qdma_data
 Ret Value:	0: successful, otherwise failed.
******************************************************************************/
int qdma_api_get_rxmsg_eth(struct ECNT_QDMA_Data *qdma_data);

int qdma_api_check_dscp_is_free(void);

int qdma_api_set_downstream_qos_mode(struct ECNT_QDMA_Data *qdma_data);

int qdma_api_get_downstream_qos_mode(struct ECNT_QDMA_Data *qdma_data);
#endif /* _QDMA_API_H_ */
