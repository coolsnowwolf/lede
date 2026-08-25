#ifndef _DRV_DEBUG_H_
#define _DRV_DEBUG_H_

#ifdef XPON_MAC_CONFIG_DEBUG
#include <linux/jiffies.h>
#endif

#include "xmcs/xmcs_const.h"

#define CHK_BUF() 		pos = begin + index ; \
						if(pos < off) { \
							index = 0 ; \
							begin = pos ; \
						} \
						if(pos > off + count) \
							goto done ;

#ifdef XPON_MAC_CONFIG_DEBUG
/* JIFFIES_PADDING can let printk show jiffies in mili-seconds */
	#if HZ == 100
		#define JIFFIES_PADDING		"0ms"
	#else
		#define JIFFIES_PADDING		""
	#endif
	#define PON_MSG(level, F, B...)	{ \
										if(gpPonSysData->debugLevel & level) { \
											printk("[%lu" JIFFIES_PADDING "]" F , jiffies, ##B) ; \
										} \
									}
#else
	#define PON_MSG(level, F, B...)	
#endif

#ifdef XPON_MAC_CONFIG_DEBUG

    extern int xpon_mac_print_open;
    extern int drop_print_flag;

    #define XPON_DPRINT_MSG_RAW(F, B...) do{ \
                                                printk("%s:%d " F ,  __FUNCTION__, __LINE__, ##B) ;\
                                    }while(0)
                                            
    #define XPON_DPRINT_MSG(F, B...) if(xpon_mac_print_open)  XPON_DPRINT_MSG_RAW(F, ##B) ;

    #define XPON_DPRINT XPON_DPRINT_MSG("!!!!XPON SPLIT DEBUG!!!====>%s:%d\n", __FUNCTION__, __LINE__)

    #define XPON_DROP_MSG(F, B...) if(drop_print_flag) XPON_DPRINT_MSG_RAW(F, ##B)

    #define XPON_DROP_PRINT XPON_DROP_MSG("!!!!XPON SPLIT DEBUG!!!====>%s:%d\n", __FUNCTION__, __LINE__)

#else /*XPON_MAC_CONFIG_DEBUG*/

    #define XPON_DPRINT_MSG(F, B...)
    #define XPON_DPRINT 
    #define XPON_DROP_PRINT 
    #define XPON_DROP_MSG(F, B...)

#endif /*XPON_MAC_CONFIG_DEBUG*/

#endif /* _DRV_DEBUG_H_ */

