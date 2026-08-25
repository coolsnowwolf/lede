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
#ifndef _QDMA_GLB_H_
#define _QDMA_GLB_H_

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/*EN7580: WAN: HWF dscp max: 16K/64K , Per-RingSize max: 4K , IRQ max depth:4K*/
/*EN7580: LAN: HWF dscp max: 16K/64K , Per-RingSize max: 4K , IRQ max depth:4K*/

/*EN7516: WAN: HWF dscp max: 8K , Per-RingSize max: 4k , IRQ max depth:4K*/
/*EN7516: LAN: HWF dscp max: 2K , Per-RingSize max: 1k , IRQ max depth:4K*/

#if defined(QDMA_LAN) // qdma1
    #define CONFIG_QDMA_QUEUE					8
	
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_HWFWD_DSCP_NUM               (1000)	// for csr2 test
#elif defined(TCSUPPORT_CPU_EN7580)
#if defined(TCSUPPORT_CPU_EN7523)
	#define CONFIG_HWFWD_DSCP_NUM               (16384)
#endif
    #define CONFIG_HWFWD_SRAM_DSCP_NUM         	(16384) //SRAM:16k
    #define CONFIG_HWFWD_DRAM_DSCP_NUM         	(65536) //DRAM:64k
#else
#ifdef	TCSUPPORT_DSL_PHYMODE
	#define CONFIG_HWFWD_DSCP_NUM				(512) 	
#else
#if defined(TCSUPPORT_NP_RAM_SHRINK)
    #define CONFIG_HWFWD_DSCP_NUM				(1024) 	/*1024 ->2048 shoule not more than 4096*/
#else
	#define CONFIG_HWFWD_DSCP_NUM				(2048)  /*1024 ->2048 shoule not more than 4096*/
#endif
#endif
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#if defined(TCSUPPORT_CPU_EN7523)
	#define CONFIG_HWFWD_DSCP_NUM_MAX           (16384)
#endif
    #define CONFIG_HWFWD_SRAM_DSCP_NUM_MAX      (16384) //SRAM:16k
    #define CONFIG_HWFWD_DRAM_DSCP_NUM_MAX      (65536) //DRAM:64k

#if defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_TX31_DSCP_NUM 				(128)
	#define CONFIG_TX30_DSCP_NUM 				(128)
	#define CONFIG_TX29_DSCP_NUM 				(128)
	#define CONFIG_TX28_DSCP_NUM 				(128)
	#define CONFIG_TX27_DSCP_NUM 				(128)
	#define CONFIG_TX26_DSCP_NUM 				(128)
	#define CONFIG_TX25_DSCP_NUM 				(128)
	#define CONFIG_TX24_DSCP_NUM 				(128)
	#define CONFIG_TX23_DSCP_NUM				(128)
	#define CONFIG_TX22_DSCP_NUM				(128)
	#define CONFIG_TX21_DSCP_NUM				(128)
	#define CONFIG_TX20_DSCP_NUM				(128)
	#define CONFIG_TX19_DSCP_NUM				(128)
	#define CONFIG_TX18_DSCP_NUM				(128)
	#define CONFIG_TX17_DSCP_NUM				(128)
	#define CONFIG_TX16_DSCP_NUM				(128)
	#define CONFIG_TX15_DSCP_NUM				(128)
	#define CONFIG_TX14_DSCP_NUM				(128)
	#define CONFIG_TX13_DSCP_NUM				(128)
	#define CONFIG_TX12_DSCP_NUM				(128)
	#define CONFIG_TX11_DSCP_NUM				(128)
	#define CONFIG_TX10_DSCP_NUM				(128)
	#define CONFIG_TX9_DSCP_NUM					(128)
	#define CONFIG_TX8_DSCP_NUM					(128)
#endif	
    #define CONFIG_TX7_DSCP_NUM					(4096)
#ifdef TCSUPPORT_NPU_WIFI_OFFLOAD
    #define CONFIG_TX6_DSCP_NUM					(1024)
#else
    #define CONFIG_TX6_DSCP_NUM					(128)
#endif
    #define CONFIG_TX5_DSCP_NUM					(128)
    #define CONFIG_TX4_DSCP_NUM					(128)
    #define CONFIG_TX3_DSCP_NUM					(128)
    #define CONFIG_TX2_DSCP_NUM					(128)
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(1536)

#if 0//defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_RX31_DSCP_NUM				(16)
	#define CONFIG_RX30_DSCP_NUM				(16)
	#define CONFIG_RX29_DSCP_NUM				(16)
	#define CONFIG_RX28_DSCP_NUM				(16)
	#define CONFIG_RX27_DSCP_NUM				(16)
	#define CONFIG_RX26_DSCP_NUM				(16)
	#define CONFIG_RX25_DSCP_NUM 				(16)
	#define CONFIG_RX24_DSCP_NUM 				(16)
	#define CONFIG_RX23_DSCP_NUM 				(16)
	#define CONFIG_RX22_DSCP_NUM 				(16)	
	#define CONFIG_RX21_DSCP_NUM 				(16)	
	#define CONFIG_RX20_DSCP_NUM 				(16)
	#define CONFIG_RX19_DSCP_NUM 				(16)
	#define CONFIG_RX18_DSCP_NUM 				(16)
	#define CONFIG_RX17_DSCP_NUM				(16)
	#define CONFIG_RX16_DSCP_NUM				(16)
#endif
    #define CONFIG_RX15_DSCP_NUM				(128)
    #define CONFIG_RX14_DSCP_NUM				(16)
    #define CONFIG_RX13_DSCP_NUM				(16)
    #define CONFIG_RX12_DSCP_NUM				(16)
    #define CONFIG_RX11_DSCP_NUM				(128)
    #define CONFIG_RX10_DSCP_NUM				(16)
    #define CONFIG_RX9_DSCP_NUM					(16)
    #if defined(TCSUPPORT_AUTOBENCH)
    #define CONFIG_RX8_DSCP_NUM					(1024)
    #else
    #define CONFIG_RX8_DSCP_NUM					(16)
    #endif
    #define CONFIG_RX7_DSCP_NUM					(16)
#if defined(TCSUPPORT_WLAN)
#if !defined(TCSUPPORT_OPENWRT) && (defined(TCSUPPORT_NPU_WIFI_OFFLOAD) || defined(TCSUPPORT_AUTOBENCH) || defined(TCSUPPORT_SECOND_IF_NONE))
    #define CONFIG_RX6_DSCP_NUM					(8)	/*WIFI-5G*/
#else
    #define CONFIG_RX6_DSCP_NUM					(4096)	/*WIFI-5G*/
#endif
#if defined(TCSUPPORT_AUTOBENCH) || defined(TCSUPPORT_FIRST_IF_NONE)
	#define CONFIG_RX5_DSCP_NUM					(8)	/*WIFI-2.4G*/
#else
#if defined(TCSUPPORT_MEMORY_SHRINK_V2) 
	#define CONFIG_RX5_DSCP_NUM					(512)	/*WIFI-2.4G*/
#elif defined(TCSUPPORT_MEMORY_SHRINK_ENHANCE)
	#define CONFIG_RX5_DSCP_NUM					(1024)	/*WIFI-2.4G*/
#else
    #define CONFIG_RX5_DSCP_NUM					(4096)	/*WIFI-2.4G*/
#endif
#endif
#else
    #define CONFIG_RX6_DSCP_NUM                 (16)  
    #define CONFIG_RX5_DSCP_NUM                 (16)  
#endif
    #define CONFIG_RX4_DSCP_NUM					(16)
    #define CONFIG_RX3_DSCP_NUM					(16)
    #define CONFIG_RX2_DSCP_NUM					(128)
    
#elif defined(TCSUPPORT_CPU_EN7516) || defined(TCSUPPORT_CPU_EN7527)
#ifdef	TCSUPPORT_DSL_PHYMODE
	#define CONFIG_HWFWD_DSCP_NUM_MAX			(2048)
	
	#define CONFIG_TX1_DSCP_NUM 				(128)
	#define CONFIG_TX0_DSCP_NUM 				(256)
    
	#define CONFIG_RX15_DSCP_NUM				(128)
	#define CONFIG_RX14_DSCP_NUM				(2)
	#define CONFIG_RX13_DSCP_NUM				(2)
	#define CONFIG_RX12_DSCP_NUM				(2)
	#define CONFIG_RX11_DSCP_NUM				(2)
	#define CONFIG_RX10_DSCP_NUM				(2)
	#define CONFIG_RX9_DSCP_NUM 				(2)
	#define CONFIG_RX8_DSCP_NUM 				(2)
	#define CONFIG_RX7_DSCP_NUM 				(256)
	#define CONFIG_RX6_DSCP_NUM 				(2) /*WIFI-5G*/
	#define CONFIG_RX5_DSCP_NUM 				(2) /*WIFI-2.4G*/
	#define CONFIG_RX4_DSCP_NUM 				(2)
	#define CONFIG_RX3_DSCP_NUM 				(2)
	#define CONFIG_RX2_DSCP_NUM 				(2)
#else

#if defined(TCSUPPORT_NP_RAM_SHRINK)
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(128)
    #define CONFIG_RX15_DSCP_NUM				(8)
    #define CONFIG_RX14_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX13_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX12_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX11_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX10_DSCP_NUM				(2)/*(16)*/
    #define CONFIG_RX9_DSCP_NUM					(2)/*(16)*/
    #define CONFIG_RX8_DSCP_NUM					(2)/*(16)*/
    #define CONFIG_RX7_DSCP_NUM					(2)/*(256)*/
    #define CONFIG_RX6_DSCP_NUM					(560)/*(2048)*/ /*WIFI-5G*/
    #define CONFIG_RX5_DSCP_NUM					(560)/*(2048)*/ /*WIFI-2.4G*/
    #define CONFIG_RX4_DSCP_NUM					(2)
    #define CONFIG_RX3_DSCP_NUM					(2)
    #define CONFIG_RX2_DSCP_NUM					(2)
#else
    #define CONFIG_HWFWD_DSCP_NUM_MAX           (2048)
	
    #define CONFIG_TX1_DSCP_NUM					(128)
#if defined(TCSUPPORT_CPU_EN7516) && defined(TCSUPPORT_WLAN_MT7615_V34)
    #define CONFIG_TX0_DSCP_NUM					(2000)
#else
    #define CONFIG_TX0_DSCP_NUM					(1536)
#endif    
    #define CONFIG_RX15_DSCP_NUM				(128)
    #define CONFIG_RX14_DSCP_NUM				(16)
    #define CONFIG_RX13_DSCP_NUM				(16)
    #define CONFIG_RX12_DSCP_NUM				(16)
    #define CONFIG_RX11_DSCP_NUM				(16)
    #define CONFIG_RX10_DSCP_NUM				(16)
    #define CONFIG_RX9_DSCP_NUM					(16)
    #define CONFIG_RX8_DSCP_NUM					(16)
    #define CONFIG_RX7_DSCP_NUM					(256)
    #define CONFIG_RX6_DSCP_NUM					(2048) /*WIFI-5G*/
    #define CONFIG_RX5_DSCP_NUM					(2048) /*WIFI-2.4G*/
    #define CONFIG_RX4_DSCP_NUM					(16)
    #define CONFIG_RX3_DSCP_NUM					(16)
    #define CONFIG_RX2_DSCP_NUM					(128)

#endif
#endif
#endif


#if defined(TCSUPPORT_NP)
	#define CONFIG_RX1_DSCP_NUM 				(256) 
#else
#ifdef MT7615E	    
#if (defined(TCSUPPORT_VOIP) || defined(WAN_GPON) || defined(WAN_EPON))
    #define CONFIG_RX1_DSCP_NUM					(1024) 
#else /* TCSUPPORT_VOIP */
    #define CONFIG_RX1_DSCP_NUM					(896)
#endif /* TCSUPPORT_VOIP */
#else
    #define CONFIG_RX1_DSCP_NUM					(1024) 
#endif	
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_JOYME2) || defined(TCSUPPORT_CF_JOYMEV2_PON)
    #define CONFIG_RX0_DSCP_NUM					(512)
#else/*TCSUPPORT_COMPILE*/
    #define CONFIG_RX0_DSCP_NUM					(1024)
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_DSCP_NUM_MAX                 (65536)
#else
    #define CONFIG_DSCP_NUM_MAX                 (4096)
#endif
    #define CONFIG_IRQ_NUM_MAX                  (4095)
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_IRQ_DEPTH					(512)
	#define CONFIG_IRQ2_DEPTH					(512)
#else
    #define CONFIG_IRQ_DEPTH					(2048)
	#define CONFIG_IRQ2_DEPTH					(1024)
#endif
    #define CONFIG_MAX_PKT_LENS					(2048)
	#define HWFWD_LOW_THRESHOLD					(128)

#if defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_RX31_DSCP_NUM				(8)
	#define CONFIG_RX30_DSCP_NUM				(8)
	#define CONFIG_RX29_DSCP_NUM				(8)
	#define CONFIG_RX28_DSCP_NUM				(8)
	#define CONFIG_RX27_DSCP_NUM				(8)
	#define CONFIG_RX26_DSCP_NUM				(8)
	#define CONFIG_RX25_DSCP_NUM 				(8)
	#define CONFIG_RX24_DSCP_NUM 				(8)
	#define CONFIG_RX23_DSCP_NUM 				(8)
	#define CONFIG_RX22_DSCP_NUM 				(8)	
	#define CONFIG_RX21_DSCP_NUM 				(8)	
	#define CONFIG_RX20_DSCP_NUM 				(8)
	#define CONFIG_RX19_DSCP_NUM 				(8)
	#define CONFIG_RX18_DSCP_NUM 				(8)
	#define CONFIG_RX17_DSCP_NUM				(8)
	#define CONFIG_RX16_DSCP_NUM				(8)
		
	#define CONFIG_RX15_DSCP_NUM				(8)
    #define CONFIG_RX14_DSCP_NUM				(8)
    #define CONFIG_RX13_DSCP_NUM				(8)
    #define CONFIG_RX12_DSCP_NUM				(8)
    #define CONFIG_RX11_DSCP_NUM				(8)
    #define CONFIG_RX10_DSCP_NUM				(8)
    #define CONFIG_RX9_DSCP_NUM					(8)
	#define CONFIG_RX8_DSCP_NUM					(8)
	#define CONFIG_RX7_DSCP_NUM					(8)
	#define CONFIG_RX6_DSCP_NUM					(8)		
	#define CONFIG_RX5_DSCP_NUM					(512)    //wifi comment
	#define CONFIG_RX4_DSCP_NUM					(16)
	#define CONFIG_RX3_DSCP_NUM					(16)
	#define CONFIG_RX2_DSCP_NUM					(128)
	#define CONFIG_RX1_DSCP_NUM					(1024)
	#define CONFIG_RX0_DSCP_NUM					(512)
#endif


#else // qdma2
    #define CONFIG_QDMA_QUEUE                   8
	
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_HWFWD_DSCP_NUM               (1000)	// for csr2 test
#elif defined TCSUPPORT_CPU_EN7580
#if defined(TCSUPPORT_CPU_EN7523)
	#define CONFIG_HWFWD_DSCP_NUM               (16384)
#endif

#if defined(TCSUPPORT_CPU_EN7581)
    #define CONFIG_HWFWD_SRAM_DSCP_NUM          (20*1024) //SRAM:20k
    #define CONFIG_HWFWD_DRAM_DSCP_NUM          (80*1024) //DRAM:80k
#else
    #define CONFIG_HWFWD_SRAM_DSCP_NUM          (16*1024) //SRAM:16k
    #define CONFIG_HWFWD_DRAM_DSCP_NUM          (64*1024) //DRAM:64k
#endif
#else
#if defined(TCSUPPORT_AUTOBENCH) && defined(TCSUPPORT_CPU_EN7528)
	#define CONFIG_HWFWD_DSCP_NUM               (512)
#else
#ifdef	TCSUPPORT_DSL_PHYMODE
    #define CONFIG_HWFWD_DSCP_NUM               (512)
#else
	#define CONFIG_HWFWD_DSCP_NUM				(8191)
#endif
#endif
#endif

#if defined(TCSUPPORT_CPU_EN7580)
#if defined(TCSUPPORT_CPU_EN7523)
	#define CONFIG_HWFWD_DSCP_NUM_MAX           (16384)
#endif
#if defined(TCSUPPORT_CPU_EN7581)
    #define CONFIG_HWFWD_SRAM_DSCP_NUM_MAX      (20480) //SRAM:20k
    #define CONFIG_HWFWD_DRAM_DSCP_NUM_MAX      (81920) //DRAM:80k
#else
    #define CONFIG_HWFWD_SRAM_DSCP_NUM_MAX      (16384) //SRAM:16k
    #define CONFIG_HWFWD_DRAM_DSCP_NUM_MAX      (65536) //DRAM:64k
#endif
#if defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_TX31_DSCP_NUM 				(128)
	#define CONFIG_TX30_DSCP_NUM 				(128)
	#define CONFIG_TX29_DSCP_NUM 				(128)
	#define CONFIG_TX28_DSCP_NUM 				(128)
	#define CONFIG_TX27_DSCP_NUM 				(128)
	#define CONFIG_TX26_DSCP_NUM 				(128)
	#define CONFIG_TX25_DSCP_NUM 				(128)
	#define CONFIG_TX24_DSCP_NUM 				(128)
	#define CONFIG_TX23_DSCP_NUM				(128)
	#define CONFIG_TX22_DSCP_NUM				(128)
	#define CONFIG_TX21_DSCP_NUM				(128)
	#define CONFIG_TX20_DSCP_NUM				(128)
	#define CONFIG_TX19_DSCP_NUM				(128)
	#define CONFIG_TX18_DSCP_NUM				(128)
	#define CONFIG_TX17_DSCP_NUM				(128)
	#define CONFIG_TX16_DSCP_NUM				(128)
	#define CONFIG_TX15_DSCP_NUM				(128)
	#define CONFIG_TX14_DSCP_NUM				(128)
	#define CONFIG_TX13_DSCP_NUM				(128)
	#define CONFIG_TX12_DSCP_NUM				(128)
	#define CONFIG_TX11_DSCP_NUM				(128)
	#define CONFIG_TX10_DSCP_NUM				(128)
	#define CONFIG_TX9_DSCP_NUM					(128)
	#define CONFIG_TX8_DSCP_NUM					(128)
#endif	
	
    #define CONFIG_TX7_DSCP_NUM					(128)
    #define CONFIG_TX6_DSCP_NUM					(128)
    #define CONFIG_TX5_DSCP_NUM					(128)
    #define CONFIG_TX4_DSCP_NUM					(128)
    #define CONFIG_TX3_DSCP_NUM					(128)
    #define CONFIG_TX2_DSCP_NUM					(128)
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(1536)

#if 0//defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_RX31_DSCP_NUM				(16)
	#define CONFIG_RX30_DSCP_NUM				(16)
	#define CONFIG_RX29_DSCP_NUM				(16)
	#define CONFIG_RX28_DSCP_NUM				(16)
	#define CONFIG_RX27_DSCP_NUM				(16)
	#define CONFIG_RX26_DSCP_NUM				(128)	//for speedtest lro
	#define CONFIG_RX25_DSCP_NUM 				(128)	//for speedtest lro
	#define CONFIG_RX24_DSCP_NUM 				(128)	//for speedtest lro
	#define CONFIG_RX23_DSCP_NUM 				(128)	//for speedtest lro
	#define CONFIG_RX22_DSCP_NUM 				(16)
	#define CONFIG_RX21_DSCP_NUM 				(16)
	#define CONFIG_RX20_DSCP_NUM 				(16)
	#define CONFIG_RX19_DSCP_NUM 				(16)
	#define CONFIG_RX18_DSCP_NUM 				(16)
	#define CONFIG_RX17_DSCP_NUM				(16)
	#define CONFIG_RX16_DSCP_NUM				(16)
#endif
#if !defined(TCSUPPORT_CPU_EN7581)
    #define CONFIG_RX15_DSCP_NUM				(128)
    #define CONFIG_RX14_DSCP_NUM				(16)
    #define CONFIG_RX13_DSCP_NUM				(16)
    #define CONFIG_RX12_DSCP_NUM				(16)
    #define CONFIG_RX11_DSCP_NUM				(16)
    #define CONFIG_RX10_DSCP_NUM				(16)
    #define CONFIG_RX9_DSCP_NUM					(16)
    #define CONFIG_RX8_DSCP_NUM					(16)
    #define CONFIG_RX7_DSCP_NUM					(16)
    #define CONFIG_RX6_DSCP_NUM					(256)	//for speedtest
    #define CONFIG_RX5_DSCP_NUM					(16)
    #define CONFIG_RX4_DSCP_NUM					(16)
    #define CONFIG_RX3_DSCP_NUM					(16)
    #define CONFIG_RX2_DSCP_NUM					(128)
    #define CONFIG_RX1_DSCP_NUM					(256)
    #define CONFIG_RX0_DSCP_NUM					(256)
#endif

#elif defined(TCSUPPORT_CPU_EN7516) || defined(TCSUPPORT_CPU_EN7527)
#ifdef	TCSUPPORT_DSL_PHYMODE
	
    #define CONFIG_HWFWD_DSCP_NUM_MAX			(8191)
	 
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(256)
		
    #define CONFIG_RX15_DSCP_NUM				(2)
    #define CONFIG_RX14_DSCP_NUM				(2)
    #define CONFIG_RX13_DSCP_NUM				(2)
    #define CONFIG_RX12_DSCP_NUM				(2)
    #define CONFIG_RX11_DSCP_NUM				(2)
    #define CONFIG_RX10_DSCP_NUM				(2)
    #define CONFIG_RX9_DSCP_NUM					(2)
    #define CONFIG_RX8_DSCP_NUM					(2)
    #define CONFIG_RX7_DSCP_NUM					(2)
    #define CONFIG_RX6_DSCP_NUM					(2)
    #define CONFIG_RX5_DSCP_NUM					(2)
    #define CONFIG_RX4_DSCP_NUM					(2)
    #define CONFIG_RX3_DSCP_NUM					(2)
    #define CONFIG_RX2_DSCP_NUM					(2)
    #define CONFIG_RX1_DSCP_NUM					(128)
    #define CONFIG_RX0_DSCP_NUM					(128)
#else

    #define CONFIG_HWFWD_DSCP_NUM_MAX			(8191)
 
    #define CONFIG_TX1_DSCP_NUM					(128)
    #define CONFIG_TX0_DSCP_NUM					(1536)
	
    #define CONFIG_RX15_DSCP_NUM				(128)
    #define CONFIG_RX14_DSCP_NUM				(16)
    #define CONFIG_RX13_DSCP_NUM				(16)
    #define CONFIG_RX12_DSCP_NUM				(16)
    #define CONFIG_RX11_DSCP_NUM				(16)
    #define CONFIG_RX10_DSCP_NUM				(16)
    #define CONFIG_RX9_DSCP_NUM					(16)
    #define CONFIG_RX8_DSCP_NUM					(16)
    #define CONFIG_RX7_DSCP_NUM					(16)
    #define CONFIG_RX6_DSCP_NUM					(16)
    #define CONFIG_RX5_DSCP_NUM					(16)
    #define CONFIG_RX4_DSCP_NUM					(16)
    #define CONFIG_RX3_DSCP_NUM					(16)
    #define CONFIG_RX2_DSCP_NUM					(128)
    #define CONFIG_RX1_DSCP_NUM					(256)
    #define CONFIG_RX0_DSCP_NUM					(1024)
#endif
#endif

#if defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_RX31_DSCP_NUM				(8)
	#define CONFIG_RX30_DSCP_NUM				(8)
	#define CONFIG_RX29_DSCP_NUM				(8)
	#define CONFIG_RX28_DSCP_NUM				(8)
	#define CONFIG_RX27_DSCP_NUM				(8)

#if defined(TCSUPPORT_LRO_ENABLE)
	#define CONFIG_RX26_DSCP_NUM				(128)
	#define CONFIG_RX25_DSCP_NUM 				(128)
	#define CONFIG_RX24_DSCP_NUM 				(128)
	#define CONFIG_RX23_DSCP_NUM 				(128)
#else
	#define CONFIG_RX26_DSCP_NUM				(8)
	#define CONFIG_RX25_DSCP_NUM 				(8)
	#define CONFIG_RX24_DSCP_NUM 				(8)
	#define CONFIG_RX23_DSCP_NUM 				(8)
#endif
	
	#define CONFIG_RX22_DSCP_NUM 				(8)	
	#define CONFIG_RX21_DSCP_NUM 				(8)	
	#define CONFIG_RX20_DSCP_NUM 				(8)
	#define CONFIG_RX19_DSCP_NUM 				(8)
	#define CONFIG_RX18_DSCP_NUM 				(8)
	#define CONFIG_RX17_DSCP_NUM				(8)
	#define CONFIG_RX16_DSCP_NUM				(8)
		
	#define CONFIG_RX15_DSCP_NUM				(128)  //ifc special  packets
    #define CONFIG_RX14_DSCP_NUM				(8)
    #define CONFIG_RX13_DSCP_NUM				(8)
    #define CONFIG_RX12_DSCP_NUM				(8)
    #define CONFIG_RX11_DSCP_NUM				(8)
    #define CONFIG_RX10_DSCP_NUM				(8)
    #define CONFIG_RX9_DSCP_NUM					(8)
	#define CONFIG_RX8_DSCP_NUM					(8)
	#define CONFIG_RX7_DSCP_NUM					(8)
	#define CONFIG_RX6_DSCP_NUM					(256)  //speedtest w/o lro		
	#define CONFIG_RX5_DSCP_NUM					(8)    
	#define CONFIG_RX4_DSCP_NUM					(8)
	#define CONFIG_RX3_DSCP_NUM					(8)
	#define CONFIG_RX2_DSCP_NUM					(128)
	#define CONFIG_RX1_DSCP_NUM					(256)
	#define CONFIG_RX0_DSCP_NUM					(256)
#endif


#if defined(TCSUPPORT_CPU_EN7581)
	#define CONFIG_DSCP_NUM_MAX                 (65536)
#else
    #define CONFIG_DSCP_NUM_MAX                 (4096)
#endif
    #define CONFIG_IRQ_NUM_MAX                  (4095)
#if defined(CONFIG_SUPPORT_SELF_TEST)
    #define CONFIG_IRQ_DEPTH                    (512)
	#define CONFIG_IRQ2_DEPTH                   (512)
#else
    #define CONFIG_IRQ_DEPTH                    (2048)
	#define CONFIG_IRQ2_DEPTH                   (512)
#endif
    #define CONFIG_MAX_PKT_LENS                 (2048)
    #define HWFWD_LOW_THRESHOLD                 (256)
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
    #define CONFIG_MAX_CNTR_NUM 				(32)	/*DBG CNT GROUP*/
#elif defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
    #define CONFIG_MAX_CNTR_NUM                 (64)    /*DBG CNT GROUP*/
#else
    #define CONFIG_MAX_CNTR_NUM                 (40)	
#endif

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
#define CONFIG_FLOWCNT_MAX_DROUP_NUM            (3)
#define CONFIG_FLOWCNT_GRP0_MAX_IDX_NUM         (63)	/* ACNT_GRP[5:0] */
#define CONFIG_FLOWCNT_GRP1_MAX_IDX_NUM         (31)	/* ACNT_GRP[10:6] */
#define CONFIG_FLOWCNT_GRP2_MAX_IDX_NUM         (127)	/* MTR_GRP[6:0] */
#else
#define CONFIG_FLOWCNT_MAX_DROUP_NUM            (2)
#define CONFIG_FLOWCNT_GRP0_MAX_IDX_NUM         (31)
#define CONFIG_FLOWCNT_GRP1_MAX_IDX_NUM         (16)
#endif
#define GENERAL_INGRESS_INDEX_MAX               (127)   /*EN7580: 128 ratelimit. or 64 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP1               (31)   /*grp1: 32 ratelimit. or 16 trtcm*/
#define GENERAL_INGRESS_INDEX_MAX_GRP2               (15)   /*grp2: 16 ratelimit. or 8 trtcm*/
#define GENERAL_SLA_INDEX_MAX					(31)    /*EN7581 || EN7523: SLA TRTCM INDEX, 2bit: SLA index, 3bit: queue index*/
#define SLA_MAX_NUM								(4)     /*EN7581 || EN7523: SLA support 4 channels at most*/
#define QUEUE_ALL_NUM                           (CONFIG_QDMA_CHANNEL*CONFIG_QDMA_QUEUE)
#define TX_WRED_THR_NUM                         (5)
#define TX_WRED_PROBABILITY_NUM                 (4)

#define CONFIG_PAYLOAD_256_BYTE                 (256)
#define CONFIG_PAYLOAD_512_BYTE                 (512)
#define CONFIG_PAYLOAD_1K_BYTE                  (1024)
#define CONFIG_PAYLOAD_2K_BYTE                  (2048)


#ifndef TIMER_FUN_PAAM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

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

#endif /* _QDMA_GLB_H_ */

