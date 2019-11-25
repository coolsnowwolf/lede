/*
 * lib/attr.c		Netlink Attributes
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/addr.h>
#include <netlink/attr.h>
#include <netlink/msg.h>
#include <linux/socket.h>

/**
 * @ingroup msg
 * @defgroup attr Attributes
 * Netlink Attributes Construction/Parsing Interface
 *
 * \section attr_sec Netlink Attributes
 * Netlink attributes allow for data chunks of arbitary length to be
 * attached to a netlink message. Each attribute is encoded with a
 * type and length field, both 16 bits, stored in the attribute header
 * preceding the attribute data. The main advantage of using attributes
 * over packing everything into the family header is that the interface
 * stays extendable as new attributes can supersede old attributes while
 * remaining backwards compatible. Also attributes can be defined optional
 * thus avoiding the transmission of unnecessary empty data blocks.
 * Special nested attributes allow for more complex data structures to
 * be transmitted, e.g. trees, lists, etc.
 *
 * While not required, netlink attributes typically follow the family
 * header of a netlink message and must be properly aligned to NLA_ALIGNTO:
 * @code
 *   +----------------+- - -+---------------+- - -+------------+- - -+
 *   | Netlink Header | Pad | Family Header | Pad | Attributes | Pad |
 *   +----------------+- - -+---------------+- - -+------------+- - -+
 * @endcode
 *
 * The actual attributes are chained together each separately aligned to
 * NLA_ALIGNTO. The position of an attribute is defined based on the
 * length field of the preceding attributes:
 * @code
 *   +-------------+- - -+-------------+- - -+------
 *   | Attribute 1 | Pad | Attribute 2 | Pad | ...
 *   +-------------+- - -+-------------+- - -+------
 *   nla_next(attr1)------^
 * @endcode
 *
 * The attribute itself consists of the attribute header followed by
 * the actual payload also aligned to NLA_ALIGNTO. The function nla_data()
 * returns a pointer to the start of the payload while nla_len() returns
 * the length of the payload in bytes.
 *
 * \b Note: Be aware, NLA_ALIGNTO equals to 4 bytes, therefore it is not
 * safe to dereference any 64 bit data types directly.
 *
 * @code
 *    <----------- nla_total_size(payload) ----------->
 *    <-------- nla_attr_size(payload) --------->
 *   +------------------+- - -+- - - - - - - - - +- - -+
 *   | Attribute Header | Pad |     Payload      | Pad |
 *   +------------------+- - -+- - - - - - - - - +- - -+
 *   nla_data(nla)-------------^
 *                             <- nla_len(nla) ->
 * @endcode
 *
 * @subsection attr_datatypes Attribute Data Types
 * A number of basic data types are supported to simplify access and
 * validation of netlink attributes. This data type information is
 * not encoded in the attribute, both the kernel and userspace part
 * are required to share this information on their own.
 *
 * One of the major advantages of these basic types is the automatic
 * validation of each attribute based on an attribute policy. The
 * validation covers most of the checks required to safely use
 * attributes and thus keeps the individual sanity check to a minimum.
 *
 * Never access attribute payload without ensuring basic validation
 * first, attributes may:
 * - not be present even though required
 * - contain less actual payload than expected
 * - fake a attribute length which exceeds the end of the message
 * - contain unterminated character strings
 *
 * Policies are defined as array of the struct nla_policy. The array is
 * indexed with the attribute type, therefore the array must be sized
 * accordingly.
 * @code
 * static struct nla_policy my_policy[ATTR_MAX+1] = {
 * 	[ATTR_FOO] = { .type = ..., .minlen = ..., .maxlen = ... },
 * };
 *
 * err = nla_validate(attrs, attrlen, ATTR_MAX, &my_policy);
 * @endcode
 *
 * Some basic validations are performed on every attribute, regardless of type.
 * - If the attribute type exceeds the maximum attribute type specified or
 *   the attribute type is lesser-or-equal than zero, the attribute will
 *   be silently ignored.
 * - If the payload length falls below the \a minlen value the attribute
 *   will be rejected.
 * - If \a maxlen is non-zero and the payload length exceeds the \a maxlen
 *   value the attribute will be rejected.
 *
 *
 * @par Unspecific Attribute (NLA_UNSPEC)
 * This is the standard type if no type is specified. It is used for
 * binary data of arbitary length. Typically this attribute carries
 * a binary structure or a stream of bytes.
 * @par
 * @code
 * // In this example, we will assume a binary structure requires to
 * // be transmitted. The definition of the structure will typically
 * // go into a header file available to both the kernel and userspace
 * // side.
 * //
 * // Note: Be careful when putting 64 bit data types into a structure.
 * // The attribute payload is only aligned to 4 bytes, dereferencing
 * // the member may fail.
 * struct my_struct {
 *     int a;
 *     int b;
 * };
 *
 * // The validation function will not enforce an exact length match to
 * // allow structures to grow as required. Note: While it is allowed
 * // to add members to the end of the structure, changing the order or
 * // inserting members in the middle of the structure will break your
 * // binary interface.
 * static struct nla_policy my_policy[ATTR_MAX+1] = {
 *     [ATTR_MY_STRICT] = { .type = NLA_UNSPEC,
 *                          .minlen = sizeof(struct my_struct) },
 *
 * // The binary structure is appened to the message using nla_put()
 * struct my_struct foo = { .a = 1, .b = 2 };
 * nla_put(msg, ATTR_MY_STRUCT, sizeof(foo), &foo);
 *
 * // On the receiving side, a pointer to the structure pointing inside
 * // the message payload is returned by nla_get().
 * if (attrs[ATTR_MY_STRUCT])
 *     struct my_struct *foo = nla_get(attrs[ATTR_MY_STRUCT]);
 * @endcode
 *
 * @par Integers (NLA_U8, NLA_U16, NLA_U32, NLA_U64)
 * Integers come in different sizes from 8 bit to 64 bit. However, since the
 * payload length is aligned to 4 bytes, integers smaller than 32 bit are
 * only useful to enforce the maximum range of values.
 * @par
 * \b Note: There is no difference made between signed and unsigned integers.
 * The validation only enforces the minimal payload length required to store
 * an integer of specified type.
 * @par
 * @code
 * // Even though possible, it does not make sense to specify .minlen or
 * // .maxlen for integer types. The data types implies the corresponding
 * // minimal payload length.
 * static struct nla_policy my_policy[ATTR_MAX+1] = {
 *     [ATTR_FOO] = { .type = NLA_U32 },
 *
 * // Numeric values can be appended directly using the respective
 * // nla_put_uxxx() function
 * nla_put_u32(msg, ATTR_FOO, 123);
 *
 * // Same for the receiving side.
 * if (attrs[ATTR_FOO])
 *     uint32_t foo = nla_get_u32(attrs[ATTR_FOO]);
 * @endcode
 *
 * @par Character string (NLA_STRING)
 * This data type represents a NUL terminated character string of variable
 * length. For binary data streams the type NLA_UNSPEC is recommended.
 * @par
 * @code
 * // Enforce a NUL terminated character string of at most 4 characters
 * // including the NUL termination.
 * static struct nla_policy my_policy[ATTR_MAX+1] = {
 *     [ATTR_BAR] = { .type = NLA_STRING, maxlen = 4 },
 *
 * // nla_put_string() creates a string attribute of the necessary length
 * // and appends it to the message including the NUL termination.
 * nla_put_string(msg, ATTR_BAR, "some text");
 *
 * // It is safe to use the returned character string directly if the
 * // attribute has been validated as the validation enforces the proper
 * // termination of the string.
 * if (attrs[ATTR_BAR])
 *     char *text = nla_get_string(attrs[ATTR_BAR]);
 * @endcode
 *
 * @par Flag (NLA_FLAG)
 * This attribute type may be used to indicate the presence of a flag. The
 * attribute is only valid if the payload length is zero. The presence of
 * the attribute header indicates the presence of the flag.
 * @par
 * @code
 * // This attribute type is special as .minlen and .maxlen have no effect.
 * static struct nla_policy my_policy[ATTR_MAX+1] = {
 *     [ATTR_FLAG] = { .type = NLA_FLAG },
 *
 * // nla_put_flag() appends a zero sized attribute to the message.
 * nla_put_flag(msg, ATTR_FLAG);
 *
 * // There is no need for a receival function, the presence is the value.
 * if (attrs[ATTR_FLAG])
 *     // flag is present
 * @endcode
 *
 * @par Micro Seconds (NLA_MSECS)
 *
 * @par Nested Attribute (NLA_NESTED)
 * Attributes can be nested and put into a container to create groups, lists
 * or to construct trees of attributes. Nested attributes are often used to
 * pass attributes to a subsystem where the top layer has no knowledge of the
 * configuration possibilities of each subsystem.
 * @par
 * \b Note: When validating the attributes using nlmsg_validate() or
 * nlmsg_parse() it will only affect the top level attributes. Each
 * level of nested attributes must be validated seperately using
 * nla_parse_nested() or nla_validate().
 * @par
 * @code
 * // The minimal length policy may be used to enforce the presence of at
 * // least one attribute.
 * static struct nla_policy my_policy[ATTR_MAX+1] = {
 *     [ATTR_OPTS] = { .type = NLA_NESTED, minlen = NLA_HDRLEN },
 *
 * // Nested attributes are constructed by enclosing the attributes
 * // to be nested with calls to nla_nest_start() respetively nla_nest_end().
 * struct nlattr *opts = nla_nest_start(msg, ATTR_OPTS);
 * nla_put_u32(msg, ATTR_FOO, 123);
 * nla_put_string(msg, ATTR_BAR, "some text");
 * nla_nest_end(msg, opts);
 *
 * // Various methods exist to parse nested attributes, the easiest being
 * // nla_parse_nested() which also allows validation in the same step.
 * if (attrs[ATTR_OPTS]) {
 *     struct nlattr *nested[ATTR_MAX+1];
 *
 *     nla_parse_nested(nested, ATTR_MAX, attrs[ATTR_OPTS], &policy);
 *
 *     if (nested[ATTR_FOO])
 *         uint32_t foo = nla_get_u32(nested[ATTR_FOO]);
 * }
 * @endcode
 *
 * @subsection attr_exceptions Exception Based Attribute Construction
 * Often a large number of attributes are added to a message in a single
 * function. In order to simplify error handling, a second set of
 * construction functions exist which jump to a error label when they
 * fail instead of returning an error code. This second set consists
 * of macros which are named after their error code based counterpart
 * except that the name is written all uppercase.
 *
 * All of the macros jump to the target \c nla_put_failure if they fail.
 * @code
 * void my_func(struct nl_msg *msg)
 * {
 *     NLA_PUT_U32(msg, ATTR_FOO, 10);
 *     NLA_PUT_STRING(msg, ATTR_BAR, "bar");
 *
 *     return 0;
 *
 * nla_put_failure:
 *     return -NLE_NOMEM;
 * }
 * @endcode
 *
 * @subsection attr_examples Examples
 * @par Example 1.1 Constructing a netlink message with attributes.
 * @code
 * struct nl_msg *build_msg(int ifindex, struct nl_addr *lladdr, int mtu)
 * {
 *     struct nl_msg *msg;
 *     struct nlattr *info, *vlan;
 *     struct ifinfomsg ifi = {
 *         .ifi_family = AF_INET,
 *         .ifi_index = ifindex,
 *     };
 *
 *     // Allocate a new netlink message, type=RTM_SETLINK, flags=NLM_F_ECHO
 *     if (!(msg = nlmsg_alloc_simple(RTM_SETLINK, NLM_F_ECHO)))
 *         return NULL;
 *
 *     // Append the family specific header (struct ifinfomsg)
 *     if (nlmsg_append(msg, &ifi, sizeof(ifi), NLMSG_ALIGNTO) < 0)
 *         goto nla_put_failure
 *
 *     // Append a 32 bit integer attribute to carry the MTU
 *     NLA_PUT_U32(msg, IFLA_MTU, mtu);
 *
 *     // Append a unspecific attribute to carry the link layer address
 *     NLA_PUT_ADDR(msg, IFLA_ADDRESS, lladdr);
 *
 *     // Append a container for nested attributes to carry link information
 *     if (!(info = nla_nest_start(msg, IFLA_LINKINFO)))
 *         goto nla_put_failure;
 *
 *     // Put a string attribute into the container
 *     NLA_PUT_STRING(msg, IFLA_INFO_KIND, "vlan");
 *
 *     // Append another container inside the open container to carry
 *     // vlan specific attributes
 *     if (!(vlan = nla_nest_start(msg, IFLA_INFO_DATA)))
 *         goto nla_put_failure;
 *
 *     // add vlan specific info attributes here...
 *
 *     // Finish nesting the vlan attributes and close the second container.
 *     nla_nest_end(msg, vlan);
 *
 *     // Finish nesting the link info attribute and close the first container.
 *     nla_nest_end(msg, info);
 *
 *     return msg;
 *
 * // If any of the construction macros fails, we end up here.
 * nla_put_failure:
 *     nlmsg_free(msg);
 *     return NULL;
 * }
 * @endcode
 *
 * @par Example 2.1 Parsing a netlink message with attributes.
 * @code
 * int parse_message(struct nl_msg *msg)
 * {
 *     // The policy defines two attributes: a 32 bit integer and a container
 *     // for nested attributes.
 *     struct nla_policy attr_policy[ATTR_MAX+1] = {
 *         [ATTR_FOO] = { .type = NLA_U32 },
 *         [ATTR_BAR] = { .type = NLA_NESTED },
 *     };
 *     struct nlattr *attrs[ATTR_MAX+1];
 *     int err;
 *
 *     // The nlmsg_parse() function will make sure that the message contains
 *     // enough payload to hold the header (struct my_hdr), validates any
 *     // attributes attached to the messages and stores a pointer to each
 *     // attribute in the attrs[] array accessable by attribute type.
 *     if ((err = nlmsg_parse(nlmsg_hdr(msg), sizeof(struct my_hdr), attrs,
 *                            ATTR_MAX, attr_policy)) < 0)
 *         goto errout;
 *
 *     if (attrs[ATTR_FOO]) {
 *         // It is safe to directly access the attribute payload without
 *         // any further checks since nlmsg_parse() enforced the policy.
 *         uint32_t foo = nla_get_u32(attrs[ATTR_FOO]);
 *     }
 *
 *     if (attrs[ATTR_BAR]) {
 *         struct nlattr *nested[NESTED_MAX+1];
 *
 *         // Attributes nested in a container can be parsed the same way
 *         // as top level attributes.
 *         if ((err = nla_parse_nested(nested, NESTED_MAX, attrs[ATTR_BAR],
 *                                     nested_policy)) < 0)
 *             goto errout;
 *
 *         // Process nested attributes here.
 *     }
 *
 *     err = 0;
 * errout:
 *     return err;
 * }
 * @endcode
 *
 * @{
 */

/**
 * @name Attribute Size Calculation
 * @{
 */

/** @} */

/**
 * @name Parsing Attributes
 * @{
 */

/**
 * Check if the attribute header and payload can be accessed safely.
 * @arg nla		Attribute of any kind.
 * @arg remaining	Number of bytes remaining in attribute stream.
 *
 * Verifies that the header and payload do not exceed the number of
 * bytes left in the attribute stream. This function must be called
 * before access the attribute header or payload when iterating over
 * the attribute stream using nla_next().
 *
 * @return True if the attribute can be accessed safely, false otherwise.
 */
int nla_ok(const struct nlattr *nla, int remaining)
{
	return remaining >= sizeof(*nla) &&
	       nla->nla_len >= sizeof(*nla) &&
	       nla->nla_len <= remaining;
}

/**
 * Return next attribute in a stream of attributes.
 * @arg nla		Attribute of any kind.
 * @arg remaining	Variable to count remaining bytes in stream.
 *
 * Calculates the offset to the next attribute based on the attribute
 * given. The attribute provided is assumed to be accessible, the
 * caller is responsible to use nla_ok() beforehand. The offset (length
 * of specified attribute including padding) is then subtracted from
 * the remaining bytes variable and a pointer to the next attribute is
 * returned.
 *
 * nla_next() can be called as long as remainig is >0.
 *
 * @return Pointer to next attribute.
 */
struct nlattr *nla_next(const struct nlattr *nla, int *remaining)
{
	int totlen = NLA_ALIGN(nla->nla_len);

	*remaining -= totlen;
	return (struct nlattr *) ((char *) nla + totlen);
}

static uint16_t nla_attr_minlen[NLA_TYPE_MAX+1] = {
	[NLA_U8]	= sizeof(uint8_t),
	[NLA_U16]	= sizeof(uint16_t),
	[NLA_U32]	= sizeof(uint32_t),
	[NLA_U64]	= sizeof(uint64_t),
	[NLA_STRING]	= 1,
};

static int validate_nla(struct nlattr *nla, int maxtype,
			struct nla_policy *policy)
{
	struct nla_policy *pt;
	int minlen = 0, type = nla_type(nla);

	if (type <= 0 || type > maxtype)
		return 0;

	pt = &policy[type];

	if (pt->type > NLA_TYPE_MAX)
		BUG();

	if (pt->minlen)
		minlen = pt->minlen;
	else if (pt->type != NLA_UNSPEC)
		minlen = nla_attr_minlen[pt->type];

	if (pt->type == NLA_FLAG && nla_len(nla) > 0)
		return -NLE_RANGE;

	if (nla_len(nla) < minlen)
		return -NLE_RANGE;

	if (pt->maxlen && nla_len(nla) > pt->maxlen)
		return -NLE_RANGE;

	if (pt->type == NLA_STRING) {
		char *data = nla_data(nla);
		if (data[nla_len(nla) - 1] != '\0')
			return -NLE_INVAL;
	}

	return 0;
}


/**
 * Create attribute index based on a stream of attributes.
 * @arg tb		Index array to be filled (maxtype+1 elements).
 * @arg maxtype		Maximum attribute type expected and accepted.
 * @arg head		Head of attribute stream.
 * @arg len		Length of attribute stream.
 * @arg policy		Attribute validation policy.
 *
 * Iterates over the stream of attributes and stores a pointer to each
 * attribute in the index array using the attribute type as index to
 * the array. Attribute with a type greater than the maximum type
 * specified will be silently ignored in order to maintain backwards
 * compatibility. If \a policy is not NULL, the attribute will be
 * validated using the specified policy.
 *
 * @see nla_validate
 * @return 0 on success or a negative error code.
 */
int nla_parse(struct nlattr *tb[], int maxtype, struct nlattr *head, int len,
	      struct nla_policy *policy)
{
	struct nlattr *nla;
	int rem, err;

	memset(tb, 0, sizeof(struct nlattr *) * (maxtype + 1));

	nla_for_each_attr(nla, head, len, rem) {
		int type = nla_type(nla);

		if (type == 0) {
			fprintf(stderr, "Illegal nla->nla_type == 0\n");
			continue;
		}

		if (type <= maxtype) {
			if (policy) {
				err = validate_nla(nla, maxtype, policy);
				if (err < 0)
					goto errout;
			}

			tb[type] = nla;
		}
	}

	if (rem > 0)
		fprintf(stderr, "netlink: %d bytes leftover after parsing "
		       "attributes.\n", rem);

	err = 0;
errout:
	return err;
}

/**
 * Validate a stream of attributes.
 * @arg head		Head of attributes stream.
 * @arg len		Length of attributes stream.
 * @arg maxtype		Maximum attribute type expected and accepted.
 * @arg policy		Validation policy.
 *
 * Iterates over the stream of attributes and validates each attribute
 * one by one using the specified policy. Attributes with a type greater
 * than the maximum type specified will be silently ignored in order to
 * maintain backwards compatibility.
 *
 * See \ref attr_datatypes for more details on what kind of validation
 * checks are performed on each attribute data type.
 *
 * @return 0 on success or a negative error code.
 */
int nla_validate(struct nlattr *head, int len, int maxtype,
		 struct nla_policy *policy)
{
	struct nlattr *nla;
	int rem, err;

	nla_for_each_attr(nla, head, len, rem) {
		err = validate_nla(nla, maxtype, policy);
		if (err < 0)
			goto errout;
	}

	err = 0;
errout:
	return err;
}

/**
 * Find a single attribute in a stream of attributes.
 * @arg head		Head of attributes stream.
 * @arg len		Length of attributes stream.
 * @arg attrtype	Attribute type to look for.
 *
 * Iterates over the stream of attributes and compares each type with
 * the type specified. Returns the first attribute which matches the
 * type.
 *
 * @return Pointer to attribute found or NULL.
 */
struct nlattr *nla_find(struct nlattr *head, int len, int attrtype)
{
	struct nlattr *nla;
	int rem;

	nla_for_each_attr(nla, head, len, rem)
		if (nla_type(nla) == attrtype)
			return nla;

	return NULL;
}

/** @} */

/**
 * @name Unspecific Attribute
 * @{
 */

/**
 * Reserve space for a attribute.
 * @arg msg		Netlink Message.
 * @arg attrtype	Attribute Type.
 * @arg attrlen		Length of payload.
 *
 * Reserves room for a attribute in the specified netlink message and
 * fills in the attribute header (type, length). Returns NULL if there
 * is unsuficient space for the attribute.
 *
 * Any padding between payload and the start of the next attribute is
 * zeroed out.
 *
 * @return Pointer to start of attribute or NULL on failure.
 */
struct nlattr *nla_reserve(struct nl_msg *msg, int attrtype, int attrlen)
{
	struct nlattr *nla;
	int tlen;
	
	tlen = NLMSG_ALIGN(msg->nm_nlh->nlmsg_len) + nla_total_size(attrlen);

	if ((tlen + msg->nm_nlh->nlmsg_len) > msg->nm_size)
		return NULL;

	nla = (struct nlattr *) nlmsg_tail(msg->nm_nlh);
	nla->nla_type = attrtype;
	nla->nla_len = nla_attr_size(attrlen);

	memset((unsigned char *) nla + nla->nla_len, 0, nla_padlen(attrlen));
	msg->nm_nlh->nlmsg_len = tlen;

	NL_DBG(2, "msg %p: Reserved %d bytes at offset +%td for attr %d "
		  "nlmsg_len=%d\n", msg, attrlen,
		  (void *) nla - nlmsg_data(msg->nm_nlh),
		  attrtype, msg->nm_nlh->nlmsg_len);

	return nla;
}

/**
 * Add a unspecific attribute to netlink message.
 * @arg msg		Netlink message.
 * @arg attrtype	Attribute type.
 * @arg datalen		Length of data to be used as payload.
 * @arg data		Pointer to data to be used as attribute payload.
 *
 * Reserves room for a unspecific attribute and copies the provided data
 * into the message as payload of the attribute. Returns an error if there
 * is insufficient space for the attribute.
 *
 * @see nla_reserve
 * @return 0 on success or a negative error code.
 */
int nla_put(struct nl_msg *msg, int attrtype, int datalen, const void *data)
{
	struct nlattr *nla;

	nla = nla_reserve(msg, attrtype, datalen);
	if (!nla)
		return -NLE_NOMEM;

	memcpy(nla_data(nla), data, datalen);
	NL_DBG(2, "msg %p: Wrote %d bytes at offset +%td for attr %d\n",
	       msg, datalen, (void *) nla - nlmsg_data(msg->nm_nlh), attrtype);

	return 0;
}



/** @} */
