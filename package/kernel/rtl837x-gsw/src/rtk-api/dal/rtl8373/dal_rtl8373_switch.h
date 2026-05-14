
/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of Switch Global API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Switch parameter settings
 *
 */

#ifndef __DAL_RTL8373_SWITCH_H__
#define __DAL_RTL8373_SWITCH_H__

/*
 * Include Files
 */
#include <rtk_types.h>
#include <rtk_switch.h>


typedef enum chip_mode_type_e
{
    CHIP_RTL8373_MODE,
    CHIP_RTL8372_MODE,
    CHIP_RTL8224_MODE,
    CHIP_RTL8221B_MODE,
	CHIP_RTL8373N_MODE,
    CHIP_RTL8372N_MODE,
    CHIP_RTL8224N_MODE,
	CHIP_RTL8366U_MODE,
    CHIP_MODE_END
}chip_mode_type_t;
/*
 * Symbol Definition
 */



typedef enum rtl8373_sds_id_e 
{
    SERDES_ID0=0,
    SERDES_ID1=1,
    SERDES_IDEND
}rtl8373_sds_id_t;
void delay_loop(rtk_uint32 loop);


void Pin_Reset_8224_via_8373(void);


void uc1_sram_write_8b(rtk_uint32 port,rtk_uint32 addr,rtk_uint32 val);

rtk_uint32 uc1_sram_read_8b(rtk_uint32 port,rtk_uint32 addr);

void uc2_sram_write_8b(rtk_uint32 port,rtk_uint32 addr,rtk_uint32 val);

rtk_uint32 uc2_sram_read_8b(rtk_uint32 port,rtk_uint32 addr);

void data_ram_write_8b (rtk_uint32 port, rtk_uint32 addr,rtk_uint32 val);

rtk_uint32 uc_sram_read_16b(rtk_uint32 port, rtk_uint32 addr);

void uc_sram_write_16b(rtk_uint32 port,rtk_uint32 addr,rtk_uint32 val);

void n0_patch_RL6818B_230724 (rtk_uint32 phymask);

void n2_patch_6818B_230724 (rtk_uint32 phymask);


void uc2_patch_6818B_230815 (rtk_uint32 phymask);

void uc_patch_6818B_20221211(rtk_uint32 phymask);

void rtct_para_6818B_221211(rtk_uint32 phymask);

void n2_patch_6818C_221026 (rtk_uint32 phymask);

void uc_patch_6818C_221117 (rtk_uint32 phymask);

void data_ram_patch_6818C_221026(rtk_uint32 phymask);

void RTCT_para_6818C_231206(rtk_uint32 phymask);

void data_ram_patch_6818B_220714(rtk_uint32 phymask);

void alg_tune_2p5G_6818B_220701 (rtk_uint32 phymask);


void alg_tune_giga_6818B_220617(rtk_uint32 phymask);


void afe_patch_6818B_220607(rtk_uint32 phymask);

void rtct_para_6818B_220713(rtk_uint16 phymask);


rtk_api_ret_t SDS_MODE_SET_SW(rtk_uint32 CHIP_MODE, rtk_uint32 SDS_INDX ,rtk_uint32 SDS_MODE);


void get_version_8373(void);

void get_version_8224(void);

void get_version_8366u(void);


void afe_patch_6818_220325(void);

void afe_patch_6818C_220607(rtk_uint16 phymask);

void RL6818C_pwr_on_patch_phy_v008_rls_lockmain(rtk_uint32 phymask);

void RL6818C_pwr_on_patch_phy_v008(rtk_uint32 phymask);




void alg_tune_fnet_6818_220628(rtk_uint32 phymask);

void n0_patch_RL6818B_221012 (rtk_uint32 phymask);

void uc_patch_6818B_20220927(rtk_uint32 phymask);

void RL6818B_pwr_on_patch_phy_v009_rls_lockmain(rtk_uint32 phymask);

void RL6818B_pwr_on_patch_phy_v009(rtk_uint32 phymask);

void n0_patch_RL6818C_230703 (rtk_uint32 phymask);

void n2_patch_6818C_231206 (rtk_uint32 phymask);

void uc_patch_6818C_221012 (rtk_uint32 phymask);
 void uc2_patch_6818C_231206(rtk_uint32 phymask);

void RL6818B_pwr_on_patch_phy_v004a_rls_lockmain(rtk_uint32 phymask);


void RL6818B_pwr_on_patch_phy_v004a(rtk_uint32 phymask);

void cfg_rl6637_sds_mode(rtk_uint32 phyId,rtk_uint32 sds_mode);


rtk_api_ret_t fw_reset_flow_tgx(rtk_uint32 sds);

rtk_api_ret_t rtl8373_8224_init(void);
rtk_api_ret_t rtl8372_init(void);
rtk_api_ret_t rtl8372n_init(void);
rtk_api_ret_t rtl8366u_init(void);
rtk_api_ret_t rtl8373N_8224N_init(void);
void Pin_Reset_8224N_via_8373N(void);


/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */

/* Function Name:
 *      dal_rtl8373_switch_init
 * Description:
 *      Initialize switch module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Module must be initialized before using all of APIs in this module
 */
extern rtk_api_ret_t
dal_rtl8373_switch_init(void);

/* Module Name    : Switch     */
/* Sub-module Name: Switch parameter settings */
/* Function Name:
 *      dal_rtl8373_switch_portMaxPktLen_set
 * Description:
 *      Set Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 *      cfgId   - Configuration ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t
dal_rtl8373_switch_portMaxPktLen_set(rtk_port_t port, rtk_switch_maxPktLen_linkSpeed_t speed, rtk_uint32 cfgId);

/* Function Name:
 *      dal_rtl8373_switch_portMaxPktLen_get
 * Description:
 *      Get Max packet length
 * Input:
 *      port    - Port ID
 *      speed   - Speed
 * Output:
 *      pCfgId  - Configuration ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t
dal_rtl8373_switch_portMaxPktLen_get(rtk_port_t port, rtk_switch_maxPktLen_linkSpeed_t speed, rtk_uint32 *pCfgId);

/* Function Name:
 *      dal_rtl8373_switch_maxPktLenCfg_set
 * Description:
 *      Set Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pktLen  - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t
dal_rtl8373_switch_maxPktLenCfg_set(rtk_uint32 cfgId, rtk_uint32 pktLen);

/* Function Name:
 *      dal_rtl8373_switch_maxPktLenCfg_get
 * Description:
 *      Get Max packet length configuration
 * Input:
 *      cfgId   - Configuration ID
 *      pPktLen - Max packet length
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 */
extern rtk_api_ret_t
dal_rtl8373_switch_maxPktLenCfg_get(rtk_uint32 cfgId, rtk_uint32 *pPktLen);

/* Function Name:
 *      dal_rtl8373_switch_greenEthernet_set
 * Description:
 *      Set all Ports Green Ethernet state.
 * Input:
 *      enable - Green Ethernet state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE   - Invalid enable input.
 * Note:
 *      This API can set all Ports Green Ethernet state.
 *      The configuration is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t
dal_rtl8373_switch_greenEthernet_set(rtk_enable_t enable);

/* Function Name:
 *      dal_rtl8373_switch_greenEthernet_get
 * Description:
 *      Get all Ports Green Ethernet state.
 * Input:
 *      None
 * Output:
 *      pEnable - Green Ethernet state.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API can get Green Ethernet state.
 */
extern rtk_api_ret_t
dal_rtl8373_switch_greenEthernet_get(rtk_enable_t *pEnable);


extern void  fiber_fc_en (rtk_uint32 SDS_INDX ,rtk_uint32 SDS_MODE, rtk_uint32 fc_en);

extern void  sds_nway_set (rtk_uint32 SDS_INDX ,rtk_uint32 SDS_MODE, rtk_uint32 an_en);

extern rtk_api_ret_t serdes_patch (rtk_uint32 CHIP_MODE, rtk_uint32 SDS_INDX ,rtk_uint32 SDS_MODE);

extern rtk_api_ret_t fw_reset_flow_tgr_tgx(rtk_uint32 sds);

extern rtk_api_ret_t fw_reset_flow_tgr(rtk_uint32 sdsid);

extern rtk_api_ret_t fw_reset_flow_tgx(rtk_uint32 sds);

extern rtk_api_ret_t SDS_MODE_SET_SW(rtk_uint32 CHIP_MODE, rtk_uint32 SDS_INDX ,rtk_uint32 SDS_MODE);

extern rtk_api_ret_t fw_reset_flow_tgr_8224(rtk_uint32 sdsid);
  /* Function Name:
  * 	 dal_rtl8373_fw_reset_flow_8221B
  * Description:
  * 	 8221B reset flow
  * Input:
  * 	 sdsid: 8221B-sdsid
  * Output:
  * 	 None
  * Return:
  * 	 RT_ERR_OK			 - OK
  * 	 RT_ERR_FAILED		 - Failed
  * 	 RT_ERR_SMI 		 - SMI access error
  * Note:
  * 	 The API can reset 8221B, Dynamic change sds to match 8221B speed.
  */
rtk_api_ret_t dal_rtl8373_fw_reset_flow_8221B(rtk_uint32 sdsid);

#endif /* __DAL_RTL8373_SWITCH_H__ */
