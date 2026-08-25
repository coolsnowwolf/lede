
/**
*@file
*@brief This file defines GPON MAC related interfaces that should be implemented by chip vendors 
*@author Zhang Yan H
*@date     2012.10.11
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


#ifndef HAL_ITF_GPON_H_
#define HAL_ITF_GPON_H_

#include "hal_type_gpon.h"

#include "hal_type_ethernet.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif 


/**
* To initialize the hw 
* @param  hw_info the hardware structure about support LAG, outside switch
* @see hal_onu_hw_mode
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @post after this function is compeleted, the client should be able to start operation 
* on the hardware such as to configure  the pon link etc.
*/
hal_ret_t hal_gpon_init(hal_onu_hw_mode* hw_info);

/* TC related 
***********************************************************/


/**
* To activate the pon link 
* @param  ponLink 
* @see  hal_pon_link_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_activate_gponlink(hal_pon_link_t* ponLink); 

/**
* To deactivate the pon link 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_deactivate_gponlink(void);
/**
* To modify the pon link password (SLID) 
* @param  password
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @note password can be modified after ponlink is activated
*/
hal_ret_t hal_gpon_modify_gponlink_pwd(unsigned char* password);

/**
* To modify the pon link sequence number 
* @param  sn    
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @note sn can be modified after ponlink is activated
*/
hal_ret_t hal_gpon_modify_gponlink_sn(unsigned char* sn);

/**
* To get current pon link information 
* @param  ponlinkinfo
* @see hal_pon_link_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_get_gponlink_info(hal_pon_link_t* ponlinkinfo);

/**
* To set rogue state
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_set_rogue_state(void);

/**
* To get rx and tx FEC status 
* @param  fecStatus the fect status get from chipset
* @see hal_pon_fec_status_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_get_gponlink_fec_status(hal_pon_fec_status_t* fecStatus);


/**
* To create a TCont  
* @param  TCont the id of the tcont need to be configured
* @param  AllocateID the allocate id of the tcont
* @param tcontPolicy the policy(SP or WRR) of the tcont
* @see hal_qos_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_create_tcont(int TCont,int AllocateID, hal_qos_policy_t tcontPolicy); 

/**
* To create a hgu TCont  
* @param  TCont the id of the tcont need to be configured
* @param  AllocateID the allocate id of the tcont
* @param tcontPolicy the policy(SP or WRR) of the tcont
* @see hal_qos_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @deprecated
*/
//hal_ret_t hal_gpon_create_tcont_hgu(int TCont,int AllocateID, hal_qos_policy_t tcontPolicy); 

/**
* To modify a TCont  information
* @param  TCont the id of the  tcont need to be modified
* @param  AllocateID the allocate id of the tcont
* @param tcontPolicy the policy(SP or WRR) of the tcont
* @see hal_qos_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_modify_tcont(int TCont,int AllocateID,hal_qos_policy_t tcontPolicy); 
/**
* To delete a TCont
* @param  TCont the id of the  tcont need to be modified
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_delete_tcont(int TCont); 

/**
* To create a gem port 
* @param  GemPortID the gemport number need to be configured
* @param  gemportType the type of the gemport(unicast, broadcast,multicast)
* @param TContId the tcont that the gemport point to
* @see hal_gemport_type_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_create_gemport(unsigned short GemPortID,hal_gemport_type_t gemportType,unsigned short TContId);

/**
* To delete a gem port and related configuation
* @param  GemPortID the gemport need to be deleted
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_delete_gemport(unsigned short GemPortID);

/**
* To create a gem port  that used the remote debug function
* @param  gemPortId the gemport number need to be configured
* @param  sn the serial number of ont, it may need not be used now
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo rename
*/
hal_ret_t hal_gpon_config_debug_gemPort(unsigned short gemPortId, unsigned int sn); 



/**
* To get omci channel gemport id
* @param  pPortId the port id of gemport omci channel
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_ploam_get_OMCIPortID(unsigned short *pPortId);

/**
* To send OMCI messages
* @param writeOmci the omci message need to be sent out
* @see hal_write_omci_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_write_omci(hal_write_omci_t* writeOmci);


/**
* To configure a ether type filter to do ratelimit
* @param etyp_filter 
* @see hal_cfg_eth_type_filter_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo move to misc or flow
*/
hal_ret_t hal_gpon_cfg_ethype_filter(hal_cfg_eth_type_filter_t *etyp_filter);

/**
* To configure a dhcp filter
* @param dhcp_filter 
* @see hal_cfg_dhcp_filter_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo merge with ethtype filter, use ethtype or protocol or some reasons
*/
hal_ret_t hal_gpon_cfg_dhcp_filter(hal_cfg_dhcp_filter_t *dhcp_filter);

/**
* To get the ethernet flow id by gemflow id
* @param gemflow  the gemflow id in hal
* @param ethflowId  the ethernet flow id in hal
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo move to internal
*/
hal_ret_t hal_gpon_get_ds_eth_flow(unsigned int gemflow, unsigned int* ethflowId);

/**
* To init the process of receive omci message 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_receive_omci_mesg_init();

/**
* To register the call back function which is called when omci message is received
* @param pf_omcicallback  the point of call back function
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_omci_mesg_callback(void *pf_omcicallback);

/**
* To init the process of receive ploam message 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_receive_ploam_mesg_init();

/**
* To register the call back function which is called when ploam state is changed
* @param pf_snstatecallback  the point of call back function
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo rename
*/
hal_ret_t hal_gpon_ploam_snState_update_callback(void *pf_snstatecallback);

/**
* To set the trace level of gpon mac module in hal
* @param trc_level  the trace level
* @todo merge the trc level
*/
void hal_gpon_set_trc_level(unsigned int trc_level);

/**
* To check the gemport is configured in hal or not
* @param gemportId  the gemport Id nee to check
* @return return HAL_RET_FAILED if not configured , return HAL_RET_SUCCESS if configured
*/
hal_bool_t hal_gpon_is_gemport_configured(unsigned short gemportId);

/**
* To get the gemflow Id in hal by the gemport Id, used by special function. 
* @param gemportId  the gemport Id 
* @return return the gemflow Id.
*/

unsigned short hal_gpon_get_gemport_flow_id(unsigned short gemportId);
/**
 * * To get the upstream gemflow Id in hal by the gemport Id, used by special function. 
 * * @param gemportId  the gemport Id 
 * * @return return the gemflow Id.
 * */

unsigned short  hal_gpon_get_gemport_us_flow_id(unsigned short gemportId);
/**
* To print the gemport table information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_gemflow_table_info(unsigned short appFlowId);

/**
* To print the downstream gemflow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_ds_gem_info(unsigned short appFlowId);

/**
* To print the upstream gemflow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_us_gem_info(unsigned short appFlowId);

/**
* To print the downstream ani flow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_ds_ani_flow_info(unsigned short appFlowId);

/**
* To print the upstream ani flow information. 
* @param appFlowId  the application flow id
* @return none
*/
void  hal_gpon_show_us_ani_flow_info(unsigned short appFlowId);

/**
* when operation mode change from uni to nni or vers
* @param mode 
* @see hal_op_mode_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_gpon_config_operation_mode(unsigned int mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


