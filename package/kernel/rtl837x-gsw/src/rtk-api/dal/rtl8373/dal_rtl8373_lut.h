#ifndef __DAL_RTL8373_LUT_H__
#define __DAL_RTL8373_LUT_H__


#include <l2.h>


#define RTL8373_LUT_IPMCGRP_TABLE_MAX  (0x3F)

#define RTL8373_LUT_BUSY_CHECK_NO      (10)

#define RTL8373_LUT_TABLE_SIZE         (3)



#define RTL8373_LUTREADMETHOD_MAC            0
#define RTL8373_LUTREADMETHOD_ADDRESS        1
#define RTL8373_LUTREADMETHOD_NEXT_ADDRESS   2
#define RTL8373_LUTREADMETHOD_NEXT_L2UC      3
#define RTL8373_LUTREADMETHOD_NEXT_L2MC      4
#define RTL8373_LUTREADMETHOD_NEXT_L3MC      5
#define RTL8373_LUTREADMETHOD_NEXT_L2L3MC    6
#define RTL8373_LUTREADMETHOD_NEXT_L2UCSPA   7


enum L2_NEWSA_BEHAVE
{
    L2_NEWSA_FWD = 0,
    L2_NEWSA_DROP,
    L2_NEWSA_TRAP,
    L2_BEHAVE_SA_END
};


enum RTL8373_FLUSHMODE
{
    FLUSHMDOE_PORT = 0,
    FLUSHMDOE_VID,
    FLUSHMDOE_FID,
    FLUSHMDOE_END,
};

enum RTL8373_L2_TRAPPORT
{
    TRAP_NONE = 0,
    TRAP_8051,
    TRAP_EXTERNAL,
    TRAP_BOTH,
};



typedef struct LUTTABLE{

    ipaddr_t sip;
    ipaddr_t dip;
    ether_addr_t mac;
    rtk_uint16 ivl_svl;
    rtk_uint16 cvid_fid;
    rtk_uint16 nosalearn;
    rtk_uint16 mbr;
    rtk_uint16 spa;
    rtk_uint16 age;
    rtk_uint16 l3lookup;
    rtk_uint16 auth;
    rtk_uint16 igmp_idx;
    rtk_uint16 igmp_asic;

    rtk_uint16 lookup_hit;
    rtk_uint16 lookup_busy;
    rtk_uint16 address;
    rtk_uint16 wait_time;

}rtl8373_luttb;


typedef struct  IPMCTBL{
    rtk_uint32 ipaddr;
    rtk_uint32 portmask;
    rtk_uint32 index;
}rtl8373_ipmctbl;


/* Function Name:
 *      dal_rtl8373_l2_init
 * Description:
 *      Initialize lut function.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_l2_init(void);



/* Function Name:
 *      dal_rtl8373_l2_addr_add
 * Description:
 *      Add LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      pL2_data - Unicast entry parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry.
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */

extern rtk_api_ret_t dal_rtl8373_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);


/* Function Name:
 *      dal_rtl8373_l2_addr_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pMac    - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port and fid where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t dal_rtl8373_l2_addr_get(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      dal_rtl8373_l2_addr_next_get
 * Description:
 *      Get Next LUT unicast entry.
 * Input:
 *      read_method     - The reading method.
 *      port            - The port number if the read_metohd is READMETHOD_NEXT_L2UCSPA
 *      pAddress        - The Address ID
 * Output:
 *      pL2_data - Unicast entry parameter
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next unicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all entries is LUT.
 */
extern rtk_api_ret_t dal_rtl8373_l2_addr_next_get(rtk_l2_read_method_t read_method, rtk_port_t port, rtk_uint32 *pAddress, rtk_l2_ucastAddr_t *pL2_data);


/* Function Name:
 *      dal_rtl8373_l2_addr_del
 * Description:
 *      Delete LUT unicast entry.
 * Input:
 *      pMac - 6 bytes unicast(I/G bit is 0) mac address to be written into LUT.
 *      fid - Filtering database
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t dal_rtl8373_l2_addr_del(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);



/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_add
 * Description:
 *      Add LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_MAC              - Invalid MAC address.
 *      RT_ERR_L2_FID           - Invalid FID .
 *      RT_ERR_L2_VID           - Invalid VID .
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced,
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
extern rtk_api_ret_t dal_rtl8373_l2_mcastAddr_add(rtk_l2_mcastAddr_t *pMcastAddr);

/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_get
 * Description:
 *      Get LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      pMcastAddr  - L2 multicast entry structure
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the multicast mac address existed in the LUT, it will return the port where
 *      the mac is learned. Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t dal_rtl8373_l2_mcastAddr_get(rtk_l2_mcastAddr_t *pMcastAddr);


/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_next_get
 * Description:
 *      Get Next L2 Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pMcastAddr  - L2 multicast entry structure
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next L2 multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all multicast entries is LUT.
 */
extern rtk_api_ret_t dal_rtl8373_l2_mcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_mcastAddr_t *pMcastAddr);


/* Function Name:
 *      dal_rtl8373_l2_mcastAddr_del
 * Description:
 *      Delete LUT multicast entry.
 * Input:
 *      pMcastAddr  - L2 multicast entry structure
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_MAC                  - Invalid MAC address.
 *      RT_ERR_L2_FID               - Invalid FID .
 *      RT_ERR_L2_VID               - Invalid VID .
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      If the mac has existed in the LUT, it will be deleted. Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t dal_rtl8373_l2_mcastAddr_del(rtk_l2_mcastAddr_t *pMcastAddr);


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_add
 * Description:
 *      Add Lut IP multicast entry
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_L2_INDEXTBL_FULL - hashed index is full of entries.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      System supports L2 entry with IP multicast DIP/SIP to forward IP multicasting frame as user
 *      desired. If this function is enabled, then system will be looked up L2 IP multicast entry to
 *      forward IP multicast frame directly without flooding.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_add(rtk_l2_ipMcastAddr_t *pIpMcastAddr);


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_get
 * Description:
 *      Get LUT IP multicast entry.
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      pIpMcastAddr    - IP Multicast entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can get Lut table of IP multicast entry.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_get(rtk_l2_ipMcastAddr_t *pIpMcastAddr);

/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_next_get
 * Description:
 *      Get Next IP Multicast entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pIpMcastAddr    - IP Multicast entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      Get the next IP multicast entry after the current entry pointed by pAddress.
 *      The address of next entry is returned by pAddress. User can use (address + 1)
 *      as pAddress to call this API again for dumping all IP multicast entries is LUT.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_ipMcastAddr_t *pIpMcastAddr);



/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddr_del
 * Description:
 *      Delete a ip multicast address entry from the specified device.
 * Input:
 *      pIpMcastAddr    - IP Multicast entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      The API can delete a IP multicast address entry from the specified device.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastAddr_del(rtk_l2_ipMcastAddr_t *pIpMcastAddr);

/* Function Name:
 *      dal_rtl8373_l2_ucastAddr_flush
 * Description:
 *      Flush L2 mac address by type in the specified device (both dynamic and static).
 * Input:
 *      pConfig - flush configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      flushByVid          - 1: Flush by VID, 0: Don't flush by VID
 *      vid                 - VID (0 ~ 4095)
 *      flushByFid          - 1: Flush by FID, 0: Don't flush by FID
 *      fid                 - FID (0 ~ 15)
 *      flushByPort         - 1: Flush by Port, 0: Don't flush by Port
 *      port                - Port ID
 *      flushByMac          - Not Supported
 *      ucastAddr           - Not Supported
 *      flushStaticAddr     - 1: Flush both Static and Dynamic entries, 0: Flush only Dynamic entries
 *      flushAddrOnAllPorts - 1: Flush VID-matched entries at all ports, 0: Flush VID-matched entries per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ucastAddr_flush(rtk_l2_flushCfg_t *pConfig);



/* Function Name:
 *      dal_rtl8373_l2_table_clear
 * Description:
 *      Flush all static & dynamic entries in LUT.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
 
extern rtk_api_ret_t dal_rtl8373_l2_table_clear(void);



/* Function Name:
 *      dal_rtl8373_l2_table_clearStatus_get
 * Description:
 *      Get table clear status
 * Input:
 *      None
 * Output:
 *      pStatus - Clear status, 1:Busy, 0:finish
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_l2_table_clearStatus_get(rtk_l2_clearStatus_t *pStatus);


/* Function Name:
 *      dal_rtl8373_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      enable - link down flush status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t dal_rtl8373_l2_flushLinkDownPortAddrEnable_set(rtk_enable_t enable);


/* Function Name:
 *      dal_rtl8373_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 * Output:
 *      pEnable - link down flush status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The status of flush linkdown port address is as following:
 *      - DISABLED
 *      - ENABLED
 */
extern rtk_api_ret_t dal_rtl8373_l2_flushLinkDownPortAddrEnable_get(rtk_enable_t *pEnable);


/* Function Name:
 *      dal_rtl8373_l2_agingEnable_set
 * Description:
 *      Set L2 LUT aging status per port setting.
 * Input:
 *      port    - Port id.
 *      enable  - Aging status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable);




/* Function Name:
 *      dal_rtl8373_l2_agingEnable_get
 * Description:
 *      Get L2 LUT aging status per port setting.
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can be used to get L2 LUT aging function per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable);




/* Function Name:
 *      dal_rtl8373_l2_ageout_timer_set
 * Description:
 *      Set L2 LUT entry age out timer.
 * Input:
 *      timer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ageout_timer_set(rtk_uint32 timer);

/* Function Name:
 *      dal_rtl8373_l2_ageout_timer_get
 * Description:
 *      Get L2 LUT entry age out timer.
 * Input:
 *      pTimer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ageout_timer_get(rtk_uint32 *pTimer);



/* Function Name:
 *      dal_rtl8373_l2_ageout_timer_set
 * Description:
 *      Set L2 LUT entry age field value when address learned in lut table.
 * Input:
 *      timer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_agefield_value_set(rtk_uint32 value);

/* Function Name:
 *      dal_rtl8373_l2_agefield_value_get
 * Description:
 *      Get L2 LUT entry age field value when address learned in lut table.
 * Input:
 *      pTimer  - the time of an lut entry ageout without update.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      This API can be used to set L2 LUT aging status per port.
 */
extern rtk_api_ret_t dal_rtl8373_l2_agefield_value_get(rtk_uint32 *pValue);



/* Function Name:
 *      dal_rtl8373_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port    - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set per-port ASIC auto learning limit number from 0(disable learning)
 *      to 2112.
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt);


/* Function Name:
 *      dal_rtl8373_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCnt_set
 * Description:
 *      Set System auto learning limit number
 * Input:
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_LIMITED_L2ENTRY_NUM  - Invalid auto learning limit number
 * Note:
 *      The API can set system ASIC auto learning limit number from 0(disable learning)
 *      to 2112.
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCnt_set(rtk_mac_cnt_t mac_cnt);


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCnt_get
 * Description:
 *      Get System auto learning limit number
 * Input:
 *      None
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get system ASIC auto learning limit number.
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCnt_get(rtk_mac_cnt_t *pMac_cnt);


/* Function Name:
 *      dal_rtl8373_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action);


/* Function Name:
 *      dal_rtl8373_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);



/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntAction_set
 * Description:
 *      Configure system auto learn over limit number action.
 * Input:
 *      port - Port id.
 *      action - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_NOT_ALLOWED  - Invalid learn over action
 * Note:
 *      The API can set SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntAction_set(rtk_l2_limitLearnCntAction_t action);


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntAction_get
 * Description:
 *      Get system auto learn over limit number action.
 * Input:
 *      None.
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get SA unknown packet action while auto learn limit number is over
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_FORWARD,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntAction_get(rtk_l2_limitLearnCntAction_t *pAction);


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntPortMask_set
 * Description:
 *      Configure system auto learn portmask
 * Input:
 *      pPortmask - Port Mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask.
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntPortMask_set(rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_rtl8373_l2_limitSystemLearningCntPortMask_get
 * Description:
 *      get system auto learn portmask
 * Input:
 *      None
 * Output:
 *      pPortmask - Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Null pointer.
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_l2_limitSystemLearningCntPortMask_get(rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_rtl8367d_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
extern rtk_api_ret_t dal_rtl8373_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddrLookup_set
 * Description:
 *      Set Lut IP multicast lookup function
 * Input:
 *      type - Lookup type for IPMC packet.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      LOOKUP_MAC      - Lookup by MAC address
 *      LOOKUP_IP       - Lookup by IP address
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastAddrLookup_set(rtk_l2_ipmc_lookup_type_t type);


/* Function Name:
 *      dal_rtl8373_l2_ipMcastAddrLookup_get
 * Description:
 *      Get Lut IP multicast lookup function
 * Input:
 *      None.
 * Output:
 *      pType - Lookup type for IPMC packet.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastAddrLookup_get(rtk_l2_ipmc_lookup_type_t *pType);



/* Function Name:
 *      dal_rtl8373_l2_ipMcastGroupEntry_add
 * Description:
 *      Add an IP Multicast entry to group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 *      pPortmask   - portmask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Add an entry to IP Multicast Group table.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastGroupEntry_add(ipaddr_t ip_addr, rtk_portmask_t * portmask);


/* Function Name:
 *      dal_rtl8373_l2_ipMcastGroupEntry_del
 * Description:
 *      Delete an entry from IP Multicast group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry from IP Multicast group table.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastGroupEntry_del(ipaddr_t ip_addr);


/* Function Name:
 *      dal_rtl8373_l2_ipMcastGroupEntry_get
 * Description:
 *      get an entry from IP Multicast group table
 * Input:
 *      ip_addr     - IP address
 *      vid         - VLAN ID
 * Output:
 *      pPortmask   - member port mask
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 *      RT_ERR_TBL_FULL    - Table Full
 * Note:
 *      Delete an entry from IP Multicast group table.
 */
extern rtk_api_ret_t dal_rtl8373_l2_ipMcastGroupEntry_get(ipaddr_t ip_addr, rtk_portmask_t *pPortmask);


/* Function Name:
 *      dal_rtl8373_l2_entry_get
 * Description:
 *      Get LUT unicast entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~2111.
 */
extern rtk_api_ret_t dal_rtl8373_l2_entry_get(rtk_l2_addr_table_t *pL2_entry);


extern rtk_api_ret_t dal_rtl8373_l2_entry_getNext(rtk_l2_addr_table_t *pL2_entry, rtk_uint32 method);

/* Function Name:
 *      dal_rtl8373_l2_entry_del
 * Description:
 *      Del LUT  entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~4096.
 */
extern rtk_api_ret_t dal_rtl8373_l2_entry_del(rtk_l2_addr_table_t *pL2_entry);




/* Function Name:
 *      dal_rtl8373_l2_lookupHitIsolationAction_set
 * Description:
 *      Set action of lookup hit & isolation.
 * Input:
 *      action          - The action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      This API is used to configure the action of packet which is lookup hit
 *      in L2 table but the destination port/portmask are not in the port isolation
 *      group.
 */
extern rtk_api_ret_t dal_rtl8373_l2_lookupHitIsolationAction_set(rtk_l2_lookupHitIsolationAction_t action);





/* Function Name:
 *      dal_rtl8373_l2_lookupHitIsolationAction_get
 * Description:
 *      Get action of lookup hit & isolation.
 * Input:
 *      None.
 * Output:
 *      pAction         - The action
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_NULL_POINTER         - Null pointer
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *      This API is used to get the action of packet which is lookup hit
 *      in L2 table but the destination port/portmask are not in the port isolation
 *      group.
 */
extern rtk_api_ret_t dal_rtl8373_l2_lookupHitIsolationAction_get(rtk_l2_lookupHitIsolationAction_t *pAction);


/* Function Name:
 *      dal_rtl8373_l2_portNewSaBehavior_set
 * Description:
 *      Set UNSA behavior
 * Input:
 *      port           - port 0~9
 *      behavior    - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_portNewSaBehavior_set(rtk_uint32 port, rtk_uint32 behavior);


/* Function Name:
 *      dal_rtl8373_l2_portNewSaBehavior_get
 * Description:
 *      Get UNSA behavior
 * Input:
 *      port           - port 0~9
 *      pBehavior   - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_portNewSaBehavior_get(rtk_uint32 port, rtk_uint32 *pBehavior);


/* Function Name:
 *      dal_rtl8373_l2_portUnmatchedSaBehavior_set
 * Description:
 *      Set Unmatched SA behavior
 * Input:
 *      port           - port 0~9
 *      behavior    - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Invalid behavior
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_portUnmatchedSaBehavior_set(rtk_uint32 port, rtk_uint32 behavior);
/* Function Name:
 *      dal_rtl8373_l2_portUnmatchedSaBehavior_get
 * Description:
 *      Get Unmatched SA behavior
 * Input:
 *      port           - port 0~9
 *      pBehavior   - 0: flooding; 1: drop; 2:trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_portUnmatchedSaBehavior_get(rtk_uint32 port, rtk_uint32 *pBehavior);



/* Function Name:
 *      dal_rtl8373_l2_portSaMovingForbid_set
 * Description:
 *      Set Unmatched SA moving state
 * Input:
 *      port        - Port ID
 *      enabled     - 0: can't move to new port; 1: can move to new port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_portSaMovingForbid_set(rtk_uint32 port, rtk_uint32 forbid);


/* Function Name:
 *      dal_rtl8373_getAsicPortUnmatchedSaMoving
 * Description:
 *      Get Unmatched SA moving state
 * Input:
 *      port        - Port ID
 * Output:
 *      pEnabled    - 0: can't move to new port; 1: can move to new port
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_portSaMovingForbid_get(rtk_uint32 port, rtk_uint32 *pForbid);



/* Function Name:
 *      dal_rtl8373_srcPortPermit_set
 * Description:
 *      Set Unmatched SA moving state
 * Input:
 *      port        - Port ID
 *      enabled     - 0: pkt cannot tx from rx port; 1: pkt can tx from rx port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_srcPortPermit_set(rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_srcPortPermit_get
 * Description:
 *      Get Unmatched SA moving state
 * Input:
 *      port        - Port ID
 * Output:
 *      pEnabled    -0: pkt cannot tx from rx port; 1: pkt can tx from rx port
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_srcPortPermit_get(rtk_uint32 port, rtk_uint32 *pEnable);


/* Function Name:
 *      dal_rtl8373_l2_unknUc_fldMsk_set
 * Description:
 *      Set unicast pkt lookup miss action
 * Input:
 *      portmask        - l2 unicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknUc_fldMsk_set(rtk_uint32 portMask);



/* Function Name:
 *      dal_rtl8373_l2_unknUc_fldMsk_get
 * Description:
 *      get unicast pkt lookup miss action
 * Input:
 *      portmask        - l2 unicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknUc_fldMsk_get(rtk_uint32* pMask);



/* Function Name:
 *      dal_rtl8373_l2_unknUc_action_set
 * Description:
 *      Set unicast pkt lookup miss action
 * Input:
 *      port        - Port ID *        - 0b00:fwd in L2_UNKNOW_UC_FLD_PMSK  0b01 drop  0b10 trap  0b11 flood
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */extern ret_t dal_rtl8373_l2_unknUc_action_set(rtk_uint32 port, rtk_uint32 );




/* Function Name:
 *      dal_rtl8373_l2_unknUc_action_get
 * Description:
 *      Get unicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in L2_UNKNOW_UC_FLD_PMSK  0b01 drop  0b10 trap  0b11 flood
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknUc_action_get(rtk_uint32 port, rtk_uint32 *pAction);



/* Function Name:
 *      dal_rtl8373_l2_unknMc_fldMsk_set
 * Description:
 *      Set multicast pkt lookup miss action
 * Input:
 *      portmask        - l2 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknMc_fldMsk_set(rtk_uint32 portMask);



/* Function Name:
 *      dal_rtl8373_l2_unknMc_fldMsk_get
 * Description:
 *      get multicast pkt lookup miss action
 * Input:
 *      portmask        - l2 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknMc_fldMsk_get(rtk_uint32* pMask);




/* Function Name:
 *      dal_rtl8373_l2_unknMc_action_set
 * Description:
 *      Set multicast pkt lookup miss action
 * Input:
 *      port        - Port ID *        - 0b00:fwd in L2_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 drop exclude RMA
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */extern ret_t dal_rtl8373_l2_unknMc_action_set(rtk_uint32 port, rtk_uint32 );




/* Function Name:
 *      dal_rtl8373_l2_unknMc_action_get
 * Description:
 *      Get multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in L2_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 drop exclude RMA
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknMc_action_get(rtk_uint32 port, rtk_uint32 *pAction);




/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_fldMsk_set
 * Description:
 *      Set ipv4 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv4 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknV4Mc_fldMsk_set(rtk_uint32 portMask);



/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_fldMsk_get
 * Description:
 *      get ipv4 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv4 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknV4Mc_fldMsk_get(rtk_uint32* pMask);






/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_action_set
 * Description:
 *      Set ipv4 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID *        - 0b00:fwd in IPV4_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */extern ret_t dal_rtl8373_l2_unknV4Mc_action_set(rtk_uint32 port, rtk_uint32 );



/* Function Name:
 *      dal_rtl8373_l2_unknV4Mc_action_get
 * Description:
 *      Get ipv4 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in IPV4_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknV4Mc_action_get(rtk_uint32 port, rtk_uint32 *pAction);



/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_fldMsk_set
 * Description:
 *      Set ipv6 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv6 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknV6Mc_fldMsk_set(rtk_uint32 portMask);


/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_fldMsk_get
 * Description:
 *      get ipv6 multicast pkt lookup miss action
 * Input:
 *      portmask        - ipv6 multicast pkt lookup miss flood portmask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknV6Mc_fldMsk_get(rtk_uint32* pMask);


/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_action_set
 * Description:
 *      Set ipv6 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID *        - 0b00:fwd in IPV6_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */extern ret_t dal_rtl8373_l2_unknV6Mc_action_set(rtk_uint32 port, rtk_uint32 );


/* Function Name:
 *      dal_rtl8373_l2_unknV6Mc_action_get
 * Description:
 *      Get ipv6 multicast pkt lookup miss action
 * Input:
 *      port        - Port ID
 *      pAction  - 0b00:fwd in IPV6_UNKNOW_MC_FLD_PMSK  0b01 drop  0b10 trap  0b11 to router port
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_unknV6Mc_action_get(rtk_uint32 port, rtk_uint32 *pAction);

/* Function Name:
 *      dal_rtl8373_l2_brdcast_fldMsk_set
 * Description:
 *      set brdcast pkt flood port mask
 * Input:
 *      portMask        - brdcast pkt flood port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_brdcast_fldMsk_set(rtk_uint32 portMask);

/* Function Name:
 *      dal_rtl8373_l2_brdcast_fldMsk_get
 * Description:
 *      get brdcast pkt flood port mask
 * Input:
 *      pMask        - brdcast pkt flood port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Error Port ID
 * Note:
 *      None
 */
extern ret_t dal_rtl8373_l2_brdcast_fldMsk_get(rtk_uint32* pMask);


/* Function Name:
 *      dal_rtl8373_l2_trapPort_set
 * Description:
 *      set l2 trap cpu port mask
 * Input:
 *      trapport        - 0b00 none    0b01:8051  0b10:external cpu    0b11: 8051&external
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_trapPort_set(rtk_uint32 trapport);


/* Function Name:
 *      dal_rtl8373_l2_trapPort_Get
 * Description:
 *      get l2 trap cpu port mask
 * Input:
 *      trapport        - 0b00 none    0b01:8051  0b10:external cpu    0b11: 8051&external
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_trapPort_get(rtk_uint32 * pTrapport);



/* Function Name:
 *      dal_rtl8373_l2_hashFull_set
 * Description:
 *      set l2 hsah full acti
 * Input:
 *      act        - 0b00 fwd    0b01:drop  0b10:trap  
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_hashFull_set(rtk_uint32 act);


/* Function Name:
 *      dal_rtl8373_l2_hashFull_get
 * Description:
 *      get l2 hsah full acti
 * Input:
 *      pAct        - 0b00 fwd    0b01:drop  0b10:trap  
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_hashFull_get(rtk_uint32* pAct);


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noVlanEgr_set
 * Description:
 *      set ipmulticast bypass vlanegress filter
 * Input:
 *      port        - port number
 *      enable    - 0 disable vlan egress filter     1 enable vlan egress filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_ipMul_noVlanEgr_set(rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noVlanEgr_get
 * Description:
 *      get ipmulticast bypass vlanegress filter
 * Input:
 *      port        - port number
 *      pEnable    - 0 disable vlan egress filter     1 enable vlan egress filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_ipMul_noVlanEgr_get(rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noPortIso_set
 * Description:
 *      set ipmulticast bypass port isolation filter
 * Input:
 *      port        - port number
 *      enable    - 0 disable port isolation filter     1 enable port isolation filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_ipMul_noPortIso_set(rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_l2_ipMul_noPortIso_get
 * Description:
 *      get ipmulticast bypass port isolation filter
 * Input:
 *      port        - port number
 *      pEnable    - 0 disable port isolation filter     1 enable port isolation filter
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_ipMul_noPortIso_get(rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_l2_forceMode_set
 * Description:
 *      set l2 force mode
 * Input:
 *      port        - port number
 *      enable    - 0 disable port force mode    1 enable port force mode
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_forceMode_set(rtk_uint32 port, rtk_uint32 enable);


/* Function Name:
 *      dal_rtl8373_l2_forceMode_get
 * Description:
 *      get l2 force mode
 * Input:
 *      port        - port number
 *      enable    - 0 disable port force mode    1 enable port force mode
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_forceMode_get(rtk_uint32 port, rtk_uint32* pEnable);


/* Function Name:
 *      dal_rtl8373_l2_forceMode_portMsk_set
 * Description:
 *      set l2 force mode
 * Input:
 *      port        - port number
 *      portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_forceMode_portMsk_set(rtk_uint32 port, rtk_uint32 portmask);

/* Function Name:
 *      dal_rtl8373_l2_forceMode_portMsk_get
 * Description:
 *      get l2 force mode
 * Input:
 *      port        - port number
 *      portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_forceMode_portMsk_get(rtk_uint32 port, rtk_uint32* pMask);



/* Function Name:
 *      dal_rtl8373_l2_entry_del
 * Description:
 *      Get LUT  entry.
 * Input:
 *      pL2_entry - Index field in the structure.
 * Output:
 *      pL2_entry - other fields such as MAC, port, age...
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_L2_EMPTY_ENTRY   - Empty LUT entry.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This API is used to get address by index from 0~2111.
 */
extern rtk_api_ret_t dal_rtl8373_l2_entry_del(rtk_l2_addr_table_t *pL2_entry);





/* Function Name:
 *      dal_rtl8373_l2_trapPri_set
 * Description:
 *      set l2 trap cpu port mask
 * Input:
 *      type           - 0: normal pkt   1: multicast
 *      trappri        - trap priority
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_trapPri_set(rtk_uint32 type, rtk_uint32 trappri);



/* Function Name:
 *      dal_rtl8373_l2_trapPri_get
 * Description:
 *      set l2 trap cpu port mask
 * Input:
 *      type           - 0: normal pkt   1: multicast
 *      pTrappri        - trap priority
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_trapPri_get(rtk_uint32 type, rtk_uint32* pTrappri);



/* Function Name:
 *      dal_rtl8373_l2_floodPortMsk_set
 * Description:
 *      set flood portmask
 * Input:
 *      flood_type       - unknown unicast, unknown l2 multicast, unknown IPV4 multicast, unknown IPV6 multicast, broadcast
 *      pFlood_portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_floodPortMsk_set(rtk_l2_flood_type_t flood_type, rtk_portmask_t *pFlood_portmask);





/* Function Name:
 *      dal_rtl8373_l2_floodPortMsk_get
 * Description:
 *      Get flood portmask
 * Input:
 *      flood_type       - unknown unicast, unknown l2 multicast, unknown IPV4 multicast, unknown IPV6 multicast, broadcast
 *      pFlood_portmask    - port mask
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      None
 */

extern rtk_api_ret_t dal_rtl8373_l2_floodPortMsk_get(rtk_l2_flood_type_t flood_type, rtk_portmask_t *pFlood_portmask);





 






#endif

