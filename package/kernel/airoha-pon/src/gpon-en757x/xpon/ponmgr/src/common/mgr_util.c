#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <sys/times.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common/mgr_util.h"
#include "core/dbmgr/dbmgr.h"

static MGR_U32 randomSeed = 1 ;

/*****************************************************************************
*****************************************************************************/
inline MGR_U32 get_ramdom_value(MGR_U32 max) 
{
	randomSeed = randomSeed + time((time_t *)NULL) ;
 	srand(randomSeed) ;
	randomSeed = rand() ;

	return (randomSeed % max) ;
}

/*****************************************************************************
*****************************************************************************/
inline MGR_U32 get_system_up_time(MGR_U32 *t) 
{
	time((time_t *)t) ;

	return *t ;
}

/*****************************************************************************
*****************************************************************************/
char *get_current_timestamp (void)
{
  time_t clock ;
  struct tm *local_tm ;
  static char str[20] ;

  time(&clock) ;
  local_tm = localtime (&clock) ;
  strftime(str, 20 - 1, "%H:%M:%S", local_tm) ;
  
  return str ;
}









