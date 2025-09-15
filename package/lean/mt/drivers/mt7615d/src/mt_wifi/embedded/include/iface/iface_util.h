/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_util.h

    Abstract:
	Common for PCI/USB/RBUS.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

#ifndef __RTMP_UTIL_H__
#define __RTMP_UTIL_H__

/* maximum of PCI, USB, or RBUS, int PCI, it is 0 but in USB, it is 11 */
#define RTMP_PKT_TAIL_PADDING	11 /* 3(max 4 byte padding) + 4 (last packet padding) + 4 (MaxBulkOutsize align padding) */

#define RTMP_PCI_DMA_TODEVICE		0xFF00
#define RTMP_PCI_DMA_FROMDEVICE		0xFF01




#define UNLINK_TIMEOUT_MS		3

#define USBD_TRANSFER_DIRECTION_OUT		0
#define USBD_TRANSFER_DIRECTION_IN		0
#define USBD_SHORT_TRANSFER_OK			0
#define PURB			purbb_t

#define OS_RTMP_MlmeUp					RtmpOsMlmeUp

#endif /* __RTMP_UTIL_H__ */
