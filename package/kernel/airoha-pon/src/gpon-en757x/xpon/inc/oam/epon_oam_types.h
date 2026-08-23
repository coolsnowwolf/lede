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
	epon_oam_types.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	JQ.Zhu		2012/7/5		Create
*/

#ifndef __EPON_TYPES_H
#define __EPON_TYPES_H
#include <sys/types.h>

#ifndef TRUE
#define TRUE     1
#endif
#ifndef FALSE
#define FALSE   0
#endif
/*
#ifndef ENABLE
#define ENABLE	1
#endif
#ifndef DISABLE
#define DISABLE	0
#endif
*/
#ifndef NACK
#define NACK 	0
#endif
#ifndef ACK
#define ACK 		1
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef FAIL
#define FAIL (-1)
#endif
#ifndef SUCCESS
#define SUCCESS (0)
#endif

#ifndef TCAPI_SUCCESS
#define TCAPI_SUCCESS (0)
#endif

#define IN
#define OUT
#define INOUT

/* debug level define */
#define EPON_DEBUG_LEVEL_NONE		0
#define EPON_DEBUG_LEVEL_ERROR	1
#define EPON_DEBUG_LEVEL_DEBUG	2
#define EPON_DEBUG_LEVEL_INFO		3
#define EPON_DEBUG_LEVEL_TRACE	4

#endif /* __EPON_TYPES_H */

