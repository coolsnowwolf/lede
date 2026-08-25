#ifndef _MGR_SDI_H_
#define _MGR_SDI_H_


#include "xpon_global/private/xpon_if.h"

extern int XMCS_IOCTL_SDI(unsigned int cmd, unsigned long arg);

/*
extern MGR_S32 gpIoctlFd ;
#define XMCS_IOCTL_SDI(cmd, arg)		ioctl(gpIoctlFd, cmd, arg)
*/


#endif /* _MGR_SDI_H_ */

