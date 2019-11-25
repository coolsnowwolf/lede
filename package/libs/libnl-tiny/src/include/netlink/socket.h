/*
 * netlink/socket.h		Netlink Socket
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_SOCKET_H_
#define NETLINK_SOCKET_H_

#include <netlink/types.h>
#include <netlink/handlers.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NL_SOCK_BUFSIZE_SET	(1<<0)
#define NL_SOCK_PASSCRED	(1<<1)
#define NL_OWN_PORT		(1<<2)
#define NL_MSG_PEEK		(1<<3)
#define NL_NO_AUTO_ACK		(1<<4)

struct nl_cb;
struct nl_sock
{
	struct sockaddr_nl	s_local;
	struct sockaddr_nl	s_peer;
	int			s_fd;
	int			s_proto;
	unsigned int		s_seq_next;
	unsigned int		s_seq_expect;
	int			s_flags;
	struct nl_cb *		s_cb;
};


extern struct nl_sock *	nl_socket_alloc(void);
extern struct nl_sock *	nl_socket_alloc_cb(struct nl_cb *);
extern void		nl_socket_free(struct nl_sock *);

extern void		nl_socket_set_local_port(struct nl_sock *, uint32_t);

extern int		nl_socket_add_memberships(struct nl_sock *, int, ...);
extern int		nl_socket_drop_memberships(struct nl_sock *, int, ...);

extern int		nl_socket_set_buffer_size(struct nl_sock *, int, int);
extern int		nl_socket_set_passcred(struct nl_sock *, int);
extern int		nl_socket_recv_pktinfo(struct nl_sock *, int);

extern void		nl_socket_disable_seq_check(struct nl_sock *);

extern int		nl_socket_set_nonblocking(struct nl_sock *);

/**
 * Use next sequence number
 * @arg sk		Netlink socket.
 *
 * Uses the next available sequence number and increases the counter
 * by one for subsequent calls.
 *
 * @return Unique serial sequence number
 */
static inline unsigned int nl_socket_use_seq(struct nl_sock *sk)
{
	return sk->s_seq_next++;
}

/**
 * Disable automatic request for ACK
 * @arg sk		Netlink socket.
 *
 * The default behaviour of a socket is to request an ACK for
 * each message sent to allow for the caller to synchronize to
 * the completion of the netlink operation. This function
 * disables this behaviour and will result in requests being
 * sent which will not have the NLM_F_ACK flag set automatically.
 * However, it is still possible for the caller to set the
 * NLM_F_ACK flag explicitely.
 */
static inline void nl_socket_disable_auto_ack(struct nl_sock *sk)
{
	sk->s_flags |= NL_NO_AUTO_ACK;
}

/**
 * Enable automatic request for ACK (default)
 * @arg sk		Netlink socket.
 * @see nl_socket_disable_auto_ack
 */
static inline void nl_socket_enable_auto_ack(struct nl_sock *sk)
{
	sk->s_flags &= ~NL_NO_AUTO_ACK;
}

/**
 * @name Source Idenficiation
 * @{
 */

static inline uint32_t nl_socket_get_local_port(struct nl_sock *sk)
{
	return sk->s_local.nl_pid;
}

/**
 * Join multicast groups (deprecated)
 * @arg sk		Netlink socket.
 * @arg groups		Bitmask of groups to join.
 *
 * This function defines the old way of joining multicast group which
 * has to be done prior to calling nl_connect(). It works on any kernel
 * version but is very limited as only 32 groups can be joined.
 */
static inline void nl_join_groups(struct nl_sock *sk, int groups)
{
	sk->s_local.nl_groups |= groups;
}

/**
 * @name Peer Identfication
 * @{
 */

static inline uint32_t nl_socket_get_peer_port(struct nl_sock *sk)
{
	return sk->s_peer.nl_pid;
}

static inline void nl_socket_set_peer_port(struct nl_sock *sk, uint32_t port)
{
	sk->s_peer.nl_pid = port;
}

/** @} */

/**
 * @name File Descriptor
 * @{
 */

static inline int nl_socket_get_fd(struct nl_sock *sk)
{
	return sk->s_fd;
}

/**
 * Enable use of MSG_PEEK when reading from socket
 * @arg sk		Netlink socket.
 */
static inline void nl_socket_enable_msg_peek(struct nl_sock *sk)
{
	sk->s_flags |= NL_MSG_PEEK;
}

/**
 * Disable use of MSG_PEEK when reading from socket
 * @arg sk		Netlink socket.
 */
static inline void nl_socket_disable_msg_peek(struct nl_sock *sk)
{
	sk->s_flags &= ~NL_MSG_PEEK;
}

static inline uint32_t nl_socket_get_peer_groups(struct nl_sock *sk)
{
	return sk->s_peer.nl_groups;
}

static inline void nl_socket_set_peer_groups(struct nl_sock *sk, uint32_t groups)
{
	sk->s_peer.nl_groups = groups;
}

/**
 * @name Callback Handler
 * @{
 */

static inline struct nl_cb *nl_socket_get_cb(struct nl_sock *sk)
{
	return nl_cb_get(sk->s_cb);
}

static inline void nl_socket_set_cb(struct nl_sock *sk, struct nl_cb *cb)
{
	nl_cb_put(sk->s_cb);
	sk->s_cb = nl_cb_get(cb);
}

/**
 * Modify the callback handler associated to the socket
 * @arg sk		Netlink socket.
 * @arg type		which type callback to set
 * @arg kind		kind of callback
 * @arg func		callback function
 * @arg arg		argument to be passwd to callback function
 *
 * @see nl_cb_set
 */
static inline int nl_socket_modify_cb(struct nl_sock *sk, enum nl_cb_type type,
			enum nl_cb_kind kind, nl_recvmsg_msg_cb_t func,
			void *arg)
{
	return nl_cb_set(sk->s_cb, type, kind, func, arg);
}

/** @} */

static inline int nl_socket_add_membership(struct nl_sock *sk, int group)
{
	return nl_socket_add_memberships(sk, group, 0);
}


static inline int nl_socket_drop_membership(struct nl_sock *sk, int group)
{
	return nl_socket_drop_memberships(sk, group, 0);
}



#ifdef __cplusplus
}
#endif

#endif
