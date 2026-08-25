#ifndef _XMCS_GPON_H_
#define _XMCS_GPON_H_


#include "xpon_ioctl_if.h"

int gpon_cmd_proc(uint cmd, ulong arg) ;
int get_counter_from_reg(STATISTIC_CFG_t *pGemCouter, unchar readClear);


#endif /* _XMCS_GPON_H_ */

