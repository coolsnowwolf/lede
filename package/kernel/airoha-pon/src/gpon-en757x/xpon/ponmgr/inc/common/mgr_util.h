#ifndef _MGR_UTIL_H_
#define _MGR_UTIL_H_

#include <regex.h>
#include "common/mgr_type.h"

inline MGR_U32 get_ramdom_value(MGR_U32 max) ;
inline MGR_U32 get_system_up_time(MGR_U32 *t) ;

char *get_current_timestamp (void);

#endif /* _MGR_UTIL_H_ */
