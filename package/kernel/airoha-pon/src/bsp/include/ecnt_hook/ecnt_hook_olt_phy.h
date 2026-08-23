/***************************************************************
SPDX-Lincense-Identifier: GPL-2.0-only
Copyright (c) 2021-2023 Airoha Inc.
***************************************************************/

#ifndef __LINUX_ENCT_HOOK_OLT_PHY_H
#define __LINUX_ENCT_HOOK_OLT_PHY_H

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

#define OLT_SET_PHY_MODE                                0x1001
#define OLT_SET_PHY_START                               0x1002
#define OLT_SET_PHY_STOP                                0x1003
#define OLT_SET_PHY_SCU_RESET                           0x1004
#define OLT_SET_PHY_LOGIC_RESET                         0x1005
#define OLT_SET_PHY_ROGUE_ONU_DET                       0x1006
#define OLT_SET_PHY_ROGUE_ONU_DET_MANUAL                0x1007
#define OLT_SET_PHY_BIP_CNT_CLEAR						0x1008
#define OLT_SET_PHY_SYM_SN_ONU_ID						0x1009
#define OLT_SET_PHY_TX_RX_SYNC_TIME						0x100a

/* Get Function */

#define OLT_GET_PHY_MODE                                0x2001
#define OLT_GET_PHY_STATUS                              0x2002
#define OLT_GET_PHY_BIP_CNT								0x2003
#define OLT_GET_PHY_TX_RX_SYNC_TIME						0x2004
#define OLT_GET_PHY_RANGING_RESULT						0x2005
#define OLT_GET_PHY_ALARM_THD							0x2006
#define OLT_GET_PHY_LOS_LOF_STATUS						0x2007
#define OLT_GET_PHY_DRIFT_ALARM							0x2008
#define OLT_GET_PHY_DRIFT_VALUE							0x2009
#define OLT_GET_PHY_ROGUE_ONU_DET_STATUS				0x200a


#define	OLT_PHY_TRUE								(1)
#define	OLT_PHY_FALSE								(0)


#define OLT_PHY_NO_API                              (-1)
#define OLT_PHY_SUCCESS                             (0)
#define OLT_PHY_FAILURE                             (1)

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/


typedef struct OLT_PHY_Losi_lofi_Status_s{
	unsigned short phy_switch_ready; // for typeB
	unsigned short los_all;
	unsigned short pre_los_all; // for typeB
	unsigned short temp0;
	unsigned long long losi;
	unsigned long long losi_change;
	unsigned long long lofi;
	unsigned long long lofi_change;	
	unsigned long long bw_drop_alarm;
}OLT_PHY_Losi_lofi_Status_t;

typedef struct OLT_PHY_Drift_Status_s{
	unsigned long long dow_alarm;
	unsigned long long tiw_alarm;
}OLT_PHY_Drift_Status_t;


typedef struct OLT_PHY_Drift_Value_s{
	unsigned char drift;
}OLT_PHY_Drift_Value_t;


typedef enum OltPhy_Mode_e{
    OLT_PHY_GPON_2G_1G_CONFIG,
    OLT_PHY_GPON_2G_2G_CONFIG,
    OLT_PHY_UNKNOWN_CONFIG,
} Olt_Phy_Mode_t ;

typedef struct{
    uint bip_sts_of_onuid[64];
	uint bip_total_of_onuid[64];
	uint bip_err_of_onuid[64];
} OLT_PHY_RX_BIP;

typedef struct{
	uint o4_resp_onu_id;
	uint o4_resp_125us_cnt;
	uint o4_resp_bit_dly;
} OLT_PCS_RANGING_RESULT;

enum ECNT_OLT_PHY_SUBTYPE {
    ECNT_OLT_PHY_API,
};


enum {
    OLT_PHY_API_TYPE_GET              = 0,
    OLT_PHY_API_TYPE_SET              = 1,
    OLT_PHY_API_TYPE_PHY_INTERNAL_GET = 2,
    OLT_PHY_API_TYPE_PHY_INTERNAL_SET = 3
};

typedef struct olt_phy_api_data_s {
    int             api_type  ;  /* [in ]    get or set API      */
    unsigned int    cmd_id    ;  /* [in ]    API command ID      */
	unsigned int	sub_id	  ;  /* [in ]    onu id for same case */  
    int             ret       ;  /* [out]    API return value    */

    union{                       /* [in|out]                     */
        int                 * data            ;
        Olt_Phy_Mode_t      * olt_phy_mode    ;
		OLT_PCS_RANGING_RESULT	* olt_phy_ranging_result	;
		OLT_PHY_RX_BIP		* olt_phy_bip	  ;
		OLT_PHY_Losi_lofi_Status_t * olt_phy_los_lof;
		OLT_PHY_Drift_Status_t *olt_phy_drift_sts ;
		OLT_PHY_Drift_Value_t *olt_phy_drift_value ;		
        void                * raw             ;
    };
}olt_phy_api_data_t,*p_olt_phy_api_data_t;

#endif // __LINUX_ENCT_HOOK_PON_PHY_H


