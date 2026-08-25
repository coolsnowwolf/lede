#ifndef _PHY_DEBUG_H_
#define _PHY_DEBUG_H_

#include <linux/string.h>
#include <linux/jiffies.h>

typedef enum {
	PHY_MSG_ERR		= 0x0001,
	PHY_MSG_INT		= 0x0002,
	PHY_MSG_ACT		= 0x0004,
	PHY_MSG_TRACE	= 0x0008,
	PHY_MSG_DBG		= 0x0010
} xPON_PHY_DebugMsg_t ;

/* JIFFIES_PADDING can let printk show jiffies in mili-seconds */
	#if HZ == 100
		#define JIFFIES_PADDING		"0ms"
	#else
		#define JIFFIES_PADDING		""
	#endif


#ifdef CONFIG_DEBUG
	#define PON_PHY_MSG(level, F, B...)	{ \
										if(gpPhyPriv->debugLevel & level) 	\
											printk("[%lu]" JIFFIES_PADDING "[%s:%d]\n  " F , jiffies, strrchr(__FILE__, '/') + 1, __LINE__, ##B) ; \
									}
#else
	#define PON_PHY_MSG(level, F, B...)	
#endif


#define CHK_BUF() 		pos = begin + index ; \
						if(pos < off) { \
							index = 0 ; \
							begin = pos ; \
						} \
						if(pos > off + count) \
							goto done ;


/* JIFFIES_PADDING can let printk show jiffies in mili-seconds */
#if HZ == 100
	#define JIFFIES_PADDING		"0ms"
#else
	#define JIFFIES_PADDING		""
#endif

extern int xpon_phy_print_open;
#define XPON_DPRINT_MSG(F, B...)    do{ \
                                        if(xpon_phy_print_open) \
                                        	printk("[%lu" JIFFIES_PADDING "]%s:%d " F , jiffies, __FUNCTION__, __LINE__, ##B) ; \
                                    }while(0)

#endif /* _PHY_DEBUG_H_ */

