/*
 * Client for the Emergency Access Daemon
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <t_pwd.h>
#include <t_read.h>
#include <t_sha.h>
#include <t_defines.h>
#include <t_client.h>
#include "ead.h"
#include "ead-crypt.h"

#include "pw_encrypt_md5.c"

#define EAD_TIMEOUT	400
#define EAD_TIMEOUT_LONG 2000

static char msgbuf[1500];
static struct ead_msg *msg = (struct ead_msg *) msgbuf;
static uint16_t nid = 0xffff;
struct sockaddr_in local, remote;
static int s = 0;
static int sockflags;
static struct in_addr serverip = {
	.s_addr = 0x01010101 /* dummy */
};

static unsigned char *skey = NULL;
static unsigned char bbuf[MAXPARAMLEN];
static unsigned char saltbuf[MAXSALTLEN];
static char *username = NULL;
static char password[MAXPARAMLEN] = "";
static char pw_md5[MD5_OUT_BUFSIZE];
static char pw_salt[MAXSALTLEN];

static struct t_client *tc = NULL;
static struct t_num salt = { .data = saltbuf };
static struct t_num *A, B;
static struct t_preconf *tcp;
static int auth_type = EAD_AUTH_DEFAULT;
static int timeout = EAD_TIMEOUT;
static uint16_t sid = 0;

static void
set_nonblock(int enable)
{
	if (enable == !!(sockflags & O_NONBLOCK))
		return;

	sockflags ^= O_NONBLOCK;
	fcntl(s, F_SETFL, sockflags);
}

static int
send_packet(int type, bool (*handler)(void), unsigned int max)
{
	struct timeval tv;
	fd_set fds;
	int nfds;
	int len;
	int res = 0;

	type = htonl(type);
	memcpy(&msg->ip, &serverip.s_addr, sizeof(msg->ip));
	set_nonblock(0);
	sendto(s, msgbuf, sizeof(struct ead_msg) + ntohl(msg->len), 0, (struct sockaddr *) &remote, sizeof(remote));
	set_nonblock(1);

	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	FD_ZERO(&fds);
	do {
		FD_SET(s, &fds);
		nfds = select(s + 1, &fds, NULL, NULL, &tv);

		if (nfds <= 0)
			break;

		if (!FD_ISSET(s, &fds))
			break;

		len = read(s, msgbuf, sizeof(msgbuf));
		if (len < 0)
			break;

		if (len < sizeof(struct ead_msg))
			continue;

		if (len < sizeof(struct ead_msg) + ntohl(msg->len))
			continue;

		if (msg->magic != htonl(EAD_MAGIC))
			continue;

		if ((nid != 0xffff) && (ntohs(msg->nid) != nid))
			continue;

		if (msg->type != type)
			continue;

		if (handler())
			res++;

		if ((max > 0) && (res >= max))
			break;
	} while (1);

	return res;
}

static void
prepare_password(void)
{
	switch(auth_type) {
	case EAD_AUTH_DEFAULT:
		break;
	case EAD_AUTH_MD5:
		md5_crypt(pw_md5, (unsigned char *) password, (unsigned char *) pw_salt);
		strncpy(password, pw_md5, sizeof(password));
		break;
	}
}

static bool
handle_pong(void)
{
	struct ead_msg_pong *pong = EAD_DATA(msg, pong);
	int len = ntohl(msg->len) - sizeof(struct ead_msg_pong);

	if (len <= 0)
		return false;

	pong->name[len] = 0;
	auth_type = ntohs(pong->auth_type);
	if (nid == 0xffff)
		printf("%04x: %s\n", ntohs(msg->nid), pong->name);
	sid = msg->sid;
	return true;
}

static bool
handle_prime(void)
{
	struct ead_msg_salt *sb = EAD_DATA(msg, salt);

	salt.len = sb->len;
	memcpy(salt.data, sb->salt, salt.len);

	if (auth_type == EAD_AUTH_MD5) {
		memcpy(pw_salt, sb->ext_salt, MAXSALTLEN);
		pw_salt[MAXSALTLEN - 1] = 0;
	}

	tcp = t_getpreparam(sb->prime);
	tc = t_clientopen(username, &tcp->modulus, &tcp->generator, &salt);
	if (!tc) {
		fprintf(stderr, "Client open failed\n");
		return false;
	}

	return true;
}

static bool
handle_b(void)
{
	struct ead_msg_number *num = EAD_DATA(msg, number);
	int len = ntohl(msg->len) - sizeof(struct ead_msg_number);

	B.data = bbuf;
	B.len = len;
	memcpy(bbuf, num->data, len);
	return true;
}

static bool
handle_none(void)
{
	return true;
}

static bool
handle_done_auth(void)
{
	struct ead_msg_auth *auth = EAD_DATA(msg, auth);
	if (t_clientverify(tc, auth->data) != 0) {
		fprintf(stderr, "Client auth verify failed\n");
		return false;
	}
	return true;
}

static bool
handle_cmd_data(void)
{
	struct ead_msg_cmd_data *cmd = EAD_ENC_DATA(msg, cmd_data);
	int datalen = ead_decrypt_message(msg) - sizeof(struct ead_msg_cmd_data);

	if (datalen < 0)
		return false;

	if (datalen > 0) {
		write(1, cmd->data, datalen);
	}

	return !!cmd->done;
}
static int
send_ping(void)
{
	msg->type = htonl(EAD_TYPE_PING);
	msg->len = 0;
	return send_packet(EAD_TYPE_PONG, handle_pong, (nid == 0xffff ? 0 : 1));
}

static int
send_username(void)
{
	msg->type = htonl(EAD_TYPE_SET_USERNAME);
	msg->len = htonl(sizeof(struct ead_msg_user));
	strcpy(EAD_DATA(msg, user)->username, username);
	return send_packet(EAD_TYPE_ACK_USERNAME, handle_none, 1);
}

static int
get_prime(void)
{
	msg->type = htonl(EAD_TYPE_GET_PRIME);
	msg->len = 0;
	return send_packet(EAD_TYPE_PRIME, handle_prime, 1);
}

static int
send_a(void)
{
	struct ead_msg_number *num = EAD_DATA(msg, number);
	A = t_clientgenexp(tc);
	msg->type = htonl(EAD_TYPE_SEND_A);
	msg->len = htonl(sizeof(struct ead_msg_number) + A->len);
	memcpy(num->data, A->data, A->len);
	return send_packet(EAD_TYPE_SEND_B, handle_b, 1);
}

static int
send_auth(void)
{
	struct ead_msg_auth *auth = EAD_DATA(msg, auth);

	prepare_password();
	t_clientpasswd(tc, password);
	skey = t_clientgetkey(tc, &B);
	if (!skey)
		return 0;

	ead_set_key(skey);
	msg->type = htonl(EAD_TYPE_SEND_AUTH);
	msg->len = htonl(sizeof(struct ead_msg_auth));
	memcpy(auth->data, t_clientresponse(tc), sizeof(auth->data));
	return send_packet(EAD_TYPE_DONE_AUTH, handle_done_auth, 1);
}

static int
send_command(const char *command)
{
	struct ead_msg_cmd *cmd = EAD_ENC_DATA(msg, cmd);

	msg->type = htonl(EAD_TYPE_SEND_CMD);
	cmd->type = htons(EAD_CMD_NORMAL);
	cmd->timeout = htons(10);
	strncpy((char *)cmd->data, command, 1024);
	ead_encrypt_message(msg, sizeof(struct ead_msg_cmd) + strlen(command) + 1);
	return send_packet(EAD_TYPE_RESULT_CMD, handle_cmd_data, 1);
}


static int
usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [-s <addr>] [-b <addr>] <node> <username>[:<password>] <command>\n"
		"\n"
		"\t-s <addr>:  Set the server's source address to <addr>\n"
		"\t-b <addr>:  Set the broadcast address to <addr>\n"
		"\t<node>:     Node ID (4 digits hex)\n"
		"\t<username>: Username to authenticate with\n"
		"\n"
		"\tPassing no arguments shows a list of active nodes on the network\n"
		"\n", prog);
	return -1;
}


int main(int argc, char **argv)
{
	int val = 1;
	char *st = NULL;
	const char *command = NULL;
	const char *prog = argv[0];
	int ch;

	msg->magic = htonl(EAD_MAGIC);
	msg->sid = 0;

	memset(&local, 0, sizeof(local));
	memset(&remote, 0, sizeof(remote));

	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = 0xffffffff;
	remote.sin_port = htons(EAD_PORT);

	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = 0;

	while ((ch = getopt(argc, argv, "b:s:h")) != -1) {
		switch(ch) {
		case 's':
			inet_aton(optarg, &serverip);
			break;
		case 'b':
			inet_aton(optarg, &remote.sin_addr);
			break;
		case 'h':
			return usage(prog);
		}
	}
	argv += optind;
	argc -= optind;

	switch(argc) {
	case 3:
		command = argv[2];
		/* fall through */
	case 2:
		username = argv[1];
		st = strchr(username, ':');
		if (st) {
			*st = 0;
			st++;
			strncpy(password, st, sizeof(password));
			password[sizeof(password) - 1] = 0;
			/* hide command line password */
			memset(st, 0, strlen(st));
		}
		/* fall through */
	case 1:
		nid = strtoul(argv[0], &st, 16);
		if (st && st[0] != 0)
			return usage(prog);
		/* fall through */
	case 0:
		break;
	default:
		return usage(prog);
	}

	msg->nid = htons(nid);
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s < 0) {
		perror("socket");
		return -1;
	}

	setsockopt(s, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val));

	if (bind(s, (struct sockaddr *)&local, sizeof(local)) < 0) {
		perror("bind");
		return -1;
	}
	sockflags = fcntl(s, F_GETFL);

	if (!send_ping()) {
		fprintf(stderr, "No devices found\n");
		return 1;
	}

	if (nid == 0xffff)
		return 0;

	if (!username || !password[0])
		return 0;

	if (!send_username()) {
		fprintf(stderr, "Device did not accept user name\n");
		return 1;
	}
	timeout = EAD_TIMEOUT_LONG;
	if (!get_prime()) {
		fprintf(stderr, "Failed to get user password info\n");
		return 1;
	}
	if (!send_a()) {
		fprintf(stderr, "Failed to send local authentication data\n");
		return 1;
	}
	if (!send_auth()) {
		fprintf(stderr, "Authentication failed\n");
		return 1;
	}
	if (!command) {
		fprintf(stderr, "Authentication succesful\n");
		return 0;
	}
	if (!send_command(command)) {
		fprintf(stderr, "Command failed\n");
		return 1;
	}

	return 0;
}
