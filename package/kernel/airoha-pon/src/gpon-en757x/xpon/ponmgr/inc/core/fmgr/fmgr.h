#ifndef _FMGR_H_
#define _FMGR_H_

#include "common/mgr_type.h"

#include "core/fmgr/fmgr_const.h"
#include "core/fmgr/fmgr_enum.h"
#include "core/fmgr/fmgr_mask.h"
#include "core/fmgr/fmgr_struct.h"
#include "core/fmgr/fmgr_proc.h"
#include "core/fmgr/fmgr_event.h"

MGR_Ret fmgr_cmd_shunter(MGR_CmdMsg_T *pMsg) ;
MGR_Ret fmgr_send_trap_to_task(MGR_U16 srcTask, MGR_U16 destTask, MACRO_CMD_STRUCT(SystemTrapInfo) *pTrapInfo) ;
void *fmgr_monitor_status_handler(void *arg) ;
void *fmgr_periph_irq_handler(void *arg) ;

#endif /* _FMGR_H_ */
