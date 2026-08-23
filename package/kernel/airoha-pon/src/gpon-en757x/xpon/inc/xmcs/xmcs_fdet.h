#ifndef _XMCS_FDET_H_
#define _XMCS_FDET_H_

#include <xpon_global/private/xmcs_fdet.h>

#define XPON_MODULE_DECS					2

void xmcs_report_event(XMCS_EventType_t type, uint id, uint value) ;
int fdet_cmd_proc(uint cmd, ulong arg) ;

#endif /* _XMCS_FDET_H_ */

