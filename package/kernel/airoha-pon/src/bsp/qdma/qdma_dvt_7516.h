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
#ifndef _QDMA_DVT_H_
#define _QDMA_DVT_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	
#define CONFIG_QDMA_RX_CHANNEL		11
#define QDMA_NAPI_WEIGHT			64  //(CONFIG_RX_DSCP_NUM[RING_IDX_1] >> 1) modify by czw 20161206

#if defined(QDMA_LAN)
#define pon_loopback_result pon_loopback_result_lan
#define self_test_path "qdma_lan/self_test"
#define test_dev "qdma_dvt_lan"
#else
#define pon_loopback_result pon_loopback_result_wan
#define self_test_path "qdma_wan/self_test"
#define test_dev "qdma_dvt_wan"
#endif

#if defined(TCSUPPORT_MAX_PACKET_2000)
#define CONFIG_LOOPBACK_MAX_PKT_LENS		(1800) //2048
#else
#define CONFIG_LOOPBACK_MAX_PKT_LENS		(1600)
#endif

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/
#ifdef CONFIG_SUPPORT_SELF_TEST

typedef union {
	struct {
#ifdef __BIG_ENDIAN
		uint loopcnt			: 8 ;
		uint resv1				: 12 ;
		uint oam				: 1 ;
		uint channel			: 8 ;
		uint queue				: 3 ;
#else
		uint queue				: 3 ;
		uint channel			: 8 ;
		uint oam				: 1 ;
		uint resv1				: 12 ;
		uint loopcnt			: 8 ;
#endif /* __BIG_ENDIAN */
	
#ifdef __BIG_ENDIAN
		uint resv2				: 10 ;
		uint fport				: 3 ;
		uint resv3				: 19 ;
#else
		uint resv3				: 19 ;
		uint fport				: 3 ;
		uint resv2				: 10 ;
#endif /* __BIG_ENDIAN */

	} raw ;
	uint msg[2] ;
} QDMA_TxMsg_T ;


#if 0

typedef union {
	struct {
#ifdef __BIG_ENDIAN
		uint loopcnt			: 8 ;
		uint resv1				: 12 ;
		uint oam				: 1 ;
		uint channel			: 8 ;
		uint queue				: 3 ;
#else
		uint queue				: 3 ;
		uint channel			: 8 ;
		uint oam				: 1 ;
		uint resv1				: 12 ;
		uint loopcnt			: 8 ;
#endif /* __BIG_ENDIAN */
	
#ifdef __BIG_ENDIAN
		uint resv2				: 10 ;
		uint fport				: 3 ;
		uint resv3				: 19 ;
#else
		uint resv3				: 19 ;
		uint fport				: 3 ;
		uint resv2				: 10 ;
#endif /* __BIG_ENDIAN */

	} raw ;
	uint msg[2] ;
} FETxMsg_T ;

typedef union {
	struct {
#ifdef __BIG_ENDIAN
		uint 					: 8 ;
		uint gem				: 12 ;
		uint oam				: 1 ;
		uint channel			: 8 ;
		uint longf				: 1 ;
		uint runtf				: 1 ;
		uint crcer				: 1 ;
#else
		uint crcer				: 1 ;
		uint runtf				: 1 ;
		uint longf				: 1 ;
		uint channel			: 8 ;
		uint oam				: 1 ;
		uint gem				: 12 ;
		uint 					: 8 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
		uint 					: 3 ;
		//uint l2b				: 1 ;
		uint ipv6				: 1 ;
		uint ipv4				: 1 ;
		uint ipv4f				: 1 ;
		uint tack				: 1 ;
		uint l4vld				: 1 ;
		uint l4f				: 1 ;
		uint sport				: 4 ; //5->4
		uint crsn				: 5 ;
		//uint 					: 1 ;
		uint ifc_fid			: 8 ;
		uint ifc_hit			: 1 ;
		uint pppoe				: 1 ;
		uint snap				: 1 ;
		uint vln				: 3 ;
#else
		uint vln				: 3 ;
		uint snap				: 1 ;
		uint pppoe				: 1 ;
		uint ifc_hit			: 1 ;
		uint ifc_fid			: 8 ;
		//uint					: 1 ;
		uint crsn				: 5 ;
		uint sport				: 4 ; //5->4
		uint l4f				: 1 ;
		uint l4vld				: 1 ;
		uint tack				: 1 ;
		uint ipv4f				: 1 ;
		uint ipv4				: 1 ;
		uint ipv6				: 1 ;
		//uint l2b				: 1 ;
		uint					: 3 ;

#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
		uint 					: 13 ;
		uint vlan				: 1 ;
		uint tpid				: 2 ;
		uint vid				: 16 ;
#else
		uint vid				: 16 ;
		uint tpid				: 2 ;
		uint vlan				: 1 ;
		uint 					: 13 ;
#endif /* __BIG_ENDIAN */

		uint timestamp ;
	} ;
	uint word[4] ;
} FERxMsg_T ;

#endif



typedef union {
	struct {
#ifdef __BIG_ENDIAN
		uint resv1				: 27 ;
		uint channel			: 5 ;
#else
		uint channel			: 5 ;
		uint resv1				: 27 ;
#endif /* __BIG_ENDIAN */
		uint resv2 ;
		uint resv3 ;
		uint resv4 ;
	} raw ;
	uint msg[2] ;
} QDMA_RxMsg_T ; ;

#else
typedef struct {
#ifdef __BIG_ENDIAN
	uint resv1				: 24 ;
	uint channel			: 4 ;
	uint queue				: 4 ;
#else
	uint queue				: 4 ;
	uint channel			: 4 ;
	uint resv1				: 24 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
	uint resv2				: 10 ;
	uint fport				: 3 ;
	uint resv3				: 19 ;
#else
	uint resv3				: 19 ;
	uint fport				: 3 ;
	uint resv2				: 10 ;
#endif /* __BIG_ENDIAN */
} QDMA_TxMsg_T ;

typedef struct {
#ifdef __BIG_ENDIAN
	uint resv1				: 28 ;
	uint channel			: 4 ;
#else
	uint channel			: 4 ;
	uint resv1				: 28 ;
#endif /* __BIG_ENDIAN */
	uint resv2 ;
	uint resv3 ;
	uint resv4 ;
} QDMA_RxMsg_T ;
#endif /* CONFIG_SUPPORT_SELF_TEST */

typedef struct {
	uint tx_frames ;
	uint rx_frames ;
	uint rx_err_frames ;
} QDMA_DbgCounters_T ;


typedef struct {
	struct net_device	*dev;
	struct napi_struct	napi;
} QDMA_Adapter_T;


/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
int qdma_dvt_init(void) ;
int qdma_dvt_deinit(void) ;

#endif /* _QDMA_DVT_H_ */
