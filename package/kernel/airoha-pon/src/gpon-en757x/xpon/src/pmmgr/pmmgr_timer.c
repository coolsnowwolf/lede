/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	omci.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/5	Create
*/

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//#include "tctimer.h"


//#include "Global_res.h"
#ifdef THREAD_REDUCE
#include <signal.h>
#endif
#include "pmmgr_timer.h"
#include "pmmgr_define.h"
#include "xpon_common.h"

uint32 tctimer_free_mib = 0;
uint32 tctimer_malloc_mib = 0;

typedef enum{
TIMER_ID_NULL = -1,
TIMER_ID_COUNT = 16
}TIMER_ID_ENUM;

typedef struct _timer_list {
	struct timeval time; //time when expire
	TimerCallback cb; /*Timer callback function*/
	void *param;  /*callback function param*/
	uint32 interval;	/*save interval for period timer*/
	int timerid;	/*timer ID*/
	int PeriodFlag;	/*Period Timer Flag*/
	char timername[16];	/*timer name*/
	struct _timer_list *next;	/*next point of timer list*/
}timer_list;

#define STATE_USED 1
#define STATE_FREE 0

typedef struct timer_context{
#ifndef THREAD_REDUCE
	pthread_t thread_id;		/*save timer thread ID*/
#endif
	pthread_mutex_t  *p_timer_mutex;	/*timer list mutex, when process timer list, will lock this mutes*/
	Bool abort;	/*timer exit flag*/
	Bool inited;	/*timer init flag*/
	timer_list *timerhead;	/*save timer list head*/
	unsigned char timeridtable[TIMER_ID_COUNT];	/*timer ID table for add timer*/
	int timerlistnum;	/*timer number of timer list*/
}Timer_Context;



void ReRegisterTimer(timer_list *timeout);
void ThreadedTimerCheck(void);
int findTimerID();



/*Global Timer context variable*/
static Timer_Context timer_context = {0};

void ReRegisterTimer(timer_list *timeout)
{
	timer_list *tmp, *prev;
	UInt32 interval_sec, interval_usec;
	int interval = timeout->interval;
	/*Caculte Delay Time*/
	MACRO_XPON_COMMON_READUPTIME(&timeout->time);
	interval_sec = interval/1000;
	interval_usec = (interval % 1000) * 1000;

	/*ReFill time struct parameter*/
	timeout->time.tv_sec += interval_sec;
	timeout->time.tv_usec += interval_usec;

	timeout->next = NULL;
	timeout->interval = interval;

	if (timer_context.timerhead == NULL) {
		timer_context.timerhead = timeout;
//		printf("ReRegisterTimer: Success! Timer id is [%d] interval is [%d]\n", timeout->timerid, interval);
		return;
	}

	prev = NULL;
	tmp = timer_context.timerhead;
	while (tmp != NULL)
    {
		if (timercmp(&timeout->time, &tmp->time, <))
			break;
		prev = tmp;
		tmp = tmp->next;
	}

	if (prev == NULL)
    {
		timeout->next = timer_context.timerhead;
		timer_context.timerhead  = timeout;
	}
    else
    {
		timeout->next = prev->next;
		prev->next = timeout;
	}
//	printf("ReRegisterTimer: Success! Timer id is [%d] interval is [%d]\n", timeout->timerid, interval);
}

#define TIMECHANGERANGE 60

void checkCPETimeChange(struct timeval now, struct timeval last)
{
	timer_list *timeout = NULL;
	int dval = now.tv_sec - last.tv_sec;
	if(abs(dval) >= TIMECHANGERANGE)
	{
		printf("checkCPETimeChange: CPE Time Modified!\n");
//		printf("checkCPETimeChange: Now tv_sec is [%d] u_sec is [%d]\n", now.tv_sec, now.tv_usec);
//		printf("checkCPETimeChange: Last tv_sec is [%d] u_sec is [%d]\n", last.tv_sec, last.tv_usec);
		
		/*Update Timer List expire time*/
		timeout = timer_context.timerhead;
		while(timeout != NULL)
		{
			timeout->time.tv_sec += dval;
			timeout = timeout->next;
		}
		
	}

}

#ifndef THREAD_REDUCE
void ThreadedTimerCheck(void)
{
	timer_list *tmp;	
	struct timeval now;
	struct timeval last;
	MACRO_XPON_COMMON_READUPTIME(&last);
	MACRO_XPON_COMMON_READUPTIME(&now);
	do{
		/*Sleep 100ms for check next timer expire*/
		usleep(100000);
		pthread_mutex_lock(timer_context.p_timer_mutex);
		MACRO_XPON_COMMON_READUPTIME(&now);
		/* check if registered timeouts have occurred */
		if(timer_context.timerhead)
		{
			checkCPETimeChange(now, last);		
			if(timercmp(&now, &timer_context.timerhead->time, >=))
            {
				tmp = timer_context.timerhead;
				timer_context.timerhead= timer_context.timerhead->next;
				tmp->cb(tmp->param);
				if(tmp->PeriodFlag == TRUE)
				{
					ReRegisterTimer(tmp);
				}
				else
				{
					if(tmp->timerid < 0 || tmp->timerid >= TIMER_ID_COUNT)
					{
						printf("Fatal ERROR: timer id not in range!\n");
					}
					timer_context.timeridtable[tmp->timerid] = STATE_FREE;
					free(tmp);
					tctimer_free_mib++;
					timer_context.timerlistnum--;
				}
			}

		}
		/*Update last timeval*/
		last = now;
		pthread_mutex_unlock(timer_context.p_timer_mutex);
		if(timer_context.abort)
		{
			printf("Timer Thread Exit");
			break;
		}
	}while(1);
	pthread_mutex_destroy(timer_context.p_timer_mutex);
	free(timer_context.p_timer_mutex);
	tctimer_free_mib++;
	timer_context.p_timer_mutex = NULL;
	if(timer_context.timerhead)
	{
		while(timer_context.timerhead)
		{
			tmp = timer_context.timerhead;
			timer_context.timerhead = tmp->next;
			free(tmp);
			tctimer_free_mib++;
		}
	}
	pthread_exit(0);
	printf( "ThreadedTimerCheck: return .\n");
}
#else
void ThreadedTimerCheck(void)
{
	timer_list *tmp;	
	struct timeval now;
	static struct timeval last = {0};
	if(last.tv_sec == 0 && last.tv_usec == 0)
	{
		printf(("ThreadedTimerCheck: get last for first time\n");
		MACRO_XPON_COMMON_READUPTIME(&last);
	}

	pthread_mutex_lock(timer_context.p_timer_mutex);
	MACRO_XPON_COMMON_READUPTIME(&now);
	/* check if registered timeouts have occurred */
	if(timer_context.timerhead)
	{
		checkCPETimeChange(now, last);		
		//printf(("ThreadedTimerCheck Entered! now time val sec is [%d] usec is [%d]\n",  now.tv_sec,  now.tv_usec);
		//printf(("ThreadedTimerCheck Entered! timerhead time val sec is [%d] usec is [%d]\n", timer_context.timerhead->time.tv_sec, timer_context.timerhead->time.tv_usec);
		if(timercmp(&now, &timer_context.timerhead->time, >=))
        {
			tmp = timer_context.timerhead;
			timer_context.timerhead = timer_context.timerhead->next;
			tmp->cb(tmp->param);
			if(tmp->PeriodFlag == TRUE)
			{
				ReRegisterTimer(tmp);
			}
			else
			{
				if(tmp->timerid < 0 || tmp->timerid >= TIMER_ID_COUNT)
				{
					printf("Fatal ERROR: timer id not in range!\n");
				}
				timer_context.timeridtable[tmp->timerid] = STATE_FREE;
				free(tmp);
				tctimer_free_mib++;
				timer_context.timerlistnum--;
			}
		}
		else
		{
			//printf(("ThreadedTimerCheck: No Timer Expire, we wait for next\n");		
		}

	}
	else
	{
		printf(("ThreadedTimerCheck: No Timer in list\n");
	}
	/*Update last timeval*/
	last = now;
	pthread_mutex_unlock(timer_context.p_timer_mutex);
}
#endif

int timerInit(void)
{
	int ret = 0;
#ifndef THREAD_REDUCE
	pthread_attr_t p_megq_attr;
#endif
	if(timer_context.inited == TRUE)
	{
		printf("Timer has been inited!\n");
		return ERROR;
	}
	memset(&timer_context, 0, sizeof(timer_context));
	timer_context.p_timer_mutex = malloc(sizeof(pthread_mutex_t));
	if(!timer_context.p_timer_mutex)  
	{
		printf("[%s]---malloc faile for  p_timer_mutex  \n", __FUNCTION__);
		return ERROR;
	}
	tctimer_malloc_mib++;
#ifndef THREAD_REDUCE
	ret = pthread_mutex_init(timer_context.p_timer_mutex, NULL);
	if(ret != 0)
	{
		printf("timerInit: mutex_init failed!\n");
		goto return_mutex_error;
	}

	/*create thread*/
	ret = pthread_attr_init(&p_megq_attr);
	if(ret != 0)
	{
		printf("\r\nThread attribute creation fail!");
		goto return_attr_error;
	}

	ret = pthread_attr_setdetachstate(&p_megq_attr,PTHREAD_CREATE_DETACHED);
	if(ret != 0)
	{
		printf("\r\nSet attribute fail!");
		goto return_error;
	}
		
	if( pthread_create(&timer_context.thread_id, &p_megq_attr, (void *)ThreadedTimerCheck, (void *)NULL))
	{
		printf("[timerInit]-->pthread_create  ThreadedTimerCheck failed!\n");
		goto return_error;
	}
	pthread_attr_destroy(&p_megq_attr);
#endif
	timer_context.inited = TRUE;
	return OK;
#ifndef THREAD_REDUCE
return_error:
		pthread_attr_destroy(&p_megq_attr);
return_attr_error:
		pthread_mutex_destroy(timer_context.p_timer_mutex);
return_mutex_error:
		free(timer_context.p_timer_mutex);
		return ERROR;
#endif

}

void timerExit(void)
{
	timer_context.abort = TRUE;
	timer_context.inited = FALSE;
#ifdef THREAD_REDUCE
	timer_list *tmp;
	pthread_mutex_lock(timer_context.p_timer_mutex);
	if(timer_context.timerhead)
	{
		while(timer_context.timerhead)
		{
			tmp = timer_context.timerhead;
			timer_context.timerhead = tmp->next;
			free(tmp);
			tctimer_free_mib++;
		}
	}
	pthread_mutex_unlock(timer_context.p_timer_mutex);
	pthread_mutex_destroy(timer_context.p_timer_mutex);
	free(timer_context.p_timer_mutex);
	tctimer_free_mib++;
	timer_context.p_timer_mutex = NULL;
#endif
	return;
}

int findTimerID()
{
	int i = 0;
	for(i = 0; i < TIMER_ID_COUNT; i++)
	{
		if(timer_context.timeridtable[i] == STATE_FREE)
		{
			timer_context.timeridtable[i] = STATE_USED;
			return i;
		}
	}
	printf("findTimerID: Failed to find Time ID for New Timer\n");
	return TIMER_ID_NULL;
}

/* ==========================================================
*     Designer: yanfei
*     Time : 2009-11-10
*     Function Name: AddTimer
*     Function Description:
*				Add Timer in Timer List
*     Paramter:
*				Int32 timer_id: TIMER_ID which want to add
*               UInt32 interval: Timer expire interval, unit us
*               TimerCallback callback: callback function of timer
*				void *param: callback function paramter
*				Bool PeriodFlag: timer register is Period 
*                         
*     Return :
*				Success: timer ID for new Timer
*				Failed:  -1
*   ========================================================== */
int AddTimer(char *timer_name, UInt32 interval, TimerCallback callback, void *param, Bool PeriodFlag)
{
	timer_list *timeout, *tmp, *prev;
	UInt32 interval_sec, interval_usec;
	if(timer_context.inited == FALSE)
	{
		printf("AddTimer: Timer don not inited\n");
		return TIMER_ID_NULL;
	}
	if((timer_context.timerlistnum >= TIMER_ID_COUNT) \
		|| (interval <= 0) || (callback==NULL))
	{
		printf("AddTimer: failed!\n");
		return TIMER_ID_NULL;
	}

	pthread_mutex_lock(timer_context.p_timer_mutex);
	timeout = (timer_list *) malloc(sizeof(*timeout));
	tctimer_malloc_mib++;
	if (timeout == NULL)
	{
		printf("AddTimer: No resource for timer add\n");
		pthread_mutex_unlock(timer_context.p_timer_mutex);
		return TIMER_ID_NULL;
	}
	memset(timeout, 0, sizeof(*timeout));
	/*Caculte Delay Time*/
	MACRO_XPON_COMMON_READUPTIME(&timeout->time);
	interval_sec = interval/1000;
	interval_usec = (interval % 1000) * 1000;

	/*Fill time struct parameter*/
	timeout->time.tv_sec += interval_sec;
	timeout->time.tv_usec += interval_usec;

	timeout->param = param;
	timeout->cb = callback;
	/*Make sure timerid  > 0*/
	timeout->timerid = findTimerID() + 1;
	timeout->next = NULL;
	timeout->interval = interval;
	if(PeriodFlag == TRUE)
	{
		timeout->PeriodFlag = TRUE;
	}
	if(timer_name != NULL)
	{
		strncpy(timeout->timername, timer_name, 15);
	}

	/*Increase timer_context number*/
	timer_context.timerlistnum++;

	if (timer_context.timerhead == NULL) {
		timer_context.timerhead = timeout;
		pthread_mutex_unlock(timer_context.p_timer_mutex);
		//printf("AddTimer: Success! Timer id is [%d]\n", timeout->timerid);
		return timeout->timerid;
	}

	prev = NULL;
	tmp = timer_context.timerhead;
	while (tmp != NULL)
    {
		if (timercmp(&timeout->time, &tmp->time, <))
			break;
		prev = tmp;
		tmp = tmp->next;
	}

	if (prev == NULL)
    {
		timeout->next = timer_context.timerhead;
		timer_context.timerhead  = timeout;
	}
    else
    {
		timeout->next = prev->next;
		prev->next = timeout;
	}

	pthread_mutex_unlock(timer_context.p_timer_mutex);
	//printf("AddTimer: Success! Timer id is [%d]\n", timeout->timerid);
	return timeout->timerid;
}

/* ==========================================================
*     Designer: yanfei
*     Time: 2009-11-10
*     Function Name: RemoveTimer
*     Function Description:
*				Delete Timer which in Timer List
*     Paramter:
*				Int32 timer_id: TIMER_ID which want to delete
*                         
*     Return :
*				Success: removed timer id
*				Failed:  -1
*   ========================================================== */

int RemoveTimer(int timer_id)
{
	timer_list *timeout, *prev, *next;	
	int delnum = 0;
	if(timer_context.inited == FALSE)
	{
		printf("AddTimer: Timer don not inited\n");
		return TIMER_ID_NULL;
	}
	
	/*To Solve Timer ID is 0 will cause error in previous linos timer ID
	we also increase timer id in AddTimer function
	*/
	if(timer_id -1 <=TIMER_ID_NULL)
	{
		printf("RemoveTimer: timer_id:%d invalid!\n",timer_id);
		return TIMER_ID_NULL;
	}
	
	pthread_mutex_lock(timer_context.p_timer_mutex);
	if (timer_context.timerhead == NULL) {
		pthread_mutex_unlock(timer_context.p_timer_mutex);
		printf("RemoveTimer: Failed! No Timer in list. timer_id:%d invalid!\n", timer_id);
		return TIMER_ID_NULL;
	}

	prev = NULL;
	next = NULL;
	timeout = timer_context.timerhead;
	while (timeout != NULL)
    {
		next = timeout->next;
		if (timeout->timerid == timer_id)
        {
			if (prev == NULL)
				timer_context.timerhead = next;
			else
				prev->next = next;

			/*Kill Timer and free source*/
/*To Solve Timer ID is 0 will cause error in previous linos timer ID
	we also increase timer id in AddTimer function
*/
			timer_context.timeridtable[timeout->timerid - 1] = STATE_FREE;
			free(timeout);
			tctimer_free_mib++;
			delnum++;
			timer_context.timerlistnum--;
		} else
			prev = timeout;

		timeout = next;
	}

	pthread_mutex_unlock(timer_context.p_timer_mutex);
	if(delnum > 1)
	{
		printf("Warn: Remove Timer number larger than 1, remove num is [%d]\n", delnum);
	}
	if(delnum == 0)
	{
		printf("RemoveTimer: Failed! No Timer in list. timer_id:%d invalid!\n", timer_id);
		return TIMER_ID_NULL;
	}
	//printf("RemoveTimer: Success! \n");	
	return timer_id;
}

