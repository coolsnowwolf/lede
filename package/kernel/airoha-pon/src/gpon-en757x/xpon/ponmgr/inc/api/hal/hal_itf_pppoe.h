/*
 * hal_itf_pppoe.h
 *
 *  Created on: Apr 10, 2012
 *      Author: dingjunh
 */

#ifndef HAL_ITF_PPPOE_H_
#define HAL_ITF_PPPOE_H_

#include "hal_type_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define HAL_ETHERNET_TYPE_PPPOE_D 0x8863
#define HAL_ETHERNET_TYPE_PPPOE_S 0x8864

typedef enum
{
    HAL_FILTERS_BEGIN = 0,
    HAL_FILTER_DHCP = HAL_FILTERS_BEGIN,  /**< Filter: DHCP                           */
    HAL_FILTER_IGMP,                       /**< Filter: IGMP                           */
    HAL_FILTER_MLD,                        /**< Filter: MLD                            */
    HAL_FILTER_ICMPV6,                     /**< Filter: ICMPv6                         */
    HAL_FILTER_ETYPE_UDEF_0,               /**< Filter: Ether-Type, User-Defined #0    */
    HAL_FILTER_ETYPE_UDEF_1,               /**< Filter: Ether-Type, User-Defined #1    */
    HAL_FILTER_ETYPE_UDEF_2,               /**< Filter: Ether-Type, User-Defined #2    */
    HAL_FILTER_ETYPE_UDEF_3,               /**< Filter: Ether-Type, User-Defined #3    */
    HAL_FILTER_ETYPE_PPPOE_D,              /**< Filter: Ether-Type, PPPoE, Discovery   */
    HAL_FILTER_ETYPE_PPPOE_S,              /**< Filter: Ether-Type, PPPoE, Session     */
    HAL_FILTER_ETYPE_ARP,                  /**< Filter: Ether-Type, ARP                */
    HAL_FILTER_ETYPE_PTP_1588,             /**< Filter: Ether-Type, PTP, 1588          */
    HAL_FILTER_ETYPE_802_1X,               /**< Filter: Ether-Type, 802.1X             */
    HAL_FILTER_ETYPE_802_1AG_CFM,          /**< Filter: Ether-Type, 802.1AG, CFM       */
    HAL_FILTER_MCAST,                      /**< Filter: Multicast                      */
    HAL_FILTER_BCAST,                      /**< Filter: Broadcast                      */
}hal_filter_eth_type;

typedef struct
{
    hal_header_t halHeader;
    unsigned int subnetId;
    unsigned short sessionId;
    unsigned int inPeerIp;
    char macAddr[6];
    char peerMacAddr[6];
} hal_pppoe_t;

int hal_pppoe_handle(hal_pppoe_t* pppoE);


/* ConfEthTypeFilter. */
typedef enum
{
    HAL_FILTER_ACTION_CPU_E = 1,
    HAL_FILTER_ACTION_DROP_E = 2,
    HAL_FILTER_ACTION_DISABLE_E = 3
}hal_eth_type_filter_cmd_t;

int hal_cfg_eth_type_filer(unsigned int phy_port, unsigned int eth_type, hal_eth_type_filter_cmd_t filter_cmd);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* HAL_ITF_PPPOE_H_ */

