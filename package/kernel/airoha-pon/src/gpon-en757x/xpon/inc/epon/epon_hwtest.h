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
	epon_hwtest.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/

#ifndef _EPON_HWTEST_H_
#define _EPON_HWTEST_H_




int eponMacRegTest(__u32 times);
int eponRegPatternTest(__u32 pattern, eponMacHwtestReg_t *regTable_p,  __u32 maxIndex);
int eponRegDefCheck(eponMacHwtestReg_t *regTable_p,  __u32 maxIndex);
#endif//_EPON_HWTEST_H_