/*
 * (C) 2008-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 */
#include <limits.h>	/* for INT_MAX */
#include <string.h>
#include <errno.h>

#include "libmnl.h"

/**
 * \defgroup attr Netlink attribute helpers
 *
 * Netlink Type-Length-Value (TLV) attribute:
 * \verbatim
	|<-- 2 bytes -->|<-- 2 bytes -->|<-- variable -->|
	-------------------------------------------------
	|     length    |      type     |      value     |
	-------------------------------------------------
	|<--------- header ------------>|<-- payload --->|
\endverbatim
 * The payload of the Netlink message contains sequences of attributes that are
 * expressed in TLV format.
 *
 * @{
 */

/**
 * mnl_attr_get_type - get type of netlink attribute
 * \param attr pointer to netlink attribute
 *
 * This function returns the attribute type.
 */
uint16_t mnl_attr_get_type(const struct nlattr *attr)
{
	return attr->nla_type & NLA_TYPE_MASK;
}

/**
 * mnl_attr_get_len - get length of netlink attribute
 * \param attr pointer to netlink attribute
 *
 * This function returns the attribute length that is the attribute header
 * plus the attribute payload.
 */
uint16_t mnl_attr_get_len(const struct nlattr *attr)
{
	return attr->nla_len;
}

/**
 * mnl_attr_get_payload_len - get the attribute payload-value length
 * \param attr pointer to netlink attribute
 *
 * This function returns the attribute payload-value length.
 */
uint16_t mnl_attr_get_payload_len(const struct nlattr *attr)
{
	return attr->nla_len - MNL_ATTR_HDRLEN;
}

/**
 * mnl_attr_get_payload - get pointer to the attribute payload
 * \param attr pointer to netlink attribute
 *
 * This function return a pointer to the attribute payload.
 */
void *mnl_attr_get_payload(const struct nlattr *attr)
{
	return (void *)attr + MNL_ATTR_HDRLEN;
}

/**
 * mnl_attr_ok - check if there is room for an attribute in a buffer
 * \param attr attribute that we want to check if there is room for
 * \param len remaining bytes in a buffer that contains the attribute
 *
 * This function is used to check that a buffer, which is supposed to contain
 * an attribute, has enough room for the attribute that it stores, i.e. this
 * function can be used to verify that an attribute is neither malformed nor
 * truncated.
 *
 * This function does not set errno in case of error since it is intended
 * for iterations. Thus, it returns true on success and false on error.
 *
 * The len parameter may be negative in the case of malformed messages during
 * attribute iteration, that is why we use a signed integer.
 */
bool mnl_attr_ok(const struct nlattr *attr, int len)
{
	return len >= (int)sizeof(struct nlattr) &&
	       attr->nla_len >= sizeof(struct nlattr) &&
	       (int)attr->nla_len <= len;
}

/**
 * mnl_attr_next - get the next attribute in the payload of a netlink message
 * \param attr pointer to the current attribute
 *
 * This function returns a pointer to the next attribute after the one passed
 * as parameter. You have to use mnl_attr_ok() to ensure that the next
 * attribute is valid.
 */
struct nlattr *mnl_attr_next(const struct nlattr *attr)
{
	return (struct nlattr *)((void *)attr + MNL_ALIGN(attr->nla_len));
}

/**
 * mnl_attr_type_valid - check if the attribute type is valid
 * \param attr pointer to attribute to be checked
 * \param max maximum attribute type
 *
 * This function allows to check if the attribute type is higher than the
 * maximum supported type. If the attribute type is invalid, this function
 * returns -1 and errno is explicitly set. On success, this function returns 1.
 *
 * Strict attribute checking in user-space is not a good idea since you may
 * run an old application with a newer kernel that supports new attributes.
 * This leads to backward compatibility breakages in user-space. Better check
 * if you support an attribute, if not, skip it.
 */
int mnl_attr_type_valid(const struct nlattr *attr, uint16_t max)
{
	if (mnl_attr_get_type(attr) > max) {
		errno = EOPNOTSUPP;
		return -1;
	}
	return 1;
}

static int __mnl_attr_validate(const struct nlattr *attr,
			       enum mnl_attr_data_type type, size_t exp_len)
{
	uint16_t attr_len = mnl_attr_get_payload_len(attr);
	const char *attr_data = mnl_attr_get_payload(attr);

	if (attr_len < exp_len) {
		errno = ERANGE;
		return -1;
	}
	switch(type) {
	case MNL_TYPE_FLAG:
		if (attr_len > 0) {
			errno = ERANGE;
			return -1;
		}
		break;
	case MNL_TYPE_NUL_STRING:
		if (attr_len == 0) {
			errno = ERANGE;
			return -1;
		}
		if (attr_data[attr_len-1] != '\0') {
			errno = EINVAL;
			return -1;
		}
		break;
	case MNL_TYPE_STRING:
		if (attr_len == 0) {
			errno = ERANGE;
			return -1;
		}
		break;
	case MNL_TYPE_NESTED:
		/* empty nested attributes are OK. */
		if (attr_len == 0)
			break;
		/* if not empty, they must contain one header, eg. flag */
		if (attr_len < MNL_ATTR_HDRLEN) {
			errno = ERANGE;
			return -1;
		}
		break;
	default:
		/* make gcc happy. */
		break;
	}
	if (exp_len && attr_len > exp_len) {
		errno = ERANGE;
		return -1;
	}
	return 0;
}

static const size_t mnl_attr_data_type_len[MNL_TYPE_MAX] = {
	[MNL_TYPE_U8]		= sizeof(uint8_t),
	[MNL_TYPE_U16]		= sizeof(uint16_t),
	[MNL_TYPE_U32]		= sizeof(uint32_t),
	[MNL_TYPE_U64]		= sizeof(uint64_t),
	[MNL_TYPE_MSECS]	= sizeof(uint64_t),
};

/**
 * mnl_attr_validate - validate netlink attribute (simplified version)
 * \param attr pointer to netlink attribute that we want to validate
 * \param type data type (see enum mnl_attr_data_type)
 *
 * The validation is based on the data type. Specifically, it checks that
 * integers (u8, u16, u32 and u64) have enough room for them. This function
 * returns -1 in case of error, and errno is explicitly set.
 */
int mnl_attr_validate(const struct nlattr *attr, enum mnl_attr_data_type type)
{
	int exp_len;

	if (type >= MNL_TYPE_MAX) {
		errno = EINVAL;
		return -1;
	}
	exp_len = mnl_attr_data_type_len[type];
	return __mnl_attr_validate(attr, type, exp_len);
}

/**
 * mnl_attr_validate2 - validate netlink attribute (extended version)
 * \param attr pointer to netlink attribute that we want to validate
 * \param type attribute type (see enum mnl_attr_data_type)
 * \param exp_len expected attribute data size
 *
 * This function allows to perform a more accurate validation for attributes
 * whose size is variable. If the size of the attribute is not what we expect,
 * this functions returns -1 and errno is explicitly set.
 */
int mnl_attr_validate2(const struct nlattr *attr,
				     enum mnl_attr_data_type type,
				     size_t exp_len)
{
	if (type >= MNL_TYPE_MAX) {
		errno = EINVAL;
		return -1;
	}
	return __mnl_attr_validate(attr, type, exp_len);
}

/**
 * mnl_attr_parse - parse attributes
 * \param nlh pointer to netlink message
 * \param offset offset to start parsing from (if payload is after any header)
 * \param cb callback function that is called for each attribute
 * \param data pointer to data that is passed to the callback function
 *
 * This function allows to iterate over the sequence of attributes that compose
 * the Netlink message. You can then put the attribute in an array as it
 * usually happens at this stage or you can use any other data structure (such
 * as lists or trees).
 *
 * This function propagates the return value of the callback, which can be
 * MNL_CB_ERROR, MNL_CB_OK or MNL_CB_STOP.
 */
int mnl_attr_parse(const struct nlmsghdr *nlh,
				 unsigned int offset, mnl_attr_cb_t cb,
				 void *data)
{
	int ret = MNL_CB_OK;
	const struct nlattr *attr;

	mnl_attr_for_each(attr, nlh, offset)
		if ((ret = cb(attr, data)) <= MNL_CB_STOP)
			return ret;
	return ret;
}

/**
 * mnl_attr_parse_nested - parse attributes inside a nest
 * \param nested pointer to netlink attribute that contains a nest
 * \param cb callback function that is called for each attribute in the nest
 * \param data pointer to data passed to the callback function
 *
 * This function allows to iterate over the sequence of attributes that compose
 * the Netlink message. You can then put the attribute in an array as it
 * usually happens at this stage or you can use any other data structure (such
 * as lists or trees).
 *
 * This function propagates the return value of the callback, which can be
 * MNL_CB_ERROR, MNL_CB_OK or MNL_CB_STOP.
 */
int mnl_attr_parse_nested(const struct nlattr *nested,
					mnl_attr_cb_t cb, void *data)
{
	int ret = MNL_CB_OK;
	const struct nlattr *attr;

	mnl_attr_for_each_nested(attr, nested)
		if ((ret = cb(attr, data)) <= MNL_CB_STOP)
			return ret;
	return ret;
}

/**
 * mnl_attr_parse_payload - parse attributes in payload of Netlink message
 * \param payload pointer to payload of the Netlink message
 * \param payload_len payload length that contains the attributes
 * \param cb callback function that is called for each attribute
 * \param data pointer to data that is passed to the callback function
 *
 * This function takes a pointer to the area that contains the attributes,
 * commonly known as the payload of the Netlink message. Thus, you have to
 * pass a pointer to the Netlink message payload, instead of the entire
 * message.
 *
 * This function allows you to iterate over the sequence of attributes that are
 * located at some payload offset. You can then put the attributes in one array
 * as usual, or you can use any other data structure (such as lists or trees).
 *
 * This function propagates the return value of the callback, which can be
 * MNL_CB_ERROR, MNL_CB_OK or MNL_CB_STOP.
 */
int mnl_attr_parse_payload(const void *payload,
					 size_t payload_len,
					 mnl_attr_cb_t cb, void *data)
{
	int ret = MNL_CB_OK;
	const struct nlattr *attr;

	mnl_attr_for_each_payload(payload, payload_len)
		if ((ret = cb(attr, data)) <= MNL_CB_STOP)
			return ret;
	return ret;
}

/**
 * mnl_attr_get_u8 - returns 8-bit unsigned integer attribute payload
 * \param attr pointer to netlink attribute
 *
 * This function returns the 8-bit value of the attribute payload.
 */
uint8_t mnl_attr_get_u8(const struct nlattr *attr)
{
	return *((uint8_t *)mnl_attr_get_payload(attr));
}

/**
 * mnl_attr_get_u16 - returns 16-bit unsigned integer attribute payload
 * \param attr pointer to netlink attribute
 *
 * This function returns the 16-bit value of the attribute payload.
 */
uint16_t mnl_attr_get_u16(const struct nlattr *attr)
{
	return *((uint16_t *)mnl_attr_get_payload(attr));
}

/**
 * mnl_attr_get_u32 - returns 32-bit unsigned integer attribute payload
 * \param attr pointer to netlink attribute
 *
 * This function returns the 32-bit value of the attribute payload.
 */
uint32_t mnl_attr_get_u32(const struct nlattr *attr)
{
	return *((uint32_t *)mnl_attr_get_payload(attr));
}

/**
 * mnl_attr_get_u64 - returns 64-bit unsigned integer attribute.
 * \param attr pointer to netlink attribute
 *
 * This function returns the 64-bit value of the attribute payload. This
 * function is align-safe, since accessing 64-bit Netlink attributes is a
 * common source of alignment issues.
 */
uint64_t mnl_attr_get_u64(const struct nlattr *attr)
{
	uint64_t tmp;
	memcpy(&tmp, mnl_attr_get_payload(attr), sizeof(tmp));
	return tmp;
}

/**
 * mnl_attr_get_str - returns pointer to string attribute.
 * \param attr pointer to netlink attribute
 *
 * This function returns the payload of string attribute value.
 */
const char *mnl_attr_get_str(const struct nlattr *attr)
{
	return mnl_attr_get_payload(attr);
}

/**
 * mnl_attr_put - add an attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type that you want to add
 * \param len netlink attribute payload length
 * \param data pointer to the data that will be stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put(struct nlmsghdr *nlh, uint16_t type,
				size_t len, const void *data)
{
	struct nlattr *attr = mnl_nlmsg_get_payload_tail(nlh);
	uint16_t payload_len = MNL_ALIGN(sizeof(struct nlattr)) + len;
	int pad;

	attr->nla_type = type;
	attr->nla_len = payload_len;
	memcpy(mnl_attr_get_payload(attr), data, len);
	pad = MNL_ALIGN(len) - len;
	if (pad > 0)
		memset(mnl_attr_get_payload(attr) + len, 0, pad);

	nlh->nlmsg_len += MNL_ALIGN(payload_len);
}

/**
 * mnl_attr_put_u8 - add 8-bit unsigned integer attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 * \param data 8-bit unsigned integer data that is stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put_u8(struct nlmsghdr *nlh, uint16_t type,
				   uint8_t data)
{
	mnl_attr_put(nlh, type, sizeof(uint8_t), &data);
}

/**
 * mnl_attr_put_u16 - add 16-bit unsigned integer attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 * \param data 16-bit unsigned integer data that is stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put_u16(struct nlmsghdr *nlh, uint16_t type,
				    uint16_t data)
{
	mnl_attr_put(nlh, type, sizeof(uint16_t), &data);
}

/**
 * mnl_attr_put_u32 - add 32-bit unsigned integer attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 * \param data 32-bit unsigned integer data that is stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put_u32(struct nlmsghdr *nlh, uint16_t type,
				    uint32_t data)
{
	mnl_attr_put(nlh, type, sizeof(uint32_t), &data);
}

/**
 * mnl_attr_put_u64 - add 64-bit unsigned integer attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 * \param data 64-bit unsigned integer data that is stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put_u64(struct nlmsghdr *nlh, uint16_t type,
				    uint64_t data)
{
	mnl_attr_put(nlh, type, sizeof(uint64_t), &data);
}

/**
 * mnl_attr_put_str - add string attribute to netlink message
 * \param nlh  pointer to the netlink message
 * \param type netlink attribute type
 * \param data pointer to string data that is stored by the new attribute
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put_str(struct nlmsghdr *nlh, uint16_t type,
				    const char *data)
{
	mnl_attr_put(nlh, type, strlen(data), data);
}

/**
 * mnl_attr_put_strz - add string attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 * \param data pointer to string data that is stored by the new attribute
 *
 * This function is similar to mnl_attr_put_str, but it includes the
 * NUL/zero ('\0') terminator at the end of the string.
 *
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
void mnl_attr_put_strz(struct nlmsghdr *nlh, uint16_t type,
				     const char *data)
{
	mnl_attr_put(nlh, type, strlen(data)+1, data);
}

/**
 * mnl_attr_nest_start - start an attribute nest
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 *
 * This function adds the attribute header that identifies the beginning of
 * an attribute nest. This function always returns a valid pointer to the
 * beginning of the nest.
 */
struct nlattr *mnl_attr_nest_start(struct nlmsghdr *nlh,
						 uint16_t type)
{
	struct nlattr *start = mnl_nlmsg_get_payload_tail(nlh);

	/* set start->nla_len in mnl_attr_nest_end() */
	start->nla_type = NLA_F_NESTED | type;
	nlh->nlmsg_len += MNL_ALIGN(sizeof(struct nlattr));

	return start;
}

/**
 * mnl_attr_put_check - add an attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type that you want to add
 * \param len netlink attribute payload length
 * \param data pointer to the data that will be stored by the new attribute
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 */
bool mnl_attr_put_check(struct nlmsghdr *nlh, size_t buflen,
				      uint16_t type, size_t len,
				      const void *data)
{
	if (nlh->nlmsg_len + MNL_ATTR_HDRLEN + MNL_ALIGN(len) > buflen)
		return false;
	mnl_attr_put(nlh, type, len, data);
	return true;
}

/**
 * mnl_attr_put_u8_check - add 8-bit unsigned int attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type
 * \param data 8-bit unsigned integer data that is stored by the new attribute
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 */
bool mnl_attr_put_u8_check(struct nlmsghdr *nlh, size_t buflen,
					 uint16_t type, uint8_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint8_t), &data);
}

/**
 * mnl_attr_put_u16_check - add 16-bit unsigned int attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type
 * \param data 16-bit unsigned integer data that is stored by the new attribute
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
bool mnl_attr_put_u16_check(struct nlmsghdr *nlh, size_t buflen,
					  uint16_t type, uint16_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint16_t), &data);
}

/**
 * mnl_attr_put_u32_check - add 32-bit unsigned int attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type
 * \param data 32-bit unsigned integer data that is stored by the new attribute
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
bool mnl_attr_put_u32_check(struct nlmsghdr *nlh, size_t buflen,
					  uint16_t type, uint32_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint32_t), &data);
}

/**
 * mnl_attr_put_u64_check - add 64-bit unsigned int attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type
 * \param data 64-bit unsigned integer data that is stored by the new attribute
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
bool mnl_attr_put_u64_check(struct nlmsghdr *nlh, size_t buflen,
					  uint16_t type, uint64_t data)
{
	return mnl_attr_put_check(nlh, buflen, type, sizeof(uint64_t), &data);
}

/**
 * mnl_attr_put_str_check - add string attribute to netlink message
 * \param nlh  pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type
 * \param data pointer to string data that is stored by the new attribute
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 * This function updates the length field of the Netlink message (nlmsg_len)
 * by adding the size (header + payload) of the new attribute.
 */
bool mnl_attr_put_str_check(struct nlmsghdr *nlh, size_t buflen,
					  uint16_t type, const char *data)
{
	return mnl_attr_put_check(nlh, buflen, type, strlen(data), data);
}

/**
 * mnl_attr_put_strz_check - add string attribute to netlink message
 * \param nlh pointer to the netlink message
 * \param buflen size of buffer which stores the message
 * \param type netlink attribute type
 * \param data pointer to string data that is stored by the new attribute
 *
 * This function is similar to mnl_attr_put_str, but it includes the
 * NUL/zero ('\0') terminator at the end of the string.
 *
 * This function first checks that the data can be added to the message
 * (fits into the buffer) and then updates the length field of the Netlink
 * message (nlmsg_len) by adding the size (header + payload) of the new
 * attribute. The function returns true if the attribute could be added
 * to the message, otherwise false is returned.
 */
bool mnl_attr_put_strz_check(struct nlmsghdr *nlh, size_t buflen,
					   uint16_t type, const char *data)
{
	return mnl_attr_put_check(nlh, buflen, type, strlen(data)+1, data);
}

/**
 * mnl_attr_nest_start_check - start an attribute nest
 * \param buflen size of buffer which stores the message
 * \param nlh pointer to the netlink message
 * \param type netlink attribute type
 *
 * This function adds the attribute header that identifies the beginning of
 * an attribute nest. If the nested attribute cannot be added then NULL,
 * otherwise valid pointer to the beginning of the nest is returned.
 */
struct nlattr *mnl_attr_nest_start_check(struct nlmsghdr *nlh,
						       size_t buflen,
						       uint16_t type)
{
	if (nlh->nlmsg_len + MNL_ATTR_HDRLEN > buflen)
		return NULL;
	return mnl_attr_nest_start(nlh, type);
}

/**
 * mnl_attr_nest_end - end an attribute nest
 * \param nlh pointer to the netlink message
 * \param start pointer to the attribute nest returned by mnl_attr_nest_start()
 *
 * This function updates the attribute header that identifies the nest.
 */
void mnl_attr_nest_end(struct nlmsghdr *nlh,
				     struct nlattr *start)
{
	start->nla_len = mnl_nlmsg_get_payload_tail(nlh) - (void *)start;
}

/**
 * mnl_attr_nest_cancel - cancel an attribute nest
 * \param nlh pointer to the netlink message
 * \param start pointer to the attribute nest returned by mnl_attr_nest_start()
 *
 * This function updates the attribute header that identifies the nest.
 */
void mnl_attr_nest_cancel(struct nlmsghdr *nlh,
					struct nlattr *start)
{
	nlh->nlmsg_len -= mnl_nlmsg_get_payload_tail(nlh) - (void *)start;
}

/**
 * @}
 */
