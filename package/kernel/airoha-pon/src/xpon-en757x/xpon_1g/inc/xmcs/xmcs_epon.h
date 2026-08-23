#ifndef _XMCS_EPON_H_
#define _XMCS_EPON_H_

#include <linux/ioctl.h>
#include "xmcs_const.h"

#define EPON_MAGIC						0xD8

/* EPON IO Command */
#define EPON_CMD_SXXXX					_IOW(EPON_MAGIC, 0x01, char)
#define EPON_CMD_GXXXX					_IOR(EPON_MAGIC, 0x01, char)




int epon_cmd_proc(uint cmd, ulong arg) ;

#endif /* _XMCS_EPON_H_ */

