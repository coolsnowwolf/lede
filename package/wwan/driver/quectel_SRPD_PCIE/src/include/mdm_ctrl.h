#ifndef _MDM_CTRL_H
#define _MDM_CTRL_H
/*
 * For mcd driver,it offer modem_ctrl_send_abnormal_to_ap
 * function for others. It means you can use this function to notify ap,
 * some errors has been catched,by this way,ap will triger this error
 * and to do something for recovery.
 */

#include <linux/notifier.h>

enum {
	MDM_CTRL_POWER_OFF = 0,
	MDM_CTRL_POWER_ON,
	MDM_CTRL_WARM_RESET,
	MDM_CTRL_COLD_RESET,
	MDM_WATCHDOG_RESET,
	MDM_ASSERT,
	MDM_PANIC,
	MDM_CTRL_PCIE_RECOVERY,
	MDM_POWER_OFF,
	MDM_CTRL_SET_CFG
};

void modem_ctrl_send_abnormal_to_ap(int status);
void modem_ctrl_poweron_modem(int on);
void modem_ctrl_enable_cp_event(void);
int modem_ctrl_register_notifier(struct notifier_block *nb);
void modem_ctrl_unregister_notifier(struct notifier_block *nb);

#endif
