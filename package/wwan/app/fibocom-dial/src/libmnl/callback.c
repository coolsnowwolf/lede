/*
 * (C) 2008-2010 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */

#include <errno.h>

#include "libmnl.h"

static int mnl_cb_noop(const struct nlmsghdr *nlh, void *data)
{
	return MNL_CB_OK;
}

static int mnl_cb_error(const struct nlmsghdr *nlh, void *data)
{
	const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);

	if (nlh->nlmsg_len < mnl_nlmsg_size(sizeof(struct nlmsgerr))) {
		errno = EBADMSG; 
		return MNL_CB_ERROR;
	}
	/* Netlink subsystems returns the errno value with different signess */
	if (err->error < 0)
		errno = -err->error;
	else
		errno = err->error;

	return err->error == 0 ? MNL_CB_STOP : MNL_CB_ERROR;
}

static int mnl_cb_stop(const struct nlmsghdr *nlh, void *data)
{
	return MNL_CB_STOP;
}

static const mnl_cb_t default_cb_array[NLMSG_MIN_TYPE] = {
	[NLMSG_NOOP]	= mnl_cb_noop,
	[NLMSG_ERROR]	= mnl_cb_error,
	[NLMSG_DONE]	= mnl_cb_stop,
	[NLMSG_OVERRUN]	= mnl_cb_noop,
};

static inline int __mnl_cb_run(const void *buf, size_t numbytes,
			       unsigned int seq, unsigned int portid,
			       mnl_cb_t cb_data, void *data,
			       const mnl_cb_t *cb_ctl_array,
			       unsigned int cb_ctl_array_len)
{
	int ret = MNL_CB_OK, len = numbytes;
	const struct nlmsghdr *nlh = buf;

	while (mnl_nlmsg_ok(nlh, len)) {
		/* check message source */
		if (!mnl_nlmsg_portid_ok(nlh, portid)) {
			errno = ESRCH;
			return -1;
		}
		/* perform sequence tracking */
		if (!mnl_nlmsg_seq_ok(nlh, seq)) {
			errno = EPROTO;
			return -1;
		}

		/* dump was interrupted */
		if (nlh->nlmsg_flags & NLM_F_DUMP_INTR) {
			errno = EINTR;
			return -1;
		}

		/* netlink data message handling */
		if (nlh->nlmsg_type >= NLMSG_MIN_TYPE) { 
			if (cb_data){
				ret = cb_data(nlh, data);
				if (ret <= MNL_CB_STOP)
					goto out;
			}
		} else if (nlh->nlmsg_type < cb_ctl_array_len) {
			if (cb_ctl_array && cb_ctl_array[nlh->nlmsg_type]) {
				ret = cb_ctl_array[nlh->nlmsg_type](nlh, data);
				if (ret <= MNL_CB_STOP)
					goto out;
			}
		} else if (default_cb_array[nlh->nlmsg_type]) {
			ret = default_cb_array[nlh->nlmsg_type](nlh, data);
			if (ret <= MNL_CB_STOP)
				goto out;
		}
		nlh = mnl_nlmsg_next(nlh, &len);
	}
out:
	return ret;
}

/**
 * \defgroup callback Callback helpers
 * @{
 */

/**
 * mnl_cb_run2 - callback runqueue for netlink messages
 * \param buf buffer that contains the netlink messages
 * \param numbytes number of bytes stored in the buffer
 * \param seq sequence number that we expect to receive
 * \param portid Netlink PortID that we expect to receive
 * \param cb_data callback handler for data messages
 * \param data pointer to data that will be passed to the data callback handler
 * \param cb_ctl_array array of custom callback handlers from control messages
 * \param cb_ctl_array_len array length of custom control callback handlers
 *
 * You can set the cb_ctl_array to NULL if you want to use the default control
 * callback handlers, in that case, the parameter cb_ctl_array_len is not
 * checked.
 *
 * Your callback may return three possible values:
 * 	- MNL_CB_ERROR (<=-1): an error has occurred. Stop callback runqueue.
 * 	- MNL_CB_STOP (=0): stop callback runqueue.
 * 	- MNL_CB_OK (>=1): no problem has occurred.
 *
 * This function propagates the callback return value. On error, it returns
 * -1 and errno is explicitly set. If the portID is not the expected, errno
 * is set to ESRCH. If the sequence number is not the expected, errno is set
 * to EPROTO. If the dump was interrupted, errno is set to EINTR and you should
 * request a new fresh dump again.
 */
int mnl_cb_run2(const void *buf, size_t numbytes,
			      unsigned int seq, unsigned int portid,
			      mnl_cb_t cb_data, void *data,
			      const mnl_cb_t *cb_ctl_array,
			      unsigned int cb_ctl_array_len)
{
	return __mnl_cb_run(buf, numbytes, seq, portid, cb_data, data,
			    cb_ctl_array, cb_ctl_array_len);
}

/**
 * mnl_cb_run - callback runqueue for netlink messages (simplified version)
 * \param buf buffer that contains the netlink messages
 * \param numbytes number of bytes stored in the buffer
 * \param seq sequence number that we expect to receive
 * \param portid Netlink PortID that we expect to receive
 * \param cb_data callback handler for data messages
 * \param data pointer to data that will be passed to the data callback handler
 *
 * This function is like mnl_cb_run2() but it does not allow you to set
 * the control callback handlers.
 *
 * Your callback may return three possible values:
 * 	- MNL_CB_ERROR (<=-1): an error has occurred. Stop callback runqueue.
 * 	- MNL_CB_STOP (=0): stop callback runqueue.
 * 	- MNL_CB_OK (>=1): no problems has occurred.
 *
 * This function propagates the callback return value.
 */
int mnl_cb_run(const void *buf, size_t numbytes, unsigned int seq,
			     unsigned int portid, mnl_cb_t cb_data, void *data)
{
	return __mnl_cb_run(buf, numbytes, seq, portid, cb_data, data, NULL, 0);
}

/**
 * @}
 */
