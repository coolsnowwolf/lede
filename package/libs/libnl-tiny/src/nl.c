/*
 * lib/nl.c		Core Netlink Interface
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup core Core
 *
 * @details
 * @par 1) Connecting the socket
 * @code
 * // Bind and connect the socket to a protocol, NETLINK_ROUTE in this example.
 * nl_connect(sk, NETLINK_ROUTE);
 * @endcode
 *
 * @par 2) Sending data
 * @code
 * // The most rudimentary method is to use nl_sendto() simply pushing
 * // a piece of data to the other netlink peer. This method is not
 * // recommended.
 * const char buf[] = { 0x01, 0x02, 0x03, 0x04 };
 * nl_sendto(sk, buf, sizeof(buf));
 *
 * // A more comfortable interface is nl_send() taking a pointer to
 * // a netlink message.
 * struct nl_msg *msg = my_msg_builder();
 * nl_send(sk, nlmsg_hdr(msg));
 *
 * // nl_sendmsg() provides additional control over the sendmsg() message
 * // header in order to allow more specific addressing of multiple peers etc.
 * struct msghdr hdr = { ... };
 * nl_sendmsg(sk, nlmsg_hdr(msg), &hdr);
 *
 * // You're probably too lazy to fill out the netlink pid, sequence number
 * // and message flags all the time. nl_send_auto_complete() automatically
 * // extends your message header as needed with an appropriate sequence
 * // number, the netlink pid stored in the netlink socket and the message
 * // flags NLM_F_REQUEST and NLM_F_ACK (if not disabled in the socket)
 * nl_send_auto_complete(sk, nlmsg_hdr(msg));
 *
 * // Simple protocols don't require the complex message construction interface
 * // and may favour nl_send_simple() to easly send a bunch of payload
 * // encapsulated in a netlink message header.
 * nl_send_simple(sk, MY_MSG_TYPE, 0, buf, sizeof(buf));
 * @endcode
 *
 * @par 3) Receiving data
 * @code
 * // nl_recv() receives a single message allocating a buffer for the message
 * // content and gives back the pointer to you.
 * struct sockaddr_nl peer;
 * unsigned char *msg;
 * nl_recv(sk, &peer, &msg);
 *
 * // nl_recvmsgs() receives a bunch of messages until the callback system
 * // orders it to state, usually after receving a compolete multi part
 * // message series.
 * nl_recvmsgs(sk, my_callback_configuration);
 *
 * // nl_recvmsgs_default() acts just like nl_recvmsg() but uses the callback
 * // configuration stored in the socket.
 * nl_recvmsgs_default(sk);
 *
 * // In case you want to wait for the ACK to be recieved that you requested
 * // with your latest message, you can call nl_wait_for_ack()
 * nl_wait_for_ack(sk);
 * @endcode
 *
 * @par 4) Closing
 * @code
 * // Close the socket first to release kernel memory
 * nl_close(sk);
 * @endcode
 * 
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

/**
 * @name Connection Management
 * @{
 */

/**
 * Create and connect netlink socket.
 * @arg sk		Netlink socket.
 * @arg protocol	Netlink protocol to use.
 *
 * Creates a netlink socket using the specified protocol, binds the socket
 * and issues a connection attempt.
 *
 * @return 0 on success or a negative error code.
 */
int nl_connect(struct nl_sock *sk, int protocol)
{
	int err;
	socklen_t addrlen;

	sk->s_fd = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (sk->s_fd < 0) {
		err = -nl_syserr2nlerr(errno);
		goto errout;
	}

	if (!(sk->s_flags & NL_SOCK_BUFSIZE_SET)) {
		err = nl_socket_set_buffer_size(sk, 0, 0);
		if (err < 0)
			goto errout;
	}

	err = bind(sk->s_fd, (struct sockaddr*) &sk->s_local,
		   sizeof(sk->s_local));
	if (err < 0) {
		err = -nl_syserr2nlerr(errno);
		goto errout;
	}

	addrlen = sizeof(sk->s_local);
	err = getsockname(sk->s_fd, (struct sockaddr *) &sk->s_local,
			  &addrlen);
	if (err < 0) {
		err = -nl_syserr2nlerr(errno);
		goto errout;
	}

	if (addrlen != sizeof(sk->s_local)) {
		err = -NLE_NOADDR;
		goto errout;
	}

	if (sk->s_local.nl_family != AF_NETLINK) {
		err = -NLE_AF_NOSUPPORT;
		goto errout;
	}

	sk->s_proto = protocol;

	return 0;
errout:
	close(sk->s_fd);
	sk->s_fd = -1;

	return err;
}

/**
 * Close/Disconnect netlink socket.
 * @arg sk		Netlink socket.
 */
void nl_close(struct nl_sock *sk)
{
	if (sk->s_fd >= 0) {
		close(sk->s_fd);
		sk->s_fd = -1;
	}

	sk->s_proto = 0;
}

/** @} */

/**
 * @name Send
 * @{
 */

/**
 * Send raw data over netlink socket.
 * @arg sk		Netlink socket.
 * @arg buf		Data buffer.
 * @arg size		Size of data buffer.
 * @return Number of characters written on success or a negative error code.
 */
int nl_sendto(struct nl_sock *sk, void *buf, size_t size)
{
	int ret;

	ret = sendto(sk->s_fd, buf, size, 0, (struct sockaddr *)
		     &sk->s_peer, sizeof(sk->s_peer));
	if (ret < 0)
		return -nl_syserr2nlerr(errno);

	return ret;
}

/**
 * Send netlink message with control over sendmsg() message header.
 * @arg sk		Netlink socket.
 * @arg msg		Netlink message to be sent.
 * @arg hdr		Sendmsg() message header.
 * @return Number of characters sent on sucess or a negative error code.
 */
int nl_sendmsg(struct nl_sock *sk, struct nl_msg *msg, struct msghdr *hdr)
{
	struct nl_cb *cb;
	int ret;

	struct iovec iov = {
		.iov_base = (void *) nlmsg_hdr(msg),
		.iov_len = nlmsg_hdr(msg)->nlmsg_len,
	};

	hdr->msg_iov = &iov;
	hdr->msg_iovlen = 1;

	nlmsg_set_src(msg, &sk->s_local);

	cb = sk->s_cb;
	if (cb->cb_set[NL_CB_MSG_OUT])
		if (nl_cb_call(cb, NL_CB_MSG_OUT, msg) != NL_OK)
			return 0;

	ret = sendmsg(sk->s_fd, hdr, 0);
	if (ret < 0)
		return -nl_syserr2nlerr(errno);

	return ret;
}


/**
 * Send netlink message.
 * @arg sk		Netlink socket.
 * @arg msg		Netlink message to be sent.
 * @see nl_sendmsg()
 * @return Number of characters sent on success or a negative error code.
 */
int nl_send(struct nl_sock *sk, struct nl_msg *msg)
{
	struct sockaddr_nl *dst;
	struct ucred *creds;
	
	struct msghdr hdr = {
		.msg_name = (void *) &sk->s_peer,
		.msg_namelen = sizeof(struct sockaddr_nl),
	};

	/* Overwrite destination if specified in the message itself, defaults
	 * to the peer address of the socket.
	 */
	dst = nlmsg_get_dst(msg);
	if (dst->nl_family == AF_NETLINK)
		hdr.msg_name = dst;

	/* Add credentials if present. */
	creds = nlmsg_get_creds(msg);
	if (creds != NULL) {
		char buf[CMSG_SPACE(sizeof(struct ucred))];
		struct cmsghdr *cmsg;

		hdr.msg_control = buf;
		hdr.msg_controllen = sizeof(buf);

		cmsg = CMSG_FIRSTHDR(&hdr);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_CREDENTIALS;
		cmsg->cmsg_len = CMSG_LEN(sizeof(struct ucred));
		memcpy(CMSG_DATA(cmsg), creds, sizeof(struct ucred));
	}

	return nl_sendmsg(sk, msg, &hdr);
}

/**
 * Send netlink message and check & extend header values as needed.
 * @arg sk		Netlink socket.
 * @arg msg		Netlink message to be sent.
 *
 * Checks the netlink message \c nlh for completness and extends it
 * as required before sending it out. Checked fields include pid,
 * sequence nr, and flags.
 *
 * @see nl_send()
 * @return Number of characters sent or a negative error code.
 */
int nl_send_auto_complete(struct nl_sock *sk, struct nl_msg *msg)
{
	struct nlmsghdr *nlh;
	struct nl_cb *cb = sk->s_cb;

	nlh = nlmsg_hdr(msg);
	if (nlh->nlmsg_pid == 0)
		nlh->nlmsg_pid = sk->s_local.nl_pid;

	if (nlh->nlmsg_seq == 0)
		nlh->nlmsg_seq = sk->s_seq_next++;

	if (msg->nm_protocol == -1)
		msg->nm_protocol = sk->s_proto;
	
	nlh->nlmsg_flags |= NLM_F_REQUEST;

	if (!(sk->s_flags & NL_NO_AUTO_ACK))
		nlh->nlmsg_flags |= NLM_F_ACK;

	if (cb->cb_send_ow)
		return cb->cb_send_ow(sk, msg);
	else
		return nl_send(sk, msg);
}

/**
 * Send simple netlink message using nl_send_auto_complete()
 * @arg sk		Netlink socket.
 * @arg type		Netlink message type.
 * @arg flags		Netlink message flags.
 * @arg buf		Data buffer.
 * @arg size		Size of data buffer.
 *
 * Builds a netlink message with the specified type and flags and
 * appends the specified data as payload to the message.
 *
 * @see nl_send_auto_complete()
 * @return Number of characters sent on success or a negative error code.
 */
int nl_send_simple(struct nl_sock *sk, int type, int flags, void *buf,
		   size_t size)
{
	int err;
	struct nl_msg *msg;

	msg = nlmsg_alloc_simple(type, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (buf && size) {
		err = nlmsg_append(msg, buf, size, NLMSG_ALIGNTO);
		if (err < 0)
			goto errout;
	}
	

	err = nl_send_auto_complete(sk, msg);
errout:
	nlmsg_free(msg);

	return err;
}

/** @} */

/**
 * @name Receive
 * @{
 */

/**
 * Receive data from netlink socket
 * @arg sk		Netlink socket.
 * @arg nla		Destination pointer for peer's netlink address.
 * @arg buf		Destination pointer for message content.
 * @arg creds		Destination pointer for credentials.
 *
 * Receives a netlink message, allocates a buffer in \c *buf and
 * stores the message content. The peer's netlink address is stored
 * in \c *nla. The caller is responsible for freeing the buffer allocated
 * in \c *buf if a positive value is returned.  Interrupted system calls
 * are handled by repeating the read. The input buffer size is determined
 * by peeking before the actual read is done.
 *
 * A non-blocking sockets causes the function to return immediately with
 * a return value of 0 if no data is available.
 *
 * @return Number of octets read, 0 on EOF or a negative error code.
 */
int nl_recv(struct nl_sock *sk, struct sockaddr_nl *nla,
	    unsigned char **buf, struct ucred **creds)
{
	int n;
	int flags = 0;
	static int page_size = 0;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = (void *) nla,
		.msg_namelen = sizeof(struct sockaddr_nl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};
	struct cmsghdr *cmsg;

	if (sk->s_flags & NL_MSG_PEEK)
		flags |= MSG_PEEK;

	if (page_size == 0)
		page_size = getpagesize() * 4;

	iov.iov_len = page_size;
	iov.iov_base = *buf = malloc(iov.iov_len);

	if (sk->s_flags & NL_SOCK_PASSCRED) {
		msg.msg_controllen = CMSG_SPACE(sizeof(struct ucred));
		msg.msg_control = calloc(1, msg.msg_controllen);
	}
retry:

	n = recvmsg(sk->s_fd, &msg, flags);
	if (!n)
		goto abort;
	else if (n < 0) {
		if (errno == EINTR) {
			NL_DBG(3, "recvmsg() returned EINTR, retrying\n");
			goto retry;
		} else if (errno == EAGAIN) {
			NL_DBG(3, "recvmsg() returned EAGAIN, aborting\n");
			goto abort;
		} else {
			free(msg.msg_control);
			free(*buf);
			return -nl_syserr2nlerr(errno);
		}
	}

	if (iov.iov_len < n ||
	    msg.msg_flags & MSG_TRUNC) {
		/* Provided buffer is not long enough, enlarge it
		 * and try again. */
		iov.iov_len *= 2;
		iov.iov_base = *buf = realloc(*buf, iov.iov_len);
		goto retry;
	} else if (msg.msg_flags & MSG_CTRUNC) {
		msg.msg_controllen *= 2;
		msg.msg_control = realloc(msg.msg_control, msg.msg_controllen);
		goto retry;
	} else if (flags != 0) {
		/* Buffer is big enough, do the actual reading */
		flags = 0;
		goto retry;
	}

	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) {
		free(msg.msg_control);
		free(*buf);
		return -NLE_NOADDR;
	}

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET &&
		    cmsg->cmsg_type == SCM_CREDENTIALS) {
			*creds = calloc(1, sizeof(struct ucred));
			memcpy(*creds, CMSG_DATA(cmsg), sizeof(struct ucred));
			break;
		}
	}

	free(msg.msg_control);
	return n;

abort:
	free(msg.msg_control);
	free(*buf);
	return 0;
}

#define NL_CB_CALL(cb, type, msg) \
do { \
	err = nl_cb_call(cb, type, msg); \
	switch (err) { \
	case NL_OK: \
		err = 0; \
		break; \
	case NL_SKIP: \
		goto skip; \
	case NL_STOP: \
		goto stop; \
	default: \
		goto out; \
	} \
} while (0)

static int recvmsgs(struct nl_sock *sk, struct nl_cb *cb)
{
	int n, err = 0, multipart = 0;
	unsigned char *buf = NULL;
	struct nlmsghdr *hdr;
	struct sockaddr_nl nla = {0};
	struct nl_msg *msg = NULL;
	struct ucred *creds = NULL;

continue_reading:
	NL_DBG(3, "Attempting to read from %p\n", sk);
	if (cb->cb_recv_ow)
		n = cb->cb_recv_ow(sk, &nla, &buf, &creds);
	else
		n = nl_recv(sk, &nla, &buf, &creds);

	if (n <= 0)
		return n;

	NL_DBG(3, "recvmsgs(%p): Read %d bytes\n", sk, n);

	hdr = (struct nlmsghdr *) buf;
	while (nlmsg_ok(hdr, n)) {
		NL_DBG(3, "recgmsgs(%p): Processing valid message...\n", sk);

		nlmsg_free(msg);
		msg = nlmsg_convert(hdr);
		if (!msg) {
			err = -NLE_NOMEM;
			goto out;
		}

		nlmsg_set_proto(msg, sk->s_proto);
		nlmsg_set_src(msg, &nla);
		if (creds)
			nlmsg_set_creds(msg, creds);

		/* Raw callback is the first, it gives the most control
		 * to the user and he can do his very own parsing. */
		if (cb->cb_set[NL_CB_MSG_IN])
			NL_CB_CALL(cb, NL_CB_MSG_IN, msg);

		/* Sequence number checking. The check may be done by
		 * the user, otherwise a very simple check is applied
		 * enforcing strict ordering */
		if (cb->cb_set[NL_CB_SEQ_CHECK])
			NL_CB_CALL(cb, NL_CB_SEQ_CHECK, msg);
		else if (hdr->nlmsg_seq != sk->s_seq_expect) {
			if (cb->cb_set[NL_CB_INVALID])
				NL_CB_CALL(cb, NL_CB_INVALID, msg);
			else {
				err = -NLE_SEQ_MISMATCH;
				goto out;
			}
		}

		if (hdr->nlmsg_type == NLMSG_DONE ||
		    hdr->nlmsg_type == NLMSG_ERROR ||
		    hdr->nlmsg_type == NLMSG_NOOP ||
		    hdr->nlmsg_type == NLMSG_OVERRUN) {
			/* We can't check for !NLM_F_MULTI since some netlink
			 * users in the kernel are broken. */
			sk->s_seq_expect++;
			NL_DBG(3, "recvmsgs(%p): Increased expected " \
			       "sequence number to %d\n",
			       sk, sk->s_seq_expect);
		}

		if (hdr->nlmsg_flags & NLM_F_MULTI)
			multipart = 1;
	
		/* Other side wishes to see an ack for this message */
		if (hdr->nlmsg_flags & NLM_F_ACK) {
			if (cb->cb_set[NL_CB_SEND_ACK])
				NL_CB_CALL(cb, NL_CB_SEND_ACK, msg);
			else {
				/* FIXME: implement */
			}
		}

		/* messages terminates a multpart message, this is
		 * usually the end of a message and therefore we slip
		 * out of the loop by default. the user may overrule
		 * this action by skipping this packet. */
		if (hdr->nlmsg_type == NLMSG_DONE) {
			multipart = 0;
			if (cb->cb_set[NL_CB_FINISH])
				NL_CB_CALL(cb, NL_CB_FINISH, msg);
		}

		/* Message to be ignored, the default action is to
		 * skip this message if no callback is specified. The
		 * user may overrule this action by returning
		 * NL_PROCEED. */
		else if (hdr->nlmsg_type == NLMSG_NOOP) {
			if (cb->cb_set[NL_CB_SKIPPED])
				NL_CB_CALL(cb, NL_CB_SKIPPED, msg);
			else
				goto skip;
		}

		/* Data got lost, report back to user. The default action is to
		 * quit parsing. The user may overrule this action by retuning
		 * NL_SKIP or NL_PROCEED (dangerous) */
		else if (hdr->nlmsg_type == NLMSG_OVERRUN) {
			if (cb->cb_set[NL_CB_OVERRUN])
				NL_CB_CALL(cb, NL_CB_OVERRUN, msg);
			else {
				err = -NLE_MSG_OVERFLOW;
				goto out;
			}
		}

		/* Message carries a nlmsgerr */
		else if (hdr->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *e = nlmsg_data(hdr);

			if (hdr->nlmsg_len < nlmsg_msg_size(sizeof(*e))) {
				/* Truncated error message, the default action
				 * is to stop parsing. The user may overrule
				 * this action by returning NL_SKIP or
				 * NL_PROCEED (dangerous) */
				if (cb->cb_set[NL_CB_INVALID])
					NL_CB_CALL(cb, NL_CB_INVALID, msg);
				else {
					err = -NLE_MSG_TRUNC;
					goto out;
				}
			} else if (e->error) {
				/* Error message reported back from kernel. */
				if (cb->cb_err) {
					err = cb->cb_err(&nla, e,
							   cb->cb_err_arg);
					if (err < 0)
						goto out;
					else if (err == NL_SKIP)
						goto skip;
					else if (err == NL_STOP) {
						err = -nl_syserr2nlerr(e->error);
						goto out;
					}
				} else {
					err = -nl_syserr2nlerr(e->error);
					goto out;
				}
			} else if (cb->cb_set[NL_CB_ACK])
				NL_CB_CALL(cb, NL_CB_ACK, msg);
		} else {
			/* Valid message (not checking for MULTIPART bit to
			 * get along with broken kernels. NL_SKIP has no
			 * effect on this.  */
			if (cb->cb_set[NL_CB_VALID])
				NL_CB_CALL(cb, NL_CB_VALID, msg);
		}
skip:
		err = 0;
		hdr = nlmsg_next(hdr, &n);
	}
	
	nlmsg_free(msg);
	free(buf);
	free(creds);
	buf = NULL;
	msg = NULL;
	creds = NULL;

	if (multipart) {
		/* Multipart message not yet complete, continue reading */
		goto continue_reading;
	}
stop:
	err = 0;
out:
	nlmsg_free(msg);
	free(buf);
	free(creds);

	return err;
}

/**
 * Receive a set of messages from a netlink socket.
 * @arg sk		Netlink socket.
 * @arg cb		set of callbacks to control behaviour.
 *
 * Repeatedly calls nl_recv() or the respective replacement if provided
 * by the application (see nl_cb_overwrite_recv()) and parses the
 * received data as netlink messages. Stops reading if one of the
 * callbacks returns NL_STOP or nl_recv returns either 0 or a negative error code.
 *
 * A non-blocking sockets causes the function to return immediately if
 * no data is available.
 *
 * @return 0 on success or a negative error code from nl_recv().
 */
int nl_recvmsgs(struct nl_sock *sk, struct nl_cb *cb)
{
	if (cb->cb_recvmsgs_ow)
		return cb->cb_recvmsgs_ow(sk, cb);
	else
		return recvmsgs(sk, cb);
}


static int ack_wait_handler(struct nl_msg *msg, void *arg)
{
	return NL_STOP;
}

/**
 * Wait for ACK.
 * @arg sk		Netlink socket.
 * @pre The netlink socket must be in blocking state.
 *
 * Waits until an ACK is received for the latest not yet acknowledged
 * netlink message.
 */
int nl_wait_for_ack(struct nl_sock *sk)
{
	int err;
	struct nl_cb *cb;

	cb = nl_cb_clone(sk->s_cb);
	if (cb == NULL)
		return -NLE_NOMEM;

	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_wait_handler, NULL);
	err = nl_recvmsgs(sk, cb);
	nl_cb_put(cb);

	return err;
}

/** @} */

/** @} */
