/*
 * netlink/utils.h		Utility Functions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_UTILS_H_
#define NETLINK_UTILS_H_

#include <netlink/netlink.h>
#include <netlink/list.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Probability Constants
 * @{
 */

/**
 * Lower probability limit
 * @ingroup utils
 */
#define NL_PROB_MIN 0x0

/**
 * Upper probability limit
 * @ingroup utils
 */
#define NL_PROB_MAX 0xffffffff

/** @} */

/* unit pretty-printing */
extern double	nl_cancel_down_bytes(unsigned long long, char **);
extern double	nl_cancel_down_bits(unsigned long long, char **);
extern double	nl_cancel_down_us(uint32_t, char **);

/* generic unit translations */
extern long	nl_size2int(const char *);
extern long	nl_prob2int(const char *);

/* time translations */
extern int	nl_get_hz(void);
extern uint32_t	nl_us2ticks(uint32_t);
extern uint32_t	nl_ticks2us(uint32_t);
extern int	nl_str2msec(const char *, uint64_t *);
extern char *	nl_msec2str(uint64_t, char *, size_t);

/* link layer protocol translations */
extern char *	nl_llproto2str(int, char *, size_t);
extern int	nl_str2llproto(const char *);

/* ethernet protocol translations */
extern char *	nl_ether_proto2str(int, char *, size_t);
extern int	nl_str2ether_proto(const char *);

/* IP protocol translations */
extern char *	nl_ip_proto2str(int, char *, size_t);
extern int	nl_str2ip_proto(const char *);

/* Dumping helpers */
extern void	nl_new_line(struct nl_dump_params *);
extern void	nl_dump(struct nl_dump_params *, const char *, ...);
extern void	nl_dump_line(struct nl_dump_params *, const char *, ...);

#ifdef __cplusplus
}
#endif

#endif
