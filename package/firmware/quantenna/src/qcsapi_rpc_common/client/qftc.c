/*
 * Copyright (c) 2015 Quantenna Communications, Inc.
 * All rights reserved.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifndef _GNU_SOURCE
#include <libgen.h>
#endif
#include <string.h>
#include "qcsapi_rpc_common/common/rpc_raw.h"

#define QFTC_READ_TIMEOUT_MS	(250)
#define QFTC_CONNECT_RET_LIMIT	5
#define QFTC_RECV_RETRY_LIMIT	4

static struct qftc_cfg_t {
	struct qftp_raw_ethpkt		*send_buf;
	struct qftp_raw_ethpkt		*recv_buf;
	struct qftp_ack_nack_pkt	*recv_payload;
	struct qftp_data_pkt		*send_payload;
	struct sockaddr_ll		dst_addr;
	int				if_index;
	int				sock_fd;
	int				fd;
} qftc_cfg;

static void qftc_clean(void)
{
	free(qftc_cfg.send_buf);
	free(qftc_cfg.recv_buf);
	if (qftc_cfg.fd >= 0)
		close(qftc_cfg.fd);
	if (qftc_cfg.sock_fd >= 0)
		close(qftc_cfg.sock_fd);
}

static int qftc_init(const char *file_path_name, const char *sif_name, const uint8_t *dmac_addr)
{
	qftc_cfg.sock_fd = -1;
	qftc_cfg.send_buf = NULL;
	qftc_cfg.recv_buf = NULL;

	qftc_cfg.fd = open(file_path_name, O_RDONLY);

	if (qftc_cfg.fd < 0) {
		printf("Failed to open %s file\n", file_path_name);
		return -1;
	}

	qftc_cfg.sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (qftc_cfg.sock_fd < 0)
		return -1;

	if (qrpc_set_prot_filter(qftc_cfg.sock_fd, QFTP_RAW_SOCK_PROT) < 0) {
		return -1;
	}

	qftc_cfg.send_buf = malloc(sizeof(*qftc_cfg.send_buf));
	qftc_cfg.recv_buf = malloc(sizeof(*qftc_cfg.recv_buf));
	if (!qftc_cfg.send_buf || !qftc_cfg.recv_buf) {
		return -1;
	}
	qftc_cfg.send_payload = (struct qftp_data_pkt *)&qftc_cfg.send_buf->payload;
	qftc_cfg.recv_payload = (struct qftp_ack_nack_pkt *)&qftc_cfg.recv_buf->payload;

	qftc_cfg.if_index = qrpc_clnt_raw_config_dst(qftc_cfg.sock_fd, sif_name,
							&qftc_cfg.dst_addr, dmac_addr,
							(struct q_raw_ethoui_hdr *)
								qftc_cfg.send_buf,
							QFTP_RAW_SOCK_PROT);

	if (qftc_cfg.if_index < 0) {
		return -1;
	}

	return 0;
}

static uint32_t qftc_compose_connect_cmd(struct qftp_connect_pkt * const connect_payload,
						const char *file_path_name)
{
	struct stat file_stat;

	memset(&file_stat, 0, sizeof(file_stat));
	if (!stat(file_path_name, &file_stat) && (file_stat.st_mode & S_IFREG)) {
		connect_payload->sub_type = QFTP_FRAME_TYPE_CONNECT;
		connect_payload->seq = 0;
		connect_payload->image_size = file_stat.st_size;
		strcpy(connect_payload->image_name, basename((char *)file_path_name));

		return (sizeof(struct qftp_connect_pkt) +
				strlen(connect_payload->image_name));
	}

	return 0;
}

static uint32_t qftc_compose_data_cmd(void)
{
	ssize_t read_bytes;
	const size_t max_data_len = ETH_FRAME_LEN - QFTP_DATA_PKT_HDR_SIZE;

	read_bytes = read(qftc_cfg.fd, qftc_cfg.send_payload->data, max_data_len);
	qftc_cfg.send_payload->sub_type = QFTP_FRAME_TYPE_DATA;
	++qftc_cfg.send_payload->seq;

	return read_bytes;
}

static int qftc_send_cmd(const uint32_t cmd_size)
{
	ssize_t	sent_bytes;

	do {
		sent_bytes = sendto(qftc_cfg.sock_fd, qftc_cfg.send_buf, cmd_size, 0,
					(struct sockaddr *)&qftc_cfg.dst_addr,
					sizeof(qftc_cfg.dst_addr));
	} while (sent_bytes < 0 && errno == EINTR);

	return sent_bytes;
}

static int qftc_recv_cmd(void)
{
	struct sockaddr_ll lladdr;
	socklen_t addrlen = sizeof(lladdr);
	ssize_t bytes_recv = -1;
	int retry_count = 0;

	memset(&lladdr, 0, sizeof(lladdr));
	do {
		if (!qrpc_raw_read_timeout(qftc_cfg.sock_fd, QFTC_READ_TIMEOUT_MS)) {
			do {
				bytes_recv = recvfrom(qftc_cfg.sock_fd, qftc_cfg.recv_buf,
							sizeof(*qftc_cfg.recv_buf),
							MSG_DONTWAIT, (struct sockaddr *)&lladdr,
							&addrlen);
			} while (bytes_recv < 0 && errno == EINTR);
		} else if (++retry_count > QFTC_RECV_RETRY_LIMIT) {
			break;
		}
	} while ((lladdr.sll_ifindex != qftc_cfg.if_index) || (lladdr.sll_pkttype != PACKET_HOST));

	return retry_count > QFTC_RECV_RETRY_LIMIT ? -1 : bytes_recv;
}

static int qftc_connect(const char *file_path_name)
{
	uint32_t connect_cmd_hdr_size;
	ssize_t bytes_recv;
	int retry_count = 0;
	int op_failed = 0;

	connect_cmd_hdr_size = qftc_compose_connect_cmd((struct qftp_connect_pkt *)
								&qftc_cfg.send_buf->payload,
								file_path_name);
	if (!connect_cmd_hdr_size) {
		return -1;
	}
	connect_cmd_hdr_size += sizeof(struct q_raw_ethoui_hdr);

	do {
		/* Sending CONNECT command */
		if (qftc_send_cmd(connect_cmd_hdr_size) < 0) {
			op_failed = 1;
			break;
		}

		/* Waiting for ACK */
		bytes_recv = qftc_recv_cmd();
		if ((bytes_recv >= (ssize_t)QFTP_ACK_NACK_FRAME_LEN) ||
			(qftc_cfg.recv_payload->sub_type == QFTP_FRAME_TYPE_ACK)) {
			break;
		}
	} while (++retry_count < QFTC_CONNECT_RET_LIMIT);

	if (op_failed || retry_count >= QFTC_CONNECT_RET_LIMIT)
		return -1;

	return 0;
}

int qftc_start(const char *file_path_name, const char *sif_name, const uint8_t *dmac_addr)
{
	ssize_t read_bytes;
	int op_failed = 0;

	if (qftc_init(file_path_name, sif_name, dmac_addr) < 0 ||
			qftc_connect(file_path_name) < 0) {
		qftc_clean();
		return -1;
	}

	read_bytes = qftc_compose_data_cmd();
	/* Start transmitting image file */
	while (read_bytes > 0) {
		/* Sending DATA command */
		if (qftc_send_cmd(QFTP_DATA_PKT_HDR_SIZE + read_bytes) < 0) {
			op_failed = 1;
			break;
		}

		/* Receiving ACK */
		if ((qftc_recv_cmd() < 0) ||
			(qftc_cfg.send_payload->seq != qftc_cfg.recv_payload->seq) ||
			(qftc_cfg.recv_payload->sub_type != QFTP_FRAME_TYPE_ACK)) {
			op_failed = 1;
			break;
		}

		read_bytes = qftc_compose_data_cmd();
	}

	qftc_clean();

	if (op_failed || (read_bytes < 0))
		return -1;

	return 0;
}

