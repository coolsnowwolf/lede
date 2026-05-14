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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Declaration of HSB/HSA
 *
 * Feature : The file have include the following module and sub-modules
 *           1) HSB, HSA get
 *
 */


/*
 * Include Files
 */
#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_hsb.h>
#include <rtl8373_asicdrv.h>


static rtk_uint32 _bit_op(rtk_uint32 *pSmiHsbData, rtk_uint32 highBit, rtk_uint32 lowBit)
{
    rtk_uint32 data = 0;
    rtk_uint32 bit;
    rtk_uint32 i;
    rtk_uint32 cnt = 0;

    for (i = lowBit; i <= highBit; i++)
    {
        bit = ((*(pSmiHsbData + (i/32)) & (0x0001 << (i % 32))) != 0) ? 1 : 0;
        data |= (bit <<(cnt));

        cnt++;
    }

    return data;
}





ret_t rtl8373_aleLatch_set(hsab_latch_t * latchParam)
{
    rtk_uint32 regData;
    ret_t retVal;

    if (NULL == latchParam)
        return RT_ERR_NULL_POINTER;

    regData = (latchParam->latch_always & 1) << RTL8373_ITA_HSAB_CTRL_LATCH_ALWAYS_OFFSET;
    regData |= (latchParam->latch_first & 1) << RTL8373_ITA_HSAB_CTRL_LATCH_FIRST_OFFSET;
    regData |= (latchParam->spa_en & 1) << RTL8373_ITA_HSAB_CTRL_SPA_EN_OFFSET;
    regData |= (latchParam->spa & 0xf) << RTL8373_ITA_HSAB_CTRL_SPA_OFFSET;
    regData |= (latchParam->fwd_en & 1) << RTL8373_ITA_HSAB_CTRL_FORWARD_EN_OFFSET;
    regData |= (latchParam->fwd & 3) << RTL8373_ITA_HSAB_CTRL_FORWARD_OFFSET;
    regData |= (latchParam->reason_en & 1) << RTL8373_ITA_HSAB_CTRL_REASON_EN_OFFSET;
    regData |= (latchParam->reason & 0x3f) << RTL8373_ITA_HSAB_CTRL_REASON_OFFSET;

    retVal = rtl8373_setAsicReg(RTL8373_ITA_HSAB_CTRL_ADDR, regData);

    return retVal;
}


ret_t rtl8373_aleLatch_get(hsab_latch_t * latchParam)
{
    rtk_uint32 regData;
    ret_t retVal;

    if (NULL == latchParam)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicReg(RTL8373_ITA_HSAB_CTRL_ADDR, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    latchParam->latch_always = (regData >> RTL8373_ITA_HSAB_CTRL_LATCH_ALWAYS_OFFSET) & 1;
    latchParam->latch_first = (regData >> RTL8373_ITA_HSAB_CTRL_LATCH_FIRST_OFFSET) & 1;
    latchParam->spa_en = (regData >> RTL8373_ITA_HSAB_CTRL_SPA_EN_OFFSET) & 1;
    latchParam->spa = (regData >> RTL8373_ITA_HSAB_CTRL_SPA_OFFSET) & 0xf;
    latchParam->fwd_en = (regData >> RTL8373_ITA_HSAB_CTRL_FORWARD_EN_OFFSET) & 1;
    latchParam->fwd = (regData >> RTL8373_ITA_HSAB_CTRL_FORWARD_OFFSET) & 0x3;
    latchParam->reason_en = (regData >> RTL8373_ITA_HSAB_CTRL_REASON_EN_OFFSET) & 1;
    latchParam->reason = (regData >> RTL8373_ITA_HSAB_CTRL_REASON_OFFSET) & 0x3f;

    return RT_ERR_OK;


}



ret_t rtl8373_aleHsb_get(rtl8373_hsb_t * pHsb)
{
    rtk_uint32 regData, i;
    ret_t retVal;
    rtk_uint32 * tableAddr;
    rtk_uint32 smiHsbData[20];

    tableAddr = (rtk_uint32 *)smiHsbData;
//read the first 5 32bits
    regData = 1 | (0 << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET) | (7 << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET) | (0 << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET);
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    for(i=0; i<5; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

//read the second 5 32bits

    regData = 1 | (0 << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET) | (7 << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET) | (1 << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET);
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    for(i=0; i<5; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

//read the third 5 32bits

    regData = 1 | (0 << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET) | (7 << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET) | (2 << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET);
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    for(i=0; i<5; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

//read the forth 5 32bits

    regData = 1 | (0 << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET) | (7 << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET) | (3 << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET);
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    for(i=0; i<5; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

    pHsb->pktlen        = _bit_op(smiHsbData, 13, 0);
    pHsb->dmac[0]         = _bit_op(smiHsbData, 61, 54);
    pHsb->dmac[1]         = _bit_op(smiHsbData, 53, 46);
    pHsb->dmac[2]         = _bit_op(smiHsbData, 45, 38);
    pHsb->dmac[3]         = _bit_op(smiHsbData, 37, 30);
    pHsb->dmac[4]         = _bit_op(smiHsbData, 29, 22);
    pHsb->dmac[5]         = _bit_op(smiHsbData, 21, 14);
    pHsb->smac[0]         = _bit_op(smiHsbData, 109, 102);
    pHsb->smac[1]         = _bit_op(smiHsbData, 101, 94);
    pHsb->smac[2]         = _bit_op(smiHsbData, 93, 86);
    pHsb->smac[3]         = _bit_op(smiHsbData, 85, 78);
    pHsb->smac[4]         = _bit_op(smiHsbData, 77, 70);
    pHsb->smac[5]         = _bit_op(smiHsbData, 69, 62);
    pHsb->cputag_if     = _bit_op(smiHsbData, 110, 110);
    pHsb->cputag        = _bit_op(smiHsbData, 142, 111);
    pHsb->stag_if       = _bit_op(smiHsbData, 143, 143);
    pHsb->stag          = _bit_op(smiHsbData, 159, 144);
    pHsb->ctag_if       = _bit_op(smiHsbData, 160, 160);
    pHsb->ctag          = _bit_op(smiHsbData, 176, 161);
    pHsb->rtag_if        = _bit_op(smiHsbData, 177, 177);
    pHsb->ethertype     = _bit_op(smiHsbData, 193, 178);
    pHsb->snap           = _bit_op(smiHsbData, 194, 194);
    pHsb->pppoe          = _bit_op(smiHsbData, 195, 195);
    pHsb->rrcp          = _bit_op(smiHsbData, 196, 196);
    pHsb->rldp           = _bit_op(smiHsbData, 197, 197);
    pHsb->rlpp            = _bit_op(smiHsbData, 198, 198);
    pHsb->oam              = _bit_op(smiHsbData, 199, 199);
    pHsb->arp             = _bit_op(smiHsbData, 200, 200);
    pHsb->ip_type       = _bit_op(smiHsbData, 202, 201);
    pHsb->tcp           = _bit_op(smiHsbData, 203, 203);
    pHsb->udp               = _bit_op(smiHsbData, 204, 204);
    pHsb->igmp             = _bit_op(smiHsbData, 205, 205);
    pHsb->icmp              = _bit_op(smiHsbData, 206, 206);
    pHsb->dip           = _bit_op(smiHsbData, 238, 207);
    pHsb->sip           = _bit_op(smiHsbData, 270, 239);
    pHsb->l4_dport       = _bit_op(smiHsbData, 286, 271);
    pHsb->l4_sport       = _bit_op(smiHsbData, 302, 287);
    pHsb->tos           = _bit_op(smiHsbData, 318, 303);
    pHsb->usr0_valid    = _bit_op(smiHsbData, 319, 319);
    pHsb->usr1_valid    = _bit_op(smiHsbData, 320, 320);
    pHsb->usr2_valid    = _bit_op(smiHsbData, 321, 321);
    pHsb->usr3_valid    = _bit_op(smiHsbData, 322, 322);
    pHsb->usr4_valid    = _bit_op(smiHsbData, 323, 323);
    pHsb->usr5_valid    = _bit_op(smiHsbData, 324, 324);
    pHsb->usr6_valid    = _bit_op(smiHsbData, 325, 325);
    pHsb->usr7_valid    = _bit_op(smiHsbData, 326, 326);
    pHsb->usr8_valid    = _bit_op(smiHsbData, 327, 327);
    pHsb->usr9_valid    = _bit_op(smiHsbData, 328, 328);
    pHsb->usr10_valid   = _bit_op(smiHsbData, 329, 329);
    pHsb->usr11_valid   = _bit_op(smiHsbData, 330, 330);
    pHsb->usr12_valid   = _bit_op(smiHsbData, 331, 331);
    pHsb->usr13_valid   = _bit_op(smiHsbData, 332, 332);
    pHsb->usr14_valid   = _bit_op(smiHsbData, 333, 333);
    pHsb->usr15_valid   = _bit_op(smiHsbData, 334, 334);
    pHsb->usr0_field    = _bit_op(smiHsbData, 350, 335);
    pHsb->usr1_field    = _bit_op(smiHsbData, 366, 351);
    pHsb->usr2_field    = _bit_op(smiHsbData, 382, 367);
    pHsb->usr3_field    = _bit_op(smiHsbData, 398, 383);
    pHsb->usr4_field    = _bit_op(smiHsbData, 414, 399);
    pHsb->usr5_field    = _bit_op(smiHsbData, 430, 415);
    pHsb->usr6_field    = _bit_op(smiHsbData, 446, 431);
    pHsb->usr7_field    = _bit_op(smiHsbData, 462, 447);
    pHsb->usr8_field    = _bit_op(smiHsbData, 478, 463);
    pHsb->usr9_field    = _bit_op(smiHsbData, 494, 479);
    pHsb->usr10_field   = _bit_op(smiHsbData, 510, 495);
    pHsb->usr11_field   = _bit_op(smiHsbData, 526, 511);
    pHsb->usr12_field   = _bit_op(smiHsbData, 542, 527);
    pHsb->usr13_field   = _bit_op(smiHsbData, 558, 543);
    pHsb->usr14_field   = _bit_op(smiHsbData, 574, 559);
    pHsb->usr15_field   = _bit_op(smiHsbData, 590, 575);
    pHsb->spa           = _bit_op(smiHsbData, 594, 591);
    pHsb->wol            = _bit_op(smiHsbData, 595, 595);
    pHsb->l2ptp            = _bit_op(smiHsbData, 596, 596);
    pHsb->udpptp        = _bit_op(smiHsbData, 597, 597);
    pHsb->ingr_err        = _bit_op(smiHsbData, 598, 598);
    pHsb->errpkt        = _bit_op(smiHsbData, 599, 599);
    pHsb->l3err            = _bit_op(smiHsbData, 600, 600);
    pHsb->begin_dsc        = _bit_op(smiHsbData, 612, 601);
    pHsb->end_dsc       = _bit_op(smiHsbData, 624, 613);

    return RT_ERR_OK;

}


ret_t rtl8373_aleHsb_display(rtl8373_hsb_t * pHsb)
{
    const char *iptypeString[4] = {"not IP", "IPv4", "Ipv6", "Ipv6 Multicast"};


    PRINT("---------------HSB----------------\n\n");
    PRINT("pktlen: %d SPA: %d\n", pHsb->pktlen, pHsb->spa);
    PRINT("DA: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", pHsb->dmac[0], pHsb->dmac[1], pHsb->dmac[2], pHsb->dmac[3], pHsb->dmac[4], pHsb->dmac[5]);
    PRINT("SA: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\n", pHsb->smac[0], pHsb->smac[1], pHsb->smac[2], pHsb->smac[3], pHsb->smac[4], pHsb->smac[5]);

    PRINT("CPUTAG_IF: %d ",pHsb->cputag_if);
    if(pHsb->cputag_if)
    {
        PRINT("[%2.2X %2.2X %2.2X %2.2X]\n",((pHsb->cputag >> 24) & 0xFF), ((pHsb->cputag >> 16) & 0xFF), ((pHsb->cputag >> 8) & 0xFF), (pHsb->cputag & 0xFF));
    }
    else
        PRINT("\n");

    PRINT("STAG_IF: %d ",pHsb->stag_if);
    if(pHsb->stag_if)
    {
        PRINT("[Pri: %d VID: %d]\n",(pHsb->stag >> 13), pHsb->stag & 0xFFF);
    }
    else
        PRINT("\n");

    PRINT("CTAG_IF: %d ",pHsb->ctag_if);
    if(pHsb->ctag_if)
    {
        PRINT("[Pri: %d VID: %d]\n",(pHsb->ctag >>13), pHsb->ctag & 0xFFF);
    }
    else
        PRINT("\n");

    PRINT("RSPAN_IF: %d", pHsb->rtag_if);
    if(pHsb->rtag_if)
    {
        PRINT("[Pri: %d]\n",(pHsb->stag >> 13));
    }
    else
        PRINT("\n");

    PRINT("EtherType: %x\n", pHsb->ethertype);
    PRINT("SNAP PPPOE RRCP RLDP RLPP OAM ARP TCP UDP ICMP IGMP IP\n");
    PRINT("%-4d %-5d %-4d %-4d %-4d %-3d %-3d %-3d %-3d %-4d %-4d %s\n\n",
        pHsb->snap,
        pHsb->pppoe,
        pHsb->rrcp,
        pHsb->rldp,
        pHsb->rlpp,
        pHsb->oam,
        pHsb->arp,
        pHsb->tcp,
        pHsb->udp,
        pHsb->icmp,
        pHsb->igmp,
        iptypeString[pHsb->ip_type]);

    PRINT("SIP: 0x%x.%x.%x.%x ", ((pHsb->sip >> 24) & 0xFF), ((pHsb->sip >> 16) & 0xFF), ((pHsb->sip >> 8) & 0xFF), (pHsb->sip & 0xFF));
    PRINT("( %d.%d.%d.%d )", ((pHsb->sip >> 24) & 0xFF), ((pHsb->sip >> 16) & 0xFF), ((pHsb->sip >> 8) & 0xFF), (pHsb->sip & 0xFF));
    PRINT("DIP: 0x%x.%x.%x.%x ", ((pHsb->dip >> 24) & 0xFF), ((pHsb->dip >> 16) & 0xFF), ((pHsb->dip >> 8) & 0xFF), (pHsb->dip & 0xFF));
    PRINT("( %d.%d.%d.%d )\n", ((pHsb->dip >> 24) & 0xFF), ((pHsb->dip >> 16) & 0xFF), ((pHsb->dip >> 8) & 0xFF), (pHsb->dip & 0xFF));
    PRINT("TOS: 0x%x ",pHsb->tos);
    PRINT("L4 SPORT: 0x%x ",pHsb->l4_sport);
    PRINT("L4 DPORT: 0x%x \n",pHsb->l4_dport);


    PRINT("User:   0   1   2   3   4   5   6   7   8   9   10  11   12   13   14   15\n");
    PRINT("        %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-3d %-4d %-4d %-4d %-4d %-4d %-4d\n",
        pHsb->usr0_valid,pHsb->usr1_valid,pHsb->usr2_valid,pHsb->usr3_valid,pHsb->usr4_valid,pHsb->usr5_valid,
        pHsb->usr6_valid,pHsb->usr7_valid,pHsb->usr8_valid,pHsb->usr9_valid,pHsb->usr10_valid,pHsb->usr11_valid,
        pHsb->usr12_valid,pHsb->usr13_valid,pHsb->usr14_valid,pHsb->usr15_valid);

    PRINT("     0x%4.4X  ", pHsb->usr0_field);
    PRINT("0x%4.4X  ", pHsb->usr1_field);
    PRINT("0x%4.4X  ", pHsb->usr2_field);
    PRINT("0x%4.4X  ", pHsb->usr3_field);
    PRINT("0x%4.4X  ", pHsb->usr4_field);
    PRINT("0x%4.4X  ", pHsb->usr5_field);
    PRINT("0x%4.4X  ", pHsb->usr6_field);
    PRINT("0x%4.4X\n", pHsb->usr7_field);
    PRINT("     0x%4.4X  ", pHsb->usr8_field);
    PRINT("0x%4.4X  ", pHsb->usr9_field);
    PRINT("0x%4.4X  ", pHsb->usr10_field);
    PRINT("0x%4.4X  ", pHsb->usr11_field);
    PRINT("0x%4.4X  ", pHsb->usr12_field);
    PRINT("0x%4.4X  ", pHsb->usr13_field);
    PRINT("0x%4.4X  ", pHsb->usr14_field);
    PRINT("0x%4.4X\n", pHsb->usr15_field);

    PRINT("WOL L2PTP UDPPTP  INGRERR ERRPKT L3ERR\n");
    PRINT("%-3d %-5d %-6d %-7d %-6d %-5d\n\n",
        pHsb->wol,
        pHsb->l2ptp,
        pHsb->udpptp,
        pHsb->ingr_err,
        pHsb->errpkt,
        pHsb->l3err);

    PRINT("Begin_DSC: %d,   End_DSC:%d\n", pHsb->begin_dsc, pHsb->end_dsc);

    return RT_ERR_OK;

}



#if 0
ret_t rtl8373_dbg_asicHsb_get(rtl8373_hsb_t * pHsb)
{
    rtk_uint32 regAddr, regData, i;
    ret_t retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_HSB_CTRL_ADDR, RTL8373_HSB_CTRL_READHSB_OFFSET,1);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_HSB_CTRL_ADDR, RTL8373_HSB_CTRL_READHSB_OFFSET,&regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    i = 0;
    while(regData & 1)
    {
        
        retVal = rtl8373_getAsicRegBit(RTL8373_HSB_CTRL_ADDR, RTL8373_HSB_CTRL_READHSB_OFFSET,&regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        i++;
        if(i>20)
            return RT_ERR_BUSYWAIT_TIMEOUT;        
    }

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA0_ADDR, &regData);
    pHsb->pktlen = regData & RTL8373_HSB_DATA0_PKE_LEN_MASK;
    pHsb->dmac[0] = (regData >> RTL8373_HSB_DATA0_DMAC17_0_OFFSET)& 0xff;
    pHsb->dmac[1] = (regData >> (RTL8373_HSB_DATA0_DMAC17_0_OFFSET + 8))& 0xff;
    pHsb->dmac[2] = (regData >> (RTL8373_HSB_DATA0_DMAC17_0_OFFSET + 16))& 0x3;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA1_ADDR, &regData);
    pHsb->dmac[2] |= (regData & 0x3f) << 2;
    pHsb->dmac[3] = (regData >> 6) & 0xff;
    pHsb->dmac[4] = (regData >> 14) & 0xff;
    pHsb->dmac[5] = (regData >> 22) & 0xff;
    pHsb->smac[0] = (regData >> 30) & 0x3;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA2_ADDR, &regData);
    pHsb->smac[0] |= (regData & 0x3f ) << 2;
    pHsb->smac[1] = (regData >> 6) & 0xff;
    pHsb->smac[2] = (regData >> 14) & 0xff;
    pHsb->smac[3] = (regData >> 22) & 0xff;
    pHsb->smac[4] = (regData >> 30) & 0x3;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA3_ADDR, &regData);
    pHsb->smac[4] |= (regData & 0x3f ) << 2;
    pHsb->smac[5] = (regData >> 6) & 0xff;
    pHsb->cputag_if = (regData >> RTL8373_HSB_DATA3_CPUTAG_IF_OFFSET) & 1;
    pHsb->cputag = (regData >> RTL8373_HSB_DATA3_CPUTAG16_0_OFFSET) & 0x1ffff;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA4_ADDR, &regData);
    pHsb->cputag |= (regData & RTL8373_HSB_DATA4_CPUTAG31_17_MASK) << 17;
    pHsb->stag_if = (regData >> RTL8373_HSB_DATA4_STAG_IF_OFFSET) & 1;
    pHsb->stag = (regData >> RTL8373_HSB_DATA4_STAG_OFFSET) & 0xff;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA5_ADDR, &regData);
    pHsb->ctag_if = (regData >> RTL8373_HSB_DATA5_CTAG_IF_OFFSET) & 1;
    pHsb->ctag = (regData >> RTL8373_HSB_DATA5_CTAG_OFFSET) & 0xffff;
    pHsb->rtag_if = (regData >> RTL8373_HSB_DATA5_RTAG_IF_OFFSET) & 0x1;
    pHsb->ethertype = (regData >> RTL8373_HSB_DATA5_ETYPE13_0_OFFSET) & 0x3fff;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA6_ADDR, &regData);
    pHsb->ethertype |= (regData & RTL8373_HSB_DATA6_ETYPE15_14_MASK) << 14;
    pHsb->snap = (regData >> RTL8373_HSB_DATA6_SNAP_OFFSET) & 1;
    pHsb->pppoe = (regData >> RTL8373_HSB_DATA6_PPPOE_OFFSET) & 1;
    pHsb->rrcp = (regData >> RTL8373_HSB_DATA6_RRCP_OFFSET) & 1;
    pHsb->rldp = (regData >> RTL8373_HSB_DATA6_RLDP_OFFSET) & 1;
    pHsb->rlpp = (regData >> RTL8373_HSB_DATA6_RLPP_OFFSET) & 1;
    pHsb->oam = (regData >> RTL8373_HSB_DATA6_OAM_OFFSET) & 1;
    pHsb->arp = (regData >> RTL8373_HSB_DATA6_ARP_OFFSET) & 1;
    pHsb->ip_type= (regData >> RTL8373_HSB_DATA6_IPTYPE_OFFSET) & 3;
    pHsb->tcp = (regData >> RTL8373_HSB_DATA6_TCP_OFFSET) & 1;
    pHsb->udp = (regData >> RTL8373_HSB_DATA6_UDP_OFFSET) & 1;
    pHsb->icmp = (regData >> RTL8373_HSB_DATA6_ICMP_OFFSET) & 1;
    pHsb->igmp = (regData >> RTL8373_HSB_DATA6_IGMP_OFFSET) & 1;
    pHsb->dip = (regData >> RTL8373_HSB_DATA6_DIP16_0_OFFSET) & 0x1ffff;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA7_ADDR, &regData);
    pHsb->dip |= (regData & RTL8373_HSB_DATA7_DIP31_17_MASK) << 17;
    pHsb->sip = (regData >> RTL8373_HSB_DATA7_SIP16_0_OFFSET) & 0x1ffff;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA8_ADDR, &regData);
    pHsb->sip |= (regData & RTL8373_HSB_DATA8_SIP31_17_MASK) << 17;
    pHsb->l4_dport = (regData >> RTL8373_HSB_DATA8_L4DPORT_OFFSET)& 0xffff;
    pHsb->l4_sport = (regData >> RTL8373_HSB_DATA8_L4SPORT0_OFFSET)& 0xffff;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA9_ADDR, &regData);
    pHsb->l4_sport |= (regData & RTL8373_HSB_DATA9_L4SPORT15_1_MASK) << 1;
    pHsb->tos = (regData >> RTL8373_HSB_DATA9_TOS_OFFSET)& 0xffff;
    pHsb->usr0_valid = (regData >> RTL8373_HSB_DATA9_UDV0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA10_ADDR, &regData);
    pHsb->usr1_valid = (regData >> RTL8373_HSB_DATA10_UDV1_OFFSET) & 1;
    pHsb->usr2_valid = (regData >> RTL8373_HSB_DATA10_UDV2_OFFSET) & 1;
    pHsb->usr3_valid = (regData >> RTL8373_HSB_DATA10_UDV3_OFFSET) & 1;
    pHsb->usr4_valid = (regData >> RTL8373_HSB_DATA10_UDV4_OFFSET) & 1;
    pHsb->usr5_valid = (regData >> RTL8373_HSB_DATA10_UDV5_OFFSET) & 1;
    pHsb->usr6_valid = (regData >> RTL8373_HSB_DATA10_UDV6_OFFSET) & 1;
    pHsb->usr7_valid = (regData >> RTL8373_HSB_DATA10_UDV7_OFFSET) & 1;
    pHsb->usr8_valid = (regData >> RTL8373_HSB_DATA10_UDV8_OFFSET) & 1;
    pHsb->usr9_valid = (regData >> RTL8373_HSB_DATA10_UDV9_OFFSET) & 1;
    pHsb->usr10_valid = (regData >> RTL8373_HSB_DATA10_UDV10_OFFSET) & 1;
    pHsb->usr11_valid = (regData >> RTL8373_HSB_DATA10_UDV11_OFFSET) & 1;
    pHsb->usr12_valid = (regData >> RTL8373_HSB_DATA10_UDV12_OFFSET) & 1;
    pHsb->usr13_valid = (regData >> RTL8373_HSB_DATA10_UDV13_OFFSET) & 1;
    pHsb->usr14_valid = (regData >> RTL8373_HSB_DATA10_UDV14_OFFSET) & 1;
    pHsb->usr15_valid = (regData >> RTL8373_HSB_DATA10_UDV15_OFFSET) & 1;
    pHsb->usr0_field = (regData >> RTL8373_HSB_DATA10_UDF0_OFFSET) & 0xffff;
    pHsb->usr1_field = (regData >> RTL8373_HSB_DATA10_UDF1_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA11_ADDR, &regData);
    pHsb->usr1_field |= (regData & RTL8373_HSB_DATA11_UDF1_15_1_MASK) << 1;
    pHsb->usr2_field = (regData >> RTL8373_HSB_DATA11_UDF2_OFFSET) & 0xffff;
    pHsb->usr3_field = (regData >> RTL8373_HSB_DATA11_UDF3_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA12_ADDR, &regData);
    pHsb->usr3_field |= (regData & RTL8373_HSB_DATA12_UDF3_15_1_MASK) << 1;
    pHsb->usr4_field = (regData >> RTL8373_HSB_DATA12_UDF4_OFFSET) & 0xffff;
    pHsb->usr5_field = (regData >> RTL8373_HSB_DATA12_UDF5_0_0_OFFSET) & 1;
    
    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA13_ADDR, &regData);
    pHsb->usr5_field |= (regData & RTL8373_HSB_DATA13_UDF5_15_1_MASK) << 1;
    pHsb->usr6_field = (regData >> RTL8373_HSB_DATA13_UDF6_OFFSET) & 0xffff;
    pHsb->usr7_field = (regData >> RTL8373_HSB_DATA13_UDF7_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA14_ADDR, &regData);
    pHsb->usr7_field |= (regData & RTL8373_HSB_DATA14_UDF7_15_1_MASK) << 1;
    pHsb->usr8_field = (regData >> RTL8373_HSB_DATA14_UDF8_OFFSET) & 0xffff;
    pHsb->usr9_field = (regData >> RTL8373_HSB_DATA14_UDF9_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA15_ADDR, &regData);
    pHsb->usr9_field |= (regData & RTL8373_HSB_DATA15_UDF9_15_1_MASK) << 1;
    pHsb->usr10_field = (regData >> RTL8373_HSB_DATA15_UDF10_OFFSET) & 0xffff;
    pHsb->usr11_field = (regData >> RTL8373_HSB_DATA15_UDF11_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA16_ADDR, &regData);
    pHsb->usr11_field |= (regData & RTL8373_HSB_DATA16_UDF11_15_1_MASK) << 1;
    pHsb->usr12_field = (regData >> RTL8373_HSB_DATA16_UDF12_OFFSET) & 0xffff;
    pHsb->usr13_field = (regData >> RTL8373_HSB_DATA16_UDF13_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA17_ADDR, &regData);
    pHsb->usr13_field |= (regData & RTL8373_HSB_DATA17_UDF13_15_1_MASK) << 1;
    pHsb->usr14_field = (regData >> RTL8373_HSB_DATA17_UDF14_OFFSET) & 0xffff;
    pHsb->usr15_field = (regData >> RTL8373_HSB_DATA17_UDF15_0_0_OFFSET) & 1;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA18_ADDR, &regData);
    pHsb->usr15_field |= (regData & RTL8373_HSB_DATA18_UDF15_15_1_MASK) << 1;
    pHsb->spa = (regData >> RTL8373_HSB_DATA18_RXPORT_OFFSET) & 0xF;
    pHsb->wol = (regData >> RTL8373_HSB_DATA18_WOL_OFFSET) & 1;
    pHsb->l2ptp= (regData >> RTL8373_HSB_DATA18_L2PTP_OFFSET) & 1;
    pHsb->udpptp = (regData >> RTL8373_HSB_DATA18_UDPPTP_OFFSET) & 1;
    pHsb->ingr_err= (regData >> RTL8373_HSB_DATA18_INGR_ERR_OFFSET) & 1;
    pHsb->errpkt= (regData >> RTL8373_HSB_DATA18_ERRPKT_OFFSET) & 1;
    pHsb->l3err= (regData >> RTL8373_HSB_DATA18_L3ERR_OFFSET) & 1;
    pHsb->begin_dsc= (regData >> RTL8373_HSB_DATA18_BDSC6_0_OFFSET) & 0x7f;

    retVal = rtl8373_getAsicReg(RTL8373_HSB_DATA19_ADDR, &regData);
    pHsb->begin_dsc |= (regData & RTL8373_HSB_DATA19_BDSC11_7_MASK) << 7;
    pHsb->end_dsc = (regData >> RTL8373_HSB_DATA19_EDSC_OFFSET) & 0xfff;
    
    return RT_ERR_OK;
}

#endif

ret_t rtl8373_aleHsa_get(rtl8373_hsa_t * pHsa)
{
    rtk_uint32 regData, i;
    ret_t retVal;
    rtk_uint32 * tableAddr;
    rtk_uint32 smiHsaData[20];

    tableAddr = (rtk_uint32 *)smiHsaData;
//read the first 5 32bits
    regData = 1 | (0 << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET) | (6 << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET) | (0 << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET);
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    for(i=0; i<5; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }

//read the second 5 32bits

    regData = 1 | (0 << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET) | (6 << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET) | (1 << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET);
    retVal = rtl8373_setAsicReg(RTL8373_ITA_CTRL0_ADDR, regData);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    for(i=0; i<5; i++)
    {
        retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *tableAddr = regData;
        tableAddr++;
    }



    pHsa->pktlen        = _bit_op(smiHsaData, 151, 138);
    pHsa->dpm0            = _bit_op(smiHsaData, 156, 155);
    pHsa->dpm1            = _bit_op(smiHsaData, 158, 157);
    pHsa->dpm2            = _bit_op(smiHsaData, 160, 159);
    pHsa->dpm3            = _bit_op(smiHsaData, 162, 161);
    pHsa->dpm4            = _bit_op(smiHsaData, 164, 163);
    pHsa->dpm5            = _bit_op(smiHsaData, 166, 165);
    pHsa->dpm6            = _bit_op(smiHsaData, 168, 167);
    pHsa->dpm7            = _bit_op(smiHsaData, 170, 169);
    pHsa->dpm8            = _bit_op(smiHsaData, 172, 171);
    pHsa->dpm9            = _bit_op(smiHsaData, 174, 173);
    pHsa->trappkt        = _bit_op(smiHsaData, 90, 90);
    pHsa->rfc1024        = _bit_op(smiHsaData, 74, 74);
    //pHsa->llcother_if    = _bit_op(smiHsaData, 128, 128);
    pHsa->pppoe_if        = _bit_op(smiHsaData, 73, 73);
    pHsa->l3fmt            = _bit_op(smiHsaData, 121, 120);
    pHsa->acl_dscprmk_en= _bit_op(smiHsaData, 67, 67);
    pHsa->dscp_1p        = _bit_op(smiHsaData, 66, 61);
    pHsa->acl_1prmk_en    = _bit_op(smiHsaData, 68, 68);
    pHsa->internal_pri    = _bit_op(smiHsaData, 231, 229);
    pHsa->user_pri0        = _bit_op(smiHsaData, 20, 18);
    pHsa->user_pri1        = _bit_op(smiHsaData, 23, 21);
    pHsa->user_pri2        = _bit_op(smiHsaData, 26, 24);
    pHsa->user_pri3        = _bit_op(smiHsaData, 29, 27);
    pHsa->user_pri4        = _bit_op(smiHsaData, 32, 30);
    pHsa->user_pri5        = _bit_op(smiHsaData, 35, 33);
    pHsa->user_pri6        = _bit_op(smiHsaData, 38, 36);
    pHsa->user_pri7        = _bit_op(smiHsaData, 41, 39);
    pHsa->user_pri8        = _bit_op(smiHsaData, 44, 42);
    pHsa->user_pri9        = _bit_op(smiHsaData, 47, 45);
    //pHsa->ingress_cpri    = _bit_op(smiHsaData, 20, 18);
    pHsa->cvid_en        = _bit_op(smiHsaData, 60, 60);
    pHsa->ctag_if        = _bit_op(smiHsaData, 5, 5);
    pHsa->cvid_if        = _bit_op(smiHsaData, 71, 71);
    pHsa->cvid            = _bit_op(smiHsaData, 59, 48);
    pHsa->cvlan_untagset= _bit_op(smiHsaData, 15, 6);
    //pHsa->cvid_zero        = _bit_op(smiHsaData, 97, 97);
    pHsa->cfmt_en        = _bit_op(smiHsaData, 119, 119);
    pHsa->cfmt            = _bit_op(smiHsaData, 118, 117);
    pHsa->ctag_act        = _bit_op(smiHsaData, 17, 16);
    pHsa->ingress_ctag    = _bit_op(smiHsaData, 137, 122);
    pHsa->stag_if        = _bit_op(smiHsaData, 72, 72);
    pHsa->svid            = _bit_op(smiHsaData, 89, 78);
    pHsa->spri            = _bit_op(smiHsaData, 77, 75);
    pHsa->svlan_untag    = _bit_op(smiHsaData, 116, 107);
    pHsa->ingress_stag    = _bit_op(smiHsaData, 106, 91);
    pHsa->keep            = _bit_op(smiHsaData, 69, 69);
    pHsa->cputag_if        = _bit_op(smiHsaData, 70, 70);
    pHsa->qid0            = _bit_op(smiHsaData, 189, 187);
    pHsa->qid1            = _bit_op(smiHsaData, 192, 190);
    pHsa->qid2            = _bit_op(smiHsaData, 195, 193);
    pHsa->qid3            = _bit_op(smiHsaData, 198, 196);
    pHsa->qid4            = _bit_op(smiHsaData, 201, 199);
    pHsa->qid5            = _bit_op(smiHsaData, 204, 202);
    pHsa->qid6            = _bit_op(smiHsaData, 207, 205);
    pHsa->qid7            = _bit_op(smiHsaData, 210, 208);
    pHsa->qid8            = _bit_op(smiHsaData, 213, 211);
    pHsa->qid9            = _bit_op(smiHsaData, 216, 214);
    pHsa->rspan            = _bit_op(smiHsaData, 0, 0);
    pHsa->spa            = _bit_op(smiHsaData, 4, 1);
    pHsa->begin_dsc        = _bit_op(smiHsaData, 228, 217);
    pHsa->end_dsc        = _bit_op(smiHsaData, 186, 175);
    pHsa->pg_id            = _bit_op(smiHsaData, 154, 152);

    pHsa->reason        = _bit_op(smiHsaData, 239, 232);
    
    return RT_ERR_OK;

}



ret_t rtl8373_aleHsa_display(rtl8373_hsa_t *hsa)
{
    rtk_uint8 tmp;
    const char *dpmString[4] = {

        "No",
        "Tx",
        "Rx",
        "FWD",
    };

    
    const char *reasonString[RTL8373_REASON_NUMBER] = {
    
            "Fwd",
            "l2 ingress/crcerr",
            "StormControl",
            "hash full",
            "cvlan efilter",
            "--",
            "cvlan ifilter",
            "svlan ifilter",
            "VS_UNTAG",
            "svlan UIFSEG/efilter",
            "port maclimit",
            "sys maclimit",
            "fld brdcast",
            "fld unknmul",
            "fld unknuni",
            "--",
            "unkn uni",
            "unkn l2multi",
            "unkn v4multi",
            "unkn v6multi",
            "newsa",
            "unmatchsa",
            "igmp/mld",
            "port_igmp",
            "igmp chkerr",
            "igmp reportspr",
            "igmp leavespr",
            "igmp tblfull",
            "zero leave",
            "dos",
            "vlan accepttype",
            "rma",
            "rspan",
            "--",
            "ingr bandwith",
            "PFC",
            "PTP",
            "acl unmatch",
            "acl policing",
            "acl trap",
            "hash redir/mir",
            "dot1x ingress",
            "dot1x egress",
            "guset op",
            "stp ingress",
            "stp egress",
            "trunk",
            "port isolation",
            "wol",
            "srcport filter",
            "other RLDP",
            "RRCP",
            "--",
            "RLPP",
            "RLDP",
            "LLDP",
            "cputag force",
            "pmirror iso",
            "--",
            "--",
            "diff speed",
            "egress drop",
            "force",
            "linkdown",
            
        };

    PRINT("---------------HSA----------------\n");
    PRINT("PKTLEN: %d\n", hsa->pktlen);
    PRINT("SPA: %d\n\n",  hsa->spa);

    PRINT("DPM:  0    1    2    3    4    5    6    7    8    9\n");
    PRINT("      %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s %-4s\n",
        dpmString[hsa->dpm0],
        dpmString[hsa->dpm1],
        dpmString[hsa->dpm2],
        dpmString[hsa->dpm3],
        dpmString[hsa->dpm4],
        dpmString[hsa->dpm5],
        dpmString[hsa->dpm6],
        dpmString[hsa->dpm7],
        dpmString[hsa->dpm8],
        dpmString[hsa->dpm9]);

    PRINT("CVLAN: CTAGIF UNTAGSET    ACT CPRI CVID CVIDEN CVIDIF CFMT CFMTEN\n");
    PRINT("       %-6d ",hsa->ctag_if);

    PRINT("%-11d ",hsa->cvlan_untagset);

    PRINT("%-3d ",hsa->ctag_act);

    tmp = (hsa->ingress_ctag >> 13) & 0x7;
    PRINT("%-4d %-4d %-6d %-6d %-4d %d\n",
            tmp,
            hsa->cvid,
            hsa->cvid_en,
            hsa->cvid_if,
            hsa->cfmt,
            hsa->cfmt_en);

    PRINT("SVLAN: IF SVID SPRI IgrStag SvlanUtag\n");
    PRINT("       %-2d %-4d %-4d ",
            hsa->stag_if,
            hsa->svid,
            hsa->spri);

    PRINT("0x%-5x ", hsa->ingress_stag);

    PRINT("%-11d\n",hsa->svlan_untag);

    PRINT("REM_1Q_EN REM_DSCP_EN REM_PRI_DSCP\n");
    PRINT("%-9d %-11d %d\n",
            hsa->acl_1prmk_en,
            hsa->acl_dscprmk_en,
            hsa->dscp_1p);

    PRINT("KEEP REGEN_CRC PPPOE SNAP RSPAN LLCOTHER TRAP CVID_ZERO\n");
    PRINT("%-4d %-9d %-5d %-4d %-5d %-4d\n",
            hsa->keep,
            hsa->cputag_if,
            hsa->pppoe_if,
            hsa->rfc1024,
            hsa->rspan,
            hsa->trappkt);

    PRINT("IntPri: %d\n",hsa->internal_pri);
    PRINT("PORT No.  0  1  2  3  4  5  6  7  8  9\n");
    PRINT("USER PRI: ");

    PRINT("%-3d%-3d%-3d%-3d%-3d%-3d%-3d%-3d%-3d%-3d\n",
            hsa->user_pri0, hsa->user_pri1, hsa->user_pri2, hsa->user_pri3,
            hsa->user_pri4, hsa->user_pri5, hsa->user_pri6, hsa->user_pri7, hsa->user_pri8, hsa->user_pri9);

    PRINT("QID:      ");
    PRINT("%-3d%-3d%-3d%-3d%-3d%-3d%-3d%-3d%-3d%-3d\n\n", hsa->qid0, hsa->qid1, hsa->qid2, hsa->qid3, hsa->qid4, hsa->qid5, hsa->qid6, hsa->qid7, hsa->qid8, hsa->qid9);

    PRINT("PG_ID : %d\n", hsa->pg_id);

    if(hsa->reason & 0x80)
        PRINT("packet is     DROP     !!!!!!!\n");
    else if(hsa->reason & 0x40)
        PRINT("packet is     TRAP     !!!!!!!\n");

    tmp = hsa->reason & 0x3f;
    PRINT("reason is : %s\n", reasonString[tmp]);
    
    PRINT("reason whole value 0x%x\n", hsa->reason);

    return RT_ERR_OK;
}





