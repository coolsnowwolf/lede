#ifndef _UAPI_WAN_MODE_SELECT_IOCTL_H
#define _UAPI_WAN_MODE_SELECT_IOCTL_H

#include <linux/ioctl.h>

struct wan_mode_select_ioctl_cmd {
	unsigned int wan;
};

typedef enum {
    WAN_MODE_ATM = 0,
    WAN_MODE_PTM,
    WAN_MODE_GE,
    WAN_MODE_SFP,
}WanMode_t;

#define WAN_MODE_SELECT_MAGIC 'c'
#define WAN_MODE_SELECT_MAJOR 161
#define WAN_MODE_SELECT_IOC_SET _IOW(WAN_MODE_SELECT_MAGIC, 0, struct wan_mode_select_ioctl_cmd)
#endif