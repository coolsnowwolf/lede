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
 * $Date: 2017-03-08 15:13:58 +0800 (¶g¤T, 08 ¤T¤ë 2017) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : ACL related function drivers
 *
 */

#ifndef _RTL8367C_ASICDRV_ACL_H_
#define _RTL8367C_ASICDRV_ACL_H_

#include <rtl8367c_asicdrv.h>

#define RTL8367C_ACLRULENO                  96

#define RTL8367C_ACLRULEMAX                 (RTL8367C_ACLRULENO-1)
#define RTL8367C_ACLRULEFIELDNO             8
#define RTL8367C_ACLTEMPLATENO              5
#define RTL8367C_ACLTYPEMAX                 (RTL8367C_ACLTEMPLATENO-1)

#define RTL8367C_ACLRULETBLEN               9
#define RTL8367C_ACLACTTBLEN                4
#define RTL8367C_ACLRULETBADDR(type, rule)  ((type << 6) | rule)
#define RTL8367C_ACLRULETBADDR2(type, rule) ((type << 5) | (rule + 64))

#define ACL_ACT_CVLAN_ENABLE_MASK           0x1
#define ACL_ACT_SVLAN_ENABLE_MASK           0x2
#define ACL_ACT_PRIORITY_ENABLE_MASK        0x4
#define ACL_ACT_POLICING_ENABLE_MASK        0x8
#define ACL_ACT_FWD_ENABLE_MASK             0x10
#define ACL_ACT_INTGPIO_ENABLE_MASK         0x20

#define RTL8367C_ACLRULETAGBITS             5

#define RTL8367C_ACLRANGENO                 16

#define RTL8367C_ACLRANGEMAX                (RTL8367C_ACLRANGENO-1)

#define RTL8367C_ACL_PORTRANGEMAX           (0xFFFF)
#define RTL8367C_ACL_ACT_TABLE_LEN          (4)

enum ACLTCAMTYPES
{
    CAREBITS= 0,
    DATABITS
};

typedef enum aclFwdAct
{
    RTL8367C_ACL_FWD_MIRROR = 0,
    RTL8367C_ACL_FWD_REDIRECT,
    RTL8367C_ACL_FWD_MIRRORFUNTION,
    RTL8367C_ACL_FWD_TRAP,
} rtl8367c_aclFwd_t;

enum ACLFIELDTYPES
{
    ACL_UNUSED,
    ACL_DMAC0,
    ACL_DMAC1,
    ACL_DMAC2,
    ACL_SMAC0,
    ACL_SMAC1,
    ACL_SMAC2,
    ACL_ETHERTYPE,
    ACL_STAG,
    ACL_CTAG,
    ACL_IP4SIP0 = 0x10,
    ACL_IP4SIP1,
    ACL_IP4DIP0,
    ACL_IP4DIP1,
    ACL_IP6SIP0WITHIPV4 = 0x20,
    ACL_IP6SIP1WITHIPV4,
    ACL_IP6DIP0WITHIPV4 = 0x28,
    ACL_IP6DIP1WITHIPV4,
    ACL_VIDRANGE = 0x30,
    ACL_IPRANGE,
    ACL_PORTRANGE,
    ACL_FIELD_VALID,
    ACL_FIELD_SELECT00 = 0x40,
    ACL_FIELD_SELECT01,
    ACL_FIELD_SELECT02,
    ACL_FIELD_SELECT03,
    ACL_FIELD_SELECT04,
    ACL_FIELD_SELECT05,
    ACL_FIELD_SELECT06,
    ACL_FIELD_SELECT07,
    ACL_FIELD_SELECT08,
    ACL_FIELD_SELECT09,
    ACL_FIELD_SELECT10,
    ACL_FIELD_SELECT11,
    ACL_FIELD_SELECT12,
    ACL_FIELD_SELECT13,
    ACL_FIELD_SELECT14,
    ACL_FIELD_SELECT15,
    ACL_TCPSPORT = 0x80,
    ACL_TCPDPORT,
    ACL_TCPFLAG,
    ACL_UDPSPORT,
    ACL_UDPDPORT,
    ACL_ICMPCODETYPE,
    ACL_IGMPTYPE,
    ACL_SPORT,
    ACL_DPORT,
    ACL_IP4TOSPROTO,
    ACL_IP4FLAGOFF,
    ACL_TCNH,
    ACL_CPUTAG,
    ACL_L2PAYLOAD,
    ACL_IP6SIP0,
    ACL_IP6SIP1,
    ACL_IP6SIP2,
    ACL_IP6SIP3,
    ACL_IP6SIP4,
    ACL_IP6SIP5,
    ACL_IP6SIP6,
    ACL_IP6SIP7,
    ACL_IP6DIP0,
    ACL_IP6DIP1,
    ACL_IP6DIP2,
    ACL_IP6DIP3,
    ACL_IP6DIP4,
    ACL_IP6DIP5,
    ACL_IP6DIP6,
    ACL_IP6DIP7,
    ACL_TYPE_END
};

struct acl_rule_smi_st{
    rtk_uint16 rule_info;
    rtk_uint16 field[RTL8367C_ACLRULEFIELDNO];
};

struct acl_rule_smi_ext_st{
    rtk_uint16 rule_info;
};

typedef struct ACLRULESMI{
    struct acl_rule_smi_st  care_bits;
    rtk_uint16      valid:1;
    struct acl_rule_smi_st  data_bits;

    struct acl_rule_smi_ext_st care_bits_ext;
    struct acl_rule_smi_ext_st data_bits_ext;
}rtl8367c_aclrulesmi;

struct acl_rule_st{
    rtk_uint16 active_portmsk:11;
    rtk_uint16 type:3;
    rtk_uint16 tag_exist:5;
    rtk_uint16 field[RTL8367C_ACLRULEFIELDNO];
};

typedef struct ACLRULE{
    struct acl_rule_st  data_bits;
    rtk_uint16      valid:1;
    struct acl_rule_st  care_bits;
}rtl8367c_aclrule;


typedef struct rtl8367c_acltemplate_s{
    rtk_uint8 field[8];
}rtl8367c_acltemplate_t;


typedef struct acl_act_s{
    rtk_uint16 cvidx_cact:7;
    rtk_uint16 cact:2;
    rtk_uint16 svidx_sact:7;
    rtk_uint16 sact:2;


    rtk_uint16 aclmeteridx:7;
    rtk_uint16 fwdpmask:11;
    rtk_uint16 fwdact:2;

    rtk_uint16 pridx:7;
    rtk_uint16 priact:2;
    rtk_uint16 gpio_pin:4;
    rtk_uint16 gpio_en:1;
    rtk_uint16 aclint:1;

    rtk_uint16 cact_ext:2;
    rtk_uint16 fwdact_ext:1;
    rtk_uint16 tag_fmt:2;
}rtl8367c_acl_act_t;

typedef struct acl_rule_union_s
{
    rtl8367c_aclrule aclRule;
    rtl8367c_acl_act_t aclAct;
    rtk_uint32 aclActCtrl;
    rtk_uint32 aclNot;
}rtl8367c_acl_rule_union_t;


extern ret_t rtl8367c_setAsicAcl(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicAcl(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicAclUnmatchedPermit(rtk_uint32 port, rtk_uint32 enabled);
extern ret_t rtl8367c_getAsicAclUnmatchedPermit(rtk_uint32 port, rtk_uint32* pEnabled);
extern ret_t rtl8367c_setAsicAclRule(rtk_uint32 index, rtl8367c_aclrule *pAclRule);
extern ret_t rtl8367c_getAsicAclRule(rtk_uint32 index, rtl8367c_aclrule *pAclRule);
extern ret_t rtl8367c_setAsicAclNot(rtk_uint32 index, rtk_uint32 not);
extern ret_t rtl8367c_getAsicAclNot(rtk_uint32 index, rtk_uint32* pNot);
extern ret_t rtl8367c_setAsicAclTemplate(rtk_uint32 index, rtl8367c_acltemplate_t* pAclType);
extern ret_t rtl8367c_getAsicAclTemplate(rtk_uint32 index, rtl8367c_acltemplate_t *pAclType);
extern ret_t rtl8367c_setAsicAclAct(rtk_uint32 index, rtl8367c_acl_act_t* pAclAct);
extern ret_t rtl8367c_getAsicAclAct(rtk_uint32 index, rtl8367c_acl_act_t *pAclAct);
extern ret_t rtl8367c_setAsicAclActCtrl(rtk_uint32 index, rtk_uint32 aclActCtrl);
extern ret_t rtl8367c_getAsicAclActCtrl(rtk_uint32 index, rtk_uint32 *aclActCtrl);
extern ret_t rtl8367c_setAsicAclPortRange(rtk_uint32 index, rtk_uint32 type, rtk_uint32 upperPort, rtk_uint32 lowerPort);
extern ret_t rtl8367c_getAsicAclPortRange(rtk_uint32 index, rtk_uint32* pType, rtk_uint32* pUpperPort, rtk_uint32* pLowerPort);
extern ret_t rtl8367c_setAsicAclVidRange(rtk_uint32 index, rtk_uint32 type, rtk_uint32 upperVid, rtk_uint32 lowerVid);
extern ret_t rtl8367c_getAsicAclVidRange(rtk_uint32 index, rtk_uint32* pType, rtk_uint32* pUpperVid, rtk_uint32* pLowerVid);
extern ret_t rtl8367c_setAsicAclIpRange(rtk_uint32 index, rtk_uint32 type, ipaddr_t upperIp, ipaddr_t lowerIp);
extern ret_t rtl8367c_getAsicAclIpRange(rtk_uint32 index, rtk_uint32* pType, ipaddr_t* pUpperIp, ipaddr_t* pLowerIp);
extern ret_t rtl8367c_setAsicAclGpioPolarity(rtk_uint32 polarity);
extern ret_t rtl8367c_getAsicAclGpioPolarity(rtk_uint32* pPolarity);

#endif /*_RTL8367C_ASICDRV_ACL_H_*/


