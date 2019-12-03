/*
 * netlink/handlers.c	default netlink message handlers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_HANDLERS_H_
#define NETLINK_HANDLERS_H_

#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netlink/netlink-compat.h>
#include <netlink/netlink-kernel.h>
#include <netlink/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_sock;
struct nl_msg;
struct nl_cb;
/**
 * @name Callback Typedefs
 * @{
 */

/**
 * nl_recvmsgs() callback for message processing customization
 * @ingroup cb
 * @arg msg		netlink message being processed
 * @arg arg		argument passwd on through caller
 */
typedef int (*nl_recvmsg_msg_cb_t)(struct nl_msg *msg, void *arg);

/**
 * nl_recvmsgs() callback for error message processing customization
 * @ingroup cb
 * @arg nla		netlink address of the peer
 * @arg nlerr		netlink error message being processed
 * @arg arg		argument passed on through caller
 */
typedef int (*nl_recvmsg_err_cb_t)(struct sockaddr_nl *nla,
				   struct nlmsgerr *nlerr, void *arg);

/** @} */

/**
 * Callback actions
 * @ingroup cb
 */
enum nl_cb_action {
	/** Proceed with wathever would come next */
	NL_OK,
	/** Skip this message */
	NL_SKIP,
	/** Stop parsing altogether and discard remaining messages */
	NL_STOP,
};

/**
 * Callback kinds
 * @ingroup cb
 */
enum nl_cb_kind {
	/** Default handlers (quiet) */
	NL_CB_DEFAULT,
	/** Verbose default handlers (error messages printed) */
	NL_CB_VERBOSE,
	/** Debug handlers for debugging */
	NL_CB_DEBUG,
	/** Customized handler specified by the user */
	NL_CB_CUSTOM,
	__NL_CB_KIND_MAX,
};

#define NL_CB_KIND_MAX (__NL_CB_KIND_MAX - 1)

/**
 * Callback types
 * @ingroup cb
 */
enum nl_cb_type {
	/** Message is valid */
	NL_CB_VALID,
	/** Last message in a series of multi part messages received */
	NL_CB_FINISH,
	/** Report received that data was lost */
	NL_CB_OVERRUN,
	/** Message wants to be skipped */
	NL_CB_SKIPPED,
	/** Message is an acknowledge */
	NL_CB_ACK,
	/** Called for every message received */
	NL_CB_MSG_IN,
	/** Called for every message sent out except for nl_sendto() */
	NL_CB_MSG_OUT,
	/** Message is malformed and invalid */
	NL_CB_INVALID,
	/** Called instead of internal sequence number checking */
	NL_CB_SEQ_CHECK,
	/** Sending of an acknowledge message has been requested */
	NL_CB_SEND_ACK,
	__NL_CB_TYPE_MAX,
};

#define NL_CB_TYPE_MAX (__NL_CB_TYPE_MAX - 1)

struct nl_cb
{
	nl_recvmsg_msg_cb_t	cb_set[NL_CB_TYPE_MAX+1];
	void *			cb_args[NL_CB_TYPE_MAX+1];
	
	nl_recvmsg_err_cb_t	cb_err;
	void *			cb_err_arg;

	/** May be used to replace nl_recvmsgs with your own implementation
	 * in all internal calls to nl_recvmsgs. */
	int			(*cb_recvmsgs_ow)(struct nl_sock *,
						  struct nl_cb *);

	/** Overwrite internal calls to nl_recv, must return the number of
	 * octets read and allocate a buffer for the received data. */
	int			(*cb_recv_ow)(struct nl_sock *,
					      struct sockaddr_nl *,
					      unsigned char **,
					      struct ucred **);

	/** Overwrites internal calls to nl_send, must send the netlink
	 * message. */
	int			(*cb_send_ow)(struct nl_sock *,
					      struct nl_msg *);

	int			cb_refcnt;
};


extern struct nl_cb *	nl_cb_alloc(enum nl_cb_kind);
extern struct nl_cb *	nl_cb_clone(struct nl_cb *);
extern void		nl_cb_put(struct nl_cb *);

extern int  nl_cb_set(struct nl_cb *, enum nl_cb_type, enum nl_cb_kind,
		      nl_recvmsg_msg_cb_t, void *);
extern int  nl_cb_err(struct nl_cb *, enum nl_cb_kind, nl_recvmsg_err_cb_t,
		      void *);

static inline struct nl_cb *nl_cb_get(struct nl_cb *cb)
{
	cb->cb_refcnt++;

	return cb;
}

/**
 * Set up a all callbacks
 * @arg cb		callback set
 * @arg kind		kind of callback
 * @arg func		callback function
 * @arg arg		argument to be passwd to callback function
 *
 * @return 0 on success or a negative error code
 */
static inline int nl_cb_set_all(struct nl_cb *cb, enum nl_cb_kind kind,
		  nl_recvmsg_msg_cb_t func, void *arg)
{
	int i, err;

	for (i = 0; i <= NL_CB_TYPE_MAX; i++) {
		err = nl_cb_set(cb,(enum nl_cb_type)i, kind, func, arg);
		if (err < 0)
			return err;
	}

	return 0;
}


/**
 * @name Overwriting
 * @{
 */

/**
 * Overwrite internal calls to nl_recvmsgs()
 * @arg cb		callback set
 * @arg func		replacement callback for nl_recvmsgs()
 */
static inline void nl_cb_overwrite_recvmsgs(struct nl_cb *cb,
			      int (*func)(struct nl_sock *, struct nl_cb *))
{
	cb->cb_recvmsgs_ow = func;
}

/**
 * Overwrite internal calls to nl_recv()
 * @arg cb		callback set
 * @arg func		replacement callback for nl_recv()
 */
static inline void nl_cb_overwrite_recv(struct nl_cb *cb,
			  int (*func)(struct nl_sock *, struct sockaddr_nl *,
				      unsigned char **, struct ucred **))
{
	cb->cb_recv_ow = func;
}

/**
 * Overwrite internal calls to nl_send()
 * @arg cb		callback set
 * @arg func		replacement callback for nl_send()
 */
static inline void nl_cb_overwrite_send(struct nl_cb *cb,
			  int (*func)(struct nl_sock *, struct nl_msg *))
{
	cb->cb_send_ow = func;
}

/** @} */


#ifdef __cplusplus
}
#endif

#endif
