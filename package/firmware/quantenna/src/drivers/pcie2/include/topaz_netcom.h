/**
 * Copyright (c) 2012-2012 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#ifndef __DRIVES_NET_TOPAZ_NETCOM_H
#define __DRIVES_NET_TOPAZ_NETCOM_H

#define IPC_BIT_EP_RX_PKT	(0)
#define IPC_BIT_RESET_EP	(1)
#define IPC_BIT_RC_STOP_TX	(2)
#define IPC_BIT_RC_RX_DONE	(3)
#define IPC_BIT_EP_PM_CTRL	(4)
#define IPC_BIT_OFFLINE_DBG	(5)

#define IPC_EP_RX_PKT		(BIT(IPC_BIT_EP_RX_PKT))
#define IPC_RESET_EP		(BIT(IPC_BIT_RESET_EP))
#define IPC_RC_STOP_TX		(BIT(IPC_BIT_RC_STOP_TX))
#define IPC_RC_RX_DONE		(BIT(IPC_BIT_RC_RX_DONE))
#define IPC_EP_PM_CTRL		(BIT(IPC_BIT_EP_PM_CTRL))
#define IPC_OFFLINE_DBG		(BIT(IPC_BIT_OFFLINE_DBG))

#define TQE_NAPI_SCHED		(0x3)
#define TQE_ENABLE_INTR		(0x1)

struct vmac_bd {
	uint32_t buff_addr;
	uint32_t buff_info;
};

struct vmac_rx_buf {
	uint32_t baddr;
	uint16_t offset;
	uint16_t len;
};

#endif /* __DRIVES_NET_TOPAZ_NETCOM_H */
