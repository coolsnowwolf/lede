/*
 * Copyright (C) 1987, Sun Microsystems, Inc.
 * Copyright (C) 2014 Quantenna Communications Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of Sun Microsystems, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <qcsapi_rpc_common/common/rpc_raw.h>

#define QRPC_CLNT_RAW_POLL_TIMEOUT	5000

enum clnt_stat qrpc_clnt_raw_call(CLIENT *cl, u_long proc, xdrproc_t xargs,
	caddr_t argsp, xdrproc_t xresults, caddr_t resultsp, struct timeval utimeout);
void qrpc_clnt_raw_abort(void);
void qrpc_clnt_raw_geterr(CLIENT *cl, struct rpc_err *errp);
bool_t qrpc_clnt_raw_freeres(CLIENT *cl, xdrproc_t xdr_res, caddr_t res_ptr);
void qrpc_clnt_raw_destroy(CLIENT *cl);
bool_t qrpc_clnt_raw_control(CLIENT *cl, int request, char *info);

struct qrpc_clnt_raw_priv {
	struct sockaddr_ll	dst_addr;
	struct rpc_err		rpc_error;
	XDR			xdrs_out;
	XDR			xdrs_in;
	uint8_t			*outbuf;
	uint8_t			*out_pktbuf;
	uint8_t			*inbuf;
	uint8_t			*in_pktbuf;
	struct qrpc_frame_hdr	out_hdr;
	uint32_t		xdrs_outpos;
	int			raw_sock;
	uint8_t			sess_id;
};

static const struct clnt_ops qrpc_clnt_raw_ops = {
	qrpc_clnt_raw_call,
	qrpc_clnt_raw_abort,
	qrpc_clnt_raw_geterr,
	qrpc_clnt_raw_freeres,
	qrpc_clnt_raw_destroy,
	qrpc_clnt_raw_control
};

static void qrpc_clnt_raw_free_priv(struct qrpc_clnt_raw_priv *const priv)
{
	free(priv->outbuf);
	free(priv->out_pktbuf);
	free(priv->inbuf);
	free(priv->in_pktbuf);
	if (priv->raw_sock >= 0)
		close(priv->raw_sock);
	free(priv);
}

CLIENT *qrpc_clnt_raw_create(u_long prog, u_long vers,
		const char *const srcif_name, const uint8_t * dmac_addr, uint8_t sess_id)
{
	CLIENT *client;
	int rawsock_fd;
	struct qrpc_clnt_raw_priv *priv;
	struct rpc_msg call_msg;

	rawsock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (rawsock_fd < 0)
		return NULL;

	if (qrpc_set_prot_filter(rawsock_fd, QRPC_RAW_SOCK_PROT) < 0) {
		close(rawsock_fd);
		return NULL;
	}

	priv = calloc(1, sizeof(*priv));
	if (!priv) {
		close(rawsock_fd);
		return NULL;
	}

	priv->raw_sock = rawsock_fd;
	priv->outbuf = calloc(1, QRPC_BUFFER_LEN);
	priv->inbuf = calloc(1, QRPC_BUFFER_LEN);
	priv->out_pktbuf = calloc(1, ETH_FRAME_LEN);
	priv->in_pktbuf = calloc(1, ETH_FRAME_LEN);
	if (!priv->outbuf || !priv->inbuf || !priv->out_pktbuf || !priv->in_pktbuf) {
		qrpc_clnt_raw_free_priv(priv);
		return NULL;
	}

	if (qrpc_clnt_raw_config_dst(rawsock_fd, srcif_name, &priv->dst_addr,
					dmac_addr, &priv->out_hdr.qhdr,
						QRPC_RAW_SOCK_PROT) < 0) {
		qrpc_clnt_raw_free_priv(priv);
		return NULL;
	}

	client = calloc(1, sizeof(*client));
	if (!client) {
		qrpc_clnt_raw_free_priv(priv);
		return NULL;
	}

	client->cl_ops = (struct clnt_ops *)&qrpc_clnt_raw_ops;
	client->cl_private = (caddr_t) priv;
	client->cl_auth = authnone_create();

	xdrmem_create(&priv->xdrs_in, (char *)priv->inbuf + sizeof(struct qrpc_frame_hdr),
			QRPC_BUFFER_LEN - sizeof(struct qrpc_frame_hdr), XDR_DECODE);

	xdrmem_create(&priv->xdrs_out, (char *)priv->outbuf,
			QRPC_BUFFER_LEN, XDR_ENCODE);
	call_msg.rm_xid = getpid();
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;
	if (!xdr_callhdr(&priv->xdrs_out, &call_msg)) {
		qrpc_clnt_raw_free_priv(priv);
		free(client);
		return NULL;
	}
	priv->xdrs_outpos = XDR_GETPOS(&(priv->xdrs_out));

	priv->sess_id = sess_id;

	return client;
}

static int qrpc_clnt_raw_call_send(struct qrpc_clnt_raw_priv *const priv, const int len)
{
	int ret;
	static const uint16_t payload_max = ETH_FRAME_LEN - sizeof(struct qrpc_frame_hdr);
	uint16_t pkt_nr;
	uint16_t i;
	uint16_t payload_done = 0;
	struct qrpc_frame_hdr *hdr;

	pkt_nr = (len + payload_max - 1) / payload_max;

	for (i = 0; i < pkt_nr; i++) {
		uint16_t payload_len = MIN((uint16_t)len - payload_done, payload_max);

		/* build an EthII frame */
		priv->out_hdr.sub_type = ((i != pkt_nr - 1) ? QRPC_FRAME_TYPE_FRAG
			: QRPC_FRAME_TYPE_COMPLETE);
		priv->out_hdr.sid = priv->sess_id;

		hdr = (struct qrpc_frame_hdr *)priv->out_pktbuf;
		memcpy(hdr, &priv->out_hdr, sizeof(priv->out_hdr));
		memcpy(hdr + 1, priv->outbuf + payload_done, payload_len);
		payload_done += payload_len;

		do {
			ret = sendto(priv->raw_sock, priv->out_pktbuf, sizeof(struct qrpc_frame_hdr) + payload_len, 0,
					(struct sockaddr *)&priv->dst_addr, sizeof(priv->dst_addr));
		} while (ret < 0 && errno == EINTR);

		if ((uint16_t)ret != sizeof(struct qrpc_frame_hdr) + payload_len) {
			priv->rpc_error.re_status = RPC_CANTSEND;
			return -1;
		}
	}

	return 0;
}

static int qrpc_clnt_raw_call_recv(struct qrpc_clnt_raw_priv *const priv)
{
	struct pollfd fds;
	struct sockaddr_ll lladdr;
	socklen_t addrlen = sizeof(lladdr);
	int ret;
	uint16_t payload_done = sizeof(struct qrpc_frame_hdr);
	struct qrpc_frame_hdr hdr;

	do {
		fds.fd = priv->raw_sock;
		fds.events = POLLIN;
		do {
			ret = poll(&fds, 1, QRPC_CLNT_RAW_POLL_TIMEOUT);
		} while (ret < 0 && errno == EINTR);
		if (!ret) {
			priv->rpc_error.re_status = RPC_TIMEDOUT;
			return -1;
		}
		if (ret < 0) {
			priv->rpc_error.re_status = RPC_SYSTEMERROR;
			return -1;
		}

		do {
			ret = recvfrom(priv->raw_sock, priv->in_pktbuf, ETH_FRAME_LEN,
					0, (struct sockaddr *)&lladdr, &addrlen);
		} while (ret < 0 && errno == EINTR);

		if (lladdr.sll_pkttype != PACKET_HOST) {
			priv->rpc_error.re_status = RPC_TIMEDOUT;
			return -1;
		}

		if ((ret < (int)sizeof(struct qrpc_frame_hdr))
				|| (ret - sizeof(struct qrpc_frame_hdr) + payload_done > QRPC_BUFFER_LEN)) {
			priv->rpc_error.re_status = RPC_CANTRECV;
			return -1;
		}

		/* assemble the buffer */
		memcpy(&hdr, priv->in_pktbuf, sizeof(struct qrpc_frame_hdr));
		memcpy(priv->inbuf + payload_done, priv->in_pktbuf + sizeof(struct qrpc_frame_hdr),
			ret - sizeof(struct qrpc_frame_hdr));

		payload_done += (ret - sizeof(struct qrpc_frame_hdr));

	} while (hdr.sub_type == QRPC_FRAME_TYPE_FRAG);

	memcpy(priv->inbuf, &hdr, sizeof(struct qrpc_frame_hdr));

	return 0;
}

enum clnt_stat qrpc_clnt_raw_call(CLIENT *cl, u_long proc, xdrproc_t xargs, caddr_t argsp,
					xdrproc_t xresults, caddr_t resultsp,
					struct timeval utimeout)
{
	struct qrpc_clnt_raw_priv *priv = (struct qrpc_clnt_raw_priv *)cl->cl_private;
	XDR *xdrs_out = &priv->xdrs_out;
	XDR *xdrs_in = &priv->xdrs_in;
	struct rpc_msg reply_msg;
	struct timeval curr_time;
	struct qrpc_frame_hdr *hdr;
	uint16_t tmp;

	if (xargs) {
		xdrs_out->x_op = XDR_ENCODE;
		XDR_SETPOS(xdrs_out, priv->xdrs_outpos);

		if ((!XDR_PUTLONG(xdrs_out, (long *)&proc)) ||
				(!AUTH_MARSHALL(cl->cl_auth, xdrs_out)) ||
				(!(*xargs) (xdrs_out, argsp))) {
			priv->rpc_error.re_status = RPC_CANTENCODEARGS;
			return priv->rpc_error.re_status;
		}
		tmp = ntohs(priv->out_hdr.seq);
		priv->out_hdr.seq = htons(tmp + 1);
		if (qrpc_clnt_raw_call_send(priv, XDR_GETPOS(xdrs_out)) < 0) {
			return priv->rpc_error.re_status;
		}
	}

	if (gettimeofday(&curr_time, NULL) < 0) {
		priv->rpc_error.re_status = RPC_SYSTEMERROR;
		return priv->rpc_error.re_status;
	}
	utimeout.tv_sec += curr_time.tv_sec;
	/* Waiting for reply */
	do {
		if (qrpc_clnt_raw_call_recv(priv) < 0) {
			if (priv->rpc_error.re_status == RPC_TIMEDOUT)
				continue;
			else
				break;
		}

		hdr = (struct qrpc_frame_hdr *)priv->inbuf;
		if (xargs && priv->out_hdr.seq != hdr->seq) {
			continue;
		}

		xdrs_in->x_op = XDR_DECODE;
		XDR_SETPOS(xdrs_in, 0);

		reply_msg.acpted_rply.ar_verf = _null_auth;
		reply_msg.acpted_rply.ar_results.where = resultsp;
		reply_msg.acpted_rply.ar_results.proc = xresults;

		if (xdr_replymsg(xdrs_in, &reply_msg)) {
			if (reply_msg.rm_xid != (unsigned long)getpid()) {
				continue;
			}
			_seterr_reply(&reply_msg, &priv->rpc_error);
			if (priv->rpc_error.re_status == RPC_SUCCESS) {
				if (!AUTH_VALIDATE(cl->cl_auth, &reply_msg.acpted_rply.ar_verf)) {
					priv->rpc_error.re_status = RPC_AUTHERROR;
					priv->rpc_error.re_why = AUTH_INVALIDRESP;
				}
				break;
			}
		} else {
			priv->rpc_error.re_status = RPC_CANTDECODERES;
		}
	} while ((gettimeofday(&curr_time, NULL) == 0) && (curr_time.tv_sec < utimeout.tv_sec));

	return priv->rpc_error.re_status;
}

void qrpc_clnt_raw_abort(void)
{
}

void qrpc_clnt_raw_geterr(CLIENT *cl, struct rpc_err *errp)
{
	struct qrpc_clnt_raw_priv *priv = (struct qrpc_clnt_raw_priv *)cl->cl_private;

	*errp = priv->rpc_error;
}

bool_t qrpc_clnt_raw_freeres(CLIENT *cl, xdrproc_t xdr_res, caddr_t res_ptr)
{
	return FALSE;
}

void qrpc_clnt_raw_destroy(CLIENT *cl)
{
	struct qrpc_clnt_raw_priv *priv = (struct qrpc_clnt_raw_priv *)cl->cl_private;

	if (priv) {
		XDR_DESTROY(&priv->xdrs_out);
		XDR_DESTROY(&priv->xdrs_in);
		qrpc_clnt_raw_free_priv(priv);
	}
	free(cl);
}

bool_t qrpc_clnt_raw_control(CLIENT *cl, int request, char *info)
{
	return FALSE;
}
