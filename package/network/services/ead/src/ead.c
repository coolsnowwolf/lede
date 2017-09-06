/*
 * Emergency Access Daemon
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
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <pcap.h>
#include <pcap-bpf.h>
#include <t_pwd.h>
#include <t_read.h>
#include <t_sha.h>
#include <t_defines.h>
#include <t_server.h>
#include <net/if.h>

#include "list.h"
#include "ead.h"
#include "ead-pcap.h"
#include "ead-crypt.h"
#include "libbridge.h"

#include "filter.c"

#ifdef linux
#include <linux/if_packet.h>
#endif

#define PASSWD_FILE	"/etc/passwd"

#ifndef DEFAULT_IFNAME
#define DEFAULT_IFNAME "eth0"
#endif

#ifndef DEFAULT_DEVNAME
#define DEFAULT_DEVNAME "Unknown"
#endif

#define PCAP_MRU		1600
#define PCAP_TIMEOUT	200

#if EAD_DEBUGLEVEL >= 1
#define DEBUG(n, format, ...) do { \
	if (EAD_DEBUGLEVEL >= n) \
		fprintf(stderr, format, ##__VA_ARGS__); \
} while (0);

#else
#define DEBUG(n, format, ...) do {} while(0)
#endif

struct ead_instance {
	struct list_head list;
	char ifname[16];
	int pid;
	char id;
	char bridge[16];
	bool br_check;
};

static char ethmac[6] = "\x00\x13\x37\x00\x00\x00"; /* last 3 bytes will be randomized */
static pcap_t *pcap_fp = NULL;
static pcap_t *pcap_fp_rx = NULL;
static char pktbuf_b[PCAP_MRU];
static struct ead_packet *pktbuf = (struct ead_packet *)pktbuf_b;
static u16_t nid = 0xffff; /* node id */
static char username[32] = "";
static int state = EAD_TYPE_SET_USERNAME;
static const char *passwd_file = PASSWD_FILE;
static const char password[MAXPARAMLEN];
static bool child_pending = false;

static unsigned char abuf[MAXPARAMLEN + 1];
static unsigned char pwbuf[MAXPARAMLEN];
static unsigned char saltbuf[MAXSALTLEN];
static unsigned char pw_saltbuf[MAXSALTLEN];
static struct list_head instances;
static const char *dev_name = DEFAULT_DEVNAME;
static bool nonfork = false;
static struct ead_instance *instance = NULL;

static struct t_pwent tpe = {
	.name = username,
	.index = 1,
	.password.data = pwbuf,
	.password.len = 0,
	.salt.data = saltbuf,
	.salt.len = 0,
};
struct t_confent *tce = NULL;
static struct t_server *ts = NULL;
static struct t_num A, *B = NULL;
unsigned char *skey;

static void
set_recv_type(pcap_t *p, bool rx)
{
#ifdef PACKET_RECV_TYPE
	struct sockaddr_ll sll;
	struct ifreq ifr;
	int mask;
	int fd;

	fd = pcap_get_selectable_fd(p);
	if (fd < 0)
		return;

	if (rx)
		mask = 1 << PACKET_BROADCAST;
	else
		mask = 0;

	setsockopt(fd, SOL_PACKET, PACKET_RECV_TYPE, &mask, sizeof(mask));
#endif
}


static pcap_t *
ead_open_pcap(const char *ifname, char *errbuf, bool rx)
{
	pcap_t *p;

	p = pcap_create(ifname, errbuf);
	if (p == NULL)
		goto out;

	pcap_set_snaplen(p, PCAP_MRU);
	pcap_set_promisc(p, rx);
	pcap_set_timeout(p, PCAP_TIMEOUT);
#ifdef HAS_PROTO_EXTENSION
	pcap_set_protocol(p, (rx ? htons(ETH_P_IP) : 0));
#endif
	pcap_set_buffer_size(p, (rx ? 10 : 1) * PCAP_MRU);
	pcap_activate(p);
	set_recv_type(p, rx);
out:
	return p;
}

static void
get_random_bytes(void *ptr, int len)
{
	int fd;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(1);
	}
	read(fd, ptr, len);
	close(fd);
}

static bool
prepare_password(void)
{
	static char lbuf[1024];
	unsigned char dig[SHA_DIGESTSIZE];
	BigInteger x, v, n, g;
	SHA1_CTX ctxt;
	int ulen = strlen(username);
	FILE *f;

	lbuf[sizeof(lbuf) - 1] = 0;

	f = fopen(passwd_file, "r");
	if (!f)
		return false;

	while (fgets(lbuf, sizeof(lbuf) - 1, f) != NULL) {
		char *str, *s2;

		if (strncmp(lbuf, username, ulen) != 0)
			continue;

		if (lbuf[ulen] != ':')
			continue;

		str = &lbuf[ulen + 1];

		if (strncmp(str, "$1$", 3) != 0)
			continue;

		s2 = strchr(str + 3, '$');
		if (!s2)
			continue;

		if (s2 - str >= MAXSALTLEN)
			continue;

		strncpy((char *) pw_saltbuf, str, s2 - str);
		pw_saltbuf[s2 - str] = 0;

		s2 = strchr(s2, ':');
		if (!s2)
			continue;

		*s2 = 0;
		if (s2 - str >= MAXPARAMLEN)
			continue;

		strncpy((char *)password, str, MAXPARAMLEN);
		fclose(f);
		goto hash_password;
	}

	/* not found */
	fclose(f);
	return false;

hash_password:
	tce = gettcid(tpe.index);
	do {
		t_random(tpe.password.data, SALTLEN);
	} while (memcmp(saltbuf, (char *)dig, sizeof(saltbuf)) == 0);
	if (saltbuf[0] == 0)
		saltbuf[0] = 0xff;

	n = BigIntegerFromBytes(tce->modulus.data, tce->modulus.len);
	g = BigIntegerFromBytes(tce->generator.data, tce->generator.len);
	v = BigIntegerFromInt(0);

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, (unsigned char *) username, strlen(username));
	SHA1Update(&ctxt, (unsigned char *) ":", 1);
	SHA1Update(&ctxt, (unsigned char *) password, strlen(password));
	SHA1Final(dig, &ctxt);

	SHA1Init(&ctxt);
	SHA1Update(&ctxt, saltbuf, tpe.salt.len);
	SHA1Update(&ctxt, dig, sizeof(dig));
	SHA1Final(dig, &ctxt);

	/* x = H(s, H(u, ':', p)) */
	x = BigIntegerFromBytes(dig, sizeof(dig));

	BigIntegerModExp(v, g, x, n);
	tpe.password.len = BigIntegerToBytes(v, (unsigned char *)pwbuf);

	BigIntegerFree(v);
	BigIntegerFree(x);
	BigIntegerFree(g);
	BigIntegerFree(n);
	return true;
}

static u16_t
chksum(u16_t sum, const u8_t *data, u16_t len)
{
	u16_t t;
	const u8_t *dataptr;
	const u8_t *last_byte;

	dataptr = data;
	last_byte = data + len - 1;

	while(dataptr < last_byte) {	/* At least two more bytes */
		t = (dataptr[0] << 8) + dataptr[1];
		sum += t;
		if(sum < t) {
			sum++;		/* carry */
		}
		dataptr += 2;
	}

	if(dataptr == last_byte) {
		t = (dataptr[0] << 8) + 0;
		sum += t;
		if(sum < t) {
			sum++;		/* carry */
		}
	}

	/* Return sum in host byte order. */
	return sum;
}

static void
ead_send_packet_clone(struct ead_packet *pkt)
{
	u16_t len, sum;

	memcpy(pktbuf, pkt, offsetof(struct ead_packet, msg));
	memcpy(pktbuf->eh.ether_shost, ethmac, 6);
	memcpy(pktbuf->eh.ether_dhost, pkt->eh.ether_shost, 6);

	/* ip header */
	len = sizeof(struct ead_packet) - sizeof(struct ether_header) + ntohl(pktbuf->msg.len);
	pktbuf->len[0] = len >> 8;
	pktbuf->len[1] = len & 0xff;
	memcpy(pktbuf->srcipaddr, &pkt->msg.ip, 4);
	memcpy(pktbuf->destipaddr, pkt->srcipaddr, 4);

	/* ip checksum */
	pktbuf->ipchksum = 0;
	sum = chksum(0, (void *) &pktbuf->vhl, UIP_IPH_LEN);
	if (sum == 0)
		sum = 0xffff;
	pktbuf->ipchksum = htons(~sum);

	/* udp header */
	pktbuf->srcport = pkt->destport;
	pktbuf->destport = pkt->srcport;

	/* udp checksum */
	len -= UIP_IPH_LEN;
	pktbuf->udplen = htons(len);
	pktbuf->udpchksum = 0;
	sum = len + UIP_PROTO_UDP;
	sum = chksum(sum, (void *) &pktbuf->srcipaddr[0], 8); /* src, dest ip */
	sum = chksum(sum, (void *) &pktbuf->srcport, len);
	if (sum == 0)
		sum = 0xffff;
	pktbuf->udpchksum = htons(~sum);
	pcap_sendpacket(pcap_fp, (void *) pktbuf, sizeof(struct ead_packet) + ntohl(pktbuf->msg.len));
}

static void
set_state(int nstate)
{
	if (state == nstate)
		return;

	if (nstate < state) {
		if ((nstate < EAD_TYPE_GET_PRIME) &&
			(state >= EAD_TYPE_GET_PRIME)) {
			t_serverclose(ts);
			ts = NULL;
		}
		goto done;
	}

	switch(state) {
	case EAD_TYPE_SET_USERNAME:
		if (!prepare_password())
			goto error;
		ts = t_serveropenraw(&tpe, tce);
		if (!ts)
			goto error;
		break;
	case EAD_TYPE_GET_PRIME:
		B = t_servergenexp(ts);
		break;
	case EAD_TYPE_SEND_A:
		skey = t_servergetkey(ts, &A);
		if (!skey)
			goto error;

		ead_set_key(skey);
		break;
	}
done:
	state = nstate;
error:
	return;
}

static bool
handle_ping(struct ead_packet *pkt, int len, int *nstate)
{
	struct ead_msg *msg = &pktbuf->msg;
	struct ead_msg_pong *pong = EAD_DATA(msg, pong);
	int slen;

	slen = strlen(dev_name);
	if (slen > 1024)
		slen = 1024;

	msg->len = htonl(sizeof(struct ead_msg_pong) + slen);
	strncpy(pong->name, dev_name, slen);
	pong->name[slen] = 0;
	pong->auth_type = htons(EAD_AUTH_MD5);

	return true;
}

static bool
handle_set_username(struct ead_packet *pkt, int len, int *nstate)
{
	struct ead_msg *msg = &pkt->msg;
	struct ead_msg_user *user = EAD_DATA(msg, user);

	set_state(EAD_TYPE_SET_USERNAME); /* clear old state */
	strncpy(username, user->username, sizeof(username));
	username[sizeof(username) - 1] = 0;

	msg = &pktbuf->msg;
	msg->len = 0;

	*nstate = EAD_TYPE_GET_PRIME;
	return true;
}

static bool
handle_get_prime(struct ead_packet *pkt, int len, int *nstate)
{
	struct ead_msg *msg = &pktbuf->msg;
	struct ead_msg_salt *salt = EAD_DATA(msg, salt);

	msg->len = htonl(sizeof(struct ead_msg_salt));
	salt->prime = tce->index - 1;
	salt->len = ts->s.len;
	memcpy(salt->salt, ts->s.data, ts->s.len);
	memcpy(salt->ext_salt, pw_saltbuf, MAXSALTLEN);

	*nstate = EAD_TYPE_SEND_A;
	return true;
}

static bool
handle_send_a(struct ead_packet *pkt, int len, int *nstate)
{
	struct ead_msg *msg = &pkt->msg;
	struct ead_msg_number *number = EAD_DATA(msg, number);
	len = ntohl(msg->len) - sizeof(struct ead_msg_number);

	if (len > MAXPARAMLEN + 1)
		return false;

	A.len = len;
	A.data = abuf;
	memcpy(A.data, number->data, len);

	msg = &pktbuf->msg;
	number = EAD_DATA(msg, number);
	msg->len = htonl(sizeof(struct ead_msg_number) + B->len);
	memcpy(number->data, B->data, B->len);

	*nstate = EAD_TYPE_SEND_AUTH;
	return true;
}

static bool
handle_send_auth(struct ead_packet *pkt, int len, int *nstate)
{
	struct ead_msg *msg = &pkt->msg;
	struct ead_msg_auth *auth = EAD_DATA(msg, auth);

	if (t_serververify(ts, auth->data) != 0) {
		DEBUG(2, "Client authentication failed\n");
		*nstate = EAD_TYPE_SET_USERNAME;
		return false;
	}

	msg = &pktbuf->msg;
	auth = EAD_DATA(msg, auth);
	msg->len = htonl(sizeof(struct ead_msg_auth));

	DEBUG(2, "Client authentication successful\n");
	memcpy(auth->data, t_serverresponse(ts), sizeof(auth->data));

	*nstate = EAD_TYPE_SEND_CMD;
	return true;
}

static bool
handle_send_cmd(struct ead_packet *pkt, int len, int *nstate)
{
	struct ead_msg *msg = &pkt->msg;
	struct ead_msg_cmd *cmd = EAD_ENC_DATA(msg, cmd);
	struct ead_msg_cmd_data *cmddata;
	struct timeval tv, to, tn;
	int pfd[2], fd;
	fd_set fds;
	pid_t pid;
	bool stream = false;
	int timeout;
	int type;
	int datalen;

	datalen = ead_decrypt_message(msg) - sizeof(struct ead_msg_cmd);
	if (datalen <= 0)
		return false;

	type = ntohs(cmd->type);
	timeout = ntohs(cmd->timeout);

	FD_ZERO(&fds);
	cmd->data[datalen] = 0;
	switch(type) {
	case EAD_CMD_NORMAL:
		if (pipe(pfd) < 0)
			return false;

		fcntl(pfd[0], F_SETFL, O_NONBLOCK | fcntl(pfd[0], F_GETFL));
		child_pending = true;
		pid = fork();
		if (pid == 0) {
			close(pfd[0]);
			fd = open("/dev/null", O_RDWR);
			if (fd > 0) {
				dup2(fd, 0);
				dup2(pfd[1], 1);
				dup2(pfd[1], 2);
			}
			system((char *)cmd->data);
			exit(0);
		} else if (pid > 0) {
			close(pfd[1]);
			if (!timeout)
				timeout = EAD_CMD_TIMEOUT;

			stream = true;
			break;
		}
		return false;
	case EAD_CMD_BACKGROUND:
		pid = fork();
		if (pid == 0) {
			/* close stdin, stdout, stderr, replace with fd to /dev/null */
			fd = open("/dev/null", O_RDWR);
			if (fd > 0) {
				dup2(fd, 0);
				dup2(fd, 1);
				dup2(fd, 2);
			}
			system((char *)cmd->data);
			exit(0);
		} else if (pid > 0) {
			break;
		}
		return false;
	default:
		return false;
	}

	msg = &pktbuf->msg;
	cmddata = EAD_ENC_DATA(msg, cmd_data);

	if (stream) {
		int nfds, bytes;

		/* send keepalive packets every 200 ms so that the client doesn't timeout */
		gettimeofday(&to, NULL);
		memcpy(&tn, &to, sizeof(tn));
		tv.tv_usec = PCAP_TIMEOUT * 1000;
		tv.tv_sec = 0;
		do {
			cmddata->done = 0;
			FD_SET(pfd[0], &fds);
			nfds = select(pfd[0] + 1, &fds, NULL, NULL, &tv);
			bytes = 0;
			if (nfds > 0) {
				bytes = read(pfd[0], cmddata->data, 1024);
				if (bytes < 0)
					bytes = 0;
			}
			if (!bytes && !child_pending)
				break;
			DEBUG(3, "Sending %d bytes of console data, type=%d, timeout=%d\n", bytes, ntohl(msg->type), timeout);
			ead_encrypt_message(msg, sizeof(struct ead_msg_cmd_data) + bytes);
			ead_send_packet_clone(pkt);
			gettimeofday(&tn, NULL);
		} while (tn.tv_sec < to.tv_sec + timeout);
		if (child_pending) {
			kill(pid, SIGKILL);
			return false;
		}
	}
	cmddata->done = 1;
	ead_encrypt_message(msg, sizeof(struct ead_msg_cmd_data));

	return true;
}



static void
parse_message(struct ead_packet *pkt, int len)
{
	bool (*handler)(struct ead_packet *pkt, int len, int *nstate);
	int min_len = sizeof(struct ead_packet);
	int nstate = state;
	int type = ntohl(pkt->msg.type);

	if ((type >= EAD_TYPE_GET_PRIME) &&
		(state != type))
		return;

	if ((type != EAD_TYPE_PING) &&
		((ntohs(pkt->msg.sid) & EAD_INSTANCE_MASK) >>
		 EAD_INSTANCE_SHIFT) != instance->id)
		return;

	switch(type) {
	case EAD_TYPE_PING:
		handler = handle_ping;
		break;
	case EAD_TYPE_SET_USERNAME:
		handler = handle_set_username;
		min_len += sizeof(struct ead_msg_user);
		break;
	case EAD_TYPE_GET_PRIME:
		handler = handle_get_prime;
		break;
	case EAD_TYPE_SEND_A:
		handler = handle_send_a;
		min_len += sizeof(struct ead_msg_number);
		break;
	case EAD_TYPE_SEND_AUTH:
		handler = handle_send_auth;
		min_len += sizeof(struct ead_msg_auth);
		break;
	case EAD_TYPE_SEND_CMD:
		handler = handle_send_cmd;
		min_len += sizeof(struct ead_msg_cmd) + sizeof(struct ead_msg_encrypted);
		break;
	default:
		return;
	}

	if (len < min_len) {
		DEBUG(2, "discarding packet: message too small\n");
		return;
	}

	pktbuf->msg.magic = htonl(EAD_MAGIC);
	pktbuf->msg.type = htonl(type + 1);
	pktbuf->msg.nid = htons(nid);
	pktbuf->msg.sid = pkt->msg.sid;
	pktbuf->msg.len = 0;

	if (handler(pkt, len, &nstate)) {
		DEBUG(2, "sending response to packet type %d: %d\n", type + 1, ntohl(pktbuf->msg.len));
		/* format response packet */
		ead_send_packet_clone(pkt);
	}
	set_state(nstate);
}

static void
handle_packet(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
	struct ead_packet *pkt = (struct ead_packet *) bytes;

	if (h->len < sizeof(struct ead_packet))
		return;

	if (pkt->eh.ether_type != htons(ETHERTYPE_IP))
		return;

	if (memcmp(pkt->eh.ether_dhost, "\xff\xff\xff\xff\xff\xff", 6) != 0)
		return;

	if (pkt->proto != UIP_PROTO_UDP)
		return;

	if (pkt->destport != htons(EAD_PORT))
		return;

	if (pkt->msg.magic != htonl(EAD_MAGIC))
		return;

	if (h->len < sizeof(struct ead_packet) + ntohl(pkt->msg.len))
		return;

	if ((pkt->msg.nid != 0xffff) &&
		(pkt->msg.nid != htons(nid)))
		return;

	parse_message(pkt, h->len);
}

static void
ead_pcap_reopen(bool first)
{
	static char errbuf[PCAP_ERRBUF_SIZE] = "";

	if (pcap_fp_rx && (pcap_fp_rx != pcap_fp))
		pcap_close(pcap_fp_rx);

	if (pcap_fp)
		pcap_close(pcap_fp);

	pcap_fp_rx = NULL;
	do {
		if (instance->bridge[0]) {
			pcap_fp_rx = ead_open_pcap(instance->bridge, errbuf, 1);
			pcap_fp = ead_open_pcap(instance->ifname, errbuf, 0);
		} else {
			pcap_fp = ead_open_pcap(instance->ifname, errbuf, 1);
		}

		if (!pcap_fp_rx)
			pcap_fp_rx = pcap_fp;
		if (first && !pcap_fp) {
			DEBUG(1, "WARNING: unable to open interface '%s'\n", instance->ifname);
			first = false;
		}
		if (!pcap_fp)
			sleep(1);
	} while (!pcap_fp);
	pcap_setfilter(pcap_fp_rx, &pktfilter);
}


static void
ead_pktloop(void)
{
	while (1) {
		if (pcap_dispatch(pcap_fp_rx, 1, handle_packet, NULL) < 0) {
			ead_pcap_reopen(false);
			continue;
		}
	}
}


static int
usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [<options>]\n"
		"Options:\n"
		"\t-B             Run in background mode\n"
		"\t-d <device>    Set the device to listen on\n"
		"\t-D <name>      Set the name of the device visible to clients\n"
		"\t-p <file>      Set the password file for authenticating\n"
		"\t-P <file>      Write a pidfile\n"
		"\n", prog);
	return -1;
}

static void
server_handle_sigchld(int sig)
{
	struct ead_instance *in;
	struct list_head *p;
	int pid = 0;
	wait(&pid);

	list_for_each(p, &instances) {
		in = list_entry(p, struct ead_instance, list);
		if (pid != in->pid)
			continue;

		in->pid = 0;
		break;
	}
}

static void
instance_handle_sigchld(int sig)
{
	int pid = 0;
	wait(&pid);
	child_pending = false;
}

static void
start_server(struct ead_instance *i)
{
	if (!nonfork) {
		i->pid = fork();
		if (i->pid != 0) {
			if (i->pid < 0)
				i->pid = 0;
			return;
		}
	}

	instance = i;
	signal(SIGCHLD, instance_handle_sigchld);
	ead_pcap_reopen(true);
	ead_pktloop();
	pcap_close(pcap_fp);
	if (pcap_fp_rx != pcap_fp)
		pcap_close(pcap_fp_rx);

	exit(0);
}


static void
start_servers(bool restart)
{
	struct ead_instance *in;
	struct list_head *p;

	list_for_each(p, &instances) {
		in = list_entry(p, struct ead_instance, list);
		if (in->pid > 0)
			continue;

		sleep(1);
		start_server(in);
	}
}

static void
stop_server(struct ead_instance *in, bool do_free)
{
	if (in->pid > 0)
		kill(in->pid, SIGKILL);
	in->pid = 0;
	if (do_free) {
		list_del(&in->list);
		free(in);
	}
}

static void
server_handle_sigint(int sig)
{
	struct ead_instance *in;
	struct list_head *p, *tmp;

	list_for_each_safe(p, tmp, &instances) {
		in = list_entry(p, struct ead_instance, list);
		stop_server(in, true);
	}
	exit(1);
}

static int
check_bridge_port(const char *br, const char *port, void *arg)
{
	struct ead_instance *in;
	struct list_head *p;

	list_for_each(p, &instances) {
		in = list_entry(p, struct ead_instance, list);

		if (strcmp(in->ifname, port) != 0)
			continue;

		in->br_check = true;
		if (strcmp(in->bridge, br) == 0)
			break;

		strncpy(in->bridge, br, sizeof(in->bridge));
		DEBUG(2, "assigning port %s to bridge %s\n", in->ifname, in->bridge);
		stop_server(in, false);
	}
	return 0;
}

static int
check_bridge(const char *name, void *arg)
{
	br_foreach_port(name, check_bridge_port, arg);
	return 0;
}

static void
check_all_interfaces(void)
{
	struct ead_instance *in;
	struct list_head *p;

	br_foreach_bridge(check_bridge, NULL);

	/* look for interfaces that are no longer part of a bridge */
	list_for_each(p, &instances) {
		in = list_entry(p, struct ead_instance, list);

		if (in->br_check) {
			in->br_check = false;
		} else if (in->bridge[0]) {
			DEBUG(2, "removing port %s from bridge %s\n", in->ifname, in->bridge);
			in->bridge[0] = 0;
			stop_server(in, false);
		}
	}
}


int main(int argc, char **argv)
{
	struct ead_instance *in;
	struct timeval tv;
	const char *pidfile = NULL;
	bool background = false;
	int n_iface = 0;
	int fd, ch;

	if (argc == 1)
		return usage(argv[0]);

	INIT_LIST_HEAD(&instances);
	while ((ch = getopt(argc, argv, "Bd:D:fhp:P:")) != -1) {
		switch(ch) {
		case 'B':
			background = true;
			break;
		case 'f':
			nonfork = true;
			break;
		case 'h':
			return usage(argv[0]);
		case 'd':
			in = malloc(sizeof(struct ead_instance));
			memset(in, 0, sizeof(struct ead_instance));
			INIT_LIST_HEAD(&in->list);
			strncpy(in->ifname, optarg, sizeof(in->ifname) - 1);
			list_add(&in->list, &instances);
			in->id = n_iface++;
			break;
		case 'D':
			dev_name = optarg;
			break;
		case 'p':
			passwd_file = optarg;
			break;
		case 'P':
			pidfile = optarg;
			break;
		}
	}
	signal(SIGCHLD, server_handle_sigchld);
	signal(SIGINT, server_handle_sigint);
	signal(SIGTERM, server_handle_sigint);
	signal(SIGKILL, server_handle_sigint);

	if (!n_iface) {
		fprintf(stderr, "Error: ead needs at least one interface\n");
		return -1;
	}

	if (background) {
		if (fork() > 0)
			exit(0);

		fd = open("/dev/null", O_RDWR);
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
	}

	if (pidfile) {
		char pid[8];
		int len;

		unlink(pidfile);
		fd = open(pidfile, O_CREAT|O_WRONLY|O_EXCL, 0644);
		if (fd > 0) {
			len = sprintf(pid, "%d\n", getpid());
			write(fd, pid, len);
			close(fd);
		}
	}

	/* randomize the mac address */
	get_random_bytes(ethmac + 3, 3);
	nid = *(((u16_t *) ethmac) + 2);

	start_servers(false);
	br_init();
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	while (1) {
		check_all_interfaces();
		start_servers(true);
		sleep(1);
	}
	br_shutdown();

	return 0;
}
