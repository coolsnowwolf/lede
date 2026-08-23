/**
*@file
*@brief This file defines ethernet port related interfaces that should be implemented by chip vendors 
*@author Zhang Yan H
*@date     2012.10.17
*@copyright 
*******************************************************************
* Copyright (c) 2012 Alcatel Lucent Shanghai Bell Software, Inc.
*             All Rights Reserved
*
* This source is confidential and proprietary and may not
* be used without the written permission of Alcatel Lucent
* Shanghai Bell, Inc.
********************************************************************
*@par Module History:
*/

#ifndef _HAL_TYPE_PM_H_
#define _HAL_TYPE_PM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/**
*@brief uni port count
*
*Deinfe the structure used to ethernet performance moniter*/
typedef struct 
{
    /** Totoal trasmit framer*/
    unsigned long long   ul_tx_total;
    /** Totoal receive framer*/
    unsigned long long   ul_rx_total;
    /** Receive fcs error counter*/
    unsigned long long   ul_rx_fcs_err;             
    /** Transmit FCS errors*/
    unsigned long long   ul_tx_fcs_err;    
    /** Excessive collision counter*/
    unsigned long long   ul_tx_exc_col_err; 
    /** Late collision counter*/
    unsigned long long   ul_tx_late_col_err;
    /** Receive frames too long counter*/
    unsigned long long   ul_rx_frm_too_long_err; 
    /** Buffer overflows on receive counter*/
    unsigned long long   ul_rx_overflow_err; 
    /** Buffer overflows on transmit counter*/
    unsigned long long   ul_tx_overflow_err;     
    /** Single collision frame counter */
    unsigned long long   ul_tx_single_col_err;   
    /** Multiple collision frame counter*/
    unsigned long long   ul_tx_multi_col_err;    
    /** sqee error counter*/
    unsigned long long   ul_sqe_err;     
    /** Deferred transmission counter  */
    unsigned long long   ul_tx_defferred_err;       
    /** Internal MAC transmit error counter */
    unsigned long long   ul_tx_intervel_mac_err;  
    /** Carrier sence error counter */
    unsigned long long   ul_tx_crs_drop_err;    
    /** Alignment error counter  */
    unsigned long long   ul_rx_align_err;     
    /** Internal MAC receive error counter   */
    unsigned long long   ul_rx_interval_mac_err;   
    /** PPPoE filtered frame counter  */
    unsigned long long   ul_rx_pppoe_filter_frame;   
    /**received packets but Drop   */
    unsigned long long   ul_rx_drop_events;
    /** transmitted packets but drop */
    unsigned long long   ul_tx_drop_events;         
    /** totoal trasmitted bytes*/
    unsigned long long   ul_tx_octets;    
    /** totoal trasmitted packets*/
    unsigned long long   ul_tx_pkts;     
    /** totoal received bytes*/
    unsigned long long   ul_rx_octets;    
    /** totoal received packets*/
    unsigned long long   ul_rx_pkts;      
    /** Received undersize packets */
    unsigned long long   ul_rx_undersize_pkts;    
    /** Received oversize packets */
    unsigned long long   ul_rx_oversize_pkts;    
    /** Received undersize packets */
    unsigned long long   ul_tx_undersize_pkts;    
    /** Received oversize packets */
    unsigned long long   ul_tx_oversize_pkts;    
    /** Received fragments packets */
    unsigned long long   ul_rx_fragments; 
    /** Received Jabbers packets */
    unsigned long long   ul_rx_jabbers;    
    /** Received packets from 64 octets  */
    unsigned long long   ul_rx_pkts_64_octets;  
    /** Received packets from 65 to 127 octets   */
    unsigned long long   ul_rx_pkts_65to127_octets;  
    /** Received packets from 128 to 255 octets */
    unsigned long long   ul_rx_pkts_128to255_octets;  
    /** Received packets from 256 to 511 octets   */
    unsigned long long   ul_rx_pkts_256to511_octets; 
    /** Received packets from 512 to 1023 octets  */
    unsigned long long   ul_rx_pkts_512to1023_octets; 
    /** Received packets from 1024 to 1518 octets */
    unsigned long long   ul_rx_pkts_1024to1518_octets;
    /** Received packets from  1519 to max size octets */
    unsigned long long   ul_rx_pkts_1519tomtu_octets;
    /** Transmitted packets from  64 octets  */
    unsigned long long   ul_tx_pkts_64_octets;
    /** Transmitted packets from  65 to 127 octets   */
    unsigned long long   ul_tx_pkts_65to127_octets;
    /** Transmitted packets from  128 to 255 octets */
    unsigned long long   ul_tx_pkts_128to255_octets;
    /** Transmitted packets from  256 to 511 octets   */
    unsigned long long   ul_tx_pkts_256to511_octets;
    /** Transmitted packets from  512 to 1023 octets  */
    unsigned long long   ul_tx_pkts_512to1023_octets;
    /** Transmitted packets from  1024 to 1518 octets */
    unsigned long long   ul_tx_pkts_1024to1518_octets;
    /** Transmitted packets from 1519 to max size octets */
    unsigned long long   ul_tx_pkts_1519tomtu_octets;
    /** Received Unicast pacekts   */
    unsigned long long   ul_rx_uc_pkts;           
    /** Transmitted Unicast pacekts   */
    unsigned long long   ul_tx_uc_pkts;    
    /** Received broadcast pacekts   */
    unsigned long long   ul_rx_bc_pkts; 
    /** Transmitted broadcast pacekts   */
    unsigned long long   ul_tx_bc_pkts;     
    /** Received Multicast pacekts   */
    unsigned long long   ul_rx_mc_pkts; 
    /** Transmitted Multicast pacekts   */
    unsigned long long   ul_tx_mc_pkts;    
    /** Received Pause flow control frame */
    unsigned long long   ul_rx_pause_frame;    
    /** Transmitted Pause flow control frame */
    unsigned long long   ul_tx_pause_frame;  
    /** Received good bytes of good packets*/
    unsigned long long   ul_rx_good_pkts_octets; 
    /** Transmitted good bytes of good packets*/
    unsigned long long   ul_tx_good_pkts_octets; 
    /** Received bad bytes of good packets*/
    unsigned long long   ul_rx_bad_pkts_octets;
    /** Transmitted bad bytes of good packets*/
    unsigned long long   ul_tx_bad_pkts_octets;
    /** Pass through  */
    unsigned int         ui_thru;    
}hal_port_count_t; 

/**
*@brief gemport count
*
*Deinfe the structure used to gemport performance moniter*/
typedef struct
{
    /** upstream frame counter */
    unsigned long long  ul_up_frm_cnt;
    /** downstream frame counter */
    unsigned long long  ul_dn_frm_cnt;
    /** downstream block counter */
    unsigned long long  ul_dn_blk_cnt;
    /** upstream block counter */
    unsigned long long  ul_up_blk_cnt;
    /** discarded downstream gemport frame counter */
    unsigned long long  ul_dn_discd_gemfrm;
    /** discarded upstream gemport frame counter */
    unsigned long long  ul_up_discd_gemfrm;
	/** downstream byte counter */
    unsigned long long  ul_dn_byte_cnt;
    /** upstream byte counter */
    unsigned long long  ul_up_byte_cnt;
}hal_gem_count_t;

/**
*@brief fect count
*
*Deinfe the structure used to fec performance moniter*/
typedef struct
{
    /** fec correct bytes*/
    unsigned int correctedBytes;
    /** fec correct code words*/
    unsigned int correctedCodewords;
    /** fec uncorrectable code words*/
    unsigned int uncorrectableCodewords;
}hal_fec_count_t;

/**
*@brief wan count
*
*Deinfe the structure used to wan performance moniter*/
typedef struct
{
    /** the subnet id need to do  statistic*/
    unsigned int subnetId;
    /** received vlaid packets*/
    unsigned int  rxValidPackets;
    /** received but dropped packets*/
    unsigned int rxDroppedPackets;
    /** transmitted vlaid packets*/
    unsigned int txValidPackets;
    /** transmitted but dropped packets*/
    unsigned int  txDroppedPackets;
    
} hal_wan_count_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif 

#endif




