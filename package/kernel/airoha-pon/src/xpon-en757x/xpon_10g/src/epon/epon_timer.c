#include <linux/timer.h>
#include "epon/epon_timer.h"
/*______________________________________________________________________________
**	function name
**		epon_create_timer
**	description:
**		epon driver create timer
**	parameters:
**		timer
**		interval
**		callback
**		param
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_create_timer(struct timer_list *timer, eponTimerCallback callback, unsigned long param)
{
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "\r\n	enter eponMacAddTimer");
	if((timer == NULL) ||( callback == NULL) ){
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "\r\n	eponMacAddTimer param error");
		return -1;
	}
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	init_timer(timer);
	timer->function = callback;
#else
	timer_setup(timer, callback, 0);
#endif
	timer->expires= param;

	return 0;
}




