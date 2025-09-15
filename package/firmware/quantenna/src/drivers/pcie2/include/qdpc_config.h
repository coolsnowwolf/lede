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

#ifndef __QDPC_CONFIG_H__
#define __QDPC_CONFIG_H__

#include "ruby_pcie_bda.h"
#define QDPC_MAC_ADDR_SIZE	6

/*
 * Using Type/Length field for checking if data packet or
 * netlink packet(call_qcsapi remote interface).
 * Using 0x0601 as netlink packet type and MAC magic number(Quantenna OUI)
 * to distinguish netlink packet
 */
#define QDPC_APP_NETLINK_TYPE 0x0601
#define QDPC_NETLINK_DST_MAGIC "\x00\x26\x86\x00\x00\x00"
#define QDPC_NETLINK_SRC_MAGIC "\x00\x26\x86\x00\x00\x00"

#define QDPC_RPC_TYPE_MASK	0x0f00
#define QDPC_RPC_TYPE_STRCALL	0x0100
#define QDPC_RPC_TYPE_LIBCALL	0x0200
#define QDPC_RPC_TYPE_FRAG_MASK	0x8000
#define QDPC_RPC_TYPE_FRAG	0x8000

 /* Used on RC side */
#define QDPC_NETLINK_RPC_PCI_CLNT	30
#define QDPC_NL_TYPE_CLNT_STR_REG	(QDPC_RPC_TYPE_STRCALL | 0x0010)
#define QDPC_NL_TYPE_CLNT_STR_REQ	(QDPC_RPC_TYPE_STRCALL | 0x0011)
#define QDPC_NL_TYPE_CLNT_LIB_REG	(QDPC_RPC_TYPE_LIBCALL | 0x0010)
#define QDPC_NL_TYPE_CLNT_LIB_REQ	(QDPC_RPC_TYPE_LIBCALL | 0x0011)

 /* Used on EP side */
#define QDPC_NETLINK_RPC_PCI_SVC	31
#define QDPC_NL_TYPE_SVC_STR_REG	(QDPC_RPC_TYPE_STRCALL | 0x0010)
#define QDPC_NL_TYPE_SVC_STR_REQ	(QDPC_RPC_TYPE_STRCALL | 0x0011)
#define QDPC_NL_TYPE_SVC_LIB_REG	(QDPC_RPC_TYPE_LIBCALL | 0x0010)
#define QDPC_NL_TYPE_SVC_LIB_REQ	(QDPC_RPC_TYPE_LIBCALL | 0x0011)

typedef struct qdpc_cmd_hdr {
	uint8_t dst_magic[ETH_ALEN];
	uint8_t src_magic[ETH_ALEN];
	__be16 type;
	__be16 len;
	__be16 rpc_type;
	__be16 total_len;
} qdpc_cmd_hdr_t;


#endif /* __QDPC_CONFIG_H__ */
