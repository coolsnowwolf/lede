/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (週三, 08 三月 2017) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in L2 module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <l2.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_lut.h>
#include <rtl8367c_asicdrv_port.h>

/* Function Name:
 *      rtk_l2_init
 * Description:
 *      Initialize l2 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      Initialize l2 module before calling any l2 APIs.
 */
rtk_api_ret_t rtk_l2_init(void)
{
    rtk_api_ret_t retVal;
    rtk_uint32 port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8367c_setAsicLutIpMulticastLookup(DISABLED)) != RT_ERR_OK)
        return retVal;

    /*Enable CAM Usage*/
    if ((retVal = rtl8367c_setAsicLutCamTbUsage(ENABLED)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicLutAgeTimerSpeed(6,2)) != RT_ERR_OK)
        return retVal;

    RTK_SCAN_ALL_LOG_PORT(port)
    {
        if ((retVal = rtl8367c_setAsicLutLearnLimitNo(rtk_switch_port_L2P_get(port), rtk_switch_maxLutAddrNumber_get())) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_l2_addr_add
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
 *      If the unicast mac address already existed in LUT, it will update the status of the entry.
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if(pL2_data == NULL)
        return RT_ERR_MAC;

    RTK_CHK_PORT_VALID(pL2_data->port);

    if (pL2_data->ivl >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->cvid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if (pL2_data->fid > RTL8367C_FIDMAX)
        return RT_ERR_L2_FID;

    if (pL2_data->is_static>= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->sa_block>= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->da_block>= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->auth>= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->efid> RTL8367C_EFIDMAX)
        return RT_ERR_INPUT;

    if (pL2_data->priority > RTL8367C_PRIMAX)
        return RT_ERR_INPUT;

    if (pL2_data->sa_pri_en >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (pL2_data->fwd_pri_en >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pL2_data->ivl;
    l2Table.fid         = pL2_data->fid;
    l2Table.cvid_fid    = pL2_data->cvid;
    l2Table.efid        = pL2_data->efid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal )
    {
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pL2_data->ivl;
        l2Table.cvid_fid    = pL2_data->cvid;
        l2Table.fid         = pL2_data->fid;
        l2Table.efid        = pL2_data->efid;
        l2Table.spa         = rtk_switch_port_L2P_get(pL2_data->port);
        l2Table.nosalearn   = pL2_data->is_static;
        l2Table.sa_block    = pL2_data->sa_block;
        l2Table.da_block    = pL2_data->da_block;
        l2Table.l3lookup    = 0;
        l2Table.auth        = pL2_data->auth;
        l2Table.age         = 6;
        l2Table.lut_pri     = pL2_data->priority;
        l2Table.sa_en       = pL2_data->sa_pri_en;
        l2Table.fwd_en      = pL2_data->fwd_pri_en;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pL2_data->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal )
    {
        memset(&l2Table, 0, sizeof(rtl8367c_luttb));
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pL2_data->ivl;
        l2Table.cvid_fid    = pL2_data->cvid;
        l2Table.fid         = pL2_data->fid;
        l2Table.efid        = pL2_data->efid;
        l2Table.spa         = rtk_switch_port_L2P_get(pL2_data->port);
        l2Table.nosalearn   = pL2_data->is_static;
        l2Table.sa_block    = pL2_data->sa_block;
        l2Table.da_block    = pL2_data->da_block;
        l2Table.l3lookup    = 0;
        l2Table.auth        = pL2_data->auth;
        l2Table.age         = 6;
        l2Table.lut_pri     = pL2_data->priority;
        l2Table.sa_en       = pL2_data->sa_pri_en;
        l2Table.fwd_en      = pL2_data->fwd_pri_en;

        if ((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pL2_data->address = l2Table.address;

        method = LUTREADMETHOD_MAC;
        retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal )
            return RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;
    }
    else
        return retVal;

}

/* Function Name:
 *      rtk_l2_addr_get
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
rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (pL2_data->fid > RTL8367C_FIDMAX || pL2_data->efid > RTL8367C_EFIDMAX)
        return RT_ERR_L2_FID;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pL2_data->ivl;
    l2Table.cvid_fid    = pL2_data->cvid;
    l2Table.fid         = pL2_data->fid;
    l2Table.efid        = pL2_data->efid;
    method = LUTREADMETHOD_MAC;

    if ((retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    memcpy(pL2_data->mac.octet, pMac->octet,ETHER_ADDR_LEN);
    pL2_data->port      = rtk_switch_port_P2L_get(l2Table.spa);
    pL2_data->fid       = l2Table.fid;
    pL2_data->efid      = l2Table.efid;
    pL2_data->ivl       = l2Table.ivl_svl;
    pL2_data->cvid      = l2Table.cvid_fid;
    pL2_data->is_static = l2Table.nosalearn;
    pL2_data->auth      = l2Table.auth;
    pL2_data->sa_block  = l2Table.sa_block;
    pL2_data->da_block  = l2Table.da_block;
    pL2_data->priority  = l2Table.lut_pri;
    pL2_data->sa_pri_en = l2Table.sa_en;
    pL2_data->fwd_pri_en= l2Table.fwd_en;
    pL2_data->address   = l2Table.address;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_addr_next_get
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
rtk_api_ret_t rtk_l2_addr_next_get(rtk_l2_read_method_t read_method, rtk_port_t port, rtk_uint32 *pAddress, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      method;
    rtl8367c_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pL2_data == NULL) || (pAddress == NULL))
        return RT_ERR_MAC;

    if(read_method == READMETHOD_NEXT_L2UC)
        method = LUTREADMETHOD_NEXT_L2UC;
    else if(read_method == READMETHOD_NEXT_L2UCSPA)
        method = LUTREADMETHOD_NEXT_L2UCSPA;
    else
        return RT_ERR_INPUT;

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));
    l2Table.address = *pAddress;

    if(read_method == READMETHOD_NEXT_L2UCSPA)
        l2Table.spa = rtk_switch_port_L2P_get(port);

    if ((retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    if(l2Table.address < *pAddress)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    memcpy(pL2_data->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN);
    pL2_data->port      = rtk_switch_port_P2L_get(l2Table.spa);
    pL2_data->fid       = l2Table.fid;
    pL2_data->efid      = l2Table.efid;
    pL2_data->ivl       = l2Table.ivl_svl;
    pL2_data->cvid      = l2Table.cvid_fid;
    pL2_data->is_static = l2Table.nosalearn;
    pL2_data->auth      = l2Table.auth;
    pL2_data->sa_block  = l2Table.sa_block;
    pL2_data->da_block  = l2Table.da_block;
    pL2_data->priority  = l2Table.lut_pri;
    pL2_data->sa_pri_en = l2Table.sa_en;
    pL2_data->fwd_pri_en= l2Table.fwd_en;
    pL2_data->address   = l2Table.address;

    *pAddress = l2Table.address;

    return RT_ERR_OK;

}

/* Function Name:
 *      rtk_l2_addr_del
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
rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* must be unicast address */
    if ((pMac == NULL) || (pMac->octet[0] & 0x1))
        return RT_ERR_MAC;

    if (pL2_data->fid > RTL8367C_FIDMAX || pL2_data->efid > RTL8367C_EFIDMAX)
        return RT_ERR_L2_FID;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pL2_data->ivl;
    l2Table.cvid_fid    = pL2_data->cvid;
    l2Table.fid         = pL2_data->fid;
    l2Table.efid        = pL2_data->efid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK ==  retVal)
    {
        memcpy(l2Table.mac.octet, pMac->octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pL2_data->ivl;
        l2Table.cvid_fid    = pL2_data->cvid;
        l2Table.fid = pL2_data->fid;
        l2Table.efid = pL2_data->efid;
        l2Table.spa = 0;
        l2Table.nosalearn = 0;
        l2Table.sa_block = 0;
        l2Table.da_block = 0;
        l2Table.auth = 0;
        l2Table.age = 0;
        l2Table.lut_pri = 0;
        l2Table.sa_en = 0;
        l2Table.fwd_en = 0;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pL2_data->address = l2Table.address;
        return RT_ERR_OK;
    }
    else
        return retVal;
}

/* Function Name:
 *      rtk_l2_mcastAddr_add
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
 *      If the multicast mac address already existed in the LUT, it will update the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced,
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      method;
    rtl8367c_luttb  l2Table;
    rtk_uint32      pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* must be L2 multicast address */
    if( (pMcastAddr->mac.octet[0] & 0x01) != 0x01)
        return RT_ERR_MAC;

    RTK_CHK_PORTMASK_VALID(&pMcastAddr->portmask);

    if(pMcastAddr->ivl == 1)
    {
        if (pMcastAddr->vid > RTL8367C_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else if(pMcastAddr->ivl == 0)
    {
        if (pMcastAddr->fid > RTL8367C_FIDMAX)
            return RT_ERR_L2_FID;
    }
    else
        return RT_ERR_INPUT;

    if(pMcastAddr->fwd_pri_en >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(pMcastAddr->priority > RTL8367C_PRIMAX)
        return RT_ERR_INPUT;

    /* Get physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(&pMcastAddr->portmask, &pmask)) != RT_ERR_OK)
        return retVal;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pMcastAddr->ivl;

    if(pMcastAddr->ivl)
        l2Table.cvid_fid    = pMcastAddr->vid;
    else
        l2Table.cvid_fid    = pMcastAddr->fid;

    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pMcastAddr->ivl;

        if(pMcastAddr->ivl)
            l2Table.cvid_fid    = pMcastAddr->vid;
        else
            l2Table.cvid_fid    = pMcastAddr->fid;

        l2Table.mbr         = pmask;
        l2Table.nosalearn   = 1;
        l2Table.l3lookup    = 0;
        l2Table.lut_pri     = pMcastAddr->priority;
        l2Table.fwd_en      = pMcastAddr->fwd_pri_en;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table, 0, sizeof(rtl8367c_luttb));
        memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pMcastAddr->ivl;
        if(pMcastAddr->ivl)
            l2Table.cvid_fid    = pMcastAddr->vid;
        else
            l2Table.cvid_fid    = pMcastAddr->fid;

        l2Table.mbr         = pmask;
        l2Table.nosalearn   = 1;
        l2Table.l3lookup    = 0;
        l2Table.lut_pri     = pMcastAddr->priority;
        l2Table.fwd_en      = pMcastAddr->fwd_pri_en;
        if ((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pMcastAddr->address = l2Table.address;

        method = LUTREADMETHOD_MAC;
        retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;
    }
    else
        return retVal;

}

/* Function Name:
 *      rtk_l2_mcastAddr_get
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
rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* must be L2 multicast address */
    if( (pMcastAddr->mac.octet[0] & 0x01) != 0x01)
        return RT_ERR_MAC;

    if(pMcastAddr->ivl == 1)
    {
        if (pMcastAddr->vid > RTL8367C_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else if(pMcastAddr->ivl == 0)
    {
        if (pMcastAddr->fid > RTL8367C_FIDMAX)
            return RT_ERR_L2_FID;
    }
    else
        return RT_ERR_INPUT;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));
    memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pMcastAddr->ivl;

    if(pMcastAddr->ivl)
        l2Table.cvid_fid    = pMcastAddr->vid;
    else
        l2Table.cvid_fid    = pMcastAddr->fid;

    method = LUTREADMETHOD_MAC;

    if ((retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    pMcastAddr->priority    = l2Table.lut_pri;
    pMcastAddr->fwd_pri_en  = l2Table.fwd_en;
    pMcastAddr->igmp_asic   = l2Table.igmp_asic;
    pMcastAddr->igmp_index  = l2Table.igmpidx;
    pMcastAddr->address     = l2Table.address;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_mcastAddr_next_get
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
rtk_api_ret_t rtk_l2_mcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtl8367c_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pAddress == NULL) || (pMcastAddr == NULL))
        return RT_ERR_INPUT;

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));
    l2Table.address = *pAddress;

    if ((retVal = rtl8367c_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L2MC, &l2Table)) != RT_ERR_OK)
        return retVal;

    if(l2Table.address < *pAddress)
        return RT_ERR_L2_ENTRY_NOTFOUND;

    memcpy(pMcastAddr->mac.octet, l2Table.mac.octet, ETHER_ADDR_LEN);
    pMcastAddr->ivl     = l2Table.ivl_svl;

    if(pMcastAddr->ivl)
        pMcastAddr->vid = l2Table.cvid_fid;
    else
        pMcastAddr->fid = l2Table.cvid_fid;

    pMcastAddr->priority    = l2Table.lut_pri;
    pMcastAddr->fwd_pri_en  = l2Table.fwd_en;
    pMcastAddr->igmp_asic   = l2Table.igmp_asic;
    pMcastAddr->igmp_index  = l2Table.igmpidx;
    pMcastAddr->address     = l2Table.address;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    *pAddress = l2Table.address;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_mcastAddr_del
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
rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_l2_mcastAddr_t *pMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* must be L2 multicast address */
    if( (pMcastAddr->mac.octet[0] & 0x01) != 0x01)
        return RT_ERR_MAC;

    if(pMcastAddr->ivl == 1)
    {
        if (pMcastAddr->vid > RTL8367C_VIDMAX)
            return RT_ERR_L2_VID;
    }
    else if(pMcastAddr->ivl == 0)
    {
        if (pMcastAddr->fid > RTL8367C_FIDMAX)
            return RT_ERR_L2_FID;
    }
    else
        return RT_ERR_INPUT;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));

    /* fill key (MAC,FID) to get L2 entry */
    memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
    l2Table.ivl_svl     = pMcastAddr->ivl;

    if(pMcastAddr->ivl)
        l2Table.cvid_fid    = pMcastAddr->vid;
    else
        l2Table.cvid_fid    = pMcastAddr->fid;

    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        memcpy(l2Table.mac.octet, pMcastAddr->mac.octet, ETHER_ADDR_LEN);
        l2Table.ivl_svl     = pMcastAddr->ivl;

        if(pMcastAddr->ivl)
            l2Table.cvid_fid    = pMcastAddr->vid;
        else
            l2Table.cvid_fid    = pMcastAddr->fid;

        l2Table.mbr         = 0;
        l2Table.nosalearn   = 0;
        l2Table.sa_block    = 0;
        l2Table.l3lookup    = 0;
        l2Table.lut_pri     = 0;
        l2Table.fwd_en      = 0;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else
        return retVal;
}

/* Function Name:
 *      rtk_l2_ipMcastAddr_add
 * Description:
 *      Add LUT IP multicast entry
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
rtk_api_ret_t rtk_l2_ipMcastAddr_add(rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* check port mask */
    RTK_CHK_PORTMASK_VALID(&pIpMcastAddr->portmask);

    if( (pIpMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    if(pIpMcastAddr->fwd_pri_en >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pIpMcastAddr->priority > RTL8367C_PRIMAX)
        return RT_ERR_INPUT;

    /* Get Physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(&pIpMcastAddr->portmask, &pmask)) != RT_ERR_OK)
        return retVal;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.sip = pIpMcastAddr->sip;
    l2Table.dip = pIpMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.l3vidlookup = 0;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        l2Table.sip = pIpMcastAddr->sip;
        l2Table.dip = pIpMcastAddr->dip;
        l2Table.mbr = pmask;
        l2Table.nosalearn = 1;
        l2Table.l3lookup = 1;
        l2Table.l3vidlookup = 0;
        l2Table.lut_pri = pIpMcastAddr->priority;
        l2Table.fwd_en  = pIpMcastAddr->fwd_pri_en;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table, 0, sizeof(rtl8367c_luttb));
        l2Table.sip = pIpMcastAddr->sip;
        l2Table.dip = pIpMcastAddr->dip;
        l2Table.mbr = pmask;
        l2Table.nosalearn = 1;
        l2Table.l3lookup = 1;
        l2Table.l3vidlookup = 0;
        l2Table.lut_pri = pIpMcastAddr->priority;
        l2Table.fwd_en  = pIpMcastAddr->fwd_pri_en;
        if ((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpMcastAddr->address = l2Table.address;

        method = LUTREADMETHOD_MAC;
        retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;

    }
    else
        return retVal;

}

/* Function Name:
 *      rtk_l2_ipMcastAddr_get
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
 *      The API can get LUT table of IP multicast entry.
 */
rtk_api_ret_t rtk_l2_ipMcastAddr_get(rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpMcastAddr)
        return RT_ERR_NULL_POINTER;

    if( (pIpMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.sip = pIpMcastAddr->sip;
    l2Table.dip = pIpMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.l3vidlookup = 0;
    method = LUTREADMETHOD_MAC;
    if ((retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pIpMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    pIpMcastAddr->priority      = l2Table.lut_pri;
    pIpMcastAddr->fwd_pri_en    = l2Table.fwd_en;
    pIpMcastAddr->igmp_asic     = l2Table.igmp_asic;
    pIpMcastAddr->igmp_index    = l2Table.igmpidx;
    pIpMcastAddr->address       = l2Table.address;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastAddr_next_get
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
rtk_api_ret_t rtk_l2_ipMcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtl8367c_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pAddress == NULL) || (pIpMcastAddr == NULL) )
        return RT_ERR_INPUT;

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));
    l2Table.address = *pAddress;

    do
    {
        if ((retVal = rtl8367c_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L3MC, &l2Table)) != RT_ERR_OK)
            return retVal;

        if(l2Table.address < *pAddress)
            return RT_ERR_L2_ENTRY_NOTFOUND;

    }while(l2Table.l3vidlookup == 1);

    pIpMcastAddr->sip = l2Table.sip;
    pIpMcastAddr->dip = l2Table.dip;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pIpMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    pIpMcastAddr->priority      = l2Table.lut_pri;
    pIpMcastAddr->fwd_pri_en    = l2Table.fwd_en;
    pIpMcastAddr->igmp_asic     = l2Table.igmp_asic;
    pIpMcastAddr->igmp_index    = l2Table.igmpidx;
    pIpMcastAddr->address       = l2Table.address;
    *pAddress = l2Table.address;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastAddr_del
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
rtk_api_ret_t rtk_l2_ipMcastAddr_del(rtk_l2_ipMcastAddr_t *pIpMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if (pIpMcastAddr == NULL)
        return RT_ERR_INPUT;

    if( (pIpMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.sip = pIpMcastAddr->sip;
    l2Table.dip = pIpMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.l3vidlookup = 0;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        l2Table.sip = pIpMcastAddr->sip;
        l2Table.dip = pIpMcastAddr->dip;
        l2Table.mbr = 0;
        l2Table.nosalearn = 0;
        l2Table.l3lookup = 1;
        l2Table.l3vidlookup = 0;
        l2Table.lut_pri = 0;
        l2Table.fwd_en  = 0;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else
        return retVal;
}

/* Function Name:
 *      rtk_l2_ipVidMcastAddr_add
 * Description:
 *      Add LUT IP multicast+VID entry
 * Input:
 *      pIpVidMcastAddr - IP & VID multicast Entry
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
 *
 */
rtk_api_ret_t rtk_l2_ipVidMcastAddr_add(rtk_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpVidMcastAddr)
        return RT_ERR_NULL_POINTER;

    /* check port mask */
    RTK_CHK_PORTMASK_VALID(&pIpVidMcastAddr->portmask);

    if (pIpVidMcastAddr->vid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if( (pIpVidMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    /* Get Physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(&pIpVidMcastAddr->portmask, &pmask)) != RT_ERR_OK)
        return retVal;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.sip = pIpVidMcastAddr->sip;
    l2Table.dip = pIpVidMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.l3vidlookup = 1;
    l2Table.l3_vid = pIpVidMcastAddr->vid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        l2Table.sip = pIpVidMcastAddr->sip;
        l2Table.dip = pIpVidMcastAddr->dip;
        l2Table.mbr = pmask;
        l2Table.nosalearn = 1;
        l2Table.l3lookup = 1;
        l2Table.l3vidlookup = 1;
        l2Table.l3_vid = pIpVidMcastAddr->vid;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpVidMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
    {
        memset(&l2Table, 0, sizeof(rtl8367c_luttb));
        l2Table.sip = pIpVidMcastAddr->sip;
        l2Table.dip = pIpVidMcastAddr->dip;
        l2Table.mbr = pmask;
        l2Table.nosalearn = 1;
        l2Table.l3lookup = 1;
        l2Table.l3vidlookup = 1;
        l2Table.l3_vid = pIpVidMcastAddr->vid;
        if ((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpVidMcastAddr->address = l2Table.address;

        method = LUTREADMETHOD_MAC;
        retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
        if (RT_ERR_L2_ENTRY_NOTFOUND == retVal)
            return     RT_ERR_L2_INDEXTBL_FULL;
        else
            return retVal;

    }
    else
        return retVal;
}

/* Function Name:
 *      rtk_l2_ipVidMcastAddr_get
 * Description:
 *      Get LUT IP multicast+VID entry.
 * Input:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Output:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *
 */
rtk_api_ret_t rtk_l2_ipVidMcastAddr_get(rtk_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpVidMcastAddr)
        return RT_ERR_NULL_POINTER;

    if (pIpVidMcastAddr->vid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if( (pIpVidMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.sip = pIpVidMcastAddr->sip;
    l2Table.dip = pIpVidMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.l3vidlookup = 1;
    l2Table.l3_vid = pIpVidMcastAddr->vid;
    method = LUTREADMETHOD_MAC;
    if ((retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    pIpVidMcastAddr->address = l2Table.address;

     /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pIpVidMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipVidMcastAddr_next_get
 * Description:
 *      Get Next IP Multicast+VID entry.
 * Input:
 *      pAddress        - The Address ID
 * Output:
 *      pIpVidMcastAddr - IP & VID multicast Entry
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
rtk_api_ret_t rtk_l2_ipVidMcastAddr_next_get(rtk_uint32 *pAddress, rtk_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtk_api_ret_t   retVal;
    rtl8367c_luttb  l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Error Checking */
    if ((pAddress == NULL) || (pIpVidMcastAddr == NULL))
        return RT_ERR_INPUT;

    if(*pAddress > RTK_MAX_LUT_ADDR_ID )
        return RT_ERR_L2_L2UNI_PARAM;

    memset(&l2Table, 0, sizeof(rtl8367c_luttb));
    l2Table.address = *pAddress;

    do
    {
        if ((retVal = rtl8367c_getAsicL2LookupTb(LUTREADMETHOD_NEXT_L3MC, &l2Table)) != RT_ERR_OK)
            return retVal;

        if(l2Table.address < *pAddress)
            return RT_ERR_L2_ENTRY_NOTFOUND;

    }while(l2Table.l3vidlookup == 0);

    pIpVidMcastAddr->sip        = l2Table.sip;
    pIpVidMcastAddr->dip        = l2Table.dip;
    pIpVidMcastAddr->vid        = l2Table.l3_vid;
    pIpVidMcastAddr->address    = l2Table.address;

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &pIpVidMcastAddr->portmask)) != RT_ERR_OK)
        return retVal;

    *pAddress = l2Table.address;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipVidMcastAddr_del
 * Description:
 *      Delete a ip multicast+VID address entry from the specified device.
 * Input:
 *      pIpVidMcastAddr - IP & VID multicast Entry
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_L2_ENTRY_NOTFOUND    - No such LUT entry.
 *      RT_ERR_INPUT                - Invalid input parameters.
 * Note:
 *
 */
rtk_api_ret_t rtk_l2_ipVidMcastAddr_del(rtk_l2_ipVidMcastAddr_t *pIpVidMcastAddr)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pIpVidMcastAddr)
        return RT_ERR_NULL_POINTER;

    if (pIpVidMcastAddr->vid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if( (pIpVidMcastAddr->dip & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.sip = pIpVidMcastAddr->sip;
    l2Table.dip = pIpVidMcastAddr->dip;
    l2Table.l3lookup = 1;
    l2Table.l3vidlookup = 1;
    l2Table.l3_vid = pIpVidMcastAddr->vid;
    method = LUTREADMETHOD_MAC;
    retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table);
    if (RT_ERR_OK == retVal)
    {
        l2Table.sip = pIpVidMcastAddr->sip;
        l2Table.dip = pIpVidMcastAddr->dip;
        l2Table.mbr= 0;
        l2Table.nosalearn = 0;
        l2Table.l3lookup = 1;
        l2Table.l3vidlookup = 1;
        l2Table.l3_vid = pIpVidMcastAddr->vid;
        if((retVal = rtl8367c_setAsicL2LookupTb(&l2Table)) != RT_ERR_OK)
            return retVal;

        pIpVidMcastAddr->address = l2Table.address;
        return RT_ERR_OK;
    }
    else
        return retVal;
}

/* Function Name:
 *      rtk_l2_ucastAddr_flush
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
rtk_api_ret_t rtk_l2_ucastAddr_flush(rtk_l2_flushCfg_t *pConfig)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pConfig == NULL)
        return RT_ERR_NULL_POINTER;

    if(pConfig->flushByVid >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushByFid >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushByPort >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushByMac >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushStaticAddr >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->flushAddrOnAllPorts >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(pConfig->vid > RTL8367C_VIDMAX)
        return RT_ERR_VLAN_VID;

    if(pConfig->fid > RTL8367C_FIDMAX)
        return RT_ERR_INPUT;

    /* check port valid */
    RTK_CHK_PORT_VALID(pConfig->port);

    if(pConfig->flushByVid == ENABLED)
    {
        if ((retVal = rtl8367c_setAsicLutFlushMode(FLUSHMDOE_VID)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutFlushVid(pConfig->vid)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8367c_setAsicLutFlushType((pConfig->flushStaticAddr == ENABLED) ? FLUSHTYPE_BOTH : FLUSHTYPE_DYNAMIC)) != RT_ERR_OK)
            return retVal;

        if(pConfig->flushAddrOnAllPorts == ENABLED)
        {
            if ((retVal = rtl8367c_setAsicLutForceFlush(RTL8367C_PORTMASK)) != RT_ERR_OK)
                return retVal;
        }
        else if(pConfig->flushByPort == ENABLED)
        {
            if ((retVal = rtl8367c_setAsicLutForceFlush(1 << rtk_switch_port_L2P_get(pConfig->port))) != RT_ERR_OK)
                return retVal;
        }
        else
            return RT_ERR_INPUT;
    }
    else if(pConfig->flushByFid == ENABLED)
    {
        if ((retVal = rtl8367c_setAsicLutFlushMode(FLUSHMDOE_FID)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutFlushFid(pConfig->fid)) != RT_ERR_OK)
                return retVal;

        if ((retVal = rtl8367c_setAsicLutFlushType((pConfig->flushStaticAddr == ENABLED) ? FLUSHTYPE_BOTH : FLUSHTYPE_DYNAMIC)) != RT_ERR_OK)
            return retVal;

        if(pConfig->flushAddrOnAllPorts == ENABLED)
        {
            if ((retVal = rtl8367c_setAsicLutForceFlush(RTL8367C_PORTMASK)) != RT_ERR_OK)
                return retVal;
        }
        else if(pConfig->flushByPort == ENABLED)
        {
            if ((retVal = rtl8367c_setAsicLutForceFlush(1 << rtk_switch_port_L2P_get(pConfig->port))) != RT_ERR_OK)
                return retVal;
        }
        else
            return RT_ERR_INPUT;
    }
    else if(pConfig->flushByPort == ENABLED)
    {
        if ((retVal = rtl8367c_setAsicLutFlushType((pConfig->flushStaticAddr == ENABLED) ? FLUSHTYPE_BOTH : FLUSHTYPE_DYNAMIC)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutFlushMode(FLUSHMDOE_PORT)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutForceFlush(1 << rtk_switch_port_L2P_get(pConfig->port))) != RT_ERR_OK)
            return retVal;
    }
    else if(pConfig->flushByMac == ENABLED)
    {
        /* Should use API "rtk_l2_addr_del" to remove a specified entry*/
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_table_clear
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
rtk_api_ret_t rtk_l2_table_clear(void)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = rtl8367c_setAsicLutFlushAll()) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_table_clearStatus_get
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
rtk_api_ret_t rtk_l2_table_clearStatus_get(rtk_l2_clearStatus_t *pStatus)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pStatus)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutFlushAllStatus((rtk_uint32 *)pStatus)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_set
 * Description:
 *      Set HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
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
rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicLutLinkDownForceAging(enable)) != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_flushLinkDownPortAddrEnable_get
 * Description:
 *      Get HW flush linkdown port mac configuration of the specified device.
 * Input:
 *      port - Port id.
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
rtk_api_ret_t rtk_l2_flushLinkDownPortAddrEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutLinkDownForceAging(pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_agingEnable_set
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
rtk_api_ret_t rtk_l2_agingEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if(enable == 1)
        enable = 0;
    else
        enable = 1;

    if ((retVal = rtl8367c_setAsicLutDisableAging(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_agingEnable_get
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
rtk_api_ret_t rtk_l2_agingEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutDisableAging(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    if(*pEnable == 1)
        *pEnable = 0;
    else
        *pEnable = 1;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
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
rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (mac_cnt > rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if ((retVal = rtl8367c_setAsicLutLearnLimitNo(rtk_switch_port_L2P_get(port), mac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
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
rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutLearnLimitNo(rtk_switch_port_L2P_get(port), pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitSystemLearningCnt_set
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
rtk_api_ret_t rtk_l2_limitSystemLearningCnt_set(rtk_mac_cnt_t mac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (mac_cnt > rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_LIMITED_L2ENTRY_NUM;

    if ((retVal = rtl8367c_setAsicSystemLutLearnLimitNo(mac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitSystemLearningCnt_get
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
rtk_api_ret_t rtk_l2_limitSystemLearningCnt_get(rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSystemLutLearnLimitNo(pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
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
rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if ( LIMIT_LEARN_CNT_ACTION_DROP == action )
        data = 1;
    else if ( LIMIT_LEARN_CNT_ACTION_FORWARD == action )
        data = 0;
    else if ( LIMIT_LEARN_CNT_ACTION_TO_CPU == action )
        data = 2;
    else
        return RT_ERR_NOT_ALLOWED;

    if ((retVal = rtl8367c_setAsicLutLearnOverAct(data)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
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
rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    rtk_api_ret_t retVal;
    rtk_uint32 action;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (port != RTK_WHOLE_SYSTEM)
        return RT_ERR_PORT_ID;

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutLearnOverAct(&action)) != RT_ERR_OK)
        return retVal;

    if ( 1 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    else if ( 0 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
    else if ( 2 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    else
    *pAction = action;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitSystemLearningCntAction_set
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
rtk_api_ret_t rtk_l2_limitSystemLearningCntAction_set(rtk_l2_limitLearnCntAction_t action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 data;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ( LIMIT_LEARN_CNT_ACTION_DROP == action )
        data = 1;
    else if ( LIMIT_LEARN_CNT_ACTION_FORWARD == action )
        data = 0;
    else if ( LIMIT_LEARN_CNT_ACTION_TO_CPU == action )
        data = 2;
    else
        return RT_ERR_NOT_ALLOWED;

    if ((retVal = rtl8367c_setAsicSystemLutLearnOverAct(data)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitSystemLearningCntAction_get
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
rtk_api_ret_t rtk_l2_limitSystemLearningCntAction_get(rtk_l2_limitLearnCntAction_t *pAction)
{
    rtk_api_ret_t retVal;
    rtk_uint32 action;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSystemLutLearnOverAct(&action)) != RT_ERR_OK)
        return retVal;

    if ( 1 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_DROP;
    else if ( 0 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_FORWARD;
    else if ( 2 == action )
        *pAction = LIMIT_LEARN_CNT_ACTION_TO_CPU;
    else
    *pAction = action;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitSystemLearningCntPortMask_set
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
rtk_api_ret_t rtk_l2_limitSystemLearningCntPortMask_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    /* Check port mask */
    RTK_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicSystemLutLearnPortMask(pmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_limitSystemLearningCntPortMask_get
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
rtk_api_ret_t rtk_l2_limitSystemLearningCntPortMask_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicSystemLutLearnPortMask(&pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_learningCnt_get
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
rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pMac_cnt)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutLearnNo(rtk_switch_port_L2P_get(port), pMac_cnt)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_floodPortMask_set
 * Description:
 *      Set flooding portmask
 * Input:
 *      type - flooding type.
 *      pFlood_portmask - flooding portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can set the flooding mask.
 *      The flooding type is as following:
 *      - FLOOD_UNKNOWNDA
 *      - FLOOD_UNKNOWNMC
 *      - FLOOD_BC
 */
rtk_api_ret_t rtk_l2_floodPortMask_set(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (floood_type >= FLOOD_END)
        return RT_ERR_INPUT;

    /* check port valid */
    RTK_CHK_PORTMASK_VALID(pFlood_portmask);

    /* Get Physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(pFlood_portmask, &pmask))!=RT_ERR_OK)
        return retVal;

    switch (floood_type)
    {
        case FLOOD_UNKNOWNDA:
            if ((retVal = rtl8367c_setAsicPortUnknownDaFloodingPortmask(pmask)) != RT_ERR_OK)
                return retVal;
            break;
        case FLOOD_UNKNOWNMC:
            if ((retVal = rtl8367c_setAsicPortUnknownMulticastFloodingPortmask(pmask)) != RT_ERR_OK)
                return retVal;
            break;
        case FLOOD_BC:
            if ((retVal = rtl8367c_setAsicPortBcastFloodingPortmask(pmask)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtk_l2_floodPortMask_get
 * Description:
 *      Get flooding portmask
 * Input:
 *      type - flooding type.
 * Output:
 *      pFlood_portmask - flooding portmask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API can get the flooding mask.
 *      The flooding type is as following:
 *      - FLOOD_UNKNOWNDA
 *      - FLOOD_UNKNOWNMC
 *      - FLOOD_BC
 */
rtk_api_ret_t rtk_l2_floodPortMask_get(rtk_l2_flood_type_t floood_type, rtk_portmask_t *pFlood_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (floood_type >= FLOOD_END)
        return RT_ERR_INPUT;

    if(NULL == pFlood_portmask)
        return RT_ERR_NULL_POINTER;

    switch (floood_type)
    {
        case FLOOD_UNKNOWNDA:
            if ((retVal = rtl8367c_getAsicPortUnknownDaFloodingPortmask(&pmask)) != RT_ERR_OK)
                return retVal;
            break;
        case FLOOD_UNKNOWNMC:
            if ((retVal = rtl8367c_getAsicPortUnknownMulticastFloodingPortmask(&pmask)) != RT_ERR_OK)
                return retVal;
            break;
        case FLOOD_BC:
            if ((retVal = rtl8367c_getAsicPortBcastFloodingPortmask(&pmask)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    /* Get Logical port mask */
    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pFlood_portmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_localPktPermit_set
 * Description:
 *      Set permission of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 *      permit - permission status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid permit value.
 * Note:
 *      This API is set to permit frame if its source port is equal to destination port.
 */
rtk_api_ret_t rtk_l2_localPktPermit_set(rtk_port_t port, rtk_enable_t permit)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (permit >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortBlockSpa(rtk_switch_port_L2P_get(port), permit)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_localPktPermit_get
 * Description:
 *      Get permission of frames if source port and destination port are the same.
 * Input:
 *      port - Port id.
 * Output:
 *      pPermit - permission status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is to get permission status for frames if its source port is equal to destination port.
 */
rtk_api_ret_t rtk_l2_localPktPermit_get(rtk_port_t port, rtk_enable_t *pPermit)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPermit)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortBlockSpa(rtk_switch_port_L2P_get(port), pPermit)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_aging_set
 * Description:
 *      Set LUT ageing out speed
 * Input:
 *      aging_time - Ageing out time.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can set LUT ageing out period for each entry and the range is from 45s to 458s.
 */
rtk_api_ret_t rtk_l2_aging_set(rtk_l2_age_time_t aging_time)
{
    rtk_uint32 i;
    CONST_T rtk_uint32 agePara[10][3] = {
        {45, 0, 1}, {88, 0, 2}, {133, 0, 3}, {177, 0, 4}, {221, 0, 5}, {266, 0, 6}, {310, 0, 7},
        {354, 2, 6}, {413, 2, 7}, {458, 3, 7}};

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (aging_time>agePara[9][0])
        return RT_ERR_OUT_OF_RANGE;

    for (i = 0; i<10; i++)
    {
        if (aging_time<=agePara[i][0])
        {
            return rtl8367c_setAsicLutAgeTimerSpeed(agePara[i][2], agePara[i][1]);
        }
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_aging_get
 * Description:
 *      Get LUT ageing out time
 * Input:
 *      None
 * Output:
 *      pEnable - Aging status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get LUT ageing out period for each entry.
 */
rtk_api_ret_t rtk_l2_aging_get(rtk_l2_age_time_t *pAging_time)
{
    rtk_api_ret_t retVal;
    rtk_uint32 i,time, speed;
    CONST_T rtk_uint32 agePara[10][3] = {
        {45, 0, 1}, {88, 0, 2}, {133, 0, 3}, {177, 0, 4}, {221, 0, 5}, {266, 0, 6}, {310, 0, 7},
        {354, 2, 6}, {413, 2, 7}, {458, 3, 7}};

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAging_time)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicLutAgeTimerSpeed(&time, &speed)) != RT_ERR_OK)
        return retVal;

    for (i = 0; i<10; i++)
    {
        if (time==agePara[i][2]&&speed==agePara[i][1])
        {
            *pAging_time = agePara[i][0];
            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_set
 * Description:
 *      Set LUT IP multicast lookup function
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
 *      LOOKUP_IP_VID   - Lookup by IP address & VLAN ID
 */
rtk_api_ret_t rtk_l2_ipMcastAddrLookup_set(rtk_l2_ipmc_lookup_type_t type)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(type == LOOKUP_MAC)
    {
        if((retVal = rtl8367c_setAsicLutIpMulticastLookup(DISABLED)) != RT_ERR_OK)
            return retVal;
    }
    else if(type == LOOKUP_IP)
    {
        if((retVal = rtl8367c_setAsicLutIpMulticastLookup(ENABLED)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutIpMulticastVidLookup(DISABLED))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutIpLookupMethod(1))!=RT_ERR_OK)
            return retVal;
    }
    else if(type == LOOKUP_IP_VID)
    {
        if((retVal = rtl8367c_setAsicLutIpMulticastLookup(ENABLED)) != RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutIpMulticastVidLookup(ENABLED))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367c_setAsicLutIpLookupMethod(1))!=RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastAddrLookup_get
 * Description:
 *      Get LUT IP multicast lookup function
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
rtk_api_ret_t rtk_l2_ipMcastAddrLookup_get(rtk_l2_ipmc_lookup_type_t *pType)
{
    rtk_api_ret_t       retVal;
    rtk_uint32          enabled, vid_lookup;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pType)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8367c_getAsicLutIpMulticastLookup(&enabled)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicLutIpMulticastVidLookup(&vid_lookup))!=RT_ERR_OK)
        return retVal;

    if(enabled == ENABLED)
    {
        if(vid_lookup == ENABLED)
            *pType = LOOKUP_IP_VID;
        else
            *pType = LOOKUP_IP;
    }
    else
        *pType = LOOKUP_MAC;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastForwardRouterPort_set
 * Description:
 *      Set IPMC packet forward to router port also or not
 * Input:
 *      enabled - 1: Include router port, 0, exclude router port
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *
 */
rtk_api_ret_t rtk_l2_ipMcastForwardRouterPort_set(rtk_enable_t enabled)
{
    rtk_api_ret_t       retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enabled >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if((retVal = rtl8367c_setAsicLutIpmcFwdRouterPort(enabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastForwardRouterPort_get
 * Description:
 *      Get IPMC packet forward to router port also or not
 * Input:
 *      None.
 * Output:
 *      pEnabled    - 1: Include router port, 0, exclude router port
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - Null pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_l2_ipMcastForwardRouterPort_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t       retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8367c_getAsicLutIpmcFwdRouterPort(pEnabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastGroupEntry_add
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
rtk_api_ret_t rtk_l2_ipMcastGroupEntry_add(ipaddr_t ip_addr, rtk_uint32 vid, rtk_portmask_t *pPortmask)
{
    rtk_uint32      empty_idx = 0xFFFF;
    rtk_int32       index;
    ipaddr_t        group_addr;
    rtk_uint32      group_vid;
    rtk_uint32      pmask;
    rtk_uint32      valid;
    rtk_uint32      physicalPortmask;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if((ip_addr & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    /* Get Physical port mask */
    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &physicalPortmask))!=RT_ERR_OK)
        return retVal;

    for(index = 0; index <= RTL8367C_LUT_IPMCGRP_TABLE_MAX; index++)
    {
        if ((retVal = rtl8367c_getAsicLutIPMCGroup((rtk_uint32)index, &group_addr, &group_vid, &pmask, &valid))!=RT_ERR_OK)
            return retVal;

        if( (valid == ENABLED) && (group_addr == ip_addr) && (group_vid == vid) )
        {
            if(pmask != physicalPortmask)
            {
                pmask = physicalPortmask;
                if ((retVal = rtl8367c_setAsicLutIPMCGroup(index, ip_addr, vid, pmask, valid))!=RT_ERR_OK)
                    return retVal;
            }

            return RT_ERR_OK;
        }

        if( (valid == DISABLED) && (empty_idx == 0xFFFF) ) /* Unused */
            empty_idx = (rtk_uint32)index;
    }

    if(empty_idx == 0xFFFF)
        return RT_ERR_TBL_FULL;

    pmask = physicalPortmask;
    if ((retVal = rtl8367c_setAsicLutIPMCGroup(empty_idx, ip_addr, vid, pmask, ENABLED))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_l2_ipMcastGroupEntry_del
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
rtk_api_ret_t rtk_l2_ipMcastGroupEntry_del(ipaddr_t ip_addr, rtk_uint32 vid)
{
    rtk_int32       index;
    ipaddr_t        group_addr;
    rtk_uint32      group_vid;
    rtk_uint32      pmask;
    rtk_uint32      valid;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if((ip_addr & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    for(index = 0; index <= RTL8367C_LUT_IPMCGRP_TABLE_MAX; index++)
    {
        if ((retVal = rtl8367c_getAsicLutIPMCGroup((rtk_uint32)index, &group_addr, &group_vid, &pmask, &valid))!=RT_ERR_OK)
            return retVal;

        if( (valid == ENABLED) && (group_addr == ip_addr) && (group_vid == vid) )
        {
            group_addr = 0xE0000000;
            group_vid = 0;
            pmask = 0;
            if ((retVal = rtl8367c_setAsicLutIPMCGroup(index, group_addr, group_vid, pmask, DISABLED))!=RT_ERR_OK)
                return retVal;

            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_ipMcastGroupEntry_get
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
rtk_api_ret_t rtk_l2_ipMcastGroupEntry_get(ipaddr_t ip_addr, rtk_uint32 vid, rtk_portmask_t *pPortmask)
{
    rtk_int32       index;
    ipaddr_t        group_addr;
    rtk_uint32      group_vid;
    rtk_uint32      valid;
    rtk_uint32      pmask;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if((ip_addr & 0xF0000000) != 0xE0000000)
        return RT_ERR_INPUT;

    if (vid > RTL8367C_VIDMAX)
        return RT_ERR_L2_VID;

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    for(index = 0; index <= RTL8367C_LUT_IPMCGRP_TABLE_MAX; index++)
    {
        if ((retVal = rtl8367c_getAsicLutIPMCGroup((rtk_uint32)index, &group_addr, &group_vid, &pmask, &valid))!=RT_ERR_OK)
            return retVal;

        if( (valid == ENABLED) && (group_addr == ip_addr) && (group_vid == vid) )
        {
            if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask))!=RT_ERR_OK)
                return retVal;

            return RT_ERR_OK;
        }
    }

    return RT_ERR_FAILED;
}

/* Function Name:
 *      rtk_l2_entry_get
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
rtk_api_ret_t rtk_l2_entry_get(rtk_l2_addr_table_t *pL2_entry)
{
    rtk_api_ret_t retVal;
    rtk_uint32 method;
    rtl8367c_luttb l2Table;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (pL2_entry->index >= rtk_switch_maxLutAddrNumber_get())
        return RT_ERR_INPUT;

    memset(&l2Table, 0x00, sizeof(rtl8367c_luttb));
    l2Table.address= pL2_entry->index;
    method = LUTREADMETHOD_ADDRESS;
    if ((retVal = rtl8367c_getAsicL2LookupTb(method, &l2Table)) != RT_ERR_OK)
        return retVal;

    if ((pL2_entry->index>0x800)&&(l2Table.lookup_hit==0))
         return RT_ERR_L2_EMPTY_ENTRY;

    if(l2Table.l3lookup)
    {
        if(l2Table.l3vidlookup)
        {
            memset(&pL2_entry->mac, 0, sizeof(rtk_mac_t));
            pL2_entry->is_ipmul  = l2Table.l3lookup;
            pL2_entry->sip       = l2Table.sip;
            pL2_entry->dip       = l2Table.dip;
            pL2_entry->is_static = l2Table.nosalearn;

            /* Get Logical port mask */
            if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
                return retVal;

            pL2_entry->fid       = 0;
            pL2_entry->age       = 0;
            pL2_entry->auth      = 0;
            pL2_entry->sa_block  = 0;
            pL2_entry->is_ipvidmul = 1;
            pL2_entry->l3_vid      = l2Table.l3_vid;
        }
        else
        {
            memset(&pL2_entry->mac, 0, sizeof(rtk_mac_t));
            pL2_entry->is_ipmul  = l2Table.l3lookup;
            pL2_entry->sip       = l2Table.sip;
            pL2_entry->dip       = l2Table.dip;
            pL2_entry->is_static = l2Table.nosalearn;

            /* Get Logical port mask */
            if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
                return retVal;

            pL2_entry->fid       = 0;
            pL2_entry->age       = 0;
            pL2_entry->auth      = 0;
            pL2_entry->sa_block  = 0;
            pL2_entry->is_ipvidmul = 0;
            pL2_entry->l3_vid      = 0;
        }
    }
    else if(l2Table.mac.octet[0]&0x01)
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->is_static = l2Table.nosalearn;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(l2Table.mbr, &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        if(l2Table.ivl_svl == 1) /* IVL */
        {
            pL2_entry->cvid      = l2Table.cvid_fid;
            pL2_entry->fid       = 0;
        }
        else /* SVL*/
        {
            pL2_entry->cvid      = 0;
            pL2_entry->fid       = l2Table.cvid_fid;
        }
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->sa_block  = l2Table.sa_block;
        pL2_entry->age       = 0;
        pL2_entry->is_ipvidmul = 0;
        pL2_entry->l3_vid      = 0;
    }
    else if((l2Table.age != 0)||(l2Table.nosalearn == 1))
    {
        memset(&pL2_entry->sip, 0, sizeof(ipaddr_t));
        memset(&pL2_entry->dip, 0, sizeof(ipaddr_t));
        pL2_entry->mac.octet[0] = l2Table.mac.octet[0];
        pL2_entry->mac.octet[1] = l2Table.mac.octet[1];
        pL2_entry->mac.octet[2] = l2Table.mac.octet[2];
        pL2_entry->mac.octet[3] = l2Table.mac.octet[3];
        pL2_entry->mac.octet[4] = l2Table.mac.octet[4];
        pL2_entry->mac.octet[5] = l2Table.mac.octet[5];
        pL2_entry->is_ipmul  = l2Table.l3lookup;
        pL2_entry->is_static = l2Table.nosalearn;

        /* Get Logical port mask */
        if ((retVal = rtk_switch_portmask_P2L_get(1<<(l2Table.spa), &(pL2_entry->portmask)))!=RT_ERR_OK)
            return retVal;

        pL2_entry->ivl       = l2Table.ivl_svl;
        pL2_entry->cvid      = l2Table.cvid_fid;
        pL2_entry->fid       = l2Table.fid;
        pL2_entry->auth      = l2Table.auth;
        pL2_entry->sa_block  = l2Table.sa_block;
        pL2_entry->age       = l2Table.age;
        pL2_entry->is_ipvidmul = 0;
        pL2_entry->l3_vid      = 0;
    }
    else
       return RT_ERR_L2_EMPTY_ENTRY;

    return RT_ERR_OK;
}


