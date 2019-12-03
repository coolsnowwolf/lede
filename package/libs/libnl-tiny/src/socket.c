/*
 * lib/socket.c		Netlink Socket
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core
 * @defgroup socket Socket
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

static uint32_t used_ports_map[32];

static uint32_t generate_local_port(void)
{
	int i, n;
	uint32_t pid = getpid() & 0x3FFFFF;

	for (i = 0; i < 32; i++) {
		if (used_ports_map[i] == 0xFFFFFFFF)
			continue;

		for (n = 0; n < 32; n++) {
			if (1UL & (used_ports_map[i] >> n))
				continue;

			used_ports_map[i] |= (1UL << n);
			n += (i * 32);

			/* PID_MAX_LIMIT is currently at 2^22, leaving 10 bit
			 * to, i.e. 1024 unique ports per application. */
			return pid + (n << 22);

		}
	}

	/* Out of sockets in our own PID namespace, what to do? FIXME */
	return UINT_MAX;
}

static void release_local_port(uint32_t port)
{
	int nr;

	if (port == UINT_MAX)
		return;
	
	nr = port >> 22;
	used_ports_map[nr / 32] &= ~(1 << nr % 32);
}

/**
 * @name Allocation
 * @{
 */

static struct nl_sock *__alloc_socket(struct nl_cb *cb)
{
	struct nl_sock *sk;

	sk = calloc(1, sizeof(*sk));
	if (!sk)
		return NULL;

	sk->s_fd = -1;
	sk->s_cb = cb;
	sk->s_local.nl_family = AF_NETLINK;
	sk->s_peer.nl_family = AF_NETLINK;
	sk->s_seq_expect = sk->s_seq_next = time(0);
	sk->s_local.nl_pid = generate_local_port();
	if (sk->s_local.nl_pid == UINT_MAX) {
		nl_socket_free(sk);
		return NULL;
	}

	return sk;
}

/**
 * Allocate new netlink socket
 *
 * @return Newly allocated netlink socket or NULL.
 */
struct nl_sock *nl_socket_alloc(void)
{
	struct nl_cb *cb;
	
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		return NULL;

	return __alloc_socket(cb);
}

/**
 * Allocate new socket with custom callbacks
 * @arg cb		Callback handler
 *
 * The reference to the callback handler is taken into account
 * automatically, it is released again upon calling nl_socket_free().
 *
 *@return Newly allocted socket handle or NULL.
 */
struct nl_sock *nl_socket_alloc_cb(struct nl_cb *cb)
{
	if (cb == NULL)
		BUG();

	return __alloc_socket(nl_cb_get(cb));
}

/**
 * Free a netlink socket.
 * @arg sk		Netlink socket.
 */
void nl_socket_free(struct nl_sock *sk)
{
	if (!sk)
		return;

	if (sk->s_fd >= 0)
		close(sk->s_fd);

	if (!(sk->s_flags & NL_OWN_PORT))
		release_local_port(sk->s_local.nl_pid);

	nl_cb_put(sk->s_cb);
	free(sk);
}

/** @} */

/**
 * @name Sequence Numbers
 * @{
 */

static int noop_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}


/**
 * Disable sequence number checking.
 * @arg sk		Netlink socket.
 *
 * Disables checking of sequence numbers on the netlink socket This is
 * required to allow messages to be processed which were not requested by
 * a preceding request message, e.g. netlink events.
 *
 * @note This function modifies the NL_CB_SEQ_CHECK configuration in
 * the callback handle associated with the socket.
 */
void nl_socket_disable_seq_check(struct nl_sock *sk)
{
	nl_cb_set(sk->s_cb, NL_CB_SEQ_CHECK,
		  NL_CB_CUSTOM, noop_seq_check, NULL);
}

/** @} */

/**
 * Set local port of socket
 * @arg sk		Netlink socket.
 * @arg port		Local port identifier
 *
 * Assigns a local port identifier to the socket. If port is 0
 * a unique port identifier will be generated automatically.
 */
void nl_socket_set_local_port(struct nl_sock *sk, uint32_t port)
{
	if (port == 0) {
		port = generate_local_port(); 
		sk->s_flags &= ~NL_OWN_PORT;
	} else  {
		if (!(sk->s_flags & NL_OWN_PORT))
			release_local_port(sk->s_local.nl_pid);
		sk->s_flags |= NL_OWN_PORT;
	}

	sk->s_local.nl_pid = port;
}

/** @} */

/**
 * @name Group Subscriptions
 * @{
 */

/**
 * Join groups
 * @arg sk		Netlink socket
 * @arg group		Group identifier
 *
 * Joins the specified groups using the modern socket option which
 * is available since kernel version 2.6.14. It allows joining an
 * almost arbitary number of groups without limitation.  The list
 * of groups has to be terminated by 0 (%NFNLGRP_NONE).
 *
 * Make sure to use the correct group definitions as the older
 * bitmask definitions for nl_join_groups() are likely to still
 * be present for backward compatibility reasons.
 *
 * @return 0 on sucess or a negative error code.
 */
int nl_socket_add_memberships(struct nl_sock *sk, int group, ...)
{
	int err;
	va_list ap;

	if (sk->s_fd == -1)
		return -NLE_BAD_SOCK;

	va_start(ap, group);

	while (group != 0) {
		if (group < 0)
			return -NLE_INVAL;

		err = setsockopt(sk->s_fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
						 &group, sizeof(group));
		if (err < 0)
			return -nl_syserr2nlerr(errno);

		group = va_arg(ap, int);
	}

	va_end(ap);

	return 0;
}

/**
 * Leave groups
 * @arg sk		Netlink socket
 * @arg group		Group identifier
 *
 * Leaves the specified groups using the modern socket option
 * which is available since kernel version 2.6.14. The list of groups
 * has to terminated by 0 (%NFNLGRP_NONE).
 *
 * @see nl_socket_add_membership
 * @return 0 on success or a negative error code.
 */
int nl_socket_drop_memberships(struct nl_sock *sk, int group, ...)
{
	int err;
	va_list ap;

	if (sk->s_fd == -1)
		return -NLE_BAD_SOCK;

	va_start(ap, group);

	while (group != 0) {
		if (group < 0)
			return -NLE_INVAL;

		err = setsockopt(sk->s_fd, SOL_NETLINK, NETLINK_DROP_MEMBERSHIP,
						 &group, sizeof(group));
		if (err < 0)
			return -nl_syserr2nlerr(errno);

		group = va_arg(ap, int);
	}

	va_end(ap);

	return 0;
}


/** @} */

/**
 * Set file descriptor of socket to non-blocking state
 * @arg sk		Netlink socket.
 *
 * @return 0 on success or a negative error code.
 */
int nl_socket_set_nonblocking(struct nl_sock *sk)
{
	if (sk->s_fd == -1)
		return -NLE_BAD_SOCK;

	if (fcntl(sk->s_fd, F_SETFL, O_NONBLOCK) < 0)
		return -nl_syserr2nlerr(errno);

	return 0;
}

/** @} */

/**
 * @name Utilities
 * @{
 */

/**
 * Set socket buffer size of netlink socket.
 * @arg sk		Netlink socket.
 * @arg rxbuf		New receive socket buffer size in bytes.
 * @arg txbuf		New transmit socket buffer size in bytes.
 *
 * Sets the socket buffer size of a netlink socket to the specified
 * values \c rxbuf and \c txbuf. Providing a value of \c 0 assumes a
 * good default value.
 *
 * @note It is not required to call this function prior to nl_connect().
 * @return 0 on sucess or a negative error code.
 */
int nl_socket_set_buffer_size(struct nl_sock *sk, int rxbuf, int txbuf)
{
	int err;

	if (rxbuf <= 0)
		rxbuf = 32768;

	if (txbuf <= 0)
		txbuf = 32768;

	if (sk->s_fd == -1)
		return -NLE_BAD_SOCK;
	
	err = setsockopt(sk->s_fd, SOL_SOCKET, SO_SNDBUF,
			 &txbuf, sizeof(txbuf));
	if (err < 0)
		return -nl_syserr2nlerr(errno);

	err = setsockopt(sk->s_fd, SOL_SOCKET, SO_RCVBUF,
			 &rxbuf, sizeof(rxbuf));
	if (err < 0)
		return -nl_syserr2nlerr(errno);

	sk->s_flags |= NL_SOCK_BUFSIZE_SET;

	return 0;
}

/**
 * Enable/disable credential passing on netlink socket.
 * @arg sk		Netlink socket.
 * @arg state		New state (0 - disabled, 1 - enabled)
 *
 * @return 0 on success or a negative error code
 */
int nl_socket_set_passcred(struct nl_sock *sk, int state)
{
	int err;

	if (sk->s_fd == -1)
		return -NLE_BAD_SOCK;

	err = setsockopt(sk->s_fd, SOL_SOCKET, SO_PASSCRED,
			 &state, sizeof(state));
	if (err < 0)
		return -nl_syserr2nlerr(errno);

	if (state)
		sk->s_flags |= NL_SOCK_PASSCRED;
	else
		sk->s_flags &= ~NL_SOCK_PASSCRED;

	return 0;
}

/**
 * Enable/disable receival of additional packet information
 * @arg sk		Netlink socket.
 * @arg state		New state (0 - disabled, 1 - enabled)
 *
 * @return 0 on success or a negative error code
 */
int nl_socket_recv_pktinfo(struct nl_sock *sk, int state)
{
	int err;

	if (sk->s_fd == -1)
		return -NLE_BAD_SOCK;

	err = setsockopt(sk->s_fd, SOL_NETLINK, NETLINK_PKTINFO,
			 &state, sizeof(state));
	if (err < 0)
		return -nl_syserr2nlerr(errno);

	return 0;
}

/** @} */

/** @} */
