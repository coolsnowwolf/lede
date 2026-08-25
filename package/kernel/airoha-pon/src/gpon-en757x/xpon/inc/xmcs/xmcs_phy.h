#ifndef _XMCS_PHY_H_
#define _XMCS_PHY_H_

#include <xpon_global/private/xmcs_phy.h>

int phy_cmd_proc(uint cmd, ulong arg) ;
void xmcs_detect_phy_trans_alarm(uint alarm) ;

#endif /* _XMCS_PHY_H_ */

