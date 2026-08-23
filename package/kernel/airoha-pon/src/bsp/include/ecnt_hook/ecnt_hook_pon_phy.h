/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef __LINUX_ENCT_HOOK_PON_PHY_H
#define __LINUX_ENCT_HOOK_PON_PHY_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <ecnt_hook/ecnt_hook.h>
#include <linux/jiffies.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/* SET function */

#define PON_SET_PHY_STOP                                0x0002               /* phy_stop()            */
#define PON_SET_PHY_START                         		0x0001 			     /* phy_start()           */
#define PON_SET_PHY_RESET_INIT                    		0x0003 			     /* pon_phy_reset_init()  */
#define PON_SET_PHY_DEV_INIT                      		0x0004 			     /* pon_dev_init()        */
#define PON_SET_PHY_FW_READY                            0x0005 			     /* phy_fw_ready()        */
#define PON_SET_EPONMODE_PHY_RESET                      0x0006 			     /* eponMode_phy_reset()  */
#define PON_SET_PHY_MODE_CONFIG                         0x0007 			     /* phy_mode_config()     */
#define PON_SET_PHY_TRANS_MODEL_SETTING                 0x0008 			     /* phy_trans_model_setting() */
#define PON_SET_PHY_COUNTER_CLEAR                       0x0009 			     /* phy_counter_clear()       */
#define PON_SET_PHY_BIT_DELAY                           0x000a 			     /* phy_bit_delay()           */
#define PON_SET_PHY_TX_POWER_CONFIG                     0x000b 			     /* phy_tx_power_config()     */
#define PON_SET_PHY_TRANS_POWER_SWITCH                  0x000c 			     /* phy_trans_power_switch()  */
#define PON_SET_PHY_TX_BURST_CONFIG                     0x000d 			     /* phy_tx_burst_config()     */
#define PON_SET_PHY_GPON_DELIMITER_GUARD                0x000e 			     /* phy_gpon_delimiter_guard()*/
#define PON_SET_PHY_TX_D_PADDING_CONFIG              0x000f
#define PON_SET_PHY_TRANS_TX_SETTINGS                   0x0010 			     /* phy_trans_tx_setting()    */
#define PON_SET_PHY_GPON_PREAMBLE                       0x0011 			     /* phy_gpon_preamble()       */
#define PON_SET_PHY_GPON_EXTEND_PREAMBLE                0x0012 			     /* phy_gpon_extend_preamble()*/
#define PON_SET_PHY_ROGUE_PRBS_CONFIG                   0x0013 			     /* phy_rogue_prbs_config()   */
#define PON_SET_PHY_RX_FEC_SETTING                      0x0014 			     /* phy_rx_fec_setting()      */
#define PON_SET_PHY_TRANS_RX_SETTING                    0x0015 			     /* phy_trans_rx_setting()    */
#define PON_SET_PHY_RESET_COUNTER                       0x0016 			     /* phy_reset_counter()       */
#define PON_SET_PHY_EPON_TS_CONTINUE_MODE               0x0017				/* phy_set_epon_ts_continue_mode() */	
#define PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT      0x0019               /* resotre_preamble_and_guard_bit */
#define PON_SET_PHY_NOTIFY_EVENT                        0x001a              
#define PON_SET_PHY_TRAFFIC_STATUS                      0x001b 			     /* phy_set_traffic_status()*/
#define PON_SET_PHY_TRANS_VCC                           0x001c 
#define PON_SET_PHY_XGPON_PROFILE                    0x001d
#define PON_SET_PHY_XGPON_RX_ENABLE					 0x001e
#define PON_SET_PHY_XGPON_RX_DISABLE				 0x001f
#define PON_SET_PHY_LOGIC_RESET                         0x0020               /* xPON_Logic_Reset()	*/
#define PON_SET_PHY_SCU_RST				 				0x0021				/* pon_phy_scu_reset_init()*/
#define PON_SET_PHY_RX_CDR                             0x0022      
#define PON_SET_PHY_EN7571_INIT_DONE                   0x0023
#define PON_SET_PHY_TX_MODE	                 			0x0024  //julia_7523
#define PON_SET_PHY_NGPON2_TX_CHAN_CONFIG				 0x0025
#define PON_SET_PHY_NGPON2_RX_CHAN_CONFIG				 0x0026
#define PON_SET_PHY_LDDLA_AUTO_MODE                      0x0027

/* Get Function */

#define PON_GET_PHY_LOS_STATUS                          0x8001               /* phy_los_status()         */
#define PON_GET_PHY_READY_STATUS                        0x8002               /* phy_ready_status()       */
#define PON_GET_PHY_TRANS_PARAM_TEMPRATRUE              0x8003               /* phy_trans_param_status() */
#define PON_GET_PHY_TRANS_PARAM_VOLTAGE                 0x8004               /* phy_trans_param_status() */
#define PON_GET_PHY_TRANS_PARAM_TX_CURRENT              0x8005               /* phy_trans_param_status() */
#define PON_GET_PHY_TRANS_PARAM_TX_POWER                0x8006               /* phy_trans_param_status() */
#define PON_GET_PHY_TRANS_PARAM_RX_POWER                0x8007               /* phy_trans_param_status() */
#define PON_GET_PHY_TX_FEC_STATUS                       0x8008               /* phy_tx_fec_status        */
#define PON_GET_PHY_TX_BURST_GETTING       			    0x8009               /* phy_tx_burst_getting     */

#define PON_GET_PHY_TRANS_TX                            0x800a               /* phy_trans_tx_getting     */
#define PON_GET_PHY_RX_FEC_COUNTER                      0x800d               /* phy_rx_fec_counter       */

#define PON_GET_PHY_BIP_COUNTER                         0x8012               /* phy_bip_counter          */
#define PON_GET_PHY_RX_FRAME_COUNTER                    0x8013               /* phy_rx_frame_counter     */

#define PON_GET_PHY_RX_FEC_STATUS                       0x8016               /* phy_rx_fec_status        */
#define PON_GET_PHY_RX_FEC_GETTING                      0x8017               /* phy_rx_fec_getting       */
#define PON_GET_PHY_TRANS_RX_GETTING                    0x8018               /* phy_trans_rx_getting     */


#define PON_GET_PHY_IS_SYNC                             0x8019 			     /* is_phy_sync              */
#define PON_GET_PHY_MODE                                0x801a 			     /* get_phy_mode             */
#define PON_GET_PHY_TX_LONG_FLAG                        0x801b               /* get_tx_log_flag          */

/* mt7570_param_status_real, phy_trans_param_status_real, phy_tx_alarm, phy_rx_power_alarm */
#define PON_GET_PHY_TRANS_STATUS                        0x801c
#define PON_GET_PHY_GET_TX_POWER_EN_FLAG                0x801d
#define PON_GET_PHY_ROUND_TRIP_DELAY                    0x801e 
#define PON_GET_PHY_EN7571_VER                    		0x801f 
#define PON_GET_PHY_LOF_STATUS                       0x8020
#define PON_GET_PHY_XGPON_PROFILE                    0x8021
#define PON_GET_PHY_TX_FRAME_COUNTER                 0x8022
#define PON_GET_PHY_INIT_STATUS		                 0x8023
#define PON_GET_PHY_REG		                 			0x8024  //julia_7523
#define PON_GET_PHY_TX_MODE	                 			0x8025  //julia_7523
#define PON_GET_PHY_PRBS_ERR_CNT	                 	0x8026  //julia_7523
#define PON_GET_PHY_NGPON2_CHAN                 		0x8027

#define PHY_LOS_HAPPEN                          (0x01)
#define PHY_NO_LOS_HAPPEN                       (0x00)

#define PHY_ENABLE								(1)
#define PHY_DISABLE 							(0)

#define DS_FEC_SETTING_AS_SPEC					(2) //for xgpon,ds fec always on,for xgspon/ngpon2 refer to ocbody
#define DS_FEC_SETTING_FORCE_OC 				(3)
#define DS_FEC_SETTING_FORCE_OFF				(0)
#define DS_FEC_SETTING_FORCE_ON 				(1)

#define BEN_HIGH							(1)
#define BEN_NORMAL 							(0)


#define	PHY_TRUE								(1)
#define	PHY_FALSE								(0)

/*Tx Rogue send PRBS */
#define PHY_TX_ROGUE_MODE						(1)
#define	PHY_TX_NORMAL_MODE						(0)

#define PHY_TX_BURST_MODE						(0)
#define	PHY_TX_CONT_MODE						(1)

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

enum {
    XPON_PHY_API_TYPE_GET = 0,
    XPON_PHY_API_TYPE_SET             = 1,
    XPON_PHY_API_TYPE_PHY_INTERNAL_GET = 2,
    XPON_PHY_API_TYPE_PHY_INTERNAL_SET = 3
};

typedef enum {
	PON_OPT_TX_MODE_PRBS7,
	PON_OPT_TX_MODE_PRBS15,
	PON_OPT_TX_MODE_PRBS23,
	PON_OPT_TX_MODE_PRBS31,
	PON_OPT_TX_MODE_AUTO,
	PON_OPT_TX_MODE_ENABLE,
	PON_OPT_TX_MODE_DISABLE,
	PON_OPT_TX_MODE_PRBS9
} PON_ONU_OPT_TX_MODE_E;


#define PHY_NO_API                              (-1)
#define PHY_SUCCESS                             (0)
#define PHY_FAILURE                             (1)

typedef enum XponPhy_NGPON2_TX_CHAN_e{
    PHY_NGPON2_TX_CHAN_0_CONFIG,	//1532.68nm
	PHY_NGPON2_TX_CHAN_1_CONFIG,	//1533.47nm
	PHY_NGPON2_TX_CHAN_2_CONFIG,	//1534.25nm
	PHY_NGPON2_TX_CHAN_3_CONFIG,	//1535.04nm

    PHY_NGPON2_TX_CHAN_UNKNOWN_CONFIG,
} Xpon_Phy_Ngpon2_Tx_Chan_t ;

typedef enum XponPhy_NGPON2_RX_CHAN_e{
    PHY_NGPON2_RX_CHAN_0_CONFIG,	//1596.34nm
	PHY_NGPON2_RX_CHAN_1_CONFIG,	//1597.19nm
	PHY_NGPON2_RX_CHAN_2_CONFIG,	//1598.04nm
	PHY_NGPON2_RX_CHAN_3_CONFIG,	//1598.09nm

    PHY_NGPON2_RX_CHAN_UNKNOWN_CONFIG,
} Xpon_Phy_Ngpon2_Rx_Chan_t ;

typedef enum XponPhy_Mode_e{
    PHY_EPON_CONFIG,
    PHY_GPON_CONFIG,
    PHY_XGPON_CONFIG,
    PHY_XGSPON_CONFIG,
    PHY_XEPON_10G_1G_CONFIG,
    PHY_XEPON_10G_10G_CONFIG,
    PHY_XEPON_1G_1G_CONFIG,
    PHY_NGPON2_10G_10G_CONFIG,
    PHY_NGPON2_10G_2G_CONFIG,
    PHY_NGPON2_2G_2G_CONFIG,
    PHY_GPON_SYM_CONFIG,
    PHY_UNKNOWN_CONFIG,
} Xpon_Phy_Mode_t ;

typedef enum PHY_Los_Status_e{
    PHY_LINK_STATUS_LOS,
    PHY_LINK_STATUS_READY,
    PHY_LINK_STATUS_UNKNOWN,
}PHY_Los_Status_t;

typedef enum {
	PHY_CALLBACK_XPON_STATE_NOTIFY =8,
} PHY_CbType_t ;


/*transceiver alarm type*/
typedef enum {
	PHY_TRANS_NO_ALARM				= 0x0,
	PHY_TRANS_TX_HIGH_POWER_ALARM 	= 0x01,
	PHY_TRANS_TX_LOW_POWER_ALARM 	= 0x02,
	PHY_TRANS_TX_HIGH_CUR_ALARM 	= 0x04,
	PHY_TRANS_TX_LOW_CUR_ALARM		= 0x08,
	PHY_TRANS_RX_HIGH_POWER_ALARM 	= 0x10,
	PHY_TRANS_RX_LOW_POWER_ALARM 	= 0x20
} ENUM_PhyTransAlarm ;


/*phy counter clear*/
typedef enum {
    PHY_ERR_CNT_CLR      = 0x01,
    PHY_BIP_CNT_CLR      = 0x02,
    PHY_RXFRAME_CNT_CLR  = 0x04,
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928) || defined(TCSUPPORT_CPU_EN7580)
    PHY_TXFRAME_CNT_CLR  = 0x08,
    PHY_EPON_ERR_CNT_CLR = 0x10 
#endif /* TCSUPPORT_WAN_EPON */
} ENUM_PhyCounterClr;

/*GPON Preamble*/
typedef enum {		
    PHY_GUARD_BIT_NUM_EN = 0x01, /* set by HY_GponPreb_T->mask to modify guard_bit_num      */
    PHY_PRE_T1_NUM_EN    = 0x02, /* set by HY_GponPreb_T->mask to modify preamble_t1_num    */
    PHY_PRE_T2_NUM_EN    = 0x04, /* set by HY_GponPreb_T->mask to modify preamble_t2_num    */
    PHY_PRE_T3_PAT_EN    = 0x08, /* set by HY_GponPreb_T->mask to modify preamble_t3_pat    */
    PHY_T3_O4_PRE_EN     = 0x10, /* set by HY_GponPreb_T->mask to modify t3_O4_preamble     */
    PHY_T3_O5_PRE_EN     = 0x20, /* set by HY_GponPreb_T->mask to modify t3_O5_preamble     */
    PHY_EXT_BUR_MODE_EN  = 0x40, /* set by HY_GponPreb_T->mask to modify extend_burst_mode  */
    PHY_OPER_RANG_EN     = 0x80, /* set by HY_GponPreb_T->mask to modify oper_ranged_st     */
    PHY_DIS_SCRAM_EN     = 0x200,/* set by HY_GponPreb_T->mask to modify dis_scramble       */
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928) || defined(TCSUPPORT_CPU_EN7580)
    PHY_EXTB_LENG_SEL_EN = 0x100//set by HY_GponPreb_T->mask to modify extb_length_sel  
#endif
} ENUM_GponPreb_T ;

/* traffic status */
typedef enum {
    TRAFFIC_STATUS_DOWN     = 0x0,
    TRAFFIC_STATUS_UP       = 0x1,
} ENUM_PhyTrafficStatus;

enum ECNT_XPON_PHY_SUBTYPE {
    ECNT_XPON_PHY_API,
};

typedef struct
{
    ushort 	 temprature;
    ushort 	 supply_voltage;
    ushort 	 tx_current;
    ushort 	 tx_power;
    ushort   rx_power;
}PHY_TransParam_T, *PPHY_TransParam_T;

typedef struct{
    PHY_TransParam_T params;
    uint             alarms;
} PHY_Trans_Status_t;

typedef struct{
    Xpon_Phy_Mode_t  mode    ;
    int              txEnable;
} PHY_Mode_Cfg_t;

typedef struct{
    Xpon_Phy_Ngpon2_Tx_Chan_t  ngpon2_tx_chan;
    Xpon_Phy_Ngpon2_Rx_Chan_t  ngpon2_rx_chan;
} PHY_Ngpon2_Chan_Sel_T, *PPHY_Ngpon2_Chan_Sel_T;


/* FEC error counter*/
typedef struct
{
	uint   correct_bytes;      /* the number of corrected bytes                                       */
	uint   correct_codewords;  /* the number of corrected codewords                                   */
	uint   uncorrect_codewords;/* the number of uncorrected codewords                                 */
	uint   total_rx_codewords; /* the number of total received codewords                              */
	uint   fec_seconds;        /* the count of second in previous 15min when there was an FEC anomaly */
}PHY_FecCount_T, *PPHY_FecCount_T;
/*received frame counter*/
typedef struct
{
	uint 			frame_count_low;    /* the number of received frame */
	uint 			frame_count_high;
	uint 			lof_counter;        /* the number of LOS            */
}PHY_FrameCount_T, *PPHY_FrameCount_T;

typedef struct{
    uint   delimiter  ;
    unchar guard_time ;
} PHY_GPON_Delimiter_Guard_t;

typedef struct
{
	unchar	trans_tx_sd_inv_status;
	unchar	trans_burst_en_inv_status;
	unchar	trans_tx_fault_inv_status;
}PHY_TransConfig_T, *PPHY_TransConfig_T;

typedef struct
{
    unchar  guard_bit_num;     /**< [8 bits ]number of "guard time" bits */
    unchar  preamble_t1_num;   /**< [8 bits ]number of "type 1 preamble" bits */
    unchar  preamble_t2_num;   /**< [8 bits ]number of "type 2 preamble" bits */
    unchar  preamble_t3_pat;   /**< [8 bits ]pattern of "type 3 preamble"  */
    unchar  t3_O4_preamble;    /**< [8 bits ]number of "type3 preamble bytes"(used in O3 ,O4) */
    unchar  t3_O5_preamble;    /**< [8 bits ]number of "type3 preamble bytes"(used in O5) */
    unchar  extend_burst_mode; /**< [1 bit ]switch to extend burst mode */
    unchar  oper_ranged_st;    /**< [2 bits]ranged state: 2'b10: O3,O4 state. 2'b11: O5 sate */
    unchar  dis_scramble;      /**< [1 bit ]set to disable scrambler */
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928) || defined(TCSUPPORT_CPU_EN7580)
    /** [1 bit ] set to release the limit of extend burst mode overhead, used 
     *  for ZTE C300
     */
    unchar  extb_length_sel;
#endif /* CONFIG_USE_MT7520_ASIC */
    uint    mask;              /**<  use ENUM_GponPreb_T to define it */
}PHY_GponPreb_T, *PPHY_GponPreb_T;

typedef struct
{
    int  rogue_onoff;           /* rogue mode on off : PHY_TX_ROGUE_MODE or PHY_TX_NORMAL_MODE */
    int  rogue_pattern;         /* rogue data pattern : 0~3 */
    int  tx_d_in_timeslot;      /* tx_d in grant timeslot is normal data or rogure pattern : 0 or 1 */
}PHY_Rogue_T, *PPHY_Rogue_T;

//According to G.987.3 Chapter 11.3.3.1 Profile message
typedef struct
{
    unchar  profile_version;       /**< [4 bits ] */	//not set by PHY
    unchar  profile_index;         /**< [2 bits ] */
    unchar  fec_indication;        /**< [2 bits ] */
    unchar  delimiter_length;      /**< [4 bits ] */
    unchar  delimiter[8];          /**< [8 bytes ] */
    unchar  preamble_length;       /**< [4 bits ] */
    unchar  preamble_repeat_count; /**< [5 bits ] */
    unchar  preamble[8];           /**< [8 bytes ] */
    unchar  pon_tag[8];            /**< [8 bytes ] */	//not set by PHY
}PHY_Xgpon_Profile_Msg_T, *PPHY_Xgpon_Profile_Msg_T;


typedef struct
{
    PHY_Xgpon_Profile_Msg_T num[4];
}PHY_Xgpon_Profile_T, *PPHY_Xgpon_Profile_T;

typedef struct xpon_phy_api_data_s {
    int             api_type  ;  /* [in ]    get or set API      */
    unsigned int    cmd_id    ;  /* [in ]    API command ID      */
    int             ret       ;  /* [out]    API return value    */

    union{                       /* [in|out]                     */
        int                 * data            ;
        PHY_Mode_Cfg_t      * phy_mode_cfg    ;
        PHY_Trans_Status_t  * trasn_status    ;
        PHY_Trans_Status_t          * trans_status    ;
        PHY_FecCount_T      * rx_fec_cnt      ;
        PHY_FrameCount_T    * rx_frame_cnt    ;
        PHY_GPON_Delimiter_Guard_t * delimiter_guard ;
        PHY_TransConfig_T   * tx_trans_cfg    ;
        PHY_GponPreb_T      * gpon_preamble   ;
        PHY_GponPreb_T      * gpon_ex_preamble;
        ENUM_PhyTrafficStatus      * traffic_status  ;
        PHY_Rogue_T                 * phy_rogue_cfg   ;
        PHY_Xgpon_Profile_Msg_T     * xgpon_profile   ;
		PHY_Ngpon2_Chan_Sel_T		*ngpon2_chan_sel;
        void                * raw             ;
    };
}xpon_phy_api_data_t,*p_xpon_phy_api_data_t;


/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/

#endif // __LINUX_ENCT_HOOK_PON_PHY_H


