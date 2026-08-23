/*
* File Name: xmcs_epon.c
* Description: EPON I/O Command Process for xPON Management Control 
*			  Subsystem
*
******************************************************************/
#include <linux/kernel.h>

#include "common/drv_global.h"
#include "xmcs/xmcs_epon.h"

/***************************************************************
***************************************************************/
int epon_cmd_proc(uint cmd, ulong arg) 
{
	switch(cmd) {
		default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %d\n", cmd) ;
			break ;
	}
	
	return 0 ;
}
