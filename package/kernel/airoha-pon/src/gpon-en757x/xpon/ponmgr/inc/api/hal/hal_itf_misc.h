/**
*@file
*@brief This file defines misc related interfaces that should be implemented by chip vendors
*@author Shen Jingguo
*@date     2013.3.10
*@copyright
*******************************************************************
* Copyright (c) 2013 Alcatel Lucent Shanghai Bell Software, Inc.
*             All Rights Reserved
*
* This source is confidential and proprietary and may not
* be used without the written permission of Alcatel Lucent
* Shanghai Bell, Inc.
********************************************************************
*@par Module History:
*/

#ifndef HAL_ITF_MISC_H_
#define HAL_ITF_MISC_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include "hal_type_misc.h"




#ifdef MIRROR_ENABLED
/**
* To set the mirror function
* @param wanIndex which wan port need to do mirror
* @param dstPort the destination port that mirrored to
* @param mirrorType the mirror type, for example, not mirror, mirror rx, mirror tx, mirror all,..
* @return return 0 if success
*/
int hal_misc_set_mirror(int wanIndex, int dstPort, int mirrorType);
#endif

/**
* To set the NAT disabled function
* @param wanIndex which wan port need to do mirror
* @param disabled 
* @return return 0 if success
*/
hal_ret_t hal_misc_set_nat_disabled(int wanIndex, unsigned int disabled);

/**
* To get the gpon capacity info, for example the number of tcont, gemport,queue,...
* @param gponInfo the gpon capacity need to get
* @see hal_gpon_conf_info_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t  hal_misc_get_gpon_capacity_info(hal_gpon_conf_info_t* gponInfo);

/**
* To init the cfm tx/rx function
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_cfm_init();

/**
* To send cfm messages
* @param writeCfm the message need to be sent out
* @see hal_write_cfm_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_write_cfm(hal_write_cfm_t* writeCfm);

/**
* To register the call back. The call back function need to be called when received the cfm message
* @param pf_cfmcallback the callback function point of the cfm message
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_cfm_recv_msg_callback(void *pf_cfmcallback);

/**
* To send ploam messages
* @param writePloam the type of message need to be sent out
* @see hal_write_ploam_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_write_ploam(hal_write_ploam_t* writeploam);

/**
* To get wan bridge port for special hw
* @return wan bridge port number
*/
unsigned int hal_get_wan_bridge_port();

/**
* To get lan bridge port for special hw
* @param uni the number of lan port in omci
* @return lan bridge port number
*/
unsigned int hal_get_lan_bridge_port(const hal_port_info_t hal_port_info);

/**
* To set port mirror
* @param enable enable/disable mirror
* @param user_sport the src_port
* @param user_dport the dst_port
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_port_mirror_set(int enable, int user_sport, int user_dport);

/**
* To set port mac limit
* @param user_port
* @param mac_num
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_port_mac_limit_set(int user_port, int mac_num);

/*
 * To get lag port for adding switch chip
 * @return lag port number
 */
unsigned int hal_get_lag_port();



hal_ret_t hal_bcm_hgu_init(int init);

/**
* To get the user_port by mac
* @param mac  the mac searched
* @param user_port the user port
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/

hal_ret_t hal_mac_port_get(char *mac, int *user_port);

/**
* To disable all lan packets
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_disable_all_lan_packets(void);

/**
* To enable all lan packets
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_misc_enable_all_lan_packets(void);


hal_ret_t hal_misc_config_port_dscp2pbit(UserPortId port_id, hal_port_class_t port_class);


hal_ret_t hal_misc_config_us_drop_threshold(unsigned int drop_threshold);

hal_ret_t hal_misc_restore_us_drop_threshold(void);



/**
* To config voip
* @return returm HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_config_voipGemport(hal_priorityPortMapping_t *map,unsigned char action);

/**
* To delete the configuration of voip
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_delete_voipGemport(unsigned char action);

//general voip voip pon side flow rule API defined here
void hal_create_voip_pon_tx_rule(hal_priorityPortMapping_t gem_map,unsigned short vlan,unsigned char dscp);
void hal_create_voip_pon_rx_rule(hal_priorityPortMapping_t gem_map,unsigned short vlan,unsigned char dscp);
void hal_delete_voip_tagRule_info(hal_voip_rule_direction_t tableDir,unsigned int nbrOfTags,unsigned int tagRuleId,char* ifname);
void hal_store_voip_pon_tagRule_info(hal_voip_rule_direction_t tableDir,unsigned int nbrOfTags,unsigned int tagRuleId,char* ifname);
void hal_clear_voip_pon_tagRule_info(void);
hal_ret_t hal_misc_dump_hw_port(UserPortId port, hal_port_class_t port_class, char hw_port_name[32]);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
