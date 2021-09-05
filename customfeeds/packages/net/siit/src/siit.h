/*
 * siit.h -- definitions for the SIIT module
 *
 *
 */

/*
 * Constants
 */

/* SIIT_ETH control the name of SIIT interface:
 * 0 - interface name is siit0,
 * 1 - interface name is ethX.
 */
#define SIIT_ETH 0

#define BUFF_SIZE 4096
#define FRAG_BUFF_SIZE 1232     /* IPv6 max fragment size without IPv6 header 
                                 * to fragmanet IPv4 if result IPv6 packet will be > 1280
                                 */

#define TRANSLATED_PREFIX 0x0000ffff /* third byte in IPv4-translated addr prefix */
#define MAPPED_PREFIX 0x0000ffff     /* third byte in IPv4-mapped addr prefix */

#define IP4_IP6_HDR_DIFF 20     /* diffirence between IPv4 and IPv6 headers */
#define IP6_FRAGMENT_SIZE 8     /* size of Fragment Header */

/* IPv6 header fields masks */
#define IP6F_OFF_MASK       0xfff8  /* mask out offset from frag_off */
#define IP6F_RESERVED_MASK  0x0006  /* reserved bits in frag_off */
#define IP6F_MORE_FRAG      0x0001  /* more-fragments flag */



/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef SIIT_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk(KERN_DEBUG "siit: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)








