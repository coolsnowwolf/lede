#ifndef __LINUX_TC_NSS_MIRRED_H
#define __LINUX_TC_NSS_MIRRED_H

#include <linux/pkt_cls.h>

/*
 * Type of nss mirred action.
 */
#define TCA_ACT_MIRRED_NSS 17

/*
 * Types of parameters for nss mirred action.
 */
enum {
	TC_NSS_MIRRED_UNSPEC,
	TC_NSS_MIRRED_TM,
	TC_NSS_MIRRED_PARMS,
	__TC_NSS_MIRRED_MAX
};
#define TC_NSS_MIRRED_MAX (__TC_NSS_MIRRED_MAX - 1)

/*
 * tc_nss_mirred
 *	tc command structure for nss mirred action.
 */
struct tc_nss_mirred {
	tc_gen;			/* General tc structure. */
	__u32 from_ifindex;	/* ifindex of the port from which traffic
				 * will be redirected.
				 */
	__u32 to_ifindex;	/* ifindex of the port to which traffic
				 * will be redirected.
				 */
};

#endif	/* __LINUX_TC_NSS_MIRRED_H */
