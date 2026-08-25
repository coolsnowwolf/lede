#ifndef _SECURE_UPGRADE_IOCTL_H
#define _SECURE_UPGRADE_IOCTL_H

#include <linux/ioctl.h>

struct secure_upgrade_ioctl_cmd {
	unsigned int offset;
	char path[128];
	unsigned int length;
};


#define SECURE_MAGIC 'c'
#define SECURE_UPGRADE_MAJOR 160
#define SECURE_IOC_SET _IOW(SECURE_MAGIC, 0, struct secure_upgrade_ioctl_cmd)
#define SECURE_IOC_GET _IOWR(SECURE_MAGIC, 1, struct secure_upgrade_ioctl_cmd)
#endif
