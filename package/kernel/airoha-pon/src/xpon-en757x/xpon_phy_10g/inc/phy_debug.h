#ifndef _PHY_DEBUG_H_
#define _PHY_DEBUG_H_

#include <linux/string.h>
#include <linux/jiffies.h>


typedef enum {
	PHY_MSG_OFF 	= 0x0000,
	PHY_MSG_ERR		= 0x0001,
	PHY_MSG_INT		= 0x0002,
	PHY_MSG_ACT		= 0x0004,
	PHY_MSG_TRACE 	= 0x0008,
	PHY_MSG_DBG		= 0x0010,
	PHY_MSG_TIME 	= 0x0020,
	PHY_MSG_API 	= 0x0040,
	PHY_MSG_TRANS 	= 0x0080,
	PHY_MSG_ALL		= 0x00FF
} xPON_PHY_DebugMsg_t ;


#define PON_PHY_PRINT(level, F, B...)	{ \
									if(gpPhyPriv->debugLevel & level) 	\
										printk(F,##B) ; \
								}


#define CHK_BUF() 		pos = begin + index ; \
						if(pos < off) { \
							index = 0 ; \
							begin = pos ; \
						} \
						if(pos > off + count) \
							goto done ;

#endif /* _PHY_DEBUG_H_ */

