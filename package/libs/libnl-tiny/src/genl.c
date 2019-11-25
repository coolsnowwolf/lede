/*
 * lib/genl/genl.c		Generic Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup genl Generic Netlink
 *
 * @par Message Format
 * @code
 *  <------- NLMSG_ALIGN(hlen) ------> <---- NLMSG_ALIGN(len) --->
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * |           Header           | Pad |       Payload       | Pad |
 * |      struct nlmsghdr       |     |                     |     |
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * @endcode
 * @code
 *  <-------- GENL_HDRLEN -------> <--- hdrlen -->
 *                                 <------- genlmsg_len(ghdr) ------>
 * +------------------------+- - -+---------------+- - -+------------+
 * | Generic Netlink Header | Pad | Family Header | Pad | Attributes |
 * |    struct genlmsghdr   |     |               |     |            |
 * +------------------------+- - -+---------------+- - -+------------+
 * genlmsg_data(ghdr)--------------^                     ^
 * genlmsg_attrdata(ghdr, hdrlen)-------------------------
 * @endcode
 *
 * @par Example
 * @code
 * #include <netlink/netlink.h>
 * #include <netlink/genl/genl.h>
 * #include <netlink/genl/ctrl.h>
 *
 * struct nl_sock *sock;
 * struct nl_msg *msg;
 * int family;
 *
 * // Allocate a new netlink socket
 * sock = nl_socket_alloc();
 *
 * // Connect to generic netlink socket on kernel side
 * genl_connect(sock);
 *
 * // Ask kernel to resolve family name to family id
 * family = genl_ctrl_resolve(sock, "generic_netlink_family_name");
 *
 * // Construct a generic netlink by allocating a new message, fill in
 * // the header and append a simple integer attribute.
 * msg = nlmsg_alloc();
 * genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, family, 0, NLM_F_ECHO,
 *             CMD_FOO_GET, FOO_VERSION);
 * nla_put_u32(msg, ATTR_FOO, 123);
 *
 * // Send message over netlink socket
 * nl_send_auto_complete(sock, msg);
 *
 * // Free message
 * nlmsg_free(msg);
 *
 * // Prepare socket to receive the answer by specifying the callback
 * // function to be called for valid messages.
 * nl_socket_modify_cb(sock, NL_CB_VALID, NL_CB_CUSTOM, parse_cb, NULL);
 *
 * // Wait for the answer and receive it
 * nl_recvmsgs_default(sock);
 *
 * static int parse_cb(struct nl_msg *msg, void *arg)
 * {
 *     struct nlmsghdr *nlh = nlmsg_hdr(msg);
 *     struct nlattr *attrs[ATTR_MAX+1];
 *
 *     // Validate message and parse attributes
 *     genlmsg_parse(nlh, 0, attrs, ATTR_MAX, policy);
 *
 *     if (attrs[ATTR_FOO]) {
 *         uint32_t value = nla_get_u32(attrs[ATTR_FOO]);
 *         ...
 *     }
 *
 *     return 0;
 * }
 * @endcode
 * @{
 */

#include <netlink-generic.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/utils.h>

/**
 * @name Socket Creating
 * @{
 */

int genl_connect(struct nl_sock *sk)
{
	return nl_connect(sk, NETLINK_GENERIC);
}

/** @} */

/**
 * @name Sending
 * @{
 */

/**
 * Send trivial generic netlink message
 * @arg sk		Netlink socket.
 * @arg family		Generic netlink family
 * @arg cmd		Command
 * @arg version		Version
 * @arg flags		Additional netlink message flags.
 *
 * Fills out a routing netlink request message and sends it out
 * using nl_send_simple().
 *
 * @return 0 on success or a negative error code.
 */
int genl_send_simple(struct nl_sock *sk, int family, int cmd,
		     int version, int flags)
{
	struct genlmsghdr hdr = {
		.cmd = cmd,
		.version = version,
	};

	return nl_send_simple(sk, family, flags, &hdr, sizeof(hdr));
}

/** @} */


/**
 * @name Message Parsing
 * @{
 */

int genlmsg_valid_hdr(struct nlmsghdr *nlh, int hdrlen)
{
	struct genlmsghdr *ghdr;

	if (!nlmsg_valid_hdr(nlh, GENL_HDRLEN))
		return 0;

	ghdr = nlmsg_data(nlh);
	if (genlmsg_len(ghdr) < NLMSG_ALIGN(hdrlen))
		return 0;

	return 1;
}

int genlmsg_validate(struct nlmsghdr *nlh, int hdrlen, int maxtype,
		   struct nla_policy *policy)
{
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	ghdr = nlmsg_data(nlh);
	return nla_validate(genlmsg_attrdata(ghdr, hdrlen),
			    genlmsg_attrlen(ghdr, hdrlen), maxtype, policy);
}

int genlmsg_parse(struct nlmsghdr *nlh, int hdrlen, struct nlattr *tb[],
		  int maxtype, struct nla_policy *policy)
{
	struct genlmsghdr *ghdr;

	if (!genlmsg_valid_hdr(nlh, hdrlen))
		return -NLE_MSG_TOOSHORT;

	ghdr = nlmsg_data(nlh);
	return nla_parse(tb, maxtype, genlmsg_attrdata(ghdr, hdrlen),
			 genlmsg_attrlen(ghdr, hdrlen), policy);
}

/**
 * Get head of message payload
 * @arg gnlh	genetlink messsage header
 */
void *genlmsg_data(const struct genlmsghdr *gnlh)
{
	return ((unsigned char *) gnlh + GENL_HDRLEN);
}

/**
 * Get lenght of message payload
 * @arg gnlh	genetlink message header
 */
int genlmsg_len(const struct genlmsghdr *gnlh)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)((unsigned char *)gnlh -
							NLMSG_HDRLEN);
	return (nlh->nlmsg_len - GENL_HDRLEN - NLMSG_HDRLEN);
}

/**
 * Get head of attribute data
 * @arg gnlh	generic netlink message header
 * @arg hdrlen	length of family specific header
 */
struct nlattr *genlmsg_attrdata(const struct genlmsghdr *gnlh, int hdrlen)
{
	return genlmsg_data(gnlh) + NLMSG_ALIGN(hdrlen);
}

/**
 * Get length of attribute data
 * @arg gnlh	generic netlink message header
 * @arg hdrlen	length of family specific header
 */
int genlmsg_attrlen(const struct genlmsghdr *gnlh, int hdrlen)
{
	return genlmsg_len(gnlh) - NLMSG_ALIGN(hdrlen);
}

/** @} */

/**
 * @name Message Building
 * @{
 */

/**
 * Add generic netlink header to netlink message
 * @arg msg		netlink message
 * @arg pid		netlink process id or NL_AUTO_PID
 * @arg seq		sequence number of message or NL_AUTO_SEQ
 * @arg family		generic netlink family
 * @arg hdrlen		length of user specific header
 * @arg flags		message flags
 * @arg cmd		generic netlink command
 * @arg version		protocol version
 *
 * Returns pointer to user specific header.
 */
void *genlmsg_put(struct nl_msg *msg, uint32_t pid, uint32_t seq, int family,
		  int hdrlen, int flags, uint8_t cmd, uint8_t version)
{
	struct nlmsghdr *nlh;
	struct genlmsghdr hdr = {
		.cmd = cmd,
		.version = version,
	};

	nlh = nlmsg_put(msg, pid, seq, family, GENL_HDRLEN + hdrlen, flags);
	if (nlh == NULL)
		return NULL;

	memcpy(nlmsg_data(nlh), &hdr, sizeof(hdr));
	NL_DBG(2, "msg %p: Added generic netlink header cmd=%d version=%d\n",
	       msg, cmd, version);

	return nlmsg_data(nlh) + GENL_HDRLEN;
}

/** @} */

/** @} */
