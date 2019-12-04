/*
 * netlink/attr.h		Netlink Attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_ATTR_H_
#define NETLINK_ATTR_H_

#include <netlink/netlink.h>
#include <netlink/object.h>
#include <netlink/addr.h>
#include <netlink/data.h>
#include <netlink/msg.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_msg;

/**
 * @name Basic Attribute Data Types
 * @{
 */

 /**
  * @ingroup attr
  * Basic attribute data types
  *
  * See \ref attr_datatypes for more details.
  */
enum {
	NLA_UNSPEC,	/**< Unspecified type, binary data chunk */
	NLA_U8,		/**< 8 bit integer */
	NLA_U16,	/**< 16 bit integer */
	NLA_U32,	/**< 32 bit integer */
	NLA_U64,	/**< 64 bit integer */
	NLA_STRING,	/**< NUL terminated character string */
	NLA_FLAG,	/**< Flag */
	NLA_MSECS,	/**< Micro seconds (64bit) */
	NLA_NESTED,	/**< Nested attributes */
	__NLA_TYPE_MAX,
};

#define NLA_TYPE_MAX (__NLA_TYPE_MAX - 1)

/** @} */

/**
 * @ingroup attr
 * Attribute validation policy.
 *
 * See \ref attr_datatypes for more details.
 */
struct nla_policy {
	/** Type of attribute or NLA_UNSPEC */
	uint16_t	type;

	/** Minimal length of payload required */
	uint16_t	minlen;

	/** Maximal length of payload allowed */
	uint16_t	maxlen;
};

/* Attribute parsing */
extern int		nla_ok(const struct nlattr *, int);
extern struct nlattr *	nla_next(const struct nlattr *, int *);
extern int		nla_parse(struct nlattr **, int, struct nlattr *,
				  int, struct nla_policy *);
extern int		nla_validate(struct nlattr *, int, int,
				     struct nla_policy *);
extern struct nlattr *	nla_find(struct nlattr *, int, int);

/* Unspecific attribute */
extern struct nlattr *	nla_reserve(struct nl_msg *, int, int);
extern int		nla_put(struct nl_msg *, int, int, const void *);

/**
 * nlmsg_find_attr - find a specific attribute in a netlink message
 * @arg nlh		netlink message header
 * @arg hdrlen		length of familiy specific header
 * @arg attrtype	type of attribute to look for
 *
 * Returns the first attribute which matches the specified type.
 */
static inline struct nlattr *nlmsg_find_attr(struct nlmsghdr *nlh, int hdrlen, int attrtype)
{
	return nla_find(nlmsg_attrdata(nlh, hdrlen),
			nlmsg_attrlen(nlh, hdrlen), attrtype);
}


/**
 * Return size of attribute whithout padding.
 * @arg payload		Payload length of attribute.
 *
 * @code
 *    <-------- nla_attr_size(payload) --------->
 *   +------------------+- - -+- - - - - - - - - +- - -+
 *   | Attribute Header | Pad |     Payload      | Pad |
 *   +------------------+- - -+- - - - - - - - - +- - -+
 * @endcode
 *
 * @return Size of attribute in bytes without padding.
 */
static inline int nla_attr_size(int payload)
{
	return NLA_HDRLEN + payload;
}

/**
 * Return size of attribute including padding.
 * @arg payload		Payload length of attribute.
 *
 * @code
 *    <----------- nla_total_size(payload) ----------->
 *   +------------------+- - -+- - - - - - - - - +- - -+
 *   | Attribute Header | Pad |     Payload      | Pad |
 *   +------------------+- - -+- - - - - - - - - +- - -+
 * @endcode
 *
 * @return Size of attribute in bytes.
 */
static inline int nla_total_size(int payload)
{
	return NLA_ALIGN(nla_attr_size(payload));
}

/**
 * Return length of padding at the tail of the attribute.
 * @arg payload		Payload length of attribute.
 *
 * @code
 *   +------------------+- - -+- - - - - - - - - +- - -+
 *   | Attribute Header | Pad |     Payload      | Pad |
 *   +------------------+- - -+- - - - - - - - - +- - -+
 *                                                <--->  
 * @endcode
 *
 * @return Length of padding in bytes.
 */
static inline int nla_padlen(int payload)
{
	return nla_total_size(payload) - nla_attr_size(payload);
}

/**
 * Return type of the attribute.
 * @arg nla		Attribute.
 *
 * @return Type of attribute.
 */
static inline int nla_type(const struct nlattr *nla)
{
	return nla->nla_type & NLA_TYPE_MASK;
}

/**
 * Return pointer to the payload section.
 * @arg nla		Attribute.
 *
 * @return Pointer to start of payload section.
 */
static inline void *nla_data(const struct nlattr *nla)
{
	return (char *) nla + NLA_HDRLEN;
}

/**
 * Return length of the payload .
 * @arg nla		Attribute
 *
 * @return Length of payload in bytes.
 */
static inline int nla_len(const struct nlattr *nla)
{
	return nla->nla_len - NLA_HDRLEN;
}

/**
 * Copy attribute payload to another memory area.
 * @arg dest		Pointer to destination memory area.
 * @arg src		Attribute
 * @arg count		Number of bytes to copy at most.
 *
 * Note: The number of bytes copied is limited by the length of
 *       the attribute payload.
 *
 * @return The number of bytes copied to dest.
 */
static inline int nla_memcpy(void *dest, struct nlattr *src, int count)
{
	int minlen;

	if (!src)
		return 0;
	
	minlen = min_t(int, count, nla_len(src));
	memcpy(dest, nla_data(src), minlen);

	return minlen;
}


/**
 * Add abstract data as unspecific attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg data		Abstract data object.
 *
 * Equivalent to nla_put() except that the length of the payload is
 * derived from the abstract data object.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_data(struct nl_msg *msg, int attrtype, struct nl_data *data)
{
	return nla_put(msg, attrtype, nl_data_get_size(data),
		       nl_data_get(data));
}

/**
 * Add abstract address as unspecific attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg addr		Abstract address object.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_addr(struct nl_msg *msg, int attrtype, struct nl_addr *addr)
{
	return nla_put(msg, attrtype, nl_addr_get_len(addr),
		       nl_addr_get_binary_addr(addr));
}

/** @} */

/**
 * @name Integer Attributes
 */

/**
 * Add 8 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value to store as payload.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_u8(struct nl_msg *msg, int attrtype, uint8_t value)
{
	return nla_put(msg, attrtype, sizeof(uint8_t), &value);
}

/**
 * Return value of 8 bit integer attribute.
 * @arg nla		8 bit integer attribute
 *
 * @return Payload as 8 bit integer.
 */
static inline uint8_t nla_get_u8(struct nlattr *nla)
{
	return *(uint8_t *) nla_data(nla);
}

/**
 * Add 16 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value to store as payload.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_u16(struct nl_msg *msg, int attrtype, uint16_t value)
{
	return nla_put(msg, attrtype, sizeof(uint16_t), &value);
}

/**
 * Return payload of 16 bit integer attribute.
 * @arg nla		16 bit integer attribute
 *
 * @return Payload as 16 bit integer.
 */
static inline uint16_t nla_get_u16(struct nlattr *nla)
{
	return *(uint16_t *) nla_data(nla);
}

/**
 * Add 32 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value to store as payload.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_u32(struct nl_msg *msg, int attrtype, uint32_t value)
{
	return nla_put(msg, attrtype, sizeof(uint32_t), &value);
}

/**
 * Return payload of 32 bit integer attribute.
 * @arg nla		32 bit integer attribute.
 *
 * @return Payload as 32 bit integer.
 */
static inline uint32_t nla_get_u32(struct nlattr *nla)
{
	return *(uint32_t *) nla_data(nla);
}

/**
 * Add 64 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value to store as payload.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_u64(struct nl_msg *msg, int attrtype, uint64_t value)
{
	return nla_put(msg, attrtype, sizeof(uint64_t), &value);
}

/**
 * Return payload of u64 attribute
 * @arg nla		u64 netlink attribute
 *
 * @return Payload as 64 bit integer.
 */
static inline uint64_t nla_get_u64(struct nlattr *nla)
{
	uint64_t tmp;

	nla_memcpy(&tmp, nla, sizeof(tmp));

	return tmp;
}

/**
 * Add string attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg str		NUL terminated string.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_string(struct nl_msg *msg, int attrtype, const char *str)
{
	return nla_put(msg, attrtype, strlen(str) + 1, str);
}

/**
 * Return payload of string attribute.
 * @arg nla		String attribute.
 *
 * @return Pointer to attribute payload.
 */
static inline char *nla_get_string(struct nlattr *nla)
{
	return (char *) nla_data(nla);
}

static inline char *nla_strdup(struct nlattr *nla)
{
	return strdup(nla_get_string(nla));
}

/** @} */

/**
 * @name Flag Attribute
 */

/**
 * Add flag netlink attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_flag(struct nl_msg *msg, int attrtype)
{
	return nla_put(msg, attrtype, 0, NULL);
}

/**
 * Return true if flag attribute is set.
 * @arg nla		Flag netlink attribute.
 *
 * @return True if flag is set, otherwise false.
 */
static inline int nla_get_flag(struct nlattr *nla)
{
	return !!nla;
}

/** @} */

/**
 * @name Microseconds Attribute
 */

/**
 * Add a msecs netlink attribute to a netlink message
 * @arg n		netlink message
 * @arg attrtype	attribute type
 * @arg msecs 		number of msecs
 */
static inline int nla_put_msecs(struct nl_msg *n, int attrtype, unsigned long msecs)
{
	return nla_put_u64(n, attrtype, msecs);
}

/**
 * Return payload of msecs attribute
 * @arg nla		msecs netlink attribute
 *
 * @return the number of milliseconds.
 */
static inline unsigned long nla_get_msecs(struct nlattr *nla)
{
	return nla_get_u64(nla);
}

/**
 * Add nested attributes to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg nested		Message containing attributes to be nested.
 *
 * Takes the attributes found in the \a nested message and appends them
 * to the message \a msg nested in a container of the type \a attrtype.
 * The \a nested message may not have a family specific header.
 *
 * @see nla_put
 * @return 0 on success or a negative error code.
 */
static inline int nla_put_nested(struct nl_msg *msg, int attrtype, struct nl_msg *nested)
{
	return nla_put(msg, attrtype, nlmsg_len(nested->nm_nlh),
		       nlmsg_data(nested->nm_nlh));
}

/**
 * Start a new level of nested attributes.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type of container.
 *
 * @return Pointer to container attribute.
 */
static inline struct nlattr *nla_nest_start(struct nl_msg *msg, int attrtype)
{
	struct nlattr *start = (struct nlattr *) nlmsg_tail(msg->nm_nlh);

	if (nla_put(msg, attrtype, 0, NULL) < 0)
		return NULL;

	return start;
}

/**
 * Finalize nesting of attributes.
 * @arg msg		Netlink message.
 * @arg start		Container attribute as returned from nla_nest_start().
 *
 * Corrects the container attribute header to include the appeneded attributes.
 *
 * @return 0
 */
static inline int nla_nest_end(struct nl_msg *msg, struct nlattr *start)
{
	start->nla_len = (unsigned char *) nlmsg_tail(msg->nm_nlh) -
				(unsigned char *) start;
	return 0;
}

/**
 * Create attribute index based on nested attribute
 * @arg tb		Index array to be filled (maxtype+1 elements).
 * @arg maxtype		Maximum attribute type expected and accepted.
 * @arg nla		Nested Attribute.
 * @arg policy		Attribute validation policy.
 *
 * Feeds the stream of attributes nested into the specified attribute
 * to nla_parse().
 *
 * @see nla_parse
 * @return 0 on success or a negative error code.
 */
static inline int nla_parse_nested(struct nlattr *tb[], int maxtype, struct nlattr *nla,
		     struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, (struct nlattr *)nla_data(nla), nla_len(nla), policy);
}

/**
 * Compare attribute payload with memory area.
 * @arg nla		Attribute.
 * @arg data		Memory area to compare to.
 * @arg size		Number of bytes to compare.
 *
 * @see memcmp(3)
 * @return An integer less than, equal to, or greater than zero.
 */
static inline int nla_memcmp(const struct nlattr *nla, const void *data, size_t size)
{
	int d = nla_len(nla) - size;

	if (d == 0)
		d = memcmp(nla_data(nla), data, size);

	return d;
}

/**
 * Compare string attribute payload with string
 * @arg nla		Attribute of type NLA_STRING.
 * @arg str		NUL terminated string.
 *
 * @see strcmp(3)
 * @return An integer less than, equal to, or greater than zero.
 */
static inline int nla_strcmp(const struct nlattr *nla, const char *str)
{
	int len = strlen(str) + 1;
	int d = nla_len(nla) - len;

	if (d == 0)
		d = memcmp(nla_data(nla), str, len);

	return d;
}

/**
 * Copy string attribute payload to a buffer.
 * @arg dst		Pointer to destination buffer.
 * @arg nla		Attribute of type NLA_STRING.
 * @arg dstsize		Size of destination buffer in bytes.
 *
 * Copies at most dstsize - 1 bytes to the destination buffer.
 * The result is always a valid NUL terminated string. Unlike
 * strlcpy the destination buffer is always padded out.
 *
 * @return The length of string attribute without the terminating NUL.
 */
static inline size_t nla_strlcpy(char *dst, const struct nlattr *nla, size_t dstsize)
{
	size_t srclen = (size_t)nla_len(nla);
	char *src = (char*)nla_data(nla);

	if (srclen > 0 && src[srclen - 1] == '\0')
		srclen--;

	if (dstsize > 0) {
		size_t len = (srclen >= dstsize) ? dstsize - 1 : srclen;

		memset(dst, 0, dstsize);
		memcpy(dst, src, len);
	}

	return srclen;
}


/**
 * @name Attribute Construction (Exception Based)
 * @{
 */

/**
 * @ingroup attr
 * Add unspecific attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg attrlen		Length of attribute payload.
 * @arg data		Head of attribute payload.
 */
#define NLA_PUT(msg, attrtype, attrlen, data) \
	do { \
		if (nla_put(msg, attrtype, attrlen, data) < 0) \
			goto nla_put_failure; \
	} while(0)

/**
 * @ingroup attr
 * Add atomic type attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg type		Atomic type.
 * @arg attrtype	Attribute type.
 * @arg value		Head of attribute payload.
 */
#define NLA_PUT_TYPE(msg, type, attrtype, value) \
	do { \
		type __tmp = value; \
		NLA_PUT(msg, attrtype, sizeof(type), &__tmp); \
	} while(0)

/**
 * Add 8 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U8(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint8_t, attrtype, value)

/**
 * Add 16 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U16(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint16_t, attrtype, value)

/**
 * Add 32 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U32(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint32_t, attrtype, value)

/**
 * Add 64 bit integer attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		Numeric value.
 */
#define NLA_PUT_U64(msg, attrtype, value) \
	NLA_PUT_TYPE(msg, uint64_t, attrtype, value)

/**
 * Add string attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg value		NUL terminated character string.
 */
#define NLA_PUT_STRING(msg, attrtype, value) \
	NLA_PUT(msg, attrtype, strlen(value) + 1, value)

/**
 * Add flag attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 */
#define NLA_PUT_FLAG(msg, attrtype) \
	NLA_PUT(msg, attrtype, 0, NULL)

/**
 * Add msecs attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg msecs		Numeric value in micro seconds.
 */
#define NLA_PUT_MSECS(msg, attrtype, msecs) \
	NLA_PUT_U64(msg, attrtype, msecs)

/**
 * Add address attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg addr		Abstract address object.
 */
#define NLA_PUT_ADDR(msg, attrtype, addr) \
	NLA_PUT(msg, attrtype, nl_addr_get_len(addr), \
		nl_addr_get_binary_addr(addr))

/** @} */

/**
 * @name Iterators
 * @{
 */

/**
 * @ingroup attr
 * Iterate over a stream of attributes
 * @arg pos	loop counter, set to current attribute
 * @arg head	head of attribute stream
 * @arg len	length of attribute stream
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nla_for_each_attr(pos, head, len, rem) \
	for (pos = head, rem = len; \
	     nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

/**
 * @ingroup attr
 * Iterate over a stream of nested attributes
 * @arg pos	loop counter, set to current attribute
 * @arg nla	attribute containing the nested attributes
 * @arg rem	initialized to len, holds bytes currently remaining in stream
 */
#define nla_for_each_nested(pos, nla, rem) \
	for (pos = (struct nlattr *)nla_data(nla), rem = nla_len(nla); \
	     nla_ok(pos, rem); \
	     pos = nla_next(pos, &(rem)))

/** @} */

#ifdef __cplusplus
}
#endif

#endif
