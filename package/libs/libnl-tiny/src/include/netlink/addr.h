/*
 * netlink/addr.h		Abstract Address
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_ADDR_H_
#define NETLINK_ADDR_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_addr;

/* Creation */
extern struct nl_addr *	nl_addr_alloc(size_t);
extern struct nl_addr *	nl_addr_alloc_attr(struct nlattr *, int);
extern struct nl_addr *	nl_addr_build(int, void *, size_t);
extern int		nl_addr_parse(const char *, int, struct nl_addr **);
extern struct nl_addr *	nl_addr_clone(struct nl_addr *);

/* Destroyage */
extern void		nl_addr_destroy(struct nl_addr *);

/* Usage Management */
extern struct nl_addr *	nl_addr_get(struct nl_addr *);
extern void		nl_addr_put(struct nl_addr *);
extern int		nl_addr_shared(struct nl_addr *);

extern int		nl_addr_cmp(struct nl_addr *, struct nl_addr *);
extern int		nl_addr_cmp_prefix(struct nl_addr *, struct nl_addr *);
extern int		nl_addr_iszero(struct nl_addr *);
extern int		nl_addr_valid(char *, int);
extern int      	nl_addr_guess_family(struct nl_addr *);
extern int		nl_addr_fill_sockaddr(struct nl_addr *,
					      struct sockaddr *, socklen_t *);
extern int		nl_addr_info(struct nl_addr *, struct addrinfo **);
extern int		nl_addr_resolve(struct nl_addr *addr, char *host, size_t hostlen);

/* Access Functions */
extern void		nl_addr_set_family(struct nl_addr *, int);
extern int		nl_addr_get_family(struct nl_addr *);
extern int		nl_addr_set_binary_addr(struct nl_addr *, void *,
						size_t);
extern void *		nl_addr_get_binary_addr(struct nl_addr *);
extern unsigned int	nl_addr_get_len(struct nl_addr *);
extern void		nl_addr_set_prefixlen(struct nl_addr *, int);
extern unsigned int	nl_addr_get_prefixlen(struct nl_addr *);

/* Address Family Translations */
extern char *		nl_af2str(int, char *, size_t);
extern int		nl_str2af(const char *);

/* Translations to Strings */
extern char *		nl_addr2str(struct nl_addr *, char *, size_t);

#ifdef __cplusplus
}
#endif

#endif
