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
    fsm_sync.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

typedef struct _SCAN_INFO_ {
	UCHAR SyncCurrState;
	ULONG LastScanTime;	/* Record last scan time for issue BSSID_SCAN_LIST */
	/* ULONG LastBeaconRxTime;*/	 /*OS's timestamp of the last BEACON RX time */

	BOOLEAN bImprovedScan;
	UCHAR ScanChannelCnt;	/* 0 at the beginning of scan, stop at 7 */
	UCHAR LastScanChannel;

    UINT32 *ChanList;    /* the channel list from from wpa_supplicant */
    UCHAR ChanListLen;   /* channel list length */
	UCHAR ChanListIdx;   /* current index in channel list when driver in scanning */

	UCHAR *ExtraIe;  /* Carry on Scan action from supplicant */
	UINT   ExtraIeLen;

	BOOLEAN bFastConnect;
	BOOLEAN bNotFirstScan;	/* Sam add for ADHOC flag to do first scan when do initialization */
} SCAN_INFO;

