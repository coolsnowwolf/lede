/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef __UAPI_ITF_POLICY_ROUTE_TABLE_H__
#define __UAPI_ITF_POLICY_ROUTE_TABLE_H__

/**
* \file  itf_policy_route_table.h 
* \brief This file is interface policy route table header file that will be exported for others to use.
* \author Lei.Zhang
* \date     2020-09-24
* \version  A001 
* \copyright EcoNet Inc                                                              
*/

struct ifPolicyRoute
{
    char    *ifName;
    char    *mark;
    char    *tabid;
    char    sk_mark;
};

static const struct ifPolicyRoute ifpr[] = 
{
    /*MAX_ECNT_ETHER_PORT_NUM*/
    /* eth0.1 : 0x(1<<DEV_OFFSET) */
    {"eth0.1",      "0x08000000",       "191",      0x1},
    {"eth0.2",      "0x10000000",       "192",      0x2},
    {"eth0.3",      "0x18000000",       "193",      0x3},
    {"eth0.4",      "0x20000000",       "194",      0x4},
    {"eth0.5",      "0x28000000",       "195",      0x5},
    {"eth0.6",      "0x30000000",       "196",      0x6},
    {"eth0.7",      "0x38000000",       "197",      0x7},
    {"eth0.8",      "0x40000000",       "198",      0x8},
    /*MAX_ECNT_WALN_PORT_NUM*/
    {"ra0",         "0x48000000",       "200",      0x9},
    {"ra1",         "0x50000000",       "201",      0xA},
    {"ra2",         "0x58000000",       "202",      0xB},
    {"ra3",         "0x60000000",       "203",      0xC},
    {"ra4",         "0x68000000",       "204",      0xD},
    {"ra5",         "0x70000000",       "205",      0xE},
    {"ra6",         "0x78000000",       "206",      0xF},
    {"ra7",         "0x80000000",       "207",      0x10},
    /*MAX_ECNT_WALNAC_PORT_NUM*/
    {"rai0",        "0x88000000",       "208",      0x11},
    {"rai1",        "0x90000000",       "209",      0x12},
    {"rai2",        "0x98000000",       "210",      0x13},
    {"rai3",        "0xA0000000",       "211",      0x14},
    {"rai4",        "0xA8000000",       "212",      0x15},
    {"rai5",        "0xB0000000",       "213",      0x16},
    {"rai6",        "0xB8000000",       "214",      0x17},
    {"rai7",        "0xc0000000",       "215",      0x18},
    /*MAX_ECNT_USB_PORT_NUM*/
    {"usb0",        "0xC8000000",       "216",      0x19},
    /*MAX_ECNT_WDS_PORT_NUM*/
    {"wds0",        "0xD0000000",       "217",      0x1A}
};

#endif

