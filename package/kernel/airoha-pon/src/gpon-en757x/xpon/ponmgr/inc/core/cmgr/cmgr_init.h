#ifndef _CMGR_INIT_H_
#define _CMGR_INIT_H_

#include "common/mgr_type.h"

#define IOCTL_NODE			"/dev/pon"

MGR_Ret cmgr_init(void) ;
MGR_Ret cmgr_destroy(void) ;


#endif /* _CMGR_INIT_H_ */
