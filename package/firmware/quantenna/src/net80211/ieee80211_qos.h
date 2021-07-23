/*
 * Copyright (c) 2012 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * Common QOS definitions.
 */
#ifndef _IEEE80211_QOS_H
#define _IEEE80211_QOS_H

/* WME stream classes */
#define	WME_AC_BE	0		/* best effort */
#define	WME_AC_BK	1		/* background */
#define	WME_AC_VI	2		/* video */
#define	WME_AC_VO	3		/* voice */

enum {
	IEEE80211_WMMPARAMS_CWMIN	= 1,
	IEEE80211_WMMPARAMS_CWMAX	= 2,
	IEEE80211_WMMPARAMS_AIFS	= 3,
	IEEE80211_WMMPARAMS_TXOPLIMIT	= 4,
	IEEE80211_WMMPARAMS_ACM		= 5,
	IEEE80211_WMMPARAMS_NOACKPOLICY	= 6,	
};

#endif
