#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/fcntl.h>

#include "core/cmgr/cmgr_init.h"
#include "core/dbmgr/dbmgr.h"

/*
MGR_S32		gpIoctlFd = 0 ;
*/

int XMCS_IOCTL_SDI(unsigned int cmd, unsigned long arg){
	MGR_Ret ret = EXEC_OK ;
	int xmcs_ioctl = 0;
	xmcs_ioctl = open(IOCTL_NODE, O_RDWR) ;
	if(xmcs_ioctl < 0) {	
		return EXEC_ERR ;
	}
	if(ioctl(xmcs_ioctl,cmd, arg)){
		close(xmcs_ioctl);
			
		return EXEC_ERR ;
	}
	close(xmcs_ioctl);
	return ret;
}
/*****************************************************************************
*****************************************************************************/
MGR_Ret cmgr_init(void)
{
	MGR_Ret ret = EXEC_OK ;
#if 0
	gpIoctlFd = open(IOCTL_NODE, O_RDWR) ;
	if(gpIoctlFd < 0) {
		return EXEC_ERR ;	
	}
#endif
	
	return ret ;	
}


/*****************************************************************************
*****************************************************************************/
MGR_Ret cmgr_destroy(void)
{
	MGR_Ret ret = EXEC_OK ;
#if 0
	close(gpIoctlFd) ;
	gpIoctlFd = 0 ;
#endif	
	return ret ;	
}

