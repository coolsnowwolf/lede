#ifndef _DBMGR_STRUCT_H_
#define _DBMGR_STRUCT_H_

#include "common/mgr_cmd.h"
#include "common/mgr_struct.h"

/******************************************************************************************
*******************************************************************************************
 The database struct
*******************************************************************************************
******************************************************************************************/
struct DBMGR_Database_S {
	struct {
		//MACRO_CMD_STRUCT(PonTestCmd)		ponTestCmd ;
	} pon ;
} ;

extern struct DBMGR_Database_S *gpSysDbAddr ;

#endif /* _DBMGR_STRUCT_H_ */
