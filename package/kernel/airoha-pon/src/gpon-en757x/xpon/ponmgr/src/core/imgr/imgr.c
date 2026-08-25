#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "common/mgr_ipc.h"
#include "common/mgr_pthread.h"
#include "common/mgr_util.h"

#include "dspch/dspch_init.h"
#include "core/imgr/imgr.h"
#include "core/cmgr/cmgr_init.h"
#include "core/pmgr/pmgr_init.h"
#include "core/fmgr/fmgr_init.h"
#include "core/dbmgr/dbmgr_init.h"

pthread_t mgrThread ;
pthread_attr_t mgrThreadAttr ;

/*****************************************************************************
*****************************************************************************/
static MGR_Ret imgr_driver_init(void)
{
	int size, newsize ;
	char cmd[100] ;

	//check the MGR_CmdMsg_T size, 
	//if larger than CONST_SYS_DEFAULT_MSGMAX, mofidy the value in
	// /proc/sys/kernel/msgmax and /proc/sys/kernel/msgmnb files
	if((size = sizeof(MGR_CmdMsg_T)) > CONST_SYS_DEFAULT_MSGMAX) {
		DEBUG_MESSAGE("The MGR_CmdMsg_T larger then default, size: %d.\n", size) ;
		newsize = (size%4)?(((size%4)+1)*4):size ;
		sprintf(cmd, "echo %d > /proc/sys/kernel/msgmax" , newsize) ;
		system(cmd) ;
		sprintf(cmd, "echo %d > /proc/sys/kernel/msgmnb" , newsize*40) ;
		system(cmd) ;
	}
	
	return EXEC_OK ;
}

/*****************************************************************************
*****************************************************************************/
static MGR_Ret imgr_pthread_init(void)
{
	int ret ;

	ret = pthread_attr_init(&mgrThreadAttr) ;
	if(ret != 0) {
		DEBUG_PERROR("Initial pthread attribution error, errno: %d.\n", EXEC_ERR_RESOURCE_INIT_FAILED) ;
		return EXEC_ERR_RESOURCE_INIT_FAILED ;
	}
	
	ret = pthread_attr_setdetachstate(&mgrThreadAttr, PTHREAD_CREATE_DETACHED) ;
	if(ret != 0) {
		DEBUG_PERROR("Setting pthread attribution error, errno: %d.\n", EXEC_ERR_RESOURCE_CONFIG_FAILED) ;
		return EXEC_ERR_RESOURCE_CONFIG_FAILED ;
	}
	
	return EXEC_OK ;
}

/*****************************************************************************
*****************************************************************************/
MGR_Ret imgr_start(void)
{
	MGR_Ret ret ;

	DEBUG_MESSAGE("Initial system driver.\n") ;
	if((ret = imgr_driver_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial system driver failed.\n") ;
		return EXEC_ERR ;	
	}
	
	DEBUG_MESSAGE("Initial pthread parameters.\n") ;
	if((ret = imgr_pthread_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial pthread parameters failed.\n") ;
		return EXEC_ERR ;
	}
	
	DEBUG_MESSAGE("Initial dispatcher.\n") ;
	if((ret = dspch_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial dispatcher failed.\n") ;
		return EXEC_ERR ;	
	}

	DEBUG_MESSAGE("Initial database manager.\n") ;
	if((ret = dbmgr_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial database manager failed.\n") ;
		return EXEC_ERR ;
	}

	DEBUG_MESSAGE("Initial config manager.\n") ;
	if((ret = cmgr_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial config manager failed.\n") ;
		return EXEC_ERR ;
	}

	DEBUG_MESSAGE("Initial fault manager.\n") ;
	if((ret = fmgr_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial fault manager failed.\n") ;
		return EXEC_ERR ;
	}

	DEBUG_MESSAGE("Initial performance manager.\n") ;
	if((ret = pmgr_init()) != EXEC_OK) {
		DEBUG_PERROR("Initial performance manager failed.\n") ;
		return EXEC_ERR ;	
	}
	
	return ret ;
}

/*****************************************************************************
*****************************************************************************/
MGR_Ret imgr_destroy(void)
{
	MGR_Ret ret ;

	DEBUG_MESSAGE("Destroy performance manager.\n") ;
	if((ret = pmgr_destroy()) != EXEC_OK) {
		DEBUG_PERROR("Destroy performance manager failed.\n") ;
		return EXEC_ERR ;	
	}
	
	DEBUG_MESSAGE("Destroy fault manager.\n") ;
	if((ret = fmgr_destroy()) != EXEC_OK) {
		DEBUG_PERROR("Destroy fault manager failed.\n") ;
		return EXEC_ERR ;
	}

	DEBUG_MESSAGE("Destroy config manager.\n") ;
	if((ret = cmgr_destroy()) != EXEC_OK) {
		DEBUG_PERROR("Destroy config manager failed.\n") ;
		return EXEC_ERR ;
	}

	DEBUG_MESSAGE("Destroy database manager.\n") ;
	if((ret = dbmgr_destroy()) != EXEC_OK) {
		DEBUG_PERROR("Destroy database manager failed.\n") ;
		return EXEC_ERR ;
	}

	DEBUG_MESSAGE("Destroy dispatcher.\n") ;
	if((ret = dspch_destroy()) != EXEC_OK) {
		DEBUG_PERROR("Destroy dispatcher failed.\n") ;
		return EXEC_ERR ;	
	}
	
	return ret ;
}
