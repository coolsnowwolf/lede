/*
 * Copyright (c) 2015 Quantenna Communications, Inc.
 * All rights reserved.
 */
#ifndef RPC_RAW_H
#define RPC_RAW_H
#include <rpc/rpc.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#endif

#define QRPC_RAW_SOCK_PROT	11
#define QFTP_RAW_SOCK_PROT	22
#define ETH_P_OUI_EXT		0x88B7
#define QUANTENNA_OUI		0x002686

#define QFTP_DATA_PKT_HDR_SIZE	(sizeof(struct q_raw_ethoui_hdr) +\
					sizeof(struct qftp_data_pkt) - 1)
#define QFTP_ACK_NACK_FRAME_LEN	(sizeof(struct q_raw_ethoui_hdr) +\
					 sizeof(struct qftp_ack_nack_pkt))

/* QFT */
#define QFTP_FRAME_TYPE_NACK		0
#define QFTP_FRAME_TYPE_ACK		1
#define QFTP_FRAME_TYPE_CONNECT		2
#define QFTP_FRAME_TYPE_DATA		3
/* RPC QCSAPI */
#define QRPC_FRAME_TYPE_COMPLETE	4
#define QRPC_FRAME_TYPE_FRAG		5

#define QRPC_BUFFER_LEN		(16 * 1024)

#define QRPC_QCSAPI_RPCD_SID		0
#define QRPC_CALL_QCSAPI_RPCD_SID	1

struct q_raw_ethoui_hdr {
	struct ethhdr	eth_hdr;
	uint8_t		prot_id[5];	/* Protocol Identifier */
	uint8_t		_pad1;
} __attribute__ ((packed));

/* QRPC frames */
struct qrpc_frame_hdr {
	struct q_raw_ethoui_hdr	qhdr;
	uint8_t			sub_type;
	uint8_t			sid;
	uint16_t		seq;
} __attribute__ ((packed));

struct qrpc_raw_ethpkt {
	struct qrpc_frame_hdr	fhdr;
	char			payload[ETH_FRAME_LEN - sizeof(struct qrpc_frame_hdr)];
} __attribute__ ((packed));

/* QFTP frame payloads */
struct qftp_raw_ethpkt {
	struct q_raw_ethoui_hdr	hdr;
	char			payload[ETH_FRAME_LEN - sizeof(struct q_raw_ethoui_hdr)];
} __attribute__ ((packed));

struct qftp_connect_pkt {
	uint16_t	sub_type;
	uint16_t	seq;
	uint32_t	image_size;
	char		image_name[1];
} __attribute__ ((packed));

struct qftp_data_pkt {
	uint16_t	sub_type;
	uint16_t	seq;
	char		data[1];
} __attribute__ ((packed));

struct qftp_ack_nack_pkt {
	uint16_t	sub_type;
	uint16_t	seq;
} __attribute__ ((packed));

extern CLIENT *qrpc_clnt_raw_create(u_long prog, u_long vers,
	const char *const srcif_name, const uint8_t * dmac_addr, uint8_t sess_id);
extern SVCXPRT *qrpc_svc_raw_create(int sock, const char *const bind_interface, uint8_t sess_id);
extern int qrpc_set_prot_filter(const int sock, const short prot);
extern int qrpc_raw_bind(const int sock, const char *const if_name, const int protocol);
extern int str_to_mac(const char *txt_mac, uint8_t * mac);
extern int qrpc_clnt_raw_config_dst(const int sock, const char *const srcif_name,
				struct sockaddr_ll *dst_addr,
				const uint8_t *dmac_addr,
				struct q_raw_ethoui_hdr *pkt_outbuf,
				uint8_t qprot);
extern int qrpc_raw_read_timeout(const int sock_fd, const int timeout);
#endif
