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
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for rtl8373
 * Feature : Here is a list of all functions and variables in ACL module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_acl.h>
#include <vlan.h>
#include <string.h>

#include <dal/rtl8373/rtl8373_asicdrv.h>

#if defined(CONFIG_RTL8373_ASICDRV_TEST)
rtl8373_aclRule_smi_t Rtl8370sVirtualAclRuleTable[RTL8373_ACLRULENO];
rtk_uint16 Rtl8370sVirtualAclActTable[RTL8373_ACLRULENO][RTL8373_ACL_ACT_ENTRY_LEN];
#endif


CONST_T rtk_uint8 rtl8373_filter_templateField[RTL8373_ACLTEMPLATENO][RTL8373_ACLRULEFIELDNO] = {
    {RTL8373_ACL_DMAC0,             RTL8373_ACL_DMAC1,          RTL8373_ACL_DMAC2,          RTL8373_ACL_SMAC0,          RTL8373_ACL_SMAC1,          RTL8373_ACL_SMAC2,          RTL8373_ACL_ETHERTYPE,      RTL8373_ACL_FIELD_SELECT15},
    {RTL8373_ACL_IP4SIP0,           RTL8373_ACL_IP4SIP1,        RTL8373_ACL_IP4DIP0,        RTL8373_ACL_IP4DIP1,            RTL8373_ACL_IPTOSPROTO, RTL8373_ACL_L4SPORT,        RTL8373_ACL_L4DPORT,        RTL8373_ACL_FIELD_SELECT00},        
    {RTL8373_ACL_IP4DIP0, RTL8373_ACL_IP4DIP1, RTL8373_ACL_FIELD_SELECT03, RTL8373_ACL_FIELD_SELECT04, RTL8373_ACL_FIELD_SELECT05, RTL8373_ACL_FIELD_SELECT06, RTL8373_ACL_FIELD_SELECT07, RTL8373_ACL_FIELD_SELECT08},
    {RTL8373_ACL_IP4SIP0,  RTL8373_ACL_IP4SIP1, RTL8373_ACL_FIELD_SELECT09,  RTL8373_ACL_FIELD_SELECT10, RTL8373_ACL_FIELD_SELECT11, RTL8373_ACL_FIELD_SELECT12, RTL8373_ACL_FIELD_SELECT13, RTL8373_ACL_FIELD_SELECT14},
//    {RTL8373_ACL_FIELD_SELECT08, RTL8373_ACL_FIELD_SELECT07, RTL8373_ACL_FIELD_SELECT06, RTL8373_ACL_FIELD_SELECT05, RTL8373_ACL_FIELD_SELECT04, RTL8373_ACL_FIELD_SELECT03, RTL8373_ACL_IP4DIP1, RTL8373_ACL_IP4DIP0},
//    {RTL8373_ACL_FIELD_SELECT14, RTL8373_ACL_FIELD_SELECT13, RTL8373_ACL_FIELD_SELECT12, RTL8373_ACL_FIELD_SELECT11, RTL8373_ACL_FIELD_SELECT10, RTL8373_ACL_FIELD_SELECT09, RTL8373_ACL_IP4SIP1, RTL8373_ACL_IP4SIP0},
    {RTL8373_ACL_VIDRANGE,    RTL8373_ACL_IPRANGE,        RTL8373_ACL_PORTRANGE,     RTL8373_ACL_CTAG,           RTL8373_ACL_STAG,           RTL8373_ACL_FIELD_SELECT01, RTL8373_ACL_FIELD_SELECT02, RTL8373_ACL_FIELD_VALID}
};

CONST_T rtk_uint8 rtl8373_filter_advanceCaretagField[RTL8373_ACLTEMPLATENO][2] = {
    {TRUE,      7},
    {TRUE,      7},
    {FALSE,     0},
    {TRUE,      7},
    {TRUE,      7},
};

/*MSB_4bit represent template IDX, LSB_4bit represent rule field idx*/
CONST_T rtk_uint8 rtl8373_filter_fieldTemplateIndex[FILTER_FIELD_END][RTK_FILTER_FIELD_USED_MAX] = {
    {0x00, 0x01,0x02},
    {0x03, 0x04,0x05},
    {0x06},
    {0x43},
    {0x44},

    {0x10, 0x11},
    {0x12, 0x13},
    {0x10, 0x11},
    {0x12, 0x13},

    {0x14},
    {0x14},
        
    {0x30, 0x31,0x32, 0x33,0x34, 0x35, 0x36, 0x37},
    {0x20, 0x21,0x22, 0x23,0x24, 0x25, 0x26, 0x27},

    {0x14},
    {0x14},
        
    {0x14},
    {0x14},

    {0x15},
    {0x16},
    {0x15},
    {0x15},
    {0x15},
    {0x16},
        
    {0x40},
    {0x41},
    {0x42},
    {0x47},

    {0x17},
    {0x45},
    {0x46},
    {0x22},
    {0x23},
    {0x24},
    {0x25},
    {0x26},
    {0x27},
    {0x32},
    {0x33},
    {0x34},
    {0x35},
    {0x36},
    {0x37},
    {0x07}
   // {0xFF} /* Pattern Match */
};

CONST_T rtk_uint8 rtl8373_filter_fieldSize[FILTER_FIELD_END] = {
    3, 3, 1, 1, 1,
    2, 2, 2,2,
    1, 1, 8, 8, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

CONST_T rtk_uint16 rtl8373_field_selector[RTL8373_FIELDSEL_FORMAT_NUMBER][2] =
{
    {RTL8373_FIELDSEL_FORMAT_DEFAULT, 0},    /* Field Selector 0 */
    {RTL8373_FIELDSEL_FORMAT_DEFAULT, 0},    /* Field Selector 1 */
    {RTL8373_FIELDSEL_FORMAT_IPPAYLOAD, 12}, /* Field Selector 2 */
    {RTL8373_FIELDSEL_FORMAT_IPV6, 10},      /* Field Selector 3 */
    {RTL8373_FIELDSEL_FORMAT_IPV6, 8},       /* Field Selector 4 */
    {RTL8373_FIELDSEL_FORMAT_IPV4, 0},       /* Field Selector 5 */
    {RTL8373_FIELDSEL_FORMAT_IPV4, 8},       /* Field Selector 6 */
    {RTL8373_FIELDSEL_FORMAT_IPV6, 0},       /* Field Selector 7 */
    {RTL8373_FIELDSEL_FORMAT_IPV6, 6},       /* Field Selector 8 */
    {RTL8373_FIELDSEL_FORMAT_IPV6, 26},      /* Field Selector 9 */
    {RTL8373_FIELDSEL_FORMAT_IPV6, 24},      /* Field Selector 10 */
    {RTL8373_FIELDSEL_FORMAT_DEFAULT, 0},    /* Field Selector 11 */
    {RTL8373_FIELDSEL_FORMAT_IPV4, 6},       /* Field Selector 12 */
    {RTL8373_FIELDSEL_FORMAT_IPPAYLOAD, 0},  /* Field Selector 13 */
    {RTL8373_FIELDSEL_FORMAT_IPPAYLOAD, 2},  /* Field Selector 14 */
    {RTL8373_FIELDSEL_FORMAT_DEFAULT, 0}     /* Field Selector 15 */
};

/*
    Exchange structure type define with MMI and SMI
*/
static void _rtl8373_aclRuleStSmi2User( rtl8373_acl_rule_t *pAclUser, rtl8373_aclRule_smi_t *pAclSmi)
{
    rtk_uint8 *care_ptr, *data_ptr;
    rtk_uint8 care_tmp, data_tmp;
    rtk_uint32 i;

    pAclUser->data_bits.templateIdx = ((pAclSmi->data_bits.rule_info >> ACL_RULE_TEMPLATE_IDX_OFFSET)& 0x7);
    pAclUser->data_bits.tagPppoe = ((pAclSmi->data_bits.rule_info >> ACL_RULE_TAG_PPPOE_OFFSET) & 0x7);
    pAclUser->data_bits.l3fmt= ((pAclSmi->data_bits.rule_info >> ACL_RULE_L3FMT_OFFSET) & 0x3);
    pAclUser->data_bits.l4fmt= ((pAclSmi->data_bits.rule_info >> ACL_RULE_L4FMT_OFFSET) & 0x7);
    pAclUser->data_bits.activePmsk = ((pAclSmi->data_bits.rule_info >> ACL_RULE_ACTIVE_PMSK_OFFSET) & RTK_MAX_PORT_MASK);

    care_ptr = (rtk_uint8*)&pAclSmi->care_bits;
    data_ptr = (rtk_uint8*)&pAclSmi->data_bits;

    for ( i = 0; i < sizeof(struct acl_rule_smi_st); i++)
    {
        care_tmp = *(care_ptr + i) ^ (*(data_ptr + i));
        data_tmp = *(data_ptr + i);

        *(care_ptr + i) = care_tmp;
        *(data_ptr + i) = data_tmp;
    }


    for(i = 0; i < RTL8373_ACLRULEFIELDNO; i++)
    {
        if(i%2)
            pAclUser->data_bits.field[i] = (rtk_uint16)( pAclSmi->data_bits.field[i/2] >>16 );
        else
            pAclUser->data_bits.field[i] = (rtk_uint16)(pAclSmi->data_bits.field[i/2]);
    }
    
    pAclUser->valid = pAclSmi->valid;

    pAclUser->care_bits.templateIdx = ((pAclSmi->care_bits.rule_info >> ACL_RULE_TEMPLATE_IDX_OFFSET) & 0x7);
    pAclUser->care_bits.tagPppoe= ((pAclSmi->care_bits.rule_info >> ACL_RULE_TAG_PPPOE_OFFSET) & 0x7);
    pAclUser->care_bits.l3fmt = ((pAclSmi->care_bits.rule_info >> ACL_RULE_L3FMT_OFFSET) & 0x3);
    pAclUser->care_bits.l4fmt = ((pAclSmi->care_bits.rule_info >> ACL_RULE_L4FMT_OFFSET) & 0x7);
    pAclUser->care_bits.activePmsk = ((pAclSmi->care_bits.rule_info >> ACL_RULE_ACTIVE_PMSK_OFFSET) & RTK_MAX_PORT_MASK);

    for(i = 0; i < RTL8373_ACLRULEFIELDNO; i++)
    {
         if(i%2)
             pAclUser->care_bits.field[i] = (rtk_uint16)( pAclSmi->care_bits.field[i/2] >>16 );
         else
             pAclUser->care_bits.field[i] = (rtk_uint16)(pAclSmi->care_bits.field[i/2]) ;
    }

}

/*
    Exchange structure type define with MMI and SMI
*/
static void _rtl8373_aclRuleStUser2Smi(rtl8373_acl_rule_t *pAclUser, rtl8373_aclRule_smi_t *pAclSmi)
{
    rtk_uint8 *care_ptr, *data_ptr;
    rtk_uint8 care_tmp, data_tmp;
    rtk_uint32 i;
    pAclSmi->data_bits.rule_info |= ((pAclUser->data_bits.templateIdx & 0x7) << ACL_RULE_TEMPLATE_IDX_OFFSET);
    pAclSmi->data_bits.rule_info |= ((pAclUser->data_bits.tagPppoe & 0x7) << ACL_RULE_TAG_PPPOE_OFFSET);
    pAclSmi->data_bits.rule_info |= ((pAclUser->data_bits.l3fmt & 0x3)<< ACL_RULE_L3FMT_OFFSET);
    pAclSmi->data_bits.rule_info |= ((pAclUser->data_bits.l4fmt & 0x7)<< ACL_RULE_L4FMT_OFFSET);
    pAclSmi->data_bits.rule_info |= ((pAclUser->data_bits.activePmsk & RTK_MAX_PORT_MASK) << ACL_RULE_ACTIVE_PMSK_OFFSET);

    for(i = 0;i < RTL8373_ACLRULEFIELDNO; i++)
    {
        if ( (i%2==1) )
            pAclSmi->data_bits.field[i/2] |= (rtk_uint32)( (pAclUser->data_bits.field[i]) << 16 );
        else
            pAclSmi->data_bits.field[i/2] = (rtk_uint32)(pAclUser->data_bits.field[i] );
    }

    pAclSmi->valid = pAclUser->valid;
    pAclSmi->care_bits.rule_info |= ((pAclUser->care_bits.templateIdx & 0x7) << ACL_RULE_TEMPLATE_IDX_OFFSET);
    pAclSmi->care_bits.rule_info |= ((pAclUser->care_bits.tagPppoe & 0x7) << ACL_RULE_TAG_PPPOE_OFFSET) ;
    pAclSmi->care_bits.rule_info |= ((pAclUser->care_bits.l3fmt & 0x3)<< ACL_RULE_L3FMT_OFFSET) ;
    pAclSmi->care_bits.rule_info |= ((pAclUser->care_bits.l4fmt & 0x7)<< ACL_RULE_L4FMT_OFFSET) ;
    pAclSmi->care_bits.rule_info |= ((pAclUser->care_bits.activePmsk & RTK_MAX_PORT_MASK) << ACL_RULE_ACTIVE_PMSK_OFFSET) ;
    
    for(i = 0; i < RTL8373_ACLRULEFIELDNO; i++)
    {
        if ( (i%2==1) )
             pAclSmi->care_bits.field[i/2] |= (rtk_uint32)( (pAclUser->care_bits.field[i])<<16 );
        else
             pAclSmi->care_bits.field[i/2] = (rtk_uint32)(pAclUser->care_bits.field[i]);
    }

    care_ptr = (rtk_uint8*)&pAclSmi->care_bits;
    data_ptr = (rtk_uint8*)&pAclSmi->data_bits;

    for ( i = 0; i < sizeof(struct acl_rule_smi_st); i++)
    {
        care_tmp = ~(*(care_ptr + i)) | ~(*(data_ptr + i));
        data_tmp = ~(*(care_ptr + i)) | *(data_ptr + i);
        *(care_ptr + i) = care_tmp;
        *(data_ptr + i) = data_tmp;
    }
}

/*
    Exchange structure type define with MMI and SMI
*/
static void _rtl8373_aclActStSmi2User(rtl8373_acl_act_t *pAclUser, rtk_uint32 *pAclSmi)
{
    pAclUser->cact = (pAclSmi[0] & 0x3);
    pAclUser->cactExt = ((pAclSmi[0] >> 2)& 0x3);
    pAclUser->cvid = ((pAclSmi[0] >> 4)& 0xFFF);
    pAclUser->tagFmt = ((pAclSmi[0] >> 16)& 0x3);
    pAclUser->sact = ((pAclSmi[0] >> 18)& 0x3);
    pAclUser->svid = ((pAclSmi[0]>>20) & 0xFFF);

    pAclUser->aclPri = (pAclSmi[1]  & 0x7);
    pAclUser->aclRmkAct = ((pAclSmi[1] >>3 ) & 1);
    pAclUser->aclRmkVal = ((pAclSmi[1] >> 4) & 0x3F);
    pAclUser->aclMeterLoggIdx =((pAclSmi[1]  >>10) & 0x3F);
    pAclUser->aclPolicingLogAct = ((pAclSmi[1] >> 16) & 0x1);
    pAclUser->fwdAct = ((pAclSmi[1] >> 17) & 0x7);
    pAclUser->fwdActExt = ((pAclSmi[1] >> 20) & 0x1);
    pAclUser->fwdPmsk = ((pAclSmi[1] >> 21) & 0x3FF);
    pAclUser->aclInt = ((pAclSmi[1] >> 31) & 0x1);    

    pAclUser->gpioPin = (pAclSmi[2] & 0xF);
    pAclUser->gpioEn = ((pAclSmi[2] >> 4) & 0x1);
    pAclUser->bypassAct = ((pAclSmi[2] >> 5) & 0x7);
}

/*
    Exchange structure type define with MMI and SMI
*/
static void _rtl8373_aclActStUser2Smi(rtl8373_acl_act_t *pAclUser, rtk_uint32 *pAclSmi)
{
    
    if(pAclUser->gpioEn == DISABLED)
        pAclUser->gpioPin = 0;

    pAclSmi[0] |= (pAclUser->cact & 0x3) ;
    pAclSmi[0] |= (pAclUser->cactExt & 0x3) << 2;
    pAclSmi[0] |= (pAclUser->cvid & 0xFFF)<<4;
    pAclSmi[0] |= (pAclUser->tagFmt & 0x3) <<16;
    pAclSmi[0] |= (pAclUser->sact & 0x3) << 18;
    pAclSmi[0] |= (pAclUser->svid & 0xFFF)<<20;

    pAclSmi[1] |= (pAclUser->aclPri & 0x7);
    pAclSmi[1] |= (pAclUser->aclRmkAct & 0x1)<<3;
    pAclSmi[1] |= (pAclUser->aclRmkVal & 0x3F)<< 4;
    pAclSmi[1] |= (pAclUser->aclMeterLoggIdx & 0x3F)<<10;
    pAclSmi[1] |= (pAclUser->aclPolicingLogAct & 0x1)<<16;
    pAclSmi[1] |= (pAclUser->fwdAct & 0x7) << 17;
    pAclSmi[1] |= (pAclUser->fwdActExt & 0x1) <<20;
    pAclSmi[1] |= (pAclUser->fwdPmsk & 0x3FF) << 21;
    pAclSmi[1] |= (pAclUser->aclInt & 0x1) << 31;    

    pAclSmi[2] |= (pAclUser->gpioPin & 0xF) ;
    pAclSmi[2] |= (pAclUser->gpioEn & 0x1) << 4;
    pAclSmi[2] |= (pAclUser->bypassAct & 0x7) << 5;

}

static rtk_api_ret_t _rtl8373_getAclTemplate(rtk_uint32 index, rtl8373_acl_template_t *pAclType)
{
    ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 regData, regAddr;

    regAddr = RTL8373_ACL_TEMPLATE_CTRL_ADDR(index) ;

    for(i = 0; i < (RTL8373_ACLRULEFIELDNO/4); i++)
    {
        if(i<1)
            retVal = rtl8373_getAsicReg(regAddr + 4, &regData);
        else
            retVal = rtl8373_getAsicReg(regAddr, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;
        pAclType->field[i*4] = regData & 0xFF;
        pAclType->field[i*4 + 1] = (regData >> 8) & 0xFF;
        pAclType->field[i*4 + 2] = (regData >> 16) & 0xFF;
        pAclType->field[i*4 + 3] = (regData >> 24) & 0xFF;
    }

    return RT_ERR_OK;

}

static rtk_api_ret_t _rtl8373_setAclTemplate(rtk_uint32 index, rtl8373_acl_template_t* pAclType)
{
    ret_t retVal;
    rtk_uint32 i;
    rtk_uint32 regAddr, regData;

    if(index >= RTL8373_ACLTEMPLATENO)
        return RT_ERR_OUT_OF_RANGE;

    regAddr = RTL8373_ACL_TEMPLATE_CTRL_ADDR(index) ;

    for(i = 0; i < (RTL8373_ACLRULEFIELDNO/4); i++)
    {
        regData = pAclType->field[i*4+3];
        regData = (regData << 8) | pAclType->field[i*4+2];
        regData = (regData << 8) | pAclType->field[i*4+1];
        regData = (regData << 8) | pAclType->field[i*4];
        if(i<1)
        {
            retVal = rtl8373_setAsicReg(regAddr + 4, regData);
        }
        else
        {
            retVal = rtl8373_setAsicReg(regAddr, regData);
        }

        if(retVal != RT_ERR_OK)
            return retVal;
    }
    return retVal;
}


static rtk_api_ret_t _rtk_igrAcl_writeDataField(rtl8373_acl_rule_t *aclRule, rtk_filter_field_t *fieldPtr)
{
    rtk_uint32 i = 0, tempIdx = 0, fieldIdx = 0, ipValue = 0, ipMask = 0;

    for(i = 0; i < fieldPtr->occupyFieldNum; i++)
    {
        tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;

        aclRule[tempIdx].valid = TRUE;
    }

    switch (fieldPtr->fieldType)
    {
        /* use DMAC structure as representative for mac structure */
        case FILTER_FIELD_DMAC:
        case FILTER_FIELD_SMAC:

            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] = fieldPtr->filter_pattern_union.mac.value.octet[5 - i*2] | (fieldPtr->filter_pattern_union.mac.value.octet[5 - (i*2 + 1)] << 8);
                aclRule[tempIdx].care_bits.field[fieldIdx] = fieldPtr->filter_pattern_union.mac.mask.octet[5 - i*2] | (fieldPtr->filter_pattern_union.mac.mask.octet[5 - (i*2 + 1)] << 8);
            }
            break;
        case FILTER_FIELD_ETHERTYPE:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.etherType.value & 0xFFFF);
                aclRule[tempIdx].care_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.etherType.mask & 0xFFFF);
            }
            break;
        case FILTER_FIELD_IPV4_SIP:
        case FILTER_FIELD_IPV4_DIP:
        case FILTER_FIELD_SENDER_PROTOCOL_ADDR:
        case FILTER_FIELD_TARGET_PROTOCOL_ADDR:

            ipValue = fieldPtr->filter_pattern_union.sip.value;
            ipMask = fieldPtr->filter_pattern_union.sip.mask;

            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] = (0xFFFF & (ipValue >> (i*16)));
                aclRule[tempIdx].care_bits.field[fieldIdx] = (0xFFFF & (ipMask >> (i*16)));
            }
            break;

        case FILTER_FIELD_IPV6_SIPV6:
        case FILTER_FIELD_IPV6_DIPV6:      
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                ipValue = fieldPtr->filter_pattern_union.ipv6.value.addr[i/2];
                ipMask = fieldPtr->filter_pattern_union.ipv6.mask.addr[i/2];
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] = (0xFFFF & (ipValue >> ((i%2)*16)));
                aclRule[tempIdx].care_bits.field[fieldIdx] = (0xFFFF & (ipMask  >> ((i%2)*16)));
            }
            break;
            
        case FILTER_FIELD_CTAG:
        case FILTER_FIELD_STAG:
            
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                aclRule[tempIdx].data_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.l2tag.pri.value << 13) | (fieldPtr->filter_pattern_union.l2tag.cfi.value << 12) | fieldPtr->filter_pattern_union.l2tag.vid.value;
                aclRule[tempIdx].care_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.l2tag.pri.mask << 13) | (fieldPtr->filter_pattern_union.l2tag.cfi.mask << 12) | fieldPtr->filter_pattern_union.l2tag.vid.mask;
            }
            break;
            
        case FILTER_FIELD_IPV4_TOS:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
                
                aclRule[tempIdx].data_bits.field[fieldIdx] &= 0xFF;
                aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.ipTos.value & 0xFF)<<8 ;
                aclRule[tempIdx].care_bits.field[fieldIdx] &= 0xFF;
                aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.ipTos.mask  & 0xFF)<<8 ;
            }
            break;
        case FILTER_FIELD_IPV4_PROTOCOL:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] &= 0xFF00;
                aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.protocol.value & 0xFF);
                aclRule[tempIdx].care_bits.field[fieldIdx] &= 0xFF00;
                aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.protocol.mask  & 0xFF) ;
            }
            break;


        case FILTER_FIELD_ARP_RARP_CODE:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.arpRarpOpcode.value & 0xFFFF);
                aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.arpRarpOpcode.mask  & 0xFFFF) ;
            }
            break;

        case FILTER_FIELD_AFTER_ETHTYPE_BYTE01:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.afterEthertypeByte0_1.value & 0xFFFF);
                aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.afterEthertypeByte0_1.mask  & 0xFFFF) ;
            }
            break;

        case FILTER_FIELD_IPV6_TRAFFIC_CLASS:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] &= 0xFF; 
                aclRule[tempIdx].data_bits.field[fieldIdx] |= ((fieldPtr->filter_pattern_union.ipv6TrafficClass.value & 0xFF) << 8);
                aclRule[tempIdx].care_bits.field[fieldIdx] &= 0xFF;
                aclRule[tempIdx].care_bits.field[fieldIdx] |= ((fieldPtr->filter_pattern_union.ipv6TrafficClass.mask & 0xFF) << 8);
            }
            break;

        case FILTER_FIELD_IPV6_NEXT_HEADER:
            for(i = 0; i < fieldPtr->occupyFieldNum; i++)
            {
                tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;

                aclRule[tempIdx].data_bits.field[fieldIdx] &= 0xFF00;
                aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.ipv6NextHeader.value & 0xFF);
                aclRule[tempIdx].care_bits.field[fieldIdx] &= 0xFF00;
                aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.ipv6NextHeader.mask  & 0xFF) ;
            }
            break;
        case FILTER_FIELD_TCP_UDP_SPORT:
              for(i = 0; i < fieldPtr->occupyFieldNum; i++)
              {
                  tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                  fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                  aclRule[tempIdx].data_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.tcpUdpSrcPort.value & 0xFFFF);
                  aclRule[tempIdx].care_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.tcpUdpSrcPort.mask & 0xFFFF);
              }
              break;
          case FILTER_FIELD_TCP_UDP_DPORT:
              for(i = 0; i < fieldPtr->occupyFieldNum; i++)
              {
                  tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                  fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                  aclRule[tempIdx].data_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.tcpUdpDstPort.value & 0xFFFF);
                  aclRule[tempIdx].care_bits.field[fieldIdx] = (fieldPtr->filter_pattern_union.tcpUdpDstPort.mask & 0xFFFF);
              }
              break;
          case FILTER_FIELD_ICMP_IGMP_CODE:
              for(i = 0; i < fieldPtr->occupyFieldNum; i++)
              {
                  tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                  fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                  aclRule[tempIdx].data_bits.field[fieldIdx] &= 0xFF00;
                  aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.icmpIgmpCode.value & 0xFF);
                  aclRule[tempIdx].care_bits.field[fieldIdx] &= 0xFF00;
                  aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.icmpIgmpCode.mask & 0xFF);
              }
              break;
          case FILTER_FIELD_ICMP_IGMP_TYPE:
              for(i = 0; i < fieldPtr->occupyFieldNum; i++)
              {
                  tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                  fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                  aclRule[tempIdx].data_bits.field[fieldIdx] &= 0x00FF;
                  aclRule[tempIdx].data_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.icmpIgmpType.value << 8);
                  aclRule[tempIdx].care_bits.field[fieldIdx] &= 0x00FF;
                  aclRule[tempIdx].care_bits.field[fieldIdx] |= (fieldPtr->filter_pattern_union.icmpIgmpType.mask << 8);
              }
              break;
          case FILTER_FIELD_L4HEADER_BYTE01:
              for(i = 0; i < fieldPtr->occupyFieldNum; i++)
              {
                  tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                  fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                  aclRule[tempIdx].data_bits.field[fieldIdx] = ((fieldPtr->filter_pattern_union.l4headerByte0_1.value) & 0xFFFF);
                  aclRule[tempIdx].care_bits.field[fieldIdx] = ((fieldPtr->filter_pattern_union.l4headerByte0_1.mask) & 0xFFFF);
              }
              break;
           case FILTER_FIELD_L4HEADER_BYTE23:
              for(i = 0; i < fieldPtr->occupyFieldNum; i++)
              {
                  tempIdx = (fieldPtr->fieldTemplateIdx[i] & 0xF0) >> 4;
                  fieldIdx = fieldPtr->fieldTemplateIdx[i] & 0x0F;
        
                  aclRule[tempIdx].data_bits.field[fieldIdx] = ((fieldPtr->filter_pattern_union.l4headerByte2_3.value) & 0xFFFF);
                  aclRule[tempIdx].care_bits.field[fieldIdx] = ((fieldPtr->filter_pattern_union.l4headerByte2_3.mask ) & 0xFFFF);
              }
              break;

        case FILTER_FIELD_PATTERN_MATCH:
        case FILTER_FIELD_VID_RANGE:
        case FILTER_FIELD_IP_RANGE:
        case FILTER_FIELD_PORT_RANGE:
        default:
            tempIdx = (fieldPtr->fieldTemplateIdx[0] & 0xF0) >> 4;
            fieldIdx = fieldPtr->fieldTemplateIdx[0] & 0x0F;

            aclRule[tempIdx].data_bits.field[fieldIdx] = fieldPtr->filter_pattern_union.inData.value;
            aclRule[tempIdx].care_bits.field[fieldIdx] = fieldPtr->filter_pattern_union.inData.mask;
            break;
    }
    return RT_ERR_OK;
}

rtk_api_ret_t _rtl8373_getAclRule(rtk_uint32 index, rtl8373_acl_rule_t *pAclRule)
{
    rtl8373_aclRule_smi_t aclRuleSmi;
    rtk_uint32 regAddr = 0, regData = 0;
    rtk_uint32* tableAddr = NULL;
    rtk_uint32 i = 0;

    if(index > RTL8373_ACLRULEMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&aclRuleSmi, 0x00, sizeof(rtl8373_aclRule_smi_t));

    
    /* Write ACS_ADR register for carebits*/
    regAddr = RTL8373_ITA_CTRL0_ADDR;

    /*prepare control Data*/
    regData = RTL8373_ACLRULETBADDR(RTL8373_CAREBITS, index) << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLRULE << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_READ << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE<< RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;

    /*Trigger*/
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr,  regData));
    /*wait access finished */
    do{
        RTK_ERR_CHK(rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData));
    }while(regData);

    /* Read Care Bits */
    tableAddr = (rtk_uint32*)&aclRuleSmi.care_bits;
    for(i = 0; i < RTL8373_ACL_RULE_ENTRY_LEN; i++)
    {
        RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData));

        *tableAddr = regData;
        tableAddr++;
    }
    
    /* Write ACS_ADR register for data bits */
    regAddr = RTL8373_ITA_CTRL0_ADDR;

    /*prepare control Data*/
    regData = RTL8373_ACLRULETBADDR(RTL8373_DATABITS, index) << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLRULE << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_READ << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE<< RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;

    /*Trigger*/
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, regData));
    /*wait access finished */
    do{
        RTK_ERR_CHK(rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData));
    }while(regData);

    /* Read Data Bits */
    tableAddr = (rtk_uint32*)&aclRuleSmi.data_bits;
    for(i = 0; i < RTL8373_ACL_RULE_ENTRY_LEN; i++)
    {
        RTK_ERR_CHK(rtl8373_getAsicReg( RTL8373_ITA_READ_DATA0_ADDR(i), &regData));

        *tableAddr = regData;
         tableAddr++;
         
    }

    /* Read Valid Bit */
    
    aclRuleSmi.valid = (regData >> RTL8373_ACLRULE_VALIDBIT_OFFSET) & 0x1;


#ifdef CONFIG_RTL8373_ASICDRV_TEST
    memcpy(&aclRuleSmi,&Rtl8370sVirtualAclRuleTable[index], sizeof(rtl8373_aclRule_smi_t));
#endif
    
         _rtl8373_aclRuleStSmi2User(pAclRule, &aclRuleSmi);

    return RT_ERR_OK;
}

rtk_api_ret_t _rtl8373_setAclRule(rtk_uint32 index, rtl8373_acl_rule_t* pAclRule)
{
    rtl8373_aclRule_smi_t aclRuleSmi;
    rtk_uint32* tableAddr;
    rtk_uint32 regAddr;
    rtk_uint32  regData;
    rtk_uint32 i;

    if(index > RTL8373_ACLRULEMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(&aclRuleSmi, 0x00, sizeof(rtl8373_aclRule_smi_t));

    _rtl8373_aclRuleStUser2Smi(pAclRule, &aclRuleSmi);


    /*----- Write Valid Bit = 0 -----*/
    /* write data */
    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_ITA_WRITE_DATA0_ADDR(4), RTL8373_ACLRULE_VALIDBIT_OFFSET, INVALID));

    /* Prepare Control register */    
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    regData = RTL8373_ACLRULETBADDR(RTL8373_DATABITS, index) << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLRULE << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_WRITE<< RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE<< RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;

    /*Trigger*/
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, regData));

    /*wait access finished */
    do{
        RTK_ERR_CHK(rtl8373_getAsicReg(regAddr,  &regData));
    }while(regData & 0x1);

    /*----- Write care bit register -----*/
    /* Write Care Bits to ACS_DATA registers */
     tableAddr = (rtk_uint32*)&aclRuleSmi.care_bits;
    for(i = 0; i < RTL8373_ACL_RULE_ENTRY_LEN; i++)
    {
        regData = *tableAddr;
        
        RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ITA_WRITE_DATA0_ADDR(i), regData));
        tableAddr++;
    }
    
    /* Prepare Control register */
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    regData = RTL8373_ACLRULETBADDR(RTL8373_CAREBITS, index) << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLRULE << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_WRITE<< RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE<< RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;
    /*Trigger*/
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr,regData));

    /*wait access finished */
    do{
        RTK_ERR_CHK(rtl8373_getAsicReg(regAddr , &regData));
    }while(regData & 1);

 
    /*----- Write  ACL data bits -----*/
    /* Write Data Bits  to ACS_DATA registers */
     tableAddr = (rtk_uint32*)&aclRuleSmi.data_bits;

    for(i = 0; i < RTL8373_ACL_RULE_ENTRY_LEN; i++)
    {
        regData = *tableAddr;
        
        RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ITA_WRITE_DATA0_ADDR(i), regData));
        tableAddr++;
    }
    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_ITA_WRITE_DATA0_ADDR(4), RTL8373_ACLRULE_VALIDBIT_OFFSET, aclRuleSmi.valid));
    
    /* Prepare Control register */
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    regData = RTL8373_ACLRULETBADDR(RTL8373_DATABITS, index) << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLRULE << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_WRITE<< RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE<< RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;
    /*Trigger*/
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, regData));

    /*wait access finished */
    do{
        RTK_ERR_CHK(rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData));
    }while(regData);


#ifdef CONFIG_RTL8373_ASICDRV_TEST
    memcpy(&Rtl8370sVirtualAclRuleTable[index], &aclRuleSmi, sizeof(rtl8373_aclRule_smi_t));
#endif

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtl8373_getAclAct(rtk_uint32 index, rtl8373_acl_act_t *pAclAct)
{
    rtk_uint32 aclActSmi[RTL8373_ACL_ACT_ENTRY_LEN];
    rtk_uint32 regAddr, regData;
    rtk_uint32 *tableAddr;
    rtk_uint32 i;

    if(index > RTL8373_ACLRULEMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(aclActSmi, 0x00, sizeof(rtk_uint32) * RTL8373_ACL_ACT_ENTRY_LEN);

    /* Prepare Control register */    
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    regData =  index << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLACT << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_READ<< RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE << RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;
     /*Trigger*/
     RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, regData));
    
     /*wait access finished */
     do{
        RTK_ERR_CHK(rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData));
     }while(regData);

    /* Read Data Bits */
    tableAddr = aclActSmi;
    for(i = 0; i < RTL8373_ACL_ACT_ENTRY_LEN; i++)
    {
        RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(i), &regData));
        *tableAddr = regData;
        tableAddr ++;
    }

#ifdef CONFIG_RTL8373_ASICDRV_TEST
    memcpy(aclActSmi, &Rtl8370sVirtualAclActTable[index][0], sizeof(rtk_uint32) * RTL8373_ACL_ACT_ENTRY_LEN);
#endif

     _rtl8373_aclActStSmi2User(pAclAct, aclActSmi);

    return RT_ERR_OK;
}

static rtk_api_ret_t _rtl8373_setAclAct(rtk_uint32 index, rtl8373_acl_act_t* pAclAct)
{
    rtk_uint32 aclActSmi[RTL8373_ACL_ACT_ENTRY_LEN];
    rtk_uint32 regAddr, regData;
    rtk_uint32* tableAddr;
    rtk_uint32 i;

    if(index > RTL8373_ACLRULEMAX)
        return RT_ERR_OUT_OF_RANGE;

    memset(aclActSmi, 0x00, sizeof(rtk_uint32) * RTL8373_ACL_ACT_ENTRY_LEN);
    _rtl8373_aclActStUser2Smi(pAclAct, aclActSmi);

    /* Write Data Bits to ACS_DATA registers */
    tableAddr = aclActSmi;
    for(i = 0; i < RTL8373_ACL_ACT_ENTRY_LEN; i++)
    {
        regData = *tableAddr;
        RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ITA_WRITE_DATA0_ADDR(i) , regData));
        tableAddr++;
        
    }
    
    /* Prepare Control register */    
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    regData =  index << RTL8373_ITA_CTRL0_TBL_ADDR_OFFSET;
    regData |= TB_TARGET_ACLACT << RTL8373_ITA_CTRL0_TLB_TYPE_OFFSET;
    regData |= TB_OP_WRITE << RTL8373_ITA_CTRL0_TLB_ACT_OFFSET;
    regData |= TB_EXECUTE << RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET;

    /*Trigger*/
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, regData));

    /*wait access finished */
    do{
      RTK_ERR_CHK(rtl8373_getAsicRegBit(regAddr, RTL8373_ITA_CTRL0_TLB_EXECUTE_OFFSET , &regData));
    }while(regData);

#ifdef CONFIG_RTL8373_ASICDRV_TEST
    memcpy(&Rtl8370sVirtualAclActTable[index][0], aclActSmi, sizeof(rtk_uint32) * RTL8373_ACL_ACT_ENTRY_LEN);
#endif

    return RT_ERR_OK;
}

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
 *      RT_ERR_NULL_POINTER - Pointer pFilter_field or pFilterCfg point to NULL.
 * Note:
 *      This function enable and intialize ACL function
 */
rtk_api_ret_t dal_rtl8373_igrAcl_init(void)
{
    rtl8373_acl_template_t       aclTemp;
    rtk_uint32                 i, j;
    rtk_api_ret_t          ret;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((ret = dal_rtl8373_igrAcl_cfg_delAll()) != RT_ERR_OK)
        return ret;

    /*init template*/
    for(i = 0; i < RTL8373_ACLTEMPLATENO; i++)
    {
        for(j = 0; j < RTL8373_ACLRULEFIELDNO;j++)
            aclTemp.field[j] = rtl8373_filter_templateField[i][j];

        if ((ret = _rtl8373_setAclTemplate(i, &aclTemp)) != RT_ERR_OK)
            return ret;
    }

    /*init hsb field selector*/
    /*
    for(i = 0; i < RTL8373_FIELDSEL_FORMAT_NUMBER; i++)
    {
        regData = (((rtl8373_field_selector[i][0] << RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_OFFSET) & RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_MASK ) |
                   ((rtl8373_field_selector[i][1] << RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_OFFSET) & RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_MASK ));

        if ((ret = rtl8373_setAsicReg(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(i), regData)) != RT_ERR_OK)
            return ret;
    }
*/
    RTK_SCAN_ALL_PHY_PORTMASK(i)
    {

        if ((ret = rtl8373_setAsicRegBit(RTL8373_ACL_PORT_EN_ADDR, i, TRUE)) != RT_ERR_OK)
            return ret;

        if ((ret = rtl8373_setAsicRegBit(RTL8373_ACL_PORT_UNMATCH_PERMIT_ADDR, i, TRUE)) != RT_ERR_OK)
            return ret;
    }

#ifdef CONFIG_RTL8373_ASICDRV_TEST
    for(i=0;i<RTL8373_ACLRULENO;i++)
        memset(&Rtl8370sVirtualAclRuleTable[i],0x00, sizeof(rtl8373_aclRule_smi_t));
#endif
    return RT_ERR_OK;
}

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
 *      RT_ERR_NULL_POINTER     - Pointer pFilter_field or pFilterCfg point to NULL.
 *      RT_ERR_INPUT            - Invalid input parameters.
 * Note:
 *      This function add a comparison rule (*pFilter_field) to an ACL configuration (*pFilterCfg).
 *      Pointer pFilterCfg points to an ACL configuration structure, this structure keeps multiple ACL
 *      comparison rules by means of linked list. Pointer pFilter_field will be added to linked
 *      list keeped by structure that pFilterCfg points to.
 */
rtk_api_ret_t dal_rtl8373_igrAcl_field_add(rtk_filter_cfg_t* pFilterCfg, rtk_filter_field_t* pFilterField)
{
    rtk_uint32 i;
    rtk_filter_field_t *tailPtr;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pFilterCfg || NULL == pFilterField)
        return RT_ERR_NULL_POINTER;

    if(pFilterField->fieldType >= FILTER_FIELD_END)
        return RT_ERR_ENTRY_INDEX;


    if(0 == pFilterField->occupyFieldNum)
    {
        pFilterField->occupyFieldNum = rtl8373_filter_fieldSize[pFilterField->fieldType];

        for(i = 0; i < pFilterField->occupyFieldNum; i++)
        {
            pFilterField->fieldTemplateIdx[i] = rtl8373_filter_fieldTemplateIndex[pFilterField->fieldType][i];
        }
    }

    if(NULL == pFilterCfg->fieldHead)
    {
        pFilterCfg->fieldHead = pFilterField;
    }
    else
    {
        if (pFilterCfg->fieldHead->next == NULL)
        {
            pFilterCfg->fieldHead->next = pFilterField;
        }
        else
        {
            tailPtr = pFilterCfg->fieldHead->next;
            while( tailPtr->next != NULL)
            {
                tailPtr = tailPtr->next;
            }
            tailPtr->next = pFilterField;
        }
    }

    return RT_ERR_OK;
}


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
 *      RT_ERR_NULL_POINTER                     - Pointer pFilter_field or pFilterCfg point to NULL.
 *      RT_ERR_INPUT                            - Invalid input parameters.
 *      RT_ERR_ENTRY_INDEX                      - Invalid filterIdx .
 *      RT_ERR_NULL_POINTER                     - Pointer pAction or pFilterCfg point to NULL.
 *      RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT     - Action is not supported in this chip.
 *      RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT    - Rule is not supported.
 * Note:
 *      This function store pFilterCfg, pAction into ASIC. The starting
 *      index(es) is filterIdx.
 */
rtk_api_ret_t dal_rtl8373_igrAcl_cfg_add(rtk_filter_id_t filterIdx, rtk_filter_cfg_t* pFilterCfg, rtk_filter_action_t* pAction, rtk_filter_number_t *ruleNum)
{
    rtk_api_ret_t               retVal = 0;
    rtk_uint32                  careTagData = 0, careTagMask = 0;
    rtk_uint32                  i = 0, actType = 0, ruleId = 0;
    rtk_uint32                  aclActCtrl = 0;
    rtk_filter_field_t*         fieldPtr;
    rtl8373_acl_rule_t        aclRule[RTL8373_ACLTEMPLATENO];
    rtl8373_acl_rule_t        tempRule;
    rtl8373_acl_act_t         aclAct ;
    rtk_uint32                  noRulesAdd = 0;
    rtk_uint32                  portmask = 0;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(filterIdx > RTL8373_ACLRULEMAX )
        return RT_ERR_ENTRY_INDEX;

    if((NULL == pFilterCfg) || (NULL == pAction) || (NULL == ruleNum))
        return RT_ERR_NULL_POINTER;

    fieldPtr = pFilterCfg->fieldHead;

    /* init RULE */
    for(i = 0; i < RTL8373_ACLTEMPLATENO; i++)
    {
        memset(&aclRule[i], 0, sizeof(rtl8373_acl_rule_t));

        aclRule[i].data_bits.templateIdx = i;
        aclRule[i].care_bits.templateIdx = RTL8373_ACLTEMPLATE_MASK;
    }
    
    while(NULL != fieldPtr)
    {
        _rtk_igrAcl_writeDataField(aclRule, fieldPtr);

        fieldPtr = fieldPtr->next;
    }

    /*Check rule number*/
    noRulesAdd = 0;
    for(i = 0; i < RTL8373_ACLTEMPLATENO; i++)
    {
        if(1 == aclRule[i].valid)
        {
            noRulesAdd ++;
        }
    }

    *ruleNum = noRulesAdd;

    if((filterIdx + noRulesAdd - 1) > RTL8373_ACLRULEMAX)
    {
        return RT_ERR_ENTRY_INDEX;
    }

    /*set care tag mask in TAG Indicator*/
    careTagData = 0;
    careTagMask = 0;

    for(i = 0; i <= CARE_TAG_IPV6;i++)
    {
        if(0 == pFilterCfg->careTag.tagType[i].mask )
        {
            careTagMask &=  ~(1 << i);
        }
        else
        {
            careTagMask |= (1 << i);
            if(0 == pFilterCfg->careTag.tagType[i].value )
                careTagData &= ~(1 << i);
            else
                careTagData |= (1 << i);
        }
    }

    for(i = 0; i < RTL8373_ACLTEMPLATENO; i++)
    {
        aclRule[i].data_bits.tagPppoe= (careTagData) & ACL_RULE_TAG_MASK;
        aclRule[i].care_bits.tagPppoe = (careTagMask) & ACL_RULE_TAG_MASK;
    }

    RTK_CHK_PORTMASK_VALID(&pFilterCfg->activeport.value);
    RTK_CHK_PORTMASK_VALID(&pFilterCfg->activeport.mask);

    for(i = 0; i < RTL8373_ACLTEMPLATENO; i++)
    {
        if(TRUE == aclRule[i].valid)
        {
            if(rtk_switch_portmask_L2P_get(&pFilterCfg->activeport.value, &portmask) != RT_ERR_OK)
                return RT_ERR_PORT_MASK;

            aclRule[i].data_bits.activePmsk = portmask;

            if(rtk_switch_portmask_L2P_get(&pFilterCfg->activeport.mask, &portmask) != RT_ERR_OK)
                return RT_ERR_PORT_MASK;

            aclRule[i].care_bits.activePmsk = portmask;
            
            aclRule[i].data_bits.l3fmt = (pFilterCfg->l3fmt.value & ACL_RULE_L3FMT_MASK);
            aclRule[i].care_bits.l3fmt = (pFilterCfg->l3fmt.mask & ACL_RULE_L3FMT_MASK);
            aclRule[i].data_bits.l4fmt = (pFilterCfg->l4fmt.value & ACL_RULE_L4FMT_MASK);
            aclRule[i].care_bits.l4fmt = (pFilterCfg->l4fmt.mask & ACL_RULE_L4FMT_MASK);
        }
    }

    if(pFilterCfg->invert >= FILTER_INVERT_END )
        return RT_ERR_INPUT;


    /*Last action gets high priority if actions are the same*/
    memset(&aclAct, 0, sizeof(rtl8373_acl_act_t));
    aclActCtrl = 0;
    for(actType = 0; actType < FILTER_ENACT_END; actType ++)
    {
        if(pAction->actEnable[actType])
        {
            switch (actType)
            {
            case FILTER_ENACT_CVLAN_INGRESS:
                if(pAction->filterCvlanVid > RTL8373_VIDMAX)
                    return RT_ERR_INPUT;

                aclAct.cact = FILTER_ENACT_CVLAN_TYPE(actType);
                aclAct.cvid = pAction->filterCvlanVid;

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_TAGONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_VLANONLY;
                }

                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
            case FILTER_ENACT_CVLAN_EGRESS:
                if(pAction->filterCvlanVid > RTL8373_VIDMAX)
                    return RT_ERR_INPUT;

                aclAct.cact = FILTER_ENACT_CVLAN_TYPE(actType);
                aclAct.cvid = pAction->filterCvlanVid;

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_TAGONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_VLANONLY;
                }

                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
             case FILTER_ENACT_CVLAN_SVID:

                aclAct.cact = FILTER_ENACT_CVLAN_TYPE(actType);

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_TAGONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_VLANONLY;
                }

                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
             case FILTER_ENACT_POLICING_1:
                if(pAction->filterPolicingIdx[1] > ( RTL8373_METERMAX ) )
                    return RT_ERR_INPUT;

                aclAct.cact = FILTER_ENACT_CVLAN_TYPE(actType);
                aclAct.cvid = pAction->filterPolicingIdx[1];

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_TAGONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_VLANONLY;
                }

                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;

            case FILTER_ENACT_SVLAN_INGRESS:
            case FILTER_ENACT_SVLAN_EGRESS:
                aclAct.sact = FILTER_ENACT_SVLAN_TYPE(actType);
                aclAct.svid = pAction->filterSvlanVid;
                aclActCtrl |= FILTER_ENACT_SVLAN_MASK;
                break;

            case FILTER_ENACT_SVLAN_CVID:
                aclAct.sact = FILTER_ENACT_SVLAN_TYPE(actType);
                aclActCtrl |= FILTER_ENACT_SVLAN_MASK;
                break;
            case FILTER_ENACT_POLICING_2:
                if(pAction->filterPolicingIdx[2] > (RTL8373_METERMAX))
                    return RT_ERR_INPUT;

                aclAct.sact = FILTER_ENACT_SVLAN_TYPE(actType);
                aclAct.svid = pAction->filterPolicingIdx[2];
                aclActCtrl |= FILTER_ENACT_SVLAN_MASK;
                break;
            case FILTER_ENACT_POLICING_0:
                if(pAction->filterPolicingIdx[0] > (RTL8373_METERMAX))
                    return RT_ERR_INPUT;

                aclAct.aclMeterLoggIdx = pAction->filterPolicingIdx[0];
                aclAct.aclPolicingLogAct = FALSE;
                aclActCtrl |= FILTER_ENACT_POLICING_LOGG_MASK;
                break;
            case FILTER_ENACT_PRIORITY:
                if(pAction->filterAclPri > RTL8373_PRIMAX)
                    return RT_ERR_INPUT;

                aclAct.aclPri = pAction->filterAclPri;
                aclActCtrl |= FILTER_ENACT_PRIORITY_MASK;
                break;

            case FILTER_ENACT_1P_RMK:
                if(pAction->filter1pRmk > RTL8373_PRIMAX)
                    return RT_ERR_INPUT;

                aclAct.aclRmkVal = pAction->filter1pRmk;
                aclAct.aclRmkAct = FALSE;
                aclActCtrl |= FILTER_ENACT_RMK_MASK;
                break;

            case FILTER_ENACT_DSCP_RMK:
                if(pAction->filterDscpRmk > RTL8373_DSCPMAX)
                    return RT_ERR_INPUT;

                aclAct.aclRmkVal = pAction->filterDscpRmk;
                aclAct.aclRmkAct = TRUE;
                aclActCtrl |= FILTER_ENACT_RMK_MASK;
                break;

            case FILTER_ENACT_LOGGING_CNTR:
                if(pAction->filterLoggCntr> RTL8373_LOGGINGMAX)
                    return RT_ERR_INPUT;

                aclAct.aclMeterLoggIdx = pAction->filterLoggCntr;
                aclAct.aclPolicingLogAct = TRUE;                
                aclActCtrl |= FILTER_ENACT_POLICING_LOGG_MASK;
                break;

            case FILTER_ENACT_ADD_DSTPORT:
                RTK_CHK_PORTMASK_VALID(&pAction->filterPortmask);

                aclAct.fwdAct = FILTER_ENACT_FWD_TYPE(actType);
                aclAct.fwdActExt = FALSE;

                if(rtk_switch_portmask_L2P_get(&pAction->filterPortmask, &portmask) != RT_ERR_OK)
                    return RT_ERR_PORT_MASK;
                aclAct.fwdPmsk = portmask;

                aclActCtrl |= FILTER_ENACT_FWD_MASK;
                break;

            case FILTER_ENACT_REDIRECT:
                RTK_CHK_PORTMASK_VALID(&pAction->filterPortmask);

                aclAct.fwdAct = FILTER_ENACT_FWD_TYPE(actType);
                aclAct.fwdActExt = FALSE;

                if(rtk_switch_portmask_L2P_get(&pAction->filterPortmask, &portmask) != RT_ERR_OK)
                    return RT_ERR_PORT_MASK;
                aclAct.fwdPmsk = portmask;

                aclActCtrl |= FILTER_ENACT_FWD_MASK;
                break;
                
            case FILTER_ENACT_DROP:            
                aclAct.fwdAct = FILTER_ENACT_FWD_TYPE(FILTER_ENACT_REDIRECT);
                aclAct.fwdActExt = FALSE;
            
                aclAct.fwdPmsk = 0;
                aclActCtrl |= FILTER_ENACT_FWD_MASK;
                break;

            case FILTER_ENACT_MIRROR:
                RTK_CHK_PORTMASK_VALID(&pAction->filterPortmask);

                aclAct.fwdAct = FILTER_ENACT_FWD_TYPE(actType);
                aclAct.fwdActExt = FALSE;

                if(rtk_switch_portmask_L2P_get(&pAction->filterPortmask, &portmask) != RT_ERR_OK)
                    return RT_ERR_PORT_MASK;
                aclAct.fwdPmsk = portmask;

                aclActCtrl |= FILTER_ENACT_FWD_MASK;
                break;

            case FILTER_ENACT_TRAP_INT_CPU:
            case FILTER_ENACT_TRAP_EXT_CPU:
            case FILTER_ENACT_TRAP_INT_EXT_CPU:
                aclAct.fwdAct = FILTER_ENACT_FWD_TYPE(actType);
                aclAct.fwdActExt = FALSE;

                aclAct.fwdPmsk = 0;
                aclActCtrl |= FILTER_ENACT_FWD_MASK;
                break;       
                
            case FILTER_ENACT_ISOLATION:
                RTK_CHK_PORTMASK_VALID(&pAction->filterPortmask);

                aclAct.fwdActExt = TRUE;

                if(rtk_switch_portmask_L2P_get(&pAction->filterPortmask, &portmask) != RT_ERR_OK)
                    return RT_ERR_PORT_MASK;
                aclAct.fwdPmsk = portmask;

                aclActCtrl |= FILTER_ENACT_FWD_MASK;
                break;

            case FILTER_ENACT_INTERRUPT:
                aclAct.aclInt = TRUE;
                aclActCtrl |= FILTER_ENACT_INTGPIO_MASK;
                break;
                
            case FILTER_ENACT_GPO:
                aclAct.gpioEn = TRUE;
                aclAct.gpioPin = pAction->filterPin;
                aclActCtrl |= FILTER_ENACT_INTGPIO_MASK;
                break;

            case FILTER_ENACT_BYPASS_IGRBW_STORMCTRL:
                aclAct.bypassAct |= ACL_BYPASS_IGRBW_STORM_MASK;
                aclActCtrl |= FILTER_ENACT_BYPASS_MASK;
                break;

            case FILTER_ENACT_BYPASS_STP_SRC_CHK:
                aclAct.bypassAct |= ACL_BYPASS_STP_SRC_CHK_MASK;
                aclActCtrl |= FILTER_ENACT_BYPASS_MASK;
                break;

            case FILTER_ENACT_BYPASS_IGRVLAN_FLTR:
                aclAct.bypassAct |= ACL_BYPASS_IGRVLAN_FLTR_MASK;
                aclActCtrl |= FILTER_ENACT_BYPASS_MASK;
                break;

             case FILTER_ENACT_EGRESSCTAG_TAG:
                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_VLANONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_TAGONLY;
                }
                aclAct.tagFmt = FILTER_CTAGFMT_TAG;
                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
             case FILTER_ENACT_EGRESSCTAG_UNTAG:

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_VLANONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_TAGONLY;
                }
                aclAct.tagFmt = FILTER_CTAGFMT_UNTAG;
                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
             case FILTER_ENACT_EGRESSCTAG_REALKEEP:

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_VLANONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_TAGONLY;
                }
                aclAct.tagFmt = FILTER_CTAGFMT_KEEP;
                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
             case FILTER_ENACT_EGRESSCTAG_KEEPAND1PRMK:

                if(aclActCtrl &(FILTER_ENACT_CVLAN_MASK))
                {
                    if(aclAct.cactExt == FILTER_ENACT_CACTEXT_VLANONLY)
                        aclAct.cactExt = FILTER_ENACT_CACTEXT_BOTHVLANTAG;
                }
                else
                {
                    aclAct.cactExt = FILTER_ENACT_CACTEXT_TAGONLY;
                }
                aclAct.tagFmt = FILTER_CTAGFMT_KEEP1PRMK;
                aclActCtrl |= FILTER_ENACT_CVLAN_MASK;
                break;
            default:
                return RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT;
            }
        }
    }

    /*check if free ACL rules are enough*/
    for(i = filterIdx; i < (filterIdx + noRulesAdd); i++)
    {
        if((retVal = _rtl8373_getAclRule(i, &tempRule)) != RT_ERR_OK )
            return retVal;

        if(tempRule.valid == TRUE)
        {
            return RT_ERR_TBL_FULL;
        }
    }
    ruleId = 0;
    for(i = 0; i < RTL8373_ACLTEMPLATENO; i++)
    {
        if(aclRule[i].valid == TRUE)
        {
            /* write ACL action control */
            RTK_ERR_CHK(rtl8373_setAsicRegBits(RTL8373_ACL_ACT_CTRL_ADDR(filterIdx+ruleId) , FILTER_ENACT_ALL_MASK, aclActCtrl));

            /* write ACL action */
            if((retVal = _rtl8373_setAclAct(filterIdx + ruleId, &aclAct)) != RT_ERR_OK )
                return retVal;

            /* write ACL not */     
            RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_ACL_ACT_CTRL_ADDR(filterIdx+ruleId) , RTL8373_ACL_ACT_CTRL_NOT_OFFSET, pFilterCfg->invert));
            
            /* write ACL rule */
            if((retVal = _rtl8373_setAclRule(filterIdx + ruleId, &aclRule[i])) != RT_ERR_OK )
                return retVal;

            /* only the first rule will be written with input action control, aclActCtrl of other rules will be zero */
            aclActCtrl = 0;
            memset(&aclAct, 0, sizeof(rtl8373_acl_act_t));

            ruleId ++;
        }
    }

    return RT_ERR_OK;
}

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
 *      RT_ERR_NULL_POINTER     - Pointer pAction or pFilterCfg point to NULL.
 *      RT_ERR_FILTER_ENTRYIDX  - Invalid entry index.
 * Note:
 *      This function get configuration from ASIC.
 */
rtk_api_ret_t dal_rtl8373_igrAcl_cfg_get(rtk_filter_id_t filterIdx, rtk_filter_cfg_raw_t *pFilterCfg, rtk_filter_action_t *pAction)
{
    rtk_api_ret_t               retVal = 0;
    rtk_uint32                  i = 0, tmp = 0, actCtrlBits = 0;
    rtl8373_acl_rule_t           aclRule;
    rtl8373_acl_act_t          aclAct;
    rtl8373_acl_template_t      type;
    rtk_uint32                  phyPmask = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pFilterCfg || NULL == pAction)
        return RT_ERR_NULL_POINTER;

    if(filterIdx > RTL8373_ACLRULEMAX)
        return RT_ERR_ENTRY_INDEX;

    if ((retVal = _rtl8373_getAclRule(filterIdx, &aclRule)) != RT_ERR_OK)
        return retVal;

    /* Check valid */
    if(aclRule.valid == INVALID)
    {
        pFilterCfg->valid = DISABLED;
        return RT_ERR_OK;
    }

    pFilterCfg->activeport.value.bits[0] = aclRule.data_bits.activePmsk;
    pFilterCfg->activeport.mask.bits[0] = aclRule.care_bits.activePmsk;

    for(i = 0; i <= CARE_TAG_PPPOE; i++)
    {
        if(aclRule.data_bits.tagPppoe & (1 << i))
            pFilterCfg->careTag.tagType[i].value = 1;
        else
            pFilterCfg->careTag.tagType[i].value = 0;

        if (aclRule.care_bits.tagPppoe & (1 << i))
            pFilterCfg->careTag.tagType[i].mask = 1;
        else
            pFilterCfg->careTag.tagType[i].mask = 0;
    }

    pFilterCfg->l3fmt.value = aclRule.data_bits.l3fmt;
    pFilterCfg->l3fmt.mask = aclRule.care_bits.l3fmt;

    pFilterCfg->l4fmt.value = aclRule.data_bits.l4fmt;
    pFilterCfg->l4fmt.mask = aclRule.care_bits.l4fmt;

    for(i = 0; i < RTL8373_ACLRULEFIELDNO; i++)
    {
        pFilterCfg->careFieldRaw[i] = aclRule.care_bits.field[i];
        pFilterCfg->dataFieldRaw[i] = aclRule.data_bits.field[i];
    }
    
    /*get not bit*/
    RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_ACL_ACT_CTRL_ADDR(filterIdx) , RTL8373_ACL_ACT_CTRL_NOT_OFFSET, &tmp));

    pFilterCfg->invert = tmp;
    pFilterCfg->valid = aclRule.valid;

    memset(pAction, 0, sizeof(rtk_filter_action_t));

    /*get action control bits*/
    RTK_ERR_CHK(rtl8373_getAsicRegBits(RTL8373_ACL_ACT_CTRL_ADDR(filterIdx) , FILTER_ENACT_ALL_MASK, &actCtrlBits));
    if ((retVal =_rtl8373_getAclAct(filterIdx, &aclAct)) != RT_ERR_OK)
        return retVal;

    if(actCtrlBits & FILTER_ENACT_FWD_MASK)
    {
        if(TRUE == aclAct.fwdActExt)
        {
            pAction->actEnable[FILTER_ENACT_ISOLATION] = TRUE;

            phyPmask = aclAct.fwdPmsk;
            if(rtk_switch_portmask_P2L_get(phyPmask,&(pAction->filterPortmask)) != RT_ERR_OK)
                return RT_ERR_FAILED;
        }
        else 
        {       
            if(aclAct.fwdAct == RTL8373_ACL_FWD_INT_TRAP)
            {
                pAction->actEnable[FILTER_ENACT_TRAP_INT_CPU] = TRUE;
            }
            else if(aclAct.fwdAct == RTL8373_ACL_FWD_EXT_TRAP)
            {
                pAction->actEnable[FILTER_ENACT_TRAP_EXT_CPU] = TRUE;
            }
            else if(aclAct.fwdAct == RTL8373_ACL_FWD_INT_EXT_TRAP)
            {
                pAction->actEnable[FILTER_ENACT_TRAP_INT_EXT_CPU] = TRUE;
            }
            else if (aclAct.fwdAct == RTL8373_ACL_FWD_MIRROR )
            {
                pAction->actEnable[FILTER_ENACT_MIRROR] = TRUE;

                phyPmask = aclAct.fwdPmsk;
                if(rtk_switch_portmask_P2L_get(phyPmask,&(pAction->filterPortmask)) != RT_ERR_OK)
                    return RT_ERR_FAILED;
            }
            else if (aclAct.fwdAct == RTL8373_ACL_FWD_REDIRECT)
            {
                if(aclAct.fwdPmsk == 0 )
                    pAction->actEnable[FILTER_ENACT_DROP] = TRUE;
                else
                {
                    pAction->actEnable[FILTER_ENACT_REDIRECT] = TRUE;

                    phyPmask = aclAct.fwdPmsk;
                    if(rtk_switch_portmask_P2L_get(phyPmask,&(pAction->filterPortmask)) != RT_ERR_OK)
                        return RT_ERR_FAILED;
                }
            }
            else if (aclAct.fwdAct == RTL8373_ACL_FWD_COPY)
            {

                pAction->actEnable[FILTER_ENACT_ADD_DSTPORT] = TRUE;
                phyPmask = aclAct.fwdPmsk;
                if(rtk_switch_portmask_P2L_get(phyPmask,&(pAction->filterPortmask)) != RT_ERR_OK)
                    return RT_ERR_FAILED;
            }
            else
            {
                return RT_ERR_FAILED;
            }
        }
    }

    if(actCtrlBits & FILTER_ENACT_PRIORITY_MASK)
    {
        pAction->actEnable[FILTER_ENACT_PRIORITY] = TRUE;
        pAction->filterAclPri = aclAct.aclPri;
    }

    if(actCtrlBits & FILTER_ENACT_RMK_MASK)
    {
        if(TRUE == aclAct.aclRmkAct)
        {
            pAction->actEnable[FILTER_ENACT_DSCP_RMK] = TRUE;
            pAction->filterDscpRmk= aclAct.aclRmkVal & 0x3F;
        }
        else 
        {
            pAction->actEnable[FILTER_ENACT_1P_RMK] = TRUE;
            pAction->filter1pRmk = aclAct.aclRmkVal & 0x7;

        }                
    }

    if(actCtrlBits & FILTER_ENACT_POLICING_LOGG_MASK)
    {
        if(TRUE == aclAct.aclPolicingLogAct)
        {
            pAction->actEnable[FILTER_ENACT_LOGGING_CNTR] = TRUE;
            pAction->filterLoggCntr = aclAct.aclMeterLoggIdx & 0x1F;
        }
        else 
        {
            pAction->actEnable[FILTER_ENACT_POLICING_0] = TRUE;
            pAction->filterPolicingIdx[0] = aclAct.aclMeterLoggIdx & 0x3F;
        } 
    }
    if(actCtrlBits & FILTER_ENACT_SVLAN_MASK)
    {
        if(aclAct.sact == FILTER_ENACT_SVLAN_TYPE(FILTER_ENACT_SVLAN_INGRESS))
        {
            pAction->actEnable[FILTER_ENACT_SVLAN_INGRESS] = TRUE;
            pAction->filterSvlanVid = aclAct.svid;
        }
        else if(aclAct.sact == FILTER_ENACT_SVLAN_TYPE(FILTER_ENACT_SVLAN_EGRESS))
        {

            pAction->actEnable[FILTER_ENACT_SVLAN_EGRESS] = TRUE;
            pAction->filterSvlanVid = aclAct.svid;
        }
        else if(aclAct.sact == FILTER_ENACT_SVLAN_TYPE(FILTER_ENACT_SVLAN_CVID))
            pAction->actEnable[FILTER_ENACT_SVLAN_CVID] = TRUE;
        else if(aclAct.sact == FILTER_ENACT_SVLAN_TYPE(FILTER_ENACT_POLICING_2))
        {
            pAction->actEnable[FILTER_ENACT_POLICING_2] = TRUE;
            pAction->filterPolicingIdx[2]  = aclAct.svid & 0x3F;
        }
    }

    if(actCtrlBits & FILTER_ENACT_CVLAN_MASK)
    {
        if(FILTER_ENACT_CACTEXT_TAGONLY == aclAct.cactExt ||
            FILTER_ENACT_CACTEXT_BOTHVLANTAG == aclAct.cactExt )
        {
            if(FILTER_CTAGFMT_UNTAG == aclAct.tagFmt)
            {
                pAction->actEnable[FILTER_ENACT_EGRESSCTAG_UNTAG] = TRUE;
            }
            else if(FILTER_CTAGFMT_TAG == aclAct.tagFmt)
            {
                pAction->actEnable[FILTER_ENACT_EGRESSCTAG_TAG] = TRUE;
            }
            else if(FILTER_CTAGFMT_KEEP == aclAct.tagFmt)
            {
                pAction->actEnable[FILTER_ENACT_EGRESSCTAG_REALKEEP] = TRUE;
            }
             else if(FILTER_CTAGFMT_KEEP1PRMK== aclAct.tagFmt)
            {
                pAction->actEnable[FILTER_ENACT_EGRESSCTAG_KEEPAND1PRMK] = TRUE;
            }

        }

        if(FILTER_ENACT_CACTEXT_VLANONLY == aclAct.cactExt ||
            FILTER_ENACT_CACTEXT_BOTHVLANTAG == aclAct.cactExt )
        {
            if(aclAct.cact == FILTER_ENACT_CVLAN_TYPE(FILTER_ENACT_CVLAN_INGRESS))
            {
                pAction->actEnable[FILTER_ENACT_CVLAN_INGRESS] = TRUE;
                pAction->filterCvlanVid  = aclAct.cvid;
            }
            else if(aclAct.cact == FILTER_ENACT_CVLAN_TYPE(FILTER_ENACT_CVLAN_EGRESS))
            {


                pAction->actEnable[FILTER_ENACT_CVLAN_EGRESS] = TRUE;
                pAction->filterCvlanVid  = aclAct.cvid;
            }
            else if(aclAct.cact == FILTER_ENACT_CVLAN_TYPE(FILTER_ENACT_CVLAN_SVID))
            {
                pAction->actEnable[FILTER_ENACT_CVLAN_SVID] = TRUE;
            }
            else if(aclAct.cact == FILTER_ENACT_CVLAN_TYPE(FILTER_ENACT_POLICING_1))
            {
                pAction->actEnable[FILTER_ENACT_POLICING_1] = TRUE;
                pAction->filterPolicingIdx[1]  = aclAct.cvid & 0x3F;
            }
        }
    }

    if(actCtrlBits & FILTER_ENACT_INTGPIO_MASK)
    {
        if(TRUE == aclAct.aclInt)
        {
            pAction->actEnable[FILTER_ENACT_INTERRUPT] = TRUE;
        }

        if(TRUE == aclAct.gpioEn)
        {
            pAction->actEnable[FILTER_ENACT_GPO] = TRUE;
            pAction->filterPin = aclAct.gpioPin;
        }
    }

    if(actCtrlBits & FILTER_ENACT_BYPASS_MASK)
    {
        if( (1<<FILTER_BYPASS_IGR_BANDWIDTH_STORM_CTRL) & aclAct.bypassAct)
        {
            pAction->actEnable[ FILTER_ENACT_BYPASS_IGRBW_STORMCTRL] = TRUE;
        }
        if( (1<<FILTER_BYPASS_STP_SRC_CHECK) & aclAct.bypassAct)
        {
            pAction->actEnable[ FILTER_ENACT_BYPASS_STP_SRC_CHK] = TRUE;
        }
        if( (1<<FILTER_BYPASS_INGRESS_VLAN_FILTER) & aclAct.bypassAct)
        {
            pAction->actEnable[ FILTER_ENACT_BYPASS_IGRVLAN_FLTR] = TRUE;
        }
    }

    /* Get field type of RAW data */
    if ((retVal = _rtl8373_getAclTemplate(aclRule.data_bits.templateIdx, &type))!= RT_ERR_OK)
        return retVal;

    for(i = 0; i < RTL8373_ACLRULEFIELDNO; i++)
    {
        
        pFilterCfg->fieldRawType[i] = type.field[i];
    }/* end of for(i...) */

    return RT_ERR_OK;
}

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
 *      RT_ERR_FILTER_ENTRYIDX  - Invalid filterIdx.
 * Note:
 *      This function delete a group of ACL rules starting from filterIdx.
 */
rtk_api_ret_t dal_rtl8373_igrAcl_cfg_del(rtk_filter_id_t filterIdx)
{
    rtl8373_acl_rule_t initRule;
    rtl8373_acl_act_t  initAct;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(filterIdx > RTL8373_ACLRULEMAX )
        return RT_ERR_FILTER_ENTRYIDX;

    memset(&initRule, 0, sizeof(rtl8373_acl_rule_t));
    memset(&initAct, 0, sizeof(rtl8373_acl_act_t));

     RTK_ERR_CHK( _rtl8373_setAclRule(filterIdx, &initRule));

    /*set all action control bit = 1 & NOT bit ==disable*/
    RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ACL_ACT_CTRL_ADDR(filterIdx), FILTER_ENACT_ALL_MASK));
    
    RTK_ERR_CHK( _rtl8373_setAclAct(filterIdx, &initAct));

    return RT_ERR_OK;
}


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
rtk_api_ret_t dal_rtl8373_igrAcl_cfg_delAll(void)
{
    rtk_uint32         idx = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    for(idx = 0; idx < RTL8373_ACLRULENO; idx++)
    {
        /*set all action control bit = 1 & NOT bit ==disable*/
        RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ACL_ACT_CTRL_ADDR(idx), FILTER_ENACT_ALL_MASK));
    }

    return rtl8373_setAsicRegBit(RTL8373_ACL_CTRL_ADDR, RTL8373_ACL_CTRL_TABLE_RST_OFFSET, TRUE);
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_unmatchAction_set(rtk_port_t port, rtk_filter_unmatch_action_t action)
{
    rtk_api_ret_t ret;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(action >= FILTER_UNMATCH_END)
        return RT_ERR_INPUT;

    if((ret = rtl8373_setAsicRegBit(RTL8373_ACL_PORT_UNMATCH_PERMIT_ADDR, rtk_switch_port_L2P_get(port), action)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_unmatchAction_get(rtk_port_t port, rtk_filter_unmatch_action_t* pAction)
{
    rtk_api_ret_t ret;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if((ret = rtl8373_getAsicRegBit(RTL8373_ACL_PORT_UNMATCH_PERMIT_ADDR, rtk_switch_port_L2P_get(port), pAction)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_state_set(rtk_port_t port, rtk_filter_state_t state)
{
    rtk_api_ret_t ret;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);       

    if(state >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((ret = rtl8373_setAsicRegBit(RTL8373_ACL_PORT_EN_ADDR, rtk_switch_port_L2P_get(port), state)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_state_get(rtk_port_t port, rtk_filter_state_t* pState)
{
    rtk_api_ret_t ret = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pState)
        return RT_ERR_NULL_POINTER;

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if((ret = rtl8373_getAsicRegBit(RTL8373_ACL_PORT_EN_ADDR, rtk_switch_port_L2P_get(port), pState)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_template_set(rtk_filter_template_t *aclTemplate)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 idxField = 0;
    rtl8373_acl_template_t aclType;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(aclTemplate->index >= RTK_MAX_NUM_OF_FILTER_TYPE)
        return RT_ERR_INPUT;

    for(idxField = 0; idxField < RTK_MAX_NUM_OF_FILTER_FIELD; idxField++)
    {
        if(aclTemplate->fieldType[idxField] < FILTER_FIELD_RAW_DMAC_15_0 ||
            (aclTemplate->fieldType[idxField] > FILTER_FIELD_RAW_CTAG && aclTemplate->fieldType[idxField] < FILTER_FIELD_RAW_SIP_15_0 ) ||
            (aclTemplate->fieldType[idxField] > FILTER_FIELD_RAW_DIP_31_16 && aclTemplate->fieldType[idxField] < FILTER_FIELD_RAW_VIDRANGE) ||
            (aclTemplate->fieldType[idxField] > FILTER_FIELD_RAW_L4DPORT && aclTemplate->fieldType[idxField] < FILTER_FIELD_RAW_FIELD_SELECT00 ) ||
            aclTemplate->fieldType[idxField] >= FILTER_FIELD_RAW_END)
        {
            return RT_ERR_INPUT;
        }
    }

    for(idxField = 0; idxField < RTK_MAX_NUM_OF_FILTER_FIELD; idxField++)
    {
        aclType.field[idxField] = aclTemplate->fieldType[idxField];
    }

    if((retVal = _rtl8373_setAclTemplate(aclTemplate->index, &aclType)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_template_get(rtk_filter_template_t *aclTemplate)
{
    rtk_api_ret_t retVal;
    rtk_uint32 idxField;
    rtl8373_acl_template_t aclType;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == aclTemplate)
        return RT_ERR_NULL_POINTER;

    if(aclTemplate->index >= RTK_MAX_NUM_OF_FILTER_TYPE)
        return RT_ERR_INPUT;
    
    if((retVal = _rtl8373_getAclTemplate(aclTemplate->index, &aclType)) != RT_ERR_OK)
          return retVal;

    for(idxField = 0; idxField < RTK_MAX_NUM_OF_FILTER_FIELD; idxField ++)
    {
        aclTemplate->fieldType[idxField] = aclType.field[idxField];
    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_fieldSel_set(rtk_uint32 index, rtk_field_sel_t format, rtk_uint32 offset)
{
    rtk_api_ret_t ret;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(index >= RTL8373_FIELDSEL_FORMAT_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    if(format >= FORMAT_END)
        return RT_ERR_OUT_OF_RANGE;

    if(offset > RTL8373_FIELDSEL_MAX_OFFSET)
        return RT_ERR_OUT_OF_RANGE;

    regData = ((rtk_uint32)format & RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_MASK ) |\
               ((offset << RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_OFFSET) & RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_MASK );

    if((ret = rtl8373_setAsicReg(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(index), regData)) != RT_ERR_OK)
       return ret;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_fieldSel_get(rtk_uint32 index, rtk_field_sel_t *pFormat, rtk_uint32 *pOffset)
{
    rtk_api_ret_t ret = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pFormat || NULL == pOffset)
        return RT_ERR_NULL_POINTER;

    if(index >= RTL8373_FIELDSEL_FORMAT_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    if((ret = rtl8373_getAsicReg(RTL8373_PARSER_FIELD_SELTOR_CTRL_ADDR(index), &regData)) != RT_ERR_OK)
       return ret;

    *pFormat    = (rtk_field_sel_t)((regData & RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_MASK) >> RTL8373_PARSER_FIELD_SELTOR_CTRL_FMT_OFFSET);
    *pOffset    = ((regData & RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_MASK) >> RTL8373_PARSER_FIELD_SELTOR_CTRL_OFFSET_OFFSET);

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_ipRange_set(rtk_uint32 index, rtk_filter_iprange_t type, ipaddr_t upperIp, ipaddr_t lowerIp)
{
    rtk_uint32 retVal = 0;
    rtk_uint32 regAddr = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(index > RTL8373_ACLRANGEMAX)
        return RT_ERR_OUT_OF_RANGE;

    if(type >= IPRANGE_END)
        return RT_ERR_OUT_OF_RANGE;

    if(lowerIp > upperIp)
        return RT_ERR_INPUT;

    regAddr= RTL8373_RNG_CHK_IP_ADDR(index);
    /*set type*/
    retVal = rtl8373_setAsicReg(regAddr,  ((rtk_uint32)type) & RTL8373_RNG_CHK_IP_TYPE_MASK);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*set lower boundary*/
    retVal = rtl8373_setAsicReg(regAddr+8, lowerIp);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*set upper boundary*/
    retVal = rtl8373_setAsicReg(regAddr+4, upperIp);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_ipRange_get(rtk_uint32 index, rtk_filter_iprange_t *pType, ipaddr_t *pUpperIp, ipaddr_t *pLowerIp)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;
    rtk_uint32 regAddr = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if((NULL == pType) || (NULL == pUpperIp) || (NULL == pLowerIp))
        return RT_ERR_NULL_POINTER;

    if(index > RTL8373_ACLRANGEMAX)
        return RT_ERR_OUT_OF_RANGE;

    regAddr= RTL8373_RNG_CHK_IP_ADDR(index);

    
    /*get type*/
    retVal = rtl8373_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pType = (rtk_filter_iprange_t)(regData & RTL8373_RNG_CHK_IP_TYPE_MASK);

    /*get lower boundary*/
    retVal = rtl8373_getAsicReg((regAddr + 8), pLowerIp);
    if(retVal != RT_ERR_OK)
        return retVal;

    /*get upper boundary*/
    retVal = rtl8373_getAsicReg(regAddr + 4, pUpperIp);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


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
rtk_api_ret_t dal_rtl8373_igrAcl_vidRange_set(rtk_uint32 index, rtk_filter_vidrange_t type, rtk_uint32 upperVid, rtk_uint32 lowerVid)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(index > RTL8373_ACLRANGEMAX)
        return RT_ERR_OUT_OF_RANGE;

    if(type >= VIDRANGE_END)
        return RT_ERR_OUT_OF_RANGE;

    if( (upperVid > RTK_VID_MAX ) || (lowerVid > upperVid))
        return RT_ERR_INPUT;

    regData = ( type  | (lowerVid << RTL8373_RNG_CHK_VID_LOWER_OFFSET ) | (upperVid << RTL8373_RNG_CHK_VID_UPPER_OFFSET) );

    retVal = rtl8373_setAsicReg(RTL8373_RNG_CHK_VID_ADDR(index), regData);
    if(retVal != RT_ERR_OK)
        return retVal;


    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_vidRange_get(rtk_uint32 index, rtk_filter_vidrange_t *pType, rtk_uint32 *pUpperVid, rtk_uint32 *pLowerVid)
{
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if((NULL == pType) || (NULL == pUpperVid) || (NULL == pLowerVid))
        return RT_ERR_NULL_POINTER;

    if(index > RTL8373_ACLRANGEMAX)
        return RT_ERR_OUT_OF_RANGE;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_RNG_CHK_VID_ADDR(index), &regData));

    *pType = (rtk_filter_vidrange_t)(regData  & RTL8373_RNG_CHK_VID_TYPE_MASK);
    *pLowerVid = (regData & RTL8373_RNG_CHK_VID_LOWER_MASK) >> RTL8373_RNG_CHK_VID_LOWER_OFFSET;
    *pUpperVid = (regData & RTL8373_RNG_CHK_VID_UPPER_MASK) >> RTL8373_RNG_CHK_VID_UPPER_OFFSET;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_portRange_set(rtk_uint32 index, rtk_filter_portrange_t type, rtk_uint32 upperPort, rtk_uint32 lowerPort)
{
    rtk_uint32 regAddr = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(index > RTL8373_ACLRANGEMAX)
        return RT_ERR_OUT_OF_RANGE;

    if(type >= PORTRANGE_END)
        return RT_ERR_OUT_OF_RANGE;

    if(lowerPort > upperPort)
        return RT_ERR_INPUT;

    if(upperPort > RTL8373_ACL_PORTRANGEMAX)
        return RT_ERR_INPUT;

    if(lowerPort > RTL8373_ACL_PORTRANGEMAX)
        return RT_ERR_INPUT;

    regAddr = RTL8373_RNG_CHK_PORT_ADDR(index);
    RTK_ERR_CHK(rtl8373_setAsicReg(regAddr, (rtk_uint32)type));
    RTK_ERR_CHK(rtl8373_setAsicReg((regAddr + 4), ((upperPort << 16) | lowerPort)));

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_portRange_get(rtk_uint32 index, rtk_filter_portrange_t *pType, rtk_uint32 *pUpperPort, rtk_uint32 *pLowerPort)
{
    rtk_uint32 regAddr = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if((NULL == pType) || (NULL == pUpperPort) || (NULL == pLowerPort))
        return RT_ERR_NULL_POINTER;

    if(index > RTL8373_ACLRANGEMAX)
        return RT_ERR_OUT_OF_RANGE;

    regAddr = RTL8373_RNG_CHK_PORT_ADDR(index);

    RTK_ERR_CHK(rtl8373_getAsicReg(regAddr, &regData));
    *pType = (rtk_filter_portrange_t)(regData & RTL8373_RNG_CHK_PORT_TYPE_MASK);
        
    RTK_ERR_CHK(rtl8373_getAsicReg((regAddr + 4), &regData));
    *pLowerPort = (regData & 0xFFFF) ;
    *pUpperPort = ((regData >> 16 )& 0xFFFF);

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_gpioPolarity_set(rtk_uint32 polarity)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(polarity >= RTL8373_GPIO_POLARITY_END)
        return RT_ERR_OUT_OF_RANGE;

     RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ACL_GPIO_CTRL_ADDR, polarity));
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_gpioPolarity_get(rtk_uint32* pPolarity)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPolarity)
        return RT_ERR_NULL_POINTER;

    RTK_ERR_CHK(rtl8373_getAsicReg(RTL8373_ACL_GPIO_CTRL_ADDR, pPolarity));
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_igrAcl_gpioEn_set
 * Description:
 *      Enable ACL gpio pin
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
rtk_api_ret_t dal_rtl8373_igrAcl_gpioEn_set(rtk_uint32 pinNum,  rtk_enable_t enabled)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enabled >= RTK_ENABLE_END)
        return RT_ERR_OUT_OF_RANGE;

    if(pinNum >=  ACL_GPIO_ACT_PINNUM_MAX)
        return RT_ERR_OUT_OF_RANGE;

    RTK_ERR_CHK(rtl8373_setAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, pinNum, enabled));
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_gpioEn_get(rtk_uint32 pinNum,  rtk_enable_t * pEnabled)
{
    rtk_uint32 regVal = 0;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if( NULL == pEnabled)
        return RT_ERR_NULL_POINTER;
    if(pinNum >=  ACL_GPIO_ACT_PINNUM_MAX)
        return RT_ERR_OUT_OF_RANGE;

     RTK_ERR_CHK(rtl8373_getAsicRegBit(RTL8373_IO_MUX_SEL_2_ADDR, pinNum, &regVal));
     *pEnabled = (rtk_enable_t)regVal;
     
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igrAcl_table_rst(void)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    RTK_ERR_CHK(rtl8373_setAsicReg(RTL8373_ACL_CTRL_ADDR, ENABLED));
    return RT_ERR_OK;
}

