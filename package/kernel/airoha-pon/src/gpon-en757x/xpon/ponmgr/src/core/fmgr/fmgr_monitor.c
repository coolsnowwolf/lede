#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <utime.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "common/mgr_util.h"
#include "common/mgr_pthread.h"



/*****************************************************************************************
*****************************************************************************************/
void *fmgr_monitor_status_handler(void *arg)
{
	int T = 0 ;

	while(1) {
		usleep(800000 + get_ramdom_value(400000)) ;
		
		T++ ;
		if(T == 86400)	T = 0 ;
	}
	
	PONMGR_PTHREAD_EXIT(NULL) ;
}
