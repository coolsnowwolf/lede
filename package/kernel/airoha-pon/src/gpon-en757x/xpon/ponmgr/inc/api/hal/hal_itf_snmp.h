/*
 * hal_itf_snmp.h
 *
 *  Created on: Jan 2, 2014
 *      Author: root
 */

#ifndef HAL_ITF_SNMP_H_
#define HAL_ITF_SNMP_H_
#include "snmp/libsnmp.h"

bool hal_applySnmpRule(SnmpConfigRule* rule);

#endif /* HAL_ITF_SNMP_H_ */
