/**
 *@file
 *@brief This file defines wan related interfaces that should be implemented by chip vendors
 *@author Shen Jingguo
 *@date     2013.3.18
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

#ifndef _HAL_ITF_WAN_H_
#define _HAL_ITF_WAN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lsw_type.h"
#include "hal_type_route.h"
#include "hal_type_common.h"

int hal_route_is_service_internet(lsw_service_type_e serviceType);

int hal_route_mac_add(char mac[MAC_ADDR_LEN], lsw_service_type_e serviceType);
int hal_route_mac_delete(char mac[MAC_ADDR_LEN]);


#ifdef __cplusplus
}
#endif

#endif

