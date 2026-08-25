#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "common/mgr_ipc.h"

#include "core/cmgr/cmgr_proc.h"
#include "core/dbmgr/dbmgr.h"
#include "core/dbmgr/dbmgr_init.h"
#include "core/dbmgr/dbmgr_proc.h"

struct DBMGR_Database_S *gpSysDbAddr = NULL ;

/*****************************************************************************
*****************************************************************************/
static MGR_Ret dbmgr_database_init(void) 
{
	
	return EXEC_OK ;
}

/*****************************************************************************
*****************************************************************************/
MGR_Ret dbmgr_init(void)
{
	MGR_Ret ret ;

	//alloc the Database memory
	DEBUG_MESSAGE("Create database memory.\n") ;
	gpSysDbAddr = malloc(sizeof(struct DBMGR_Database_S)) ;
	if(gpSysDbAddr == NULL) {
		DEBUG_PERROR("Create database memory failed, errno: %d.\n", EXEC_ERR_RESOURCE_CREATE_FAILED) ;
		return EXEC_ERR_RESOURCE_CREATE_FAILED ;
	} else {
		DEBUG_LOG("Create the share database memory successful.\n") ;		
	}		

	DEBUG_MESSAGE("The total share database size is %d.\n", sizeof(struct DBMGR_Database_S)) ;
		
	memset(gpSysDbAddr, 0, sizeof(struct DBMGR_Database_S)) ;

	if((ret = dbmgr_database_init()) != 0) {
		DEBUG_PERROR("Initial system database error, errno: %d.", ret) ;
		return ret ;
	}

	return EXEC_OK ;
}


/*****************************************************************************
*****************************************************************************/
MGR_Ret dbmgr_destroy(void)
{
	MGR_Ret ret = EXEC_OK ;

	free(gpSysDbAddr) ;
	gpSysDbAddr = NULL ;
	
	return ret ;	
}
