#ifndef _DSPCH_H_
#define _DSPCH_H_

#include "common/mgr_type.h"
#include "common/mgr_struct.h"

MGR_Ret dspch_start(void) ;
MGR_Ret dspch_report_trap_msg(IPC_Msg_T *pIpcMsg) ;
void dspch_shunt_cmd_request(MGR_CmdMsg_T *pMsg) ;

#endif /* _DSPCH_H_ */
