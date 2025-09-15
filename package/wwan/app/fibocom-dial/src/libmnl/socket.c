/*
 * (C) 2008-2010 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "libmnl.h"

/**
 * \mainpage
 *
 * libmnl is a minimalistic user-space library oriented to Netlink developers.
 * There are a lot of common tasks in parsing, validating, constructing of
 * both the Netlink header and TLVs that are repetitive and easy to get wrong.
 * This library aims to provide simple helpers that allows you to avoid
 * re-inventing the wheel in common Netlink tasks.
 *
 * \verbatim
"Simplify, simplify" -- Henry David Thoureau. Walden (1854)
\endverbatim
 *
 * The acronym libmnl stands for LIBrary Minimalistic NetLink.
 *
 * libmnl homepage is:
 *      http://www.netfilter.org/projects/libmnl/
 *
 * \section features Main Features
 * - Small: the shared library requires around 30KB for an x86-based computer.
 * - Simple: this library avoids complex abstractions that tend to hide Netlink
 *   details. It avoids elaborated object-oriented infrastructure and complex
 *   callback-based workflow.
 * - Easy to use: the library simplifies the work for Netlink-wise developers.
 *   It provides functions to make socket handling, message building,
 *   validating, parsing and sequence tracking, easier.
 * - Easy to re-use: you can use this library to build your own abstraction
 *   layer upon this library, if you want to provide another library that
 *   hides Netlink details to your users.
 * - Decoupling: the interdependency of the main bricks that compose this
 *   library is reduced, i.e. the library provides many helpers, but the
 *   programmer is not forced to use them.
 *
 * \section licensing Licensing terms
 * This library is released under the LGPLv2.1 or any later (at your option).
 *
 * \section Dependencies
 * You have to install the Linux kernel headers that you want to use to develop
 * your application. Moreover, this library requires that you have some basics
 * on Netlink.
 *
 * \section scm Git Tree
 * The current development version of libmnl can be accessed at:
 * http://git.netfilter.org/cgi-bin/gitweb.cgi?p=libmnl.git;a=summary
 *
 * \section using Using libmnl
 * You can access several example files under examples/ in the libmnl source
 * code tree.
 */

struct mnl_socket {
	int 			fd;
	struct sockaddr_nl	addr;
};

/**
 * \defgroup socket Netlink socket helpers
 * @{
 */

/**
 * mnl_socket_get_fd - obtain file descriptor from netlink socket
 * \param nl netlink socket obtained via mnl_socket_open()
 *
 * This function returns the file descriptor of a given netlink socket.
 */
int mnl_socket_get_fd(const struct mnl_socket *nl)
{
	return nl->fd;
}

/**
 * mnl_socket_get_portid - obtain Netlink PortID from netlink socket
 * \param nl netlink socket obtained via mnl_socket_open()
 *
 * This function returns the Netlink PortID of a given netlink socket.
 * It's a common mistake to assume that this PortID equals the process ID
 * which is not always true. This is the case if you open more than one
 * socket that is binded to the same Netlink subsystem from the same process.
 */
unsigned int mnl_socket_get_portid(const struct mnl_socket *nl)
{
	return nl->addr.nl_pid;
}

static struct mnl_socket *__mnl_socket_open(int bus, int flags)
{
	struct mnl_socket *nl;

	nl = calloc(1, sizeof(struct mnl_socket));
	if (nl == NULL)
		return NULL;

	nl->fd = socket(AF_NETLINK, SOCK_RAW | flags, bus);
	if (nl->fd == -1) {
		free(nl);
		return NULL;
	}

	return nl;
}

/**
 * mnl_socket_open - open a netlink socket
 * \param bus the netlink socket bus ID (see NETLINK_* constants)
 *
 * On error, it returns NULL and errno is appropriately set. Otherwise, it
 * returns a valid pointer to the mnl_socket structure.
 */
struct mnl_socket *mnl_socket_open(int bus)
{
	return __mnl_socket_open(bus, 0);
}

/**
 * mnl_socket_open2 - open a netlink socket with appropriate flags
 * \param bus the netlink socket bus ID (see NETLINK_* constants)
 * \param flags the netlink socket flags (see SOCK_* constants in socket(2))
 *
 * This is similar to mnl_socket_open(), but allows to set flags like
 * SOCK_CLOEXEC at socket creation time (useful for multi-threaded programs
 * performing exec calls).
 *
 * On error, it returns NULL and errno is appropriately set. Otherwise, it
 * returns a valid pointer to the mnl_socket structure.
 */
struct mnl_socket *mnl_socket_open2(int bus, int flags)
{
	return __mnl_socket_open(bus, flags);
}

/**
 * mnl_socket_fdopen - associates a mnl_socket object with pre-existing socket.
 * \param fd pre-existing socket descriptor.
 *
 * On error, it returns NULL and errno is appropriately set. Otherwise, it
 * returns a valid pointer to the mnl_socket structure. It also sets the portID
 * if the socket fd is already bound and it is AF_NETLINK.
 *
 * Note that mnl_socket_get_portid() returns 0 if this function is used with
 * non-netlink socket.
 */
struct mnl_socket *mnl_socket_fdopen(int fd)
{
	int ret;
	struct mnl_socket *nl;
	struct sockaddr_nl addr;
	socklen_t addr_len = sizeof(struct sockaddr_nl);

	ret = getsockname(fd, (struct sockaddr *) &addr, &addr_len);
	if (ret == -1)
		return NULL;

	nl = calloc(1, sizeof(struct mnl_socket));
	if (nl == NULL)
		return NULL;

	nl->fd = fd;
	if (addr.nl_family == AF_NETLINK)
		nl->addr = addr;

	return nl;
}

/**
 * mnl_socket_bind - bind netlink socket
 * \param nl netlink socket obtained via mnl_socket_open()
 * \param groups the group of message you're interested in
 * \param pid the port ID you want to use (use zero for automatic selection)
 *
 * On error, this function returns -1 and errno is appropriately set. On
 * success, 0 is returned. You can use MNL_SOCKET_AUTOPID which is 0 for
 * automatic port ID selection.
 */
int mnl_socket_bind(struct mnl_socket *nl, unsigned int groups,
				  pid_t pid)
{
	int ret;
	socklen_t addr_len;

	nl->addr.nl_family = AF_NETLINK;
	nl->addr.nl_groups = groups;
	nl->addr.nl_pid = pid;

	ret = bind(nl->fd, (struct sockaddr *) &nl->addr, sizeof (nl->addr));
	if (ret < 0)
		return ret;

	addr_len = sizeof(nl->addr);
	ret = getsockname(nl->fd, (struct sockaddr *) &nl->addr, &addr_len);
	if (ret < 0)	
		return ret;

	if (addr_len != sizeof(nl->addr)) {
		errno = EINVAL;
		return -1;
	}
	if (nl->addr.nl_family != AF_NETLINK) {
		errno = EINVAL;
		return -1;
	}
	return 0;
}

/**
 * mnl_socket_sendto - send a netlink message of a certain size
 * \param nl netlink socket obtained via mnl_socket_open()
 * \param buf buffer containing the netlink message to be sent
 * \param len number of bytes in the buffer that you want to send
 *
 * On error, it returns -1 and errno is appropriately set. Otherwise, it 
 * returns the number of bytes sent.
 */
ssize_t mnl_socket_sendto(const struct mnl_socket *nl,
					const void *buf, size_t len)
{
	static const struct sockaddr_nl snl = {
		.nl_family = AF_NETLINK
	};
	return sendto(nl->fd, buf, len, 0, 
		      (struct sockaddr *) &snl, sizeof(snl));
}

/**
 * mnl_socket_recvfrom - receive a netlink message
 * \param nl netlink socket obtained via mnl_socket_open()
 * \param buf buffer that you want to use to store the netlink message
 * \param bufsiz size of the buffer passed to store the netlink message
 *
 * On error, it returns -1 and errno is appropriately set. If errno is set
 * to ENOSPC, it means that the buffer that you have passed to store the
 * netlink message is too small, so you have received a truncated message.
 * To avoid this, you have to allocate a buffer of MNL_SOCKET_BUFFER_SIZE
 * (which is 8KB, see linux/netlink.h for more information). Using this
 * buffer size ensures that your buffer is big enough to store the netlink
 * message without truncating it.
 */
ssize_t mnl_socket_recvfrom(const struct mnl_socket *nl,
					  void *buf, size_t bufsiz)
{
	ssize_t ret;
	struct sockaddr_nl addr;
	struct iovec iov = {
		.iov_base	= buf,
		.iov_len	= bufsiz,
	};
	struct msghdr msg = {
		.msg_name	= &addr,
		.msg_namelen	= sizeof(struct sockaddr_nl),
		.msg_iov	= &iov,
		.msg_iovlen	= 1,
		.msg_control	= NULL,
		.msg_controllen	= 0,
		.msg_flags	= 0,
	};
	ret = recvmsg(nl->fd, &msg, 0);
	if (ret == -1)
		return ret;

	if (msg.msg_flags & MSG_TRUNC) {
		errno = ENOSPC;
		return -1;
	}
	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		errno = EINVAL;
		return -1;
	}
	return ret;
}

/**
 * mnl_socket_close - close a given netlink socket
 * \param nl netlink socket obtained via mnl_socket_open()
 *
 * On error, this function returns -1 and errno is appropriately set.
 * On success, it returns 0.
 */
int mnl_socket_close(struct mnl_socket *nl)
{
	int ret = close(nl->fd);
	free(nl);
	return ret;
}

/**
 * mnl_socket_setsockopt - set Netlink socket option
 * \param nl netlink socket obtained via mnl_socket_open()
 * \param type type of Netlink socket options
 * \param buf the buffer that contains the data about this option
 * \param len the size of the buffer passed
 *
 * This function allows you to set some Netlink socket option. As of writing
 * this (see linux/netlink.h), the existing options are:
 *
 *	- \#define NETLINK_ADD_MEMBERSHIP  1
 *	- \#define NETLINK_DROP_MEMBERSHIP 2
 *	- \#define NETLINK_PKTINFO         3
 *	- \#define NETLINK_BROADCAST_ERROR 4
 *	- \#define NETLINK_NO_ENOBUFS      5
 *
 * In the early days, Netlink only supported 32 groups expressed in a
 * 32-bits mask. However, since 2.6.14, Netlink may have up to 2^32 multicast
 * groups but you have to use setsockopt() with NETLINK_ADD_MEMBERSHIP to
 * join a given multicast group. This function internally calls setsockopt()
 * to join a given netlink multicast group. You can still use mnl_bind()
 * and the 32-bit mask to join a set of Netlink multicast groups.
 *
 * On error, this function returns -1 and errno is appropriately set.
 */
int mnl_socket_setsockopt(const struct mnl_socket *nl, int type,
					void *buf, socklen_t len)
{
	return setsockopt(nl->fd, SOL_NETLINK, type, buf, len);
}

/**
 * mnl_socket_getsockopt - get a Netlink socket option
 * \param nl netlink socket obtained via mnl_socket_open()
 * \param type type of Netlink socket options
 * \param buf pointer to the buffer to store the value of this option
 * \param len size of the information written in the buffer
 *
 * On error, this function returns -1 and errno is appropriately set.
 */
int mnl_socket_getsockopt(const struct mnl_socket *nl, int type,
					void *buf, socklen_t *len)
{
	return getsockopt(nl->fd, SOL_NETLINK, type, buf, len);
}

/**
 * @}
 */
