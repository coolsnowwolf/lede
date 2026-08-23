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
	omci_timer.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/6/28	Create
*/


#ifndef _OMCI_TIMER_H 
#define _OMCI_TIMER_H

typedef unsigned int UInt32;
typedef int	Int;
typedef int Int32;
typedef unsigned int Bool;


typedef int(*TimerCallback)(void *param);

#define OK (0)
#define ERROR (-1)

#ifndef TRUE
#define    TRUE    1
#endif
#ifndef FALSE
#define    FALSE   0
#endif


#define timerStartS(tm_out, func, param) \
	AddTimer(NULL, tm_out, func, param, FALSE)

#define timerPeriodicStartS(tm_out, func, param) \
	AddTimer(NULL, tm_out, func, param, TRUE)
	
#define timerStopS(tm_id) \
	RemoveTimer(tm_id)
	
#define timerPeriodicStop(tm_id) \
	RemoveTimer(tm_id)


int timer_init(void);
void timer_exit(void);
int AddTimer(char *timer_name, UInt32 interval, TimerCallback callback, void *param, Bool PeriodFlag);
int RemoveTimer(int timer_id);

#endif
