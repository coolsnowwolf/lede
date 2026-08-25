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
	omci_utily.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Andy.Yi		2012/7/3	Create
*/

#ifndef _OMCI_UTILY_H 
#define _OMCI_UTILY_H
uint16 get16(uint8 *cp);/* get16 */

uint32 get32 (uint8 *cp) ;/* get32 */

uint64 get64 (uint8 *cp) ;/* get64 */

uint8 *put32(uint8 *cp, uint32 x);/* put32 */

uint8 *put16(	uint8 *cp, uint16 x);/* put16 */

uint8 *put64(uint8 *cp, uint64 x);
 /* put64 */

void omcidbgPrintf(uint8 debugLevel, char *fmt,...);
unsigned long compute_crc32buf(char *buf, uint32 len);

#endif

