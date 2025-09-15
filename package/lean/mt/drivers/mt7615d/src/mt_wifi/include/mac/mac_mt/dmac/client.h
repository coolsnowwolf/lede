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
	client.h
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

#define CLIENT_BASE	0xe000
#define RXINF (CLIENT_BASE + 0x0030)
#define RXSH_GROUP1_EN (1 << 0)
#define RXSH_GROUP2_EN (1 << 1)
#define RXSH_GROUP3_EN (1 << 2)

#define RST (CLIENT_BASE + 0x0070)
#define TX_R_E_1 (1 << 16)
#define TX_R_E_2 (1 << 17)
#define TX_R_E_1_S (1 << 20)
#define TX_R_E_2_S (1 << 21)

#endif

