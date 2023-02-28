#ifndef _LIBMNL_H_
#define _LIBMNL_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h> /* for sa_family_t */
#include <linux/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Netlink socket API
 */

#define MNL_SOCKET_AUTOPID	0
#define MNL_SOCKET_BUFFER_SIZE (sysconf(_SC_PAGESIZE) < 8192L ? sysconf(_SC_PAGESIZE) : 8192L)
#define MNL_SOCKET_DUMP_SIZE	32768

struct mnl_socket;

extern struct mnl_socket *mnl_socket_open(int bus);
extern struct mnl_socket *mnl_socket_open2(int bus, int flags);
extern struct mnl_socket *mnl_socket_fdopen(int fd);
extern int mnl_socket_bind(struct mnl_socket *nl, unsigned int groups, pid_t pid);
extern int mnl_socket_close(struct mnl_socket *nl);
extern int mnl_socket_get_fd(const struct mnl_socket *nl);
extern unsigned int mnl_socket_get_portid(const struct mnl_socket *nl);
extern ssize_t mnl_socket_sendto(const struct mnl_socket *nl, const void *req, size_t siz);
extern ssize_t mnl_socket_recvfrom(const struct mnl_socket *nl, void *buf, size_t siz);
extern int mnl_socket_setsockopt(const struct mnl_socket *nl, int type, void *buf, socklen_t len);
extern int mnl_socket_getsockopt(const struct mnl_socket *nl, int type, void *buf, socklen_t *len);

/*
 * Netlink message API
 */

#define MNL_ALIGNTO		4
#define MNL_ALIGN(len)		(((len)+MNL_ALIGNTO-1) & ~(MNL_ALIGNTO-1))
#define MNL_NLMSG_HDRLEN	MNL_ALIGN(sizeof(struct nlmsghdr))

extern size_t mnl_nlmsg_size(size_t len);
extern size_t mnl_nlmsg_get_payload_len(const struct nlmsghdr *nlh);

/* Netlink message header builder */
extern struct nlmsghdr *mnl_nlmsg_put_header(void *buf);
extern void *mnl_nlmsg_put_extra_header(struct nlmsghdr *nlh, size_t size);

/* Netlink message iterators */
extern bool mnl_nlmsg_ok(const struct nlmsghdr *nlh, int len);
extern struct nlmsghdr *mnl_nlmsg_next(const struct nlmsghdr *nlh, int *len);

/* Netlink sequence tracking */
extern bool mnl_nlmsg_seq_ok(const struct nlmsghdr *nlh, unsigned int seq);

/* Netlink portID checking */
extern bool mnl_nlmsg_portid_ok(const struct nlmsghdr *nlh, unsigned int portid);

/* Netlink message getters */
extern void *mnl_nlmsg_get_payload(const struct nlmsghdr *nlh);
extern void *mnl_nlmsg_get_payload_offset(const struct nlmsghdr *nlh, size_t offset);
extern void *mnl_nlmsg_get_payload_tail(const struct nlmsghdr *nlh);

/* Netlink message printer */
extern void mnl_nlmsg_fprintf(FILE *fd, const void *data, size_t datalen, size_t extra_header_size);

/* Message batch helpers */
struct mnl_nlmsg_batch;
extern struct mnl_nlmsg_batch *mnl_nlmsg_batch_start(void *buf, size_t bufsiz);
extern bool mnl_nlmsg_batch_next(struct mnl_nlmsg_batch *b);
extern void mnl_nlmsg_batch_stop(struct mnl_nlmsg_batch *b);
extern size_t mnl_nlmsg_batch_size(struct mnl_nlmsg_batch *b);
extern void mnl_nlmsg_batch_reset(struct mnl_nlmsg_batch *b);
extern void *mnl_nlmsg_batch_head(struct mnl_nlmsg_batch *b);
extern void *mnl_nlmsg_batch_current(struct mnl_nlmsg_batch *b);
extern bool mnl_nlmsg_batch_is_empty(struct mnl_nlmsg_batch *b);

/*
 * Netlink attributes API
 */
#define MNL_ATTR_HDRLEN	MNL_ALIGN(sizeof(struct nlattr))

/* TLV attribute getters */
extern uint16_t mnl_attr_get_type(const struct nlattr *attr);
extern uint16_t mnl_attr_get_len(const struct nlattr *attr);
extern uint16_t mnl_attr_get_payload_len(const struct nlattr *attr);
extern void *mnl_attr_get_payload(const struct nlattr *attr);
extern uint8_t mnl_attr_get_u8(const struct nlattr *attr);
extern uint16_t mnl_attr_get_u16(const struct nlattr *attr);
extern uint32_t mnl_attr_get_u32(const struct nlattr *attr);
extern uint64_t mnl_attr_get_u64(const struct nlattr *attr);
extern const char *mnl_attr_get_str(const struct nlattr *attr);

/* TLV attribute putters */
extern void mnl_attr_put(struct nlmsghdr *nlh, uint16_t type, size_t len, const void *data);
extern void mnl_attr_put_u8(struct nlmsghdr *nlh, uint16_t type, uint8_t data);
extern void mnl_attr_put_u16(struct nlmsghdr *nlh, uint16_t type, uint16_t data);
extern void mnl_attr_put_u32(struct nlmsghdr *nlh, uint16_t type, uint32_t data);
extern void mnl_attr_put_u64(struct nlmsghdr *nlh, uint16_t type, uint64_t data);
extern void mnl_attr_put_str(struct nlmsghdr *nlh, uint16_t type, const char *data);
extern void mnl_attr_put_strz(struct nlmsghdr *nlh, uint16_t type, const char *data);

/* TLV attribute putters with buffer boundary checkings */
extern bool mnl_attr_put_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, size_t len, const void *data);
extern bool mnl_attr_put_u8_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint8_t data);
extern bool mnl_attr_put_u16_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint16_t data);
extern bool mnl_attr_put_u32_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint32_t data);
extern bool mnl_attr_put_u64_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, uint64_t data);
extern bool mnl_attr_put_str_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, const char *data);
extern bool mnl_attr_put_strz_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type, const char *data);

/* TLV attribute nesting */
extern struct nlattr *mnl_attr_nest_start(struct nlmsghdr *nlh, uint16_t type);
extern struct nlattr *mnl_attr_nest_start_check(struct nlmsghdr *nlh, size_t buflen, uint16_t type);
extern void mnl_attr_nest_end(struct nlmsghdr *nlh, struct nlattr *start);
extern void mnl_attr_nest_cancel(struct nlmsghdr *nlh, struct nlattr *start);

/* TLV validation */
extern int mnl_attr_type_valid(const struct nlattr *attr, uint16_t maxtype);

enum mnl_attr_data_type {
	MNL_TYPE_UNSPEC,
	MNL_TYPE_U8,
	MNL_TYPE_U16,
	MNL_TYPE_U32,
	MNL_TYPE_U64,
	MNL_TYPE_STRING,
	MNL_TYPE_FLAG,
	MNL_TYPE_MSECS,
	MNL_TYPE_NESTED,
	MNL_TYPE_NESTED_COMPAT,
	MNL_TYPE_NUL_STRING,
	MNL_TYPE_BINARY,
	MNL_TYPE_MAX,
};

extern int mnl_attr_validate(const struct nlattr *attr, enum mnl_attr_data_type type);
extern int mnl_attr_validate2(const struct nlattr *attr, enum mnl_attr_data_type type, size_t len);

/* TLV iterators */
extern bool mnl_attr_ok(const struct nlattr *attr, int len);
extern struct nlattr *mnl_attr_next(const struct nlattr *attr);

#define mnl_attr_for_each(attr, nlh, offset) \
	for ((attr) = mnl_nlmsg_get_payload_offset((nlh), (offset)); \
	     mnl_attr_ok((attr), (char *)mnl_nlmsg_get_payload_tail(nlh) - (char *)(attr)); \
	     (attr) = mnl_attr_next(attr))

#define mnl_attr_for_each_nested(attr, nest) \
	for ((attr) = mnl_attr_get_payload(nest); \
	     mnl_attr_ok((attr), (char *)mnl_attr_get_payload(nest) + mnl_attr_get_payload_len(nest) - (char *)(attr)); \
	     (attr) = mnl_attr_next(attr))

#define mnl_attr_for_each_payload(payload, payload_size) \
	for ((attr) = (payload); \
	     mnl_attr_ok((attr), (char *)(payload) + payload_size - (char *)(attr)); \
	     (attr) = mnl_attr_next(attr))

/* TLV callback-based attribute parsers */
typedef int (*mnl_attr_cb_t)(const struct nlattr *attr, void *data);

extern int mnl_attr_parse(const struct nlmsghdr *nlh, unsigned int offset, mnl_attr_cb_t cb, void *data);
extern int mnl_attr_parse_nested(const struct nlattr *attr, mnl_attr_cb_t cb, void *data);
extern int mnl_attr_parse_payload(const void *payload, size_t payload_len, mnl_attr_cb_t cb, void *data);

/*
 * callback API
 */
#define MNL_CB_ERROR		-1
#define MNL_CB_STOP		 0
#define MNL_CB_OK		 1

typedef int (*mnl_cb_t)(const struct nlmsghdr *nlh, void *data);

extern int mnl_cb_run(const void *buf, size_t numbytes, unsigned int seq,
		      unsigned int portid, mnl_cb_t cb_data, void *data);

extern int mnl_cb_run2(const void *buf, size_t numbytes, unsigned int seq,
		       unsigned int portid, mnl_cb_t cb_data, void *data,
		       const mnl_cb_t *cb_ctl_array,
		       unsigned int cb_ctl_array_len);

/*
 * other declarations
 */

#ifndef SOL_NETLINK
#define SOL_NETLINK	270
#endif

#ifndef MNL_ARRAY_SIZE
#define MNL_ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
