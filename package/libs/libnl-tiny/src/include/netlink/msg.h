/*
 * netlink/msg.c		Netlink Messages Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_MSG_H_
#define NETLINK_MSG_H_

#include <netlink/netlink.h>
#include <netlink/object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nla_policy;

#define NL_DONTPAD	0

/**
 * @ingroup msg
 * @brief
 * Will cause the netlink pid to be set to the pid assigned to
 * the netlink handle (socket) just before sending the message off.
 * @note Requires the use of nl_send_auto_complete()!
 */
#define NL_AUTO_PID	0

/**
 * @ingroup msg
 * @brief
 * May be used to refer to a sequence number which should be
 * automatically set just before sending the message off.
 * @note Requires the use of nl_send_auto_complete()!
 */
#define NL_AUTO_SEQ	0

#define NL_MSG_CRED_PRESENT 1

struct nl_msg
{
	int			nm_protocol;
	int			nm_flags;
	struct sockaddr_nl	nm_src;
	struct sockaddr_nl	nm_dst;
	struct ucred		nm_creds;
	struct nlmsghdr *	nm_nlh;
	size_t			nm_size;
	int			nm_refcnt;
};


struct nl_msg;
struct nl_tree;
struct ucred;

/* message parsing */
extern int		  nlmsg_ok(const struct nlmsghdr *, int);
extern struct nlmsghdr *  nlmsg_next(struct nlmsghdr *, int *);
extern int		  nlmsg_parse(struct nlmsghdr *, int, struct nlattr **,
				      int, struct nla_policy *);
extern int		  nlmsg_validate(struct nlmsghdr *, int, int,
					 struct nla_policy *);

extern struct nl_msg *	  nlmsg_alloc(void);
extern struct nl_msg *	  nlmsg_alloc_size(size_t);
extern struct nl_msg *	  nlmsg_alloc_simple(int, int);
extern void		  nlmsg_set_default_size(size_t);
extern struct nl_msg *	  nlmsg_inherit(struct nlmsghdr *);
extern struct nl_msg *	  nlmsg_convert(struct nlmsghdr *);
extern void *		  nlmsg_reserve(struct nl_msg *, size_t, int);
extern int		  nlmsg_append(struct nl_msg *, void *, size_t, int);

extern struct nlmsghdr *  nlmsg_put(struct nl_msg *, uint32_t, uint32_t,
				    int, int, int);
extern void		  nlmsg_free(struct nl_msg *);

extern int		  nl_msg_parse(struct nl_msg *,
				       void (*cb)(struct nl_object *, void *),
				       void *);

extern void		nl_msg_dump(struct nl_msg *, FILE *);

/**
 * length of netlink message not including padding
 * @arg payload		length of message payload
 */
static inline int nlmsg_msg_size(int payload)
{
	return NLMSG_HDRLEN + payload;
}

/**
 * length of netlink message including padding
 * @arg payload		length of message payload
 */
static inline int nlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(nlmsg_msg_size(payload));
}

/**
 * length of padding at the message's tail
 * @arg payload		length of message payload
 */
static inline int nlmsg_padlen(int payload)
{
	return nlmsg_total_size(payload) - nlmsg_msg_size(payload);
}

/**
 * head of message payload
 * @arg nlh		netlink messsage header
 */
static inline void *nlmsg_data(const struct nlmsghdr *nlh)
{
	return (unsigned char *) nlh + NLMSG_HDRLEN;
}

static inline void *nlmsg_tail(const struct nlmsghdr *nlh)
{
	return (unsigned char *) nlh + NLMSG_ALIGN(nlh->nlmsg_len);
}

/**
 * length of message payload
 * @arg nlh		netlink message header
 */
static inline int nlmsg_len(const struct nlmsghdr *nlh)
{
	return nlh->nlmsg_len - NLMSG_HDRLEN;
}

/**
 * head of attributes data
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 */
static inline struct nlattr *nlmsg_attrdata(const struct nlmsghdr *nlh, int hdrlen)
{
	unsigned char *data = (unsigned char*)nlmsg_data(nlh);
	return (struct nlattr *) (data + NLMSG_ALIGN(hdrlen));
}

/**
 * length of attributes data
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 */
static inline int nlmsg_attrlen(const struct nlmsghdr *nlh, int hdrlen)
{
	return nlmsg_len(nlh) - NLMSG_ALIGN(hdrlen);
}

static inline int nlmsg_valid_hdr(const struct nlmsghdr *nlh, int hdrlen)
{
	if (nlh->nlmsg_len < (uint)nlmsg_msg_size(hdrlen))
		return 0;

	return 1;
}


static inline void nlmsg_set_proto(struct nl_msg *msg, int protocol)
{
	msg->nm_protocol = protocol;
}

static inline int nlmsg_get_proto(struct nl_msg *msg)
{
	return msg->nm_protocol;
}

static inline size_t nlmsg_get_max_size(struct nl_msg *msg)
{
	return msg->nm_size;
}

static inline void nlmsg_set_src(struct nl_msg *msg, struct sockaddr_nl *addr)
{
	memcpy(&msg->nm_src, addr, sizeof(*addr));
}

static inline struct sockaddr_nl *nlmsg_get_src(struct nl_msg *msg)
{
	return &msg->nm_src;
}

static inline void nlmsg_set_dst(struct nl_msg *msg, struct sockaddr_nl *addr)
{
	memcpy(&msg->nm_dst, addr, sizeof(*addr));
}

static inline struct sockaddr_nl *nlmsg_get_dst(struct nl_msg *msg)
{
	return &msg->nm_dst;
}

static inline void nlmsg_set_creds(struct nl_msg *msg, struct ucred *creds)
{
	memcpy(&msg->nm_creds, creds, sizeof(*creds));
	msg->nm_flags |= NL_MSG_CRED_PRESENT;
}

static inline struct ucred *nlmsg_get_creds(struct nl_msg *msg)
{
	if (msg->nm_flags & NL_MSG_CRED_PRESENT)
		return &msg->nm_creds;
	return NULL;
}

/**
 * Return actual netlink message
 * @arg n		netlink message
 * 
 * Returns the actual netlink message casted to the type of the netlink
 * message header.
 * 
 * @return A pointer to the netlink message.
 */
static inline struct nlmsghdr *nlmsg_hdr(struct nl_msg *n)
{
	return n->nm_nlh;
}

/**
 * Acquire a reference on a netlink message
 * @arg msg		message to acquire reference from
 */
static inline void nlmsg_get(struct nl_msg *msg)
{
	msg->nm_refcnt++;
}

/**
 * Expand maximum payload size of a netlink message
 * @arg n		Netlink message.
 * @arg newlen		New maximum payload size.
 *
 * Reallocates the payload section of a netlink message and increases
 * the maximum payload size of the message.
 *
 * @note Any pointers pointing to old payload block will be stale and
 *       need to be refetched. Therfore, do not expand while constructing
 *       nested attributes or while reserved data blocks are held.
 *
 * @return 0 on success or a negative error code.
 */
static inline int nlmsg_expand(struct nl_msg *n, size_t newlen)
{
	void *tmp;

	if (newlen <= n->nm_size)
		return -NLE_INVAL;

	tmp = realloc(n->nm_nlh, newlen);
	if (tmp == NULL)
		return -NLE_NOMEM;

	n->nm_nlh = (struct nlmsghdr*)tmp;
	n->nm_size = newlen;

	return 0;
}


/**
 * @name Iterators
 * @{
 */

/**
 * @ingroup msg
 * Iterate over a stream of attributes in a message
 * @arg pos	loop counter, set to current attribute
 * @arg nlh	netlink message header
 * @arg hdrlen	length of family header
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nlmsg_for_each_attr(pos, nlh, hdrlen, rem) \
	nla_for_each_attr(pos, nlmsg_attrdata(nlh, hdrlen), \
			  nlmsg_attrlen(nlh, hdrlen), rem)

/**
 * Iterate over a stream of messages
 * @arg pos	loop counter, set to current message
 * @arg head	head of message stream
 * @arg len	length of message stream
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nlmsg_for_each_msg(pos, head, len, rem) \
	for (pos = head, rem = len; \
	     nlmsg_ok(pos, rem); \
	     pos = nlmsg_next(pos, &(rem)))

/** @} */

#ifdef __cplusplus
}
#endif

#endif
