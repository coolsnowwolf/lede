/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : RTL8373 switch high-level API
 *
 * Feature : The file includes ACL module high-layer API defination
 *
 */

#ifndef __DAL_RTL8373_ACL_H__
#define __DAL_RTL8373_ACL_H__

#include <acl.h>

#define RTL8373_ACLRULENO                   (96)

#define RTL8373_ACLRULEMAX                 (RTL8373_ACLRULENO-1)
#define RTL8373_ACLRULEFIELDNO             (8)
#define RTL8373_ACLTEMPLATENO              (5)
#define RTL8373_ACLTYPEMAX                 (RTL8373_ACLTEMPLATENO-1)

#define RTL8373_ACL_RULE_ENTRY_LEN               (5)
#define RTL8373_ACL_ACT_ENTRY_LEN                 (3)
#define RTL8373_ACLRULE_VALIDBIT_OFFSET                 (21)

#define RTL8373_ACLRULETBADDR(type, rule)  ((type << 7) | rule)
#define RTL8373_ACLRULETBADDR2(type, rule) ((type << 5) | (rule + 64))

#define ACL_ACT_CVLAN_ENABLE_MASK                                  (0x1)
#define ACL_ACT_SVLAN_ENABLE_MASK                                  (0x2)
#define ACL_ACT_PRI_ENABLE_MASK                                       (0x4)
#define ACL_ACT_1PRMK_ENABLE_MASK                                  (0x8)
#define ACL_ACT_DSCPRMK_ENABLE_MASK                              (0x10)
#define ACL_ACT_POLICING_ENABLE_MASK                              (0x20)
#define ACL_ACT_LOGGING_ENABLE_MASK                               (0x40)
#define ACL_ACT_FWD_ENABLE_MASK                                       (0x80)
#define ACL_ACT_INTGPIO_ENABLE_MASK                                (0x100)
#define ACL_ACT_BYPASS_ENABLE_MASK                                  (0x200)

#define RTL8373_ACLRULETAGBITS             (5)
#define RTL8373_ACLRANGENO                    (16)
#define RTL8373_ACLTEMPLATE_MASK                     (7)
#define ACL_BYPASS_IGRBW_STORM_MASK               (1)
#define ACL_BYPASS_STP_SRC_CHK_MASK                (2)
#define ACL_BYPASS_IGRVLAN_FLTR_MASK              (4)


#define RTL8373_ACLRANGEMAX                (RTL8373_ACLRANGENO-1)

#define RTL8373_ACL_PORTRANGEMAX           (0xFFFF)

#define RTL8373_FIELDSEL_FORMAT_NUMBER     (16)
#define RTL8373_FIELDSEL_MAX_OFFSET            (255)

#define RTL8373_MAX_LOG_CNT_NUM            (32)
#define RTL8373_RTK_IPV6_ADDR_WORD_LENGTH           (2UL)

#define RTL8373_ACLGPIOPINNO               (16)

#define ACL_RULE_TEMPLATE_IDX_OFFSET  (0)
#define ACL_RULE_TAG_PPPOE_OFFSET         (3)
#define ACL_RULE_L3FMT_OFFSET         (6)
#define ACL_RULE_L4FMT_OFFSET         (8)
#define ACL_RULE_ACTIVE_PMSK_OFFSET      (11)

enum RTL8373_ACL_GPIO_POLARITY
{
    RTL8373_GPIO_PULL_LOW = 0,
    RTL8373_GPIO_PILL_HIGH,
    RTL8373_GPIO_POLARITY_END
};

enum RTL8373_FIELDSEL_FORMAT_FORMAT
{
    RTL8373_FIELDSEL_FORMAT_DEFAULT = 0,
    RTL8373_FIELDSEL_FORMAT_RAW,
    RTL8373_FIELDSEL_FORMAT_LLC,
    RTL8373_FIELDSEL_FORMAT_IPV4,
    RTL8373_FIELDSEL_FORMAT_ARP,
    RTL8373_FIELDSEL_FORMAT_IPV6,
    RTL8373_FIELDSEL_FORMAT_IPPAYLOAD,
    RTL8373_FIELDSEL_FORMAT_L4PAYLOAD,
    RTL8373_FIELDSEL_FORMAT_END
};

enum RTL8373_ACLFIELDTYPES
{
    RTL8373_ACL_DMAC0 = 0,
    RTL8373_ACL_DMAC1,
    RTL8373_ACL_DMAC2,
    RTL8373_ACL_SMAC0,
    RTL8373_ACL_SMAC1,
    RTL8373_ACL_SMAC2,
    RTL8373_ACL_ETHERTYPE,
    RTL8373_ACL_STAG,
    RTL8373_ACL_CTAG,
    
    RTL8373_ACL_IP4SIP0 = 0x10,
    RTL8373_ACL_IP4SIP1,
    RTL8373_ACL_IP4DIP0,
    RTL8373_ACL_IP4DIP1,

    RTL8373_ACL_VIDRANGE = 0x30,
    RTL8373_ACL_IPRANGE,
    RTL8373_ACL_PORTRANGE,
    RTL8373_ACL_FIELD_VALID,
    RTL8373_ACL_IPTOSPROTO,
    RTL8373_ACL_L4SPORT,
    RTL8373_ACL_L4DPORT,
    
    RTL8373_ACL_FIELD_SELECT00 = 0x40,
    RTL8373_ACL_FIELD_SELECT01,
    RTL8373_ACL_FIELD_SELECT02,
    RTL8373_ACL_FIELD_SELECT03,
    RTL8373_ACL_FIELD_SELECT04,
    RTL8373_ACL_FIELD_SELECT05,
    RTL8373_ACL_FIELD_SELECT06,
    RTL8373_ACL_FIELD_SELECT07,
    RTL8373_ACL_FIELD_SELECT08,
    RTL8373_ACL_FIELD_SELECT09,
    RTL8373_ACL_FIELD_SELECT10,
    RTL8373_ACL_FIELD_SELECT11,
    RTL8373_ACL_FIELD_SELECT12,
    RTL8373_ACL_FIELD_SELECT13,
    RTL8373_ACL_FIELD_SELECT14,
    RTL8373_ACL_FIELD_SELECT15,
    RTL8373_ACL_TYPE_END
};

enum RTL8373_ACLTCAMTYPES
{
    RTL8373_CAREBITS= 0,
    RTL8373_DATABITS
};

typedef enum rtl8373_aclFwd
{
    RTL8373_ACL_FWD_COPY = 0,
    RTL8373_ACL_FWD_REDIRECT,
    RTL8373_ACL_FWD_MIRROR ,
    RTL8373_ACL_FWD_INT_TRAP,
    RTL8373_ACL_FWD_EXT_TRAP,
    RTL8373_ACL_FWD_INT_EXT_TRAP,
} rtl8373_aclFwd_t;


struct acl_rule_smi_st{
    rtk_uint32 field[RTL8373_ACLRULEFIELDNO/2];    
    rtk_uint32 rule_info;
};

typedef struct RTL8373_aclRule_SMI{
    struct acl_rule_smi_st  care_bits;
    rtk_uint32      valid:1;
    struct acl_rule_smi_st  data_bits;
}rtl8373_aclRule_smi_t;

typedef struct acl_ruleContent_s{
    rtk_uint32 templateIdx:3;
    rtk_uint32 tagPppoe:3;
    rtk_uint32 l3fmt:2;
    rtk_uint32 l4fmt:3;
    rtk_uint32 activePmsk:10;

    rtk_uint16 field[RTL8373_ACLRULEFIELDNO];
}acl_ruleContent_t;

typedef struct rtl8373_acl_rule_s{
    acl_ruleContent_t  data_bits;
    rtk_uint32      valid:1;
    acl_ruleContent_t  care_bits;
}rtl8373_acl_rule_t;


typedef struct rtl8373_acl_template_s{
    rtk_uint8 field[RTL8373_ACLRULEFIELDNO];
}rtl8373_acl_template_t;


typedef struct rtl8373_acl_act_s{
    rtk_uint32 cact:2;
    rtk_uint32 cactExt:2;
    rtk_uint32 cvid:12;
    rtk_uint32 tagFmt:2;
    rtk_uint32 sact:2;
    rtk_uint32 svid:12;


    rtk_uint32 aclPri:3;
    rtk_uint32 aclRmkAct:1;
    rtk_uint32 aclRmkVal:6;
    rtk_uint32 aclPolicingLogAct:1;
    rtk_uint32 aclMeterLoggIdx:6;

    rtk_uint32 fwdPmsk:10;
    rtk_uint32 fwdAct:3;
    rtk_uint32 fwdActExt:1;
    rtk_uint32 aclInt:1;
    rtk_uint32 gpioEn:1;
    rtk_uint32 gpioPin:4;
    rtk_uint32 bypassAct:3;

}rtl8373_acl_act_t;

typedef struct rtl8373_acl_ruleUnion_s
{
    rtl8373_acl_rule_t aclRule;
    rtl8373_acl_act_t aclAct;
    rtk_uint32 aclActCtrl;
    rtk_uint32 aclNot;
}rtl8373_acl_ruleUnion_t;

extern rtk_api_ret_t _rtl8373_setAclRule(rtk_uint32 index, rtl8373_acl_rule_t* pAclRule);
extern rtk_api_ret_t _rtl8373_getAclRule(rtk_uint32 index, rtl8373_acl_rule_t* pAclRule);


/* Function Name:
 *      dal_rtl8373_igrAcl_init
 * Description:
 *      ACL initialization function
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - Pointer pFilter_field or pFilter_cfg point to NULL.
 * Note:
 *      This function enable and intialize ACL function
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_init(void);

/* Function Name:
 *      dal_rtl8373_igrAcl_field_add
 * Description:
 *      Add comparison rule to an ACL configuration
 * Input:
 *      pFilterCfg     - The ACL configuration that this function will add comparison rule
 *      pFilterField   - The comparison rule that will be added.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_NULL_POINTER     - Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This function add a comparison rule (*pFilter_field) to an ACL configuration (*pFilter_cfg).
 *      Pointer pFilter_cfg points to an ACL configuration structure, this structure keeps multiple ACL
 *      comparison rules by means of linked list. Pointer pFilter_field will be added to linked
 *      list keeped by structure that pFilter_cfg points to.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_field_add(rtk_filter_cfg_t* pFilterCfg, rtk_filter_field_t* pFilterField);

/* Function Name:
 *      dal_rtl8373_igrAcl_cfg_delAll
 * Description:
 *      Delete all ACL entries from ASIC
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This function delete all ACL configuration from ASIC.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_cfg_delAll(void);

/* Function Name:
 *      dal_rtl8373_igrAcl_cfg_add
 * Description:
 *      Add an ACL configuration to ASIC
 * Input:
 *      filterIdx       - Start index of ACL configuration.
 *      pFilterCfg     - The ACL configuration that this function will add comparison rule
 *      pAction  - Action(s) of ACL configuration.
 * Output:
 *      ruleNum - number of rules written in acl table
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 *      RT_ERR_SMI                              - SMI access error
 *      RT_ERR_NULL_POINTER                     - Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT                            - Invalid input parameters.
 *      RT_ERR_ENTRY_INDEX                      - Invalid filter_id .
 *      RT_ERR_NULL_POINTER                     - Pointer pFilter_action or pFilter_cfg point to NULL.
 *      RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT     - Action is not supported in this chip.
 *      RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT    - Rule is not supported.
 * Note:
 *      This function store pFilter_cfg, pFilter_action into ASIC. The starting
 *      index(es) is filter_id.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_cfg_add(rtk_filter_id_t filterIdx, rtk_filter_cfg_t* pFilterCfg, rtk_filter_action_t* pAction, rtk_filter_number_t *ruleNum);

/* Function Name:
 *      dal_rtl8373_igrAcl_cfg_del
 * Description:
 *      Delete an ACL configuration from ASIC
 * Input:
 *      filterIdx   - Start index of ACL configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_ENTRYIDX  - Invalid filter_id.
 * Note:
 *      This function delete a group of ACL rules starting from filter_id.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_cfg_del(rtk_filter_id_t filterIdx);


/* Function Name:
 *      dal_rtl8373_igrAcl_cfg_get
 * Description:
 *      Get one ingress acl configuration from ASIC.
 * Input:
 *      filterIdx       - Start index of ACL configuration.
 * Output:
 *      pFilterCfg     - buffer pointer of ingress acl data
 *      pAction  - buffer pointer of ingress acl action
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_NULL_POINTER     - Pointer pFilter_action or pFilter_cfg point to NULL.
 *      RT_ERR_FILTER_ENTRYIDX  - Invalid entry index.
 * Note:
 *      This function get configuration from ASIC.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_cfg_get(rtk_filter_id_t filterIdx, rtk_filter_cfg_raw_t *pFilterCfg, rtk_filter_action_t *pAction);

/* Function Name:
 *      dal_rtl8373_igrAcl_unmatchAction_set
 * Description:
 *      Set action to packets when no ACL configuration match
 * Input:
 *      port    - Port id.
 *      action  - Action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function sets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_unmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_t action);

/* Function Name:
 *      dal_rtl8373_igrAcl_unmatchAction_get
 * Description:
 *      Get action to packets when no ACL configuration match
 * Input:
 *      port    - Port id.
 * Output:
 *      pAction - Action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_unmatchAction_get(rtk_port_t port, rtk_filter_unmatch_action_t* pAction);

/* Function Name:
 *      dal_rtl8373_igrAcl_state_set
 * Description:
 *      Set state of ingress ACL.
 * Input:
 *      port    - Port id.
 *      state   - Ingress ACL state.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_state_set(rtk_port_t port, rtk_filter_state_t state);

/* Function Name:
 *      dal_rtl8373_igrAcl_state_get
 * Description:
 *      Get state of ingress ACL.
 * Input:
 *      port    - Port id.
 * Output:
 *      pState  - Ingress ACL state.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port id.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no ACL configruation matches.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_state_get(rtk_port_t port, rtk_filter_state_t* pState);

/* Function Name:
 *      dal_rtl8373_igrAcl_template_set
 * Description:
 *      Set template of ingress ACL.
 * Input:
 *      template - Ingress ACL template
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Invalid input parameters.
 * Note:
 *      This function set ACL template.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_template_set(rtk_filter_template_t *aclTemplate);

/* Function Name:
 *      dal_rtl8373_igrAcl_template_get
 * Description:
 *      Get template of ingress ACL.
 * Input:
 *      template - Ingress ACL template
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This function gets template of ACL.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_template_get(rtk_filter_template_t *aclTemplate);

/* Function Name:
 *      dal_rtl8373_igrAcl_fieldSel_set
 * Description:
 *      Set user defined field selectors in HSB
 * Input:
 *      index       - index of field selector 0-15
 *      format      - Format of field selector
 *      offset      - Retrieving data offset
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      System support 16 user defined field selctors.
 *      Each selector can be enabled or disable.
 *      User can defined retrieving 16-bits in many predefiend
 *      standard l2/l3/l4 payload.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_fieldSel_set(rtk_uint32 index, rtk_field_sel_t format, rtk_uint32 offset);

/* Function Name:
 *      dal_rtl8373_igrAcl_fieldSel_get
 * Description:
 *      Get user defined field selectors in HSB
 * Input:
 *      index       - index of field selector 0-15
 * Output:
 *      pFormat     - Format of field selector
 *      pOffset     - Retrieving data offset
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_fieldSel_get(rtk_uint32 index, rtk_field_sel_t *pFormat, rtk_uint32 *pOffset);

/* Function Name:
 *      dal_rtl8373_igrAcl_ipRange_set
 * Description:
 *      Set IP Range check
 * Input:
 *      index       - index of IP Range 0-15
 *      type        - IP Range check type, 0:Delete a entry, 1: IPv4_SIP, 2: IPv4_DIP, 3:IPv6_SIP, 4:IPv6_DIP
 *      upperIp     - The upper bound of IP range
 *      lowerIp     - The lower Bound of IP range
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upperIp must be larger or equal than lowerIp.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_ipRange_set(rtk_uint32 index, rtk_filter_iprange_t type, ipaddr_t upperIp, ipaddr_t lowerIp);

/* Function Name:
 *      dal_rtl8373_igrAcl_ipRange_get
 * Description:
 *      Set IP Range check
 * Input:
 *      index       - index of IP Range 0-15
 * Output:
 *      pType        - IP Range check type, 0:Delete a entry, 1: IPv4_SIP, 2: IPv4_DIP, 3:IPv6_SIP, 4:IPv6_DIP
 *      pUpperIp     - The upper bound of IP range
 *      pLowerIp     - The lower Bound of IP range
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_ipRange_get(rtk_uint32 index, rtk_filter_iprange_t *pType, ipaddr_t *pUpperIp, ipaddr_t *pLowerIp);

/* Function Name:
 *      dal_rtl8373_igrAcl_vidRange_set
 * Description:
 *      Set VID Range check
 * Input:
 *      index       - index of VID Range 0-15
 *      type        - IP Range check type, 0:Delete a entry, 1: CVID, 2: SVID
 *      upperVid    - The upper bound of VID range
 *      lowerVid    - The lower Bound of VID range
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upperVid must be larger or equal than lowerVid.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_vidRange_set(rtk_uint32 index, rtk_filter_vidrange_t type, rtk_uint32 upperVid, rtk_uint32 lowerVid);

/* Function Name:
 *      dal_rtl8373_igrAcl_vidRange_get
 * Description:
 *      Get VID Range check
 * Input:
 *      index       - index of VID Range 0-15
 * Output:
 *      pType        - IP Range check type, 0:Unused, 1: CVID, 2: SVID
 *      pUpperVid    - The upper bound of VID range
 *      pLowerVid    - The lower Bound of VID range
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_vidRange_get(rtk_uint32 index, rtk_filter_vidrange_t *pType, rtk_uint32 *pUpperVid, rtk_uint32 *pLowerVid);

/* Function Name:
 *      dal_rtl8373_igrAcl_portRange_set
 * Description:
 *      Set Port Range check
 * Input:
 *      index       - index of Port Range 0-15
 *      type        - IP Range check type, 0:Delete a entry, 1: Source Port, 2: Destnation Port
 *      upperPort   - The upper bound of Port range
 *      lowerPort   - The lower Bound of Port range
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      upperPort must be larger or equal than lowerPort.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_portRange_set(rtk_uint32 index, rtk_filter_portrange_t type, rtk_uint32 upperPort, rtk_uint32 lowerPort);

/* Function Name:
 *      dal_rtl8373_igrAcl_portRange_get
 * Description:
 *      Set Port Range check
 * Input:
 *      index       - index of Port Range 0-15
 * Output:
 *      pType       - IP Range check type, 0:Delete a entry, 1: Source Port, 2: Destnation Port
 *      pUpperPort  - The upper bound of Port range
 *      pLowerPort  - The lower Bound of Port range
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_portRange_get(rtk_uint32 index, rtk_filter_portrange_t *pType, rtk_uint32 *pUpperPort, rtk_uint32 *pLowerPort);

/* Function Name:
 *      dal_rtl8373_igrAcl_gpioPolarity_set
 * Description:
 *      Set ACL Goip control palarity
 * Input:
 *      polarity - 1: High, 0: Low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      none
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_gpioPolarity_set(rtk_uint32 polarity);

/* Function Name:
 *      dal_rtl8373_igrAcl_gpioPolarity_get
 * Description:
 *      Get ACL Goip control palarity
 * Input:
 *      pPolarity - 1: High, 0: Low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      none
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_gpioPolarity_get(rtk_uint32* pPolarity);

/* Function Name:
 *      dal_rtl8373_igrAcl_gpioEn_set
 * Description:
 *      Enable ACL gpio function
 * Input:
 *      pinNum - gpio pin number
 *      enabled - enable or disable acl gpio funciton
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      none
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_gpioEn_set(rtk_uint32 pinNum,  rtk_enable_t enabled);

/* Function Name:
 *      dal_rtl8373_igrAcl_gpioEn_get
 * Description:
 *      Get ACL gpio enable or not
 * Input:
 *      pinNum - gpio pin number
 * Output:
 *      *pEnabled - acl gpio pin status
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      none
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_gpioEn_get(rtk_uint32 pinNum,  rtk_enable_t * pEnabled);

/* Function Name:
 *      dal_rtl8373_igrAcl_table_rst
 * Description:
 *     reset acl table :include acl rule table and action table
 * Input:
 *      none
 * Output:
 *
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_igrAcl_table_rst(void);


#endif /* __DAL_RTL8373_ACL_H__ */
