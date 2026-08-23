/**
*@file
*@brief This file defines gimp  related interfaces that should be implemented by chip vendors 
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

#ifndef HAL_ITF_IGMP_H_
#define HAL_ITF_IGMP_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hal_type_igmp.h"

/**
* To init the igmp service
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_dp_init(void);

/**
* To dump the igmp configurations
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_dump(unsigned int dump_flag);

/**
* To dump the igmp snooping data
* @param action the data type of dbg dump
* @param port   port number, 0 means all
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_dump_data(unsigned field, unsigned int action, unsigned int uni_port); //R31_IGMP

/**
* To set the downstream multicast vlan
* @param  mc_vlan the downstream mc vlan want to set
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_ds_mcast_vlan(unsigned int mc_vlan);

/**
* To remove the downstream multicast vlan
* @param  mc_vlan the downstream mc vlan want to remove
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_remove_ds_mcast_vlan(unsigned int mc_vlan);

/**
* To get the downstream multicast vlan
* @param  mc_vlan the mc vlan get from hal
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_get_ds_mcast_vlan(unsigned int *mc_vlan);

/**
* To set the downstream vlan action of multicast stream
* @param  uni_port the vlan action is configured for which uni port
* @param  vlan_mode the vlan action mode
* @param  tag_num the tag number need add in downstream
* @param  outer_vlan the outer vlan need to be added 
* @param  inner_vlan the inner vlan need to be added
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_ds_user_vlan_action(hal_port_info_t hal_port_info, unsigned int vlan_mode,
 	unsigned int tag_num, unsigned int outer_vlan, unsigned int inner_vlan);

/**
* To set the downstream vlan priority of multicast stream
* @param  uni_port the vlan action is configured for which uni port
* @param  pri_num the number of priority is supported 
* @param  outer_pri the outer tag priority 
* @param  inner_pri  the inner tag priority 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_ds_vlan_priority(hal_port_info_t hal_port_info, unsigned int pri_num,
	 unsigned short outer_pri, unsigned short inner_pri);

/**
* To set the upstream vlan information of IGMP signals
* @param  uni_port the vlan action is configured for which uni port
* @param  tag_mode the vlan action mode
* @param  upstream_tci the tag information, include vlan id and priority
* @see  hal_tag_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_us_vlan_action(hal_port_info_t hal_port_info, unsigned int tag_mode,hal_tag_t upstream_tci);

/**
* To get the number of IGMP discard packets
* @param  discard_pkt the number of discard packetser
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_get_discard_igmp_pkt(unsigned int *discard_pkt);

/**
* To clear the number of IGMP discard packets
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_clear_discard_igmp_pkt(void);


/**
 * * To get the number of ARP discard packets
 * * @param  discard_pkt the number of discard packetser
 * * @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
 * */
hal_ret_t hal_igmp_get_discard_arp_pkt_count(unsigned int *discard_pkt);

/**
 * * To clear the number of ARP discard packets
 * * @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
 * */
hal_ret_t hal_igmp_clear_discard_arp_pkt_count(void);


/**
 *  * * To get the number of DHCP discard packets
 *   * * @param  discard_pkt the number of discard packetser
 *    * * @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
 *     * */
hal_ret_t hal_igmp_get_discard_dhcp_pkt_count(unsigned int *discard_pkt);

/**
 *  * * To clear the number of DHCP discard packets
 *   * * @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
 *    * */
hal_ret_t hal_igmp_clear_discard_dhcp_pkt_count(void);



/**
* To set the age time of the user
* @param  age_time the age time want to set
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_age_time(unsigned int age_time);

/**
* To set the query interval of the user
* @param  query_interval the query interval want to set
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_query_interval(unsigned int query_interval);

/**
* To set the max response time of the user
* @param  max_response the max response time want to set
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_max_response_time(unsigned int max_response);


/**
* To set use snooping mode or not
* @param  enable enbale snooping or not, If 1 then enable, else disable
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_snooping_mode(unsigned int enable);

/**
* To set use proxy mode or not
* @param  type  igmp or mld(1 for igmp and 2 for mld)
* @param  enable enbale proxy or not, If 1 then enable, else disable
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_proxy_mode(unsigned int type, unsigned int enable);

/**
* To set use globe snooping mode or not
* @param  type  igmp or mld(1 for igmp and 2 for mld)
* @param  enable enbale globe snooping or not, If 1 then enable, else disable
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_globe_snooping_mode(unsigned int type, unsigned int enable);

/**
* To set the max group number  of a special uni port
* @param  uni_port  the uni port that will be configured
* @param  max_grp_num the max group number
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_uni_max_grp_num(hal_port_info_t hal_port_info, unsigned int max_grp_num);

/**
* To set the max host number  of a special uni port
* @param  uni_port  the uni port that will be configured
* @param  max_hosts the max host number
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_uni_max_hosts(hal_port_info_t hal_port_info, unsigned int max_hosts); //R31_IGMP

/**
* To set use fast leave mode or not
* @param  uni_port  the uni port that will be configured
* @param  enable enable fast leave mode or not, 1-enable;0-disable
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_uni_fast_leave_mode( hal_port_info_t hal_port_info, unsigned int enable);

/**
* To set rate limit of the upstream IGMP signal
* @param  uni_port  the uni port that will be configured
* @param  max_rate the number of max  igmp signals per second
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_uni_usr_rate_limit( hal_port_info_t hal_port_info, unsigned int max_rate);

/**
* To set the igmp version
* @param  igmp_ver the verion of igmp version
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_igmp_version(hal_port_info_t hal_port_info, unsigned char igmp_ver);

/**
* To set the match mode of the downstream igmp signal,support MAC or Vlan+MAC
* @param  matchmode the mode need to be configured
* @see  hal_igmp_match_mode_type_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_matchmode(hal_igmp_match_mode_type_t matchmode);


/**
* To set the uni port type, managed by TR069 or OMCI
* @param  uni_port  the uni port that will be configured
* @param  port_type the port type
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/

hal_ret_t hal_igmp_set_uni_port_type(unsigned int uni_port, unsigned int port_type);

/**
* To set the onu type
* @param onu_type  the type of onu
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_onu_type( unsigned int onu_type);

/**
* To set the trace level of igmp module. The level of the trace between min_level and max_level will be print
* @param  mod_name the name of module need to print
* @param  min_level the min trace level should be print
* @param  max_level the max trace level should be print
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_igmp_log_level(char * mod_name,
            unsigned int min_level, unsigned int max_level);

/**
* To get the trace level of igmp module. 
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_get_igmp_log_level(void); //R31_IGMP


/*The APIs need to be remove later*/

/**
* To the vlan entry id. AONTR2.x(with brdlt chipset) special API.
* @param  uni_port  the uni port that will be configured
* @param  vlan_ety_id the vlan entry id in driver
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_igmp_set_vlan_entry_id(hal_port_info_t hal_port_info, unsigned int vlan_ety_id);


hal_ret_t hal_gpon_cfg_arp_rate_limit(int value);
hal_ret_t hal_gpon_cfg_dhcp_rate_limit(int value);

hal_ret_t hal_igmp_send_dynamic_acl(unsigned int mc_eid,
				      hal_port_info_t hal_port_info,
				      unsigned int mc_gemid,
				      unsigned int mc_anivlan,
				      unsigned int src_ip,
				      unsigned int dst_ip_start,
				      unsigned int dst_ip_end,
				      unsigned int bw_info);
hal_ret_t hal_igmp_delete_dynamic_acl(unsigned int mc_eid,
				      hal_port_info_t hal_port_info,
				      unsigned int mc_gemid,
				      unsigned int mc_anivlan,
				      unsigned int src_ip,
				      unsigned int dst_ip_start,
				      unsigned int dst_ip_end,
				      unsigned int bw_info);
hal_ret_t hal_igmp_deleteall_dynamic_acl(hal_port_info_t hal_port_info, unsigned int mc_anivlan);
hal_ret_t hal_igmp_remove_uni_ds_mcast_vlan(hal_port_info_t hal_port_info, unsigned int mc_anivlan);
hal_ret_t hal_igmp_set_uni_max_mcast_bw(hal_port_info_t hal_port_info, unsigned int mc_bw_max);
hal_ret_t hal_igmp_set_uni_bw_enforce_mode(hal_port_info_t hal_port_info, unsigned int enable);
hal_ret_t hal_igmp_send_static_acl(unsigned int static_eid,
			       hal_port_info_t hal_port_info,
			       unsigned int static_gemid,
			       unsigned int static_anivlan,
			       unsigned int static_src_ip,
			       unsigned int static_dst_ip_start,
				   unsigned int static_dst_ip_end,
				   unsigned int static_bw_info);
hal_ret_t hal_igmp_delete_static_acl(unsigned int static_eid, 
			       hal_port_info_t hal_port_info,
			       unsigned int static_gemid, 
			       unsigned int static_anivlan,
			       unsigned int static_src_ip,
			       unsigned int static_dst_ip_start,
			       unsigned int static_dst_ip_end);
hal_ret_t hal_igmp_deleteall_static_acl(hal_port_info_t hal_port_info, unsigned int static_anivlan);

hal_ret_t hal_igmp_set_uni_access_method(hal_port_info_t hal_port_info, unsigned int acs_mthd);

hal_ret_t hal_igmp_set_board_info();
hal_ret_t hal_igmp_set_channel_mode(hal_port_info_t hal_port_info, unsigned int enable);
#ifdef __cplusplus
}
#endif


#endif
