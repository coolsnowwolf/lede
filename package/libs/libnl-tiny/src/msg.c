/*
 * lib/msg.c		Netlink Messages Interface
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
 * @defgroup msg Messages
 * Netlink Message Construction/Parsing Interface
 * 
 * The following information is partly extracted from RFC3549
 * (ftp://ftp.rfc-editor.org/in-notes/rfc3549.txt)
 *
 * @par Message Format
 * Netlink messages consist of a byte stream with one or multiple
 * Netlink headers and an associated payload.  If the payload is too big
 * to fit into a single message it, can be split over multiple Netlink
 * messages, collectively called a multipart message.  For multipart
 * messages, the first and all following headers have the \c NLM_F_MULTI
 * Netlink header flag set, except for the last header which has the
 * Netlink header type \c NLMSG_DONE.
 *
 * @par
 * The Netlink message header (\link nlmsghdr struct nlmsghdr\endlink) is shown below.
 * @code   
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Length                             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |            Type              |           Flags              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Sequence Number                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      Process ID (PID)                       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endcode
 *
 * @par
 * The netlink message header and payload must be aligned properly:
 * @code
 *  <------- NLMSG_ALIGN(hlen) ------> <---- NLMSG_ALIGN(len) --->
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * |           Header           | Pad |       Payload       | Pad |
 * |      struct nlmsghdr       |     |                     |     |
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * @endcode
 * @par
 * Message Format:
 * @code
 *    <--- nlmsg_total_size(payload)  --->
 *    <-- nlmsg_msg_size(payload) ->
 *   +----------+- - -+-------------+- - -+-------- - -
 *   | nlmsghdr | Pad |   Payload   | Pad | nlmsghdr
 *   +----------+- - -+-------------+- - -+-------- - -
 *   nlmsg_data(nlh)---^                   ^
 *   nlmsg_next(nlh)-----------------------+
 * @endcode
 * @par
 * The payload may consist of arbitary data but may have strict
 * alignment and formatting rules depening on the specific netlink
 * families.
 * @par
 * @code
 *    <---------------------- nlmsg_len(nlh) --------------------->
 *    <------ hdrlen ------>       <- nlmsg_attrlen(nlh, hdrlen) ->
 *   +----------------------+- - -+--------------------------------+
 *   |     Family Header    | Pad |           Attributes           |
 *   +----------------------+- - -+--------------------------------+
 *   nlmsg_attrdata(nlh, hdrlen)---^
 * @endcode
 * @par The ACK Netlink Message
 * This message is actually used to denote both an ACK and a NACK.
 * Typically, the direction is from FEC to CPC (in response to an ACK
 * request message).  However, the CPC should be able to send ACKs back
 * to FEC when requested.
 * @code
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       Netlink message header                  |
 * |                       type = NLMSG_ERROR                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                          Error code                           |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                       OLD Netlink message header              |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * @endcode
 *
 * @par Example
 * @code
 * // Various methods exist to create/allocate a new netlink
 * // message. 
 * //
 * // nlmsg_alloc() will allocate an empty netlink message with
 * // a maximum payload size which defaults to the page size of
 * // the system. This default size can be modified using the
 * // function nlmsg_set_default_size().
 * struct nl_msg *msg = nlmsg_alloc();
 *
 * // Very often, the message type and message flags are known
 * // at allocation time while the other fields are auto generated:
 * struct nl_msg *msg = nlmsg_alloc_simple(MY_TYPE, MY_FLAGS);
 *
 * // Alternatively an existing netlink message header can be used
 * // to inherit the header values:
 * struct nlmsghdr hdr = {
 * 	.nlmsg_type = MY_TYPE,
 * 	.nlmsg_flags = MY_FLAGS,
 * };
 * struct nl_msg *msg = nlmsg_inherit(&hdr);
 *
 * // Last but not least, netlink messages received from netlink sockets
 * // can be converted into nl_msg objects using nlmsg_convert(). This
 * // will create a message with a maximum payload size which equals the
 * // length of the existing netlink message, therefore no more data can
 * // be appened without calling nlmsg_expand() first.
 * struct nl_msg *msg = nlmsg_convert(nlh_from_nl_sock);
 *
 * // Payload may be added to the message via nlmsg_append(). The fourth
 * // parameter specifies the number of alignment bytes the data should
 * // be padding with at the end. Common values are 0 to disable it or
 * // NLMSG_ALIGNTO to ensure proper netlink message padding.
 * nlmsg_append(msg, &mydata, sizeof(mydata), 0);
 *
 * // Sometimes it may be necessary to reserve room for data but defer
 * // the actual copying to a later point, nlmsg_reserve() can be used
 * // for this purpose:
 * void *data = nlmsg_reserve(msg, sizeof(mydata), NLMSG_ALIGNTO);
 *
 * // Attributes may be added using the attributes interface.
 *
 * // After successful use of the message, the memory must be freed
 * // using nlmsg_free()
 * nlmsg_free(msg);
 * @endcode
 * 
 * @par 4) Parsing messages
 * @code
 * int n;
 * unsigned char *buf;
 * struct nlmsghdr *hdr;
 *
 * n = nl_recv(handle, NULL, &buf);
 * 
 * hdr = (struct nlmsghdr *) buf;
 * while (nlmsg_ok(hdr, n)) {
 * 	// Process message here...
 * 	hdr = nlmsg_next(hdr, &n);
 * }
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/cache.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <linux/socket.h>

static size_t default_msg_size = 4096;

/**
 * @name Attribute Access
 * @{
 */

//** @} */

/**
 * @name Message Parsing
 * @{
 */

/**
 * check if the netlink message fits into the remaining bytes
 * @arg nlh		netlink message header
 * @arg remaining	number of bytes remaining in message stream
 */
int nlmsg_ok(const struct nlmsghdr *nlh, int remaining)
{
	return (remaining >= sizeof(struct nlmsghdr) &&
		nlh->nlmsg_len >= sizeof(struct nlmsghdr) &&
		nlh->nlmsg_len <= remaining);
}

/**
 * next netlink message in message stream
 * @arg nlh		netlink message header
 * @arg remaining	number of bytes remaining in message stream
 *
 * @returns the next netlink message in the message stream and
 * decrements remaining by the size of the current message.
 */
struct nlmsghdr *nlmsg_next(struct nlmsghdr *nlh, int *remaining)
{
	int totlen = NLMSG_ALIGN(nlh->nlmsg_len);

	*remaining -= totlen;

	return (struct nlmsghdr *) ((unsigned char *) nlh + totlen);
}

/**
 * parse attributes of a netlink message
 * @arg nlh		netlink message header
 * @arg hdrlen		length of family specific header
 * @arg tb		destination array with maxtype+1 elements
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 *
 * See nla_parse()
 */
int nlmsg_parse(struct nlmsghdr *nlh, int hdrlen, struct nlattr *tb[],
		int maxtype, struct nla_policy *policy)
{
	if (!nlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	return nla_parse(tb, maxtype, nlmsg_attrdata(nlh, hdrlen),
			 nlmsg_attrlen(nlh, hdrlen), policy);
}

/**
 * nlmsg_validate - validate a netlink message including attributes
 * @arg nlh		netlinket message header
 * @arg hdrlen		length of familiy specific header
 * @arg maxtype		maximum attribute type to be expected
 * @arg policy		validation policy
 */
int nlmsg_validate(struct nlmsghdr *nlh, int hdrlen, int maxtype,
		   struct nla_policy *policy)
{
	if (!nlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	return nla_validate(nlmsg_attrdata(nlh, hdrlen),
			    nlmsg_attrlen(nlh, hdrlen), maxtype, policy);
}

/** @} */

/**
 * @name Message Building/Access
 * @{
 */

static struct nl_msg *__nlmsg_alloc(size_t len)
{
	struct nl_msg *nm;

	nm = calloc(1, sizeof(*nm));
	if (!nm)
		goto errout;

	nm->nm_refcnt = 1;

	nm->nm_nlh = malloc(len);
	if (!nm->nm_nlh)
		goto errout;

	memset(nm->nm_nlh, 0, sizeof(struct nlmsghdr));

	nm->nm_protocol = -1;
	nm->nm_size = len;
	nm->nm_nlh->nlmsg_len = nlmsg_total_size(0);

	NL_DBG(2, "msg %p: Allocated new message, maxlen=%zu\n", nm, len);

	return nm;
errout:
	free(nm);
	return NULL;
}

/**
 * Allocate a new netlink message with the default maximum payload size.
 *
 * Allocates a new netlink message without any further payload. The
 * maximum payload size defaults to PAGESIZE or as otherwise specified
 * with nlmsg_set_default_size().
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nlmsg_alloc(void)
{
	return __nlmsg_alloc(default_msg_size);
}

/**
 * Allocate a new netlink message with maximum payload size specified.
 */
struct nl_msg *nlmsg_alloc_size(size_t max)
{
	return __nlmsg_alloc(max);
}

/**
 * Allocate a new netlink message and inherit netlink message header
 * @arg hdr		Netlink message header template
 *
 * Allocates a new netlink message and inherits the original message
 * header. If \a hdr is not NULL it will be used as a template for
 * the netlink message header, otherwise the header is left blank.
 * 
 * @return Newly allocated netlink message or NULL
 */
struct nl_msg *nlmsg_inherit(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = nlmsg_alloc();
	if (nm && hdr) {
		struct nlmsghdr *new = nm->nm_nlh;

		new->nlmsg_type = hdr->nlmsg_type;
		new->nlmsg_flags = hdr->nlmsg_flags;
		new->nlmsg_seq = hdr->nlmsg_seq;
		new->nlmsg_pid = hdr->nlmsg_pid;
	}

	return nm;
}

/**
 * Allocate a new netlink message
 * @arg nlmsgtype	Netlink message type
 * @arg flags		Message flags.
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nlmsg_alloc_simple(int nlmsgtype, int flags)
{
	struct nl_msg *msg;
	struct nlmsghdr nlh = {
		.nlmsg_type = nlmsgtype,
		.nlmsg_flags = flags,
	};

	msg = nlmsg_inherit(&nlh);
	if (msg)
		NL_DBG(2, "msg %p: Allocated new simple message\n", msg);

	return msg;
}

/**
 * Set the default maximum message payload size for allocated messages
 * @arg max		Size of payload in bytes.
 */
void nlmsg_set_default_size(size_t max)
{
	if (max < nlmsg_total_size(0))
		max = nlmsg_total_size(0);

	default_msg_size = max;
}

/**
 * Convert a netlink message received from a netlink socket to a nl_msg
 * @arg hdr		Netlink message received from netlink socket.
 *
 * Allocates a new netlink message and copies all of the data pointed to
 * by \a hdr into the new message object.
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nlmsg_convert(struct nlmsghdr *hdr)
{
	struct nl_msg *nm;

	nm = __nlmsg_alloc(NLMSG_ALIGN(hdr->nlmsg_len));
	if (!nm)
		goto errout;

	memcpy(nm->nm_nlh, hdr, hdr->nlmsg_len);

	return nm;
errout:
	nlmsg_free(nm);
	return NULL;
}

/**
 * Reserve room for additional data in a netlink message
 * @arg n		netlink message
 * @arg len		length of additional data to reserve room for
 * @arg pad		number of bytes to align data to
 *
 * Reserves room for additional data at the tail of the an
 * existing netlink message. Eventual padding required will
 * be zeroed out.
 *
 * @return Pointer to start of additional data tailroom or NULL.
 */
void *nlmsg_reserve(struct nl_msg *n, size_t len, int pad)
{
	void *buf = n->nm_nlh;
	size_t nlmsg_len = n->nm_nlh->nlmsg_len;
	size_t tlen;

	tlen = pad ? ((len + (pad - 1)) & ~(pad - 1)) : len;

	if ((tlen + nlmsg_len) > n->nm_size)
		return NULL;

	buf += nlmsg_len;
	n->nm_nlh->nlmsg_len += tlen;

	if (tlen > len)
		memset(buf + len, 0, tlen - len);

	NL_DBG(2, "msg %p: Reserved %zu bytes, pad=%d, nlmsg_len=%d\n",
		  n, len, pad, n->nm_nlh->nlmsg_len);

	return buf;
}

/**
 * Append data to tail of a netlink message
 * @arg n		netlink message
 * @arg data		data to add
 * @arg len		length of data
 * @arg pad		Number of bytes to align data to.
 *
 * Extends the netlink message as needed and appends the data of given
 * length to the message. 
 *
 * @return 0 on success or a negative error code
 */
int nlmsg_append(struct nl_msg *n, void *data, size_t len, int pad)
{
	void *tmp;

	tmp = nlmsg_reserve(n, len, pad);
	if (tmp == NULL)
		return -NLE_NOMEM;

	memcpy(tmp, data, len);
	NL_DBG(2, "msg %p: Appended %zu bytes with padding %d\n", n, len, pad);

	return 0;
}

/**
 * Add a netlink message header to a netlink message
 * @arg n		netlink message
 * @arg pid		netlink process id or NL_AUTO_PID
 * @arg seq		sequence number of message or NL_AUTO_SEQ
 * @arg type		message type
 * @arg payload		length of message payload
 * @arg flags		message flags
 *
 * Adds or overwrites the netlink message header in an existing message
 * object. If \a payload is greater-than zero additional room will be
 * reserved, f.e. for family specific headers. It can be accesed via
 * nlmsg_data().
 *
 * @return A pointer to the netlink message header or NULL.
 */
struct nlmsghdr *nlmsg_put(struct nl_msg *n, uint32_t pid, uint32_t seq,
			   int type, int payload, int flags)
{
	struct nlmsghdr *nlh;

	if (n->nm_nlh->nlmsg_len < NLMSG_HDRLEN)
		BUG();

	nlh = (struct nlmsghdr *) n->nm_nlh;
	nlh->nlmsg_type = type;
	nlh->nlmsg_flags = flags;
	nlh->nlmsg_pid = pid;
	nlh->nlmsg_seq = seq;

	NL_DBG(2, "msg %p: Added netlink header type=%d, flags=%d, pid=%d, "
		  "seq=%d\n", n, type, flags, pid, seq);

	if (payload > 0 &&
	    nlmsg_reserve(n, payload, NLMSG_ALIGNTO) == NULL)
		return NULL;

	return nlh;
}

/**
 * Release a reference from an netlink message
 * @arg msg		message to release reference from
 *
 * Frees memory after the last reference has been released.
 */
void nlmsg_free(struct nl_msg *msg)
{
	if (!msg)
		return;

	msg->nm_refcnt--;
	NL_DBG(4, "Returned message reference %p, %d remaining\n",
	       msg, msg->nm_refcnt);

	if (msg->nm_refcnt < 0)
		BUG();

	if (msg->nm_refcnt <= 0) {
		free(msg->nm_nlh);
		free(msg);
		NL_DBG(2, "msg %p: Freed\n", msg);
	}
}

/** @} */

/**
 * @name Direct Parsing
 * @{
 */

/** @cond SKIP */
struct dp_xdata {
	void (*cb)(struct nl_object *, void *);
	void *arg;
};
/** @endcond */

static int parse_cb(struct nl_object *obj, struct nl_parser_param *p)
{
	struct dp_xdata *x = p->pp_arg;

	x->cb(obj, x->arg);
	return 0;
}

int nl_msg_parse(struct nl_msg *msg, void (*cb)(struct nl_object *, void *),
		 void *arg)
{
	struct nl_cache_ops *ops;
	struct nl_parser_param p = {
		.pp_cb = parse_cb
	};
	struct dp_xdata x = {
		.cb = cb,
		.arg = arg,
	};

	ops = nl_cache_ops_associate(nlmsg_get_proto(msg),
				     nlmsg_hdr(msg)->nlmsg_type);
	if (ops == NULL)
		return -NLE_MSGTYPE_NOSUPPORT;
	p.pp_arg = &x;

	return nl_cache_parse(ops, NULL, nlmsg_hdr(msg), &p);
}

/** @} */
