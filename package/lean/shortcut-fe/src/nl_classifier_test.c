/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <arpa/inet.h>

#define NL_CLASSIFIER_GENL_VERSION	1
#define NL_CLASSIFIER_GENL_FAMILY	"FC"
#define NL_CLASSIFIER_GENL_GROUP	"FC_MCGRP"
#define NL_CLASSIFIER_GENL_HDRSIZE	0

enum NL_CLASSIFIER_CMD {
	NL_CLASSIFIER_CMD_UNSPEC,
	NL_CLASSIFIER_CMD_ACCEL,
	NL_CLASSIFIER_CMD_ACCEL_OK,
	NL_CLASSIFIER_CMD_CONNECTION_CLOSED,
	NL_CLASSIFIER_CMD_MAX,
};

enum NL_CLASSIFIER_ATTR {
	NL_CLASSIFIER_ATTR_UNSPEC,
	NL_CLASSIFIER_ATTR_TUPLE,
	NL_CLASSIFIER_ATTR_MAX,
};

union nl_classifier_tuple_ip {
	struct in_addr in;
	struct in6_addr in6;
};

struct nl_classifier_tuple {
	unsigned short af;
	unsigned char proto;
	union nl_classifier_tuple_ip src_ip;
	union nl_classifier_tuple_ip dst_ip;
	unsigned short sport;
	unsigned short dport;
	unsigned char smac[6];
	unsigned char dmac[6];
};

struct nl_classifier_instance {
	struct nl_sock *sock;
	int family_id;
	int group_id;
	int stop;
};

struct nl_classifier_instance nl_cls_inst;

static struct nla_policy nl_classifier_genl_policy[(NL_CLASSIFIER_ATTR_MAX+1)] = {
	[NL_CLASSIFIER_ATTR_TUPLE] = { .type = NLA_UNSPEC },
};

void nl_classifier_dump_nl_tuple(struct nl_classifier_tuple *tuple)
{
	char ip_str[64];

	printf("protocol = %s\n", (tuple->proto == IPPROTO_UDP) ? "udp" : ((tuple->proto == IPPROTO_TCP) ? "tcp" : "unknown"));
	printf("source ip = %s\n", inet_ntop(tuple->af, &tuple->src_ip, ip_str, sizeof(ip_str)));
	printf("destination ip = %s\n", inet_ntop(tuple->af, &tuple->dst_ip, ip_str, sizeof(ip_str)));
	printf("source port = %d\n", ntohs(tuple->sport));
	printf("destination port = %d\n", ntohs(tuple->dport));
}

int nl_classifier_msg_recv(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *gnlh = nlmsg_data(nlh);
	struct nlattr *attrs[(NL_CLASSIFIER_ATTR_MAX+1)];

	genlmsg_parse(nlh, NL_CLASSIFIER_GENL_HDRSIZE, attrs, NL_CLASSIFIER_ATTR_MAX, nl_classifier_genl_policy);

	switch (gnlh->cmd) {
	case NL_CLASSIFIER_CMD_ACCEL_OK:
		printf("Acceleration successful:\n");
		nl_classifier_dump_nl_tuple(nla_data(attrs[NL_CLASSIFIER_ATTR_TUPLE]));
		return NL_OK;
	case NL_CLASSIFIER_CMD_CONNECTION_CLOSED:
		printf("Connection is closed:\n");
		nl_classifier_dump_nl_tuple(nla_data(attrs[NL_CLASSIFIER_ATTR_TUPLE]));
		return NL_OK;
	default:
		printf("nl classifier received unknow message %d\n", gnlh->cmd);
	}

	return NL_SKIP;
}

void nl_classifier_offload(struct nl_classifier_instance *inst,
			   unsigned char proto, unsigned long *src_saddr,
			   unsigned long *dst_saddr, unsigned short sport,
			   unsigned short dport, int af)
{
	struct nl_msg *msg;
	int ret;
	struct nl_classifier_tuple classifier_msg;

	memset(&classifier_msg, 0, sizeof(classifier_msg));
	classifier_msg.af = af;
	classifier_msg.proto = proto;
	memcpy(&classifier_msg.src_ip, src_saddr, (af == AF_INET ? 4 : 16));
	memcpy(&classifier_msg.dst_ip, dst_saddr, (af == AF_INET ? 4 : 16));
	classifier_msg.sport = sport;
	classifier_msg.dport = dport;

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Unable to allocate message\n");
		return;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, inst->family_id,
		    NL_CLASSIFIER_GENL_HDRSIZE, NLM_F_REQUEST,
		    NL_CLASSIFIER_CMD_ACCEL, NL_CLASSIFIER_GENL_VERSION);
	nla_put(msg, NL_CLASSIFIER_ATTR_TUPLE, sizeof(classifier_msg), &classifier_msg);

	ret = nl_send_auto(inst->sock, msg);
	if (ret < 0) {
		printf("send netlink message failed.\n");
		nlmsg_free(msg);
		return;
	}

	nlmsg_free(msg);
	printf("nl classifier offload connection successful\n");
}

int nl_classifier_init(struct nl_classifier_instance *inst)
{
	int ret;

	inst->sock = nl_socket_alloc();
	if (!inst->sock) {
		printf("Unable to allocation socket.\n");
		return -1;
	}
	genl_connect(inst->sock);

	inst->family_id = genl_ctrl_resolve(inst->sock, NL_CLASSIFIER_GENL_FAMILY);
	if (inst->family_id < 0) {
		printf("Unable to resolve family %s\n", NL_CLASSIFIER_GENL_FAMILY);
		goto init_failed;
	}

	inst->group_id = genl_ctrl_resolve_grp(inst->sock, NL_CLASSIFIER_GENL_FAMILY, NL_CLASSIFIER_GENL_GROUP);
	if (inst->group_id < 0) {
		printf("Unable to resolve mcast group %s\n", NL_CLASSIFIER_GENL_GROUP);
		goto init_failed;
	}

	ret = nl_socket_add_membership(inst->sock, inst->group_id);
	if (ret < 0) {
		printf("Unable to add membership\n");
		goto init_failed;
	}

	nl_socket_disable_seq_check(inst->sock);
	nl_socket_modify_cb(inst->sock, NL_CB_VALID, NL_CB_CUSTOM, nl_classifier_msg_recv, NULL);

	printf("nl classifier init successful\n");
	return 0;

init_failed:
	if (inst->sock) {
		nl_close(inst->sock);
		nl_socket_free(inst->sock);
		inst->sock = NULL;
	}
	return -1;
}

void nl_classifier_exit(struct nl_classifier_instance *inst)
{
	if (inst->sock) {
		nl_close(inst->sock);
		nl_socket_free(inst->sock);
		inst->sock = NULL;
	}
	printf("nl classifier exit successful\n");
}

int nl_classifier_parse_arg(int argc, char *argv[], unsigned char *proto, unsigned long *src_saddr,
			    unsigned long *dst_saddr, unsigned short *sport, unsigned short *dport, int *af)
{
	int ret;
	unsigned short port;

	if (argc < 7) {
		printf("help: nl_classifier <v4|v6> <udp|tcp> <source ip> <destination ip> <source port> <destination port>\n");
		return -1;
	}

	if (0 == strncmp(argv[1], "v4", 2)) {
		*af = AF_INET;
	} else if (0 == strncmp(argv[1], "v6", 2)) {
		*af = AF_INET6;
	} else {
		printf("Address family is not supported");
		return -1;
	}

	if (0 == strncmp(argv[2], "udp", 3)) {
		*proto = IPPROTO_UDP;
	} else if (0 == strncmp(argv[2], "tcp", 3)) {
		*proto = IPPROTO_TCP;
	} else {
		printf("Protocol is not supported");
		return -1;
	}

	ret = inet_pton(*af, argv[3], src_saddr);
	if (ret <= 0) {
		printf("source ip has wrong format\n");
		return -1;
	}

	ret = inet_pton(*af, argv[4], dst_saddr);
	if (ret <= 0) {
		printf("destination ip has wrong format\n");
		return -1;
	}

	port = strtol(argv[5], NULL, 0);
	*sport = htons(port);
	port = strtol(argv[6], NULL, 0);
	*dport = htons(port);

	printf("nl classifier parse arguments successful\n");
	return 0;
}

int main(int argc, char *argv[])
{
	struct nl_classifier_instance *inst = &nl_cls_inst;
	unsigned char proto;
	unsigned long src_addr[4];
	unsigned long dst_addr[4];
	unsigned short sport;
	unsigned short dport;
	int af;
	int ret;

	ret = nl_classifier_parse_arg(argc, argv, &proto, src_addr, dst_addr, &sport, &dport, &af);
	if (ret < 0) {
		printf("Failed to parse arguments\n");
		return ret;
	}

	ret = nl_classifier_init(inst);
	if (ret < 0) {
		printf("Unable to init generic netlink\n");
		return ret;
	}

	nl_classifier_offload(inst, proto, src_addr, dst_addr, sport, dport, af);

	/* main loop to listen on message */
	while (!inst->stop) {
		nl_recvmsgs_default(inst->sock);
	}

	nl_classifier_exit(inst);

	return 0;
}
