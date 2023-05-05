/* Copyright (c) 2013-2014, 2016-2019 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _RMNET_PRIVATE_H_
#define _RMNET_PRIVATE_H_

#define RMNET_MAX_PACKET_SIZE      16384
#define RMNET_DFLT_PACKET_SIZE     1500
#define RMNET_NEEDED_HEADROOM      16
#define RMNET_TX_QUEUE_LEN         1000

/* Constants */
#define RMNET_EGRESS_FORMAT_AGGREGATION         BIT(31)
#define RMNET_INGRESS_FORMAT_DL_MARKER_V1       BIT(30)
#define RMNET_INGRESS_FORMAT_DL_MARKER_V2       BIT(29)

#define RMNET_INGRESS_FORMAT_DL_MARKER  (RMNET_INGRESS_FORMAT_DL_MARKER_V1 |\
RMNET_INGRESS_FORMAT_DL_MARKER_V2)

/* Replace skb->dev to a virtual rmnet device and pass up the stack */
#define RMNET_EPMODE_VND (1)
/* Pass the frame directly to another device with dev_queue_xmit() */
#define RMNET_EPMODE_BRIDGE (2)

#endif /* _RMNET_PRIVATE_H_ */
