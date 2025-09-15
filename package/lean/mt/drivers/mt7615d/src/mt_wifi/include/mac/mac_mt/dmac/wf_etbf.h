/*
***************************************************************************
* MediaTek Inc.
*
* All rights reserved. source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of MediaTek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of MediaTek, Inc. is obtained.
***************************************************************************

	Module Name:
	wf_etbf.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	CT Lin		2017/04/11	  For RvR debug command
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_ETBF_H__
#define __WF_ETBF_H__

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
#define WF_ETBF_BASE		0x24000
#else
#define WF_ETBF_BASE		0x24000
#endif /* defined(MT7615) || defined(MT7622) */


#endif /* __WF_ETBF_H__ */
