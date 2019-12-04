/*
 * netlink/netlink-types.h	Netlink Types
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __NETLINK_TYPES_H_
#define __NETLINK_TYPES_H_

#include <stdio.h>

/**
 * Dumping types (dp_type)
 * @ingroup utils
 */
enum nl_dump_type {
	NL_DUMP_LINE,		/**< Dump object briefly on one line */
	NL_DUMP_DETAILS,	/**< Dump all attributes but no statistics */
	NL_DUMP_STATS,		/**< Dump all attributes including statistics */
	NL_DUMP_ENV,		/**< Dump all attribtues as env variables */
	__NL_DUMP_MAX,
};
#define NL_DUMP_MAX (__NL_DUMP_MAX - 1)

/**
 * Dumping parameters
 * @ingroup utils
 */
struct nl_dump_params
{
	/**
	 * Specifies the type of dump that is requested.
	 */
	enum nl_dump_type	dp_type;

	/**
	 * Specifies the number of whitespaces to be put in front
	 * of every new line (indentation).
	 */
	int			dp_prefix;

	/**
	 * Causes the cache index to be printed for each element.
	 */
	int			dp_print_index;

	/**
	 * Causes each element to be prefixed with the message type.
	 */
	int			dp_dump_msgtype;

	/**
	 * A callback invoked for output
	 *
	 * Passed arguments are:
	 *  - dumping parameters
	 *  - string to append to the output
	 */
	void			(*dp_cb)(struct nl_dump_params *, char *);

	/**
	 * A callback invoked for every new line, can be used to
	 * customize the indentation.
	 *
	 * Passed arguments are:
	 *  - dumping parameters
	 *  - line number starting from 0
	 */
	void			(*dp_nl_cb)(struct nl_dump_params *, int);

	/**
	 * User data pointer, can be used to pass data to callbacks.
	 */
	void			*dp_data;

	/**
	 * File descriptor the dumping output should go to
	 */
	FILE *			dp_fd;

	/**
	 * Alternatively the output may be redirected into a buffer
	 */
	char *			dp_buf;

	/**
	 * Length of the buffer dp_buf
	 */
	size_t			dp_buflen;

	/**
	 * PRIVATE
	 * Set if a dump was performed prior to the actual dump handler.
	 */
	int			dp_pre_dump;

	/**
	 * PRIVATE
	 * Owned by the current caller
	 */
	int			dp_ivar;

	unsigned int		dp_line;
};

#ifndef __GNUC__
#define __extension__
#endif

#define min_t(type,x,y) \
	__extension__({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	__extension__({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })


#endif
