/**
*@file
*@brief This file defines IGMP related macro and struct  that used by hal interface
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

#ifndef HAL_TYPE_IGMP_H_
#define HAL_TYPE_IGMP_H_

#include "hal_type_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/**
*@enum hal_igmp_match_mode_type_t
*Define the match method mode of igmp upstream signals*/
typedef enum
{
    /** use vlan and mac to filter upstream igmp signals*/                      
    LSW_IGMP_MATCH_VLAN_MAC_E = 0,
    /** use mac to filter upstream igmp signals*/                      
    LSW_IGMP_MACTH_MAC_E      = 1,
}hal_igmp_match_mode_type_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
