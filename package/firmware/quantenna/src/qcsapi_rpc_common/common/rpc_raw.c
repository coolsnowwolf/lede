/*
 * Copyright (c) 2015 Quantenna Communications, Inc.
 * All rights reserved.
 */
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <qcsapi_rpc_common/common/rpc_raw.h>

int qrpc_set_prot_filter(const int sock, const short prot)
{
	struct sock_filter filter[] = {
		BPF_STMT(BPF_LD + BPF_H + BPF_ABS, ETH_ALEN * 2),	/* read packet type id */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			ETH_P_OUI_EXT, 0, 5),				/* if OUI Extended Ethertype */

		BPF_STMT(BPF_LD + BPF_W + BPF_ABS, ETH_HLEN),		/* read OUI */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			QUANTENNA_OUI << 8, 0, 3),			/* if QUANTENNA OUI */

		BPF_STMT(BPF_LD + BPF_B + BPF_ABS, ETH_HLEN + 4),	/* read protocol */
		BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K,
			prot, 0, 1),					/* if matches */

		BPF_STMT(BPF_RET + BPF_K, ETH_FRAME_LEN),		/* accept packet */
		BPF_STMT(BPF_RET + BPF_K, 0)				/* else  ignore packet */
	};
	struct sock_fprog fp;

	fp.filter = filter;
	fp.len = ARRAY_SIZE(filter);

	if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &fp, sizeof(fp)) < 0) {
		printf("Cannot set rpc packet filter\n");
		return -1;
	}

	return 0;

}

int qrpc_clnt_raw_config_dst(const int sock, const char *const srcif_name,
				struct sockaddr_ll *dst_addr,
				const uint8_t *dmac_addr,
				struct q_raw_ethoui_hdr *pkt_outbuf,
				uint8_t qprot)
{
	struct ifreq ifreq;
	struct ethhdr *const eth_packet = &pkt_outbuf->eth_hdr;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, srcif_name, IFNAMSIZ - 1);
	if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0) {
		printf("%s interface doesn't exist\n", srcif_name);
		return -1;
	}

	dst_addr->sll_family = AF_PACKET;
	dst_addr->sll_protocol = htons(ETH_P_OUI_EXT);
	dst_addr->sll_ifindex = ifreq.ifr_ifindex;
	dst_addr->sll_halen = ETH_ALEN;
	memcpy(dst_addr->sll_addr, dmac_addr, ETH_ALEN);

	memcpy(eth_packet->h_dest, dst_addr->sll_addr, ETH_ALEN);
	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
		return -1;
	memcpy(eth_packet->h_source, ifreq.ifr_addr.sa_data, ETH_ALEN);
	eth_packet->h_proto = htons(ETH_P_OUI_EXT);

	pkt_outbuf->prot_id[0] = QUANTENNA_OUI >> 16;
	pkt_outbuf->prot_id[1] = QUANTENNA_OUI >> 8;
	pkt_outbuf->prot_id[2] = QUANTENNA_OUI & 0xFF;

	pkt_outbuf->prot_id[3] = 0;
	pkt_outbuf->prot_id[4] = qprot;

	return dst_addr->sll_ifindex;
}

int qrpc_raw_read_timeout(const int sock_fd, const int timeout)
{
	struct pollfd fds;
	int ret;

	fds.fd = sock_fd;
	fds.events = POLLIN;

	do {
		ret = poll(&fds, 1, timeout);
	} while (ret < 0 && errno == EINTR);

	if (ret <= 0) {
		return -1;
	}

	return 0;
}

int qrpc_raw_bind(const int sock, const char *const if_name, const int protocol)
{
	struct sockaddr_ll addr;
	struct ifreq ifreq;

	memset(&ifreq, 0, sizeof(ifreq));
	strncpy(ifreq.ifr_name, if_name, IFNAMSIZ - 1);
	if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(protocol);
	addr.sll_ifindex = ifreq.ifr_ifindex;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return -1;

	return 0;
}

int str_to_mac(const char *txt_mac, uint8_t *mac)
{
	uint32_t mac_buf[ETH_ALEN];
	int ret;

	if (!txt_mac || !mac)
		return -1;

	ret = sscanf(txt_mac, "%02x:%02x:%02x:%02x:%02x:%02x", &mac_buf[0], &mac_buf[1],
			&mac_buf[2], &mac_buf[3], &mac_buf[4], &mac_buf[5]);

	if (ret != ETH_ALEN)
		return -1;

	while (ret) {
		mac[ret - 1] = (uint8_t)mac_buf[ret - 1];
		--ret;
	}

	return 0;
}

