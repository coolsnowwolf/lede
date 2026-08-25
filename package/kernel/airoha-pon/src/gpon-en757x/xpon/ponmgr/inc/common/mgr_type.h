#ifndef _MGR_TYPE_H_
#define _MGR_TYPE_H_

#include "common/mgr_error.h"

#ifdef MGR_U64
#undef MGR_U64
#endif
typedef unsigned long long		MGR_U64 ;
#ifdef MGR_U32
#undef MGR_U32
#endif
typedef unsigned int 			MGR_U32 ;
#ifdef MGR_U16
#undef MGR_U16
#endif
typedef unsigned short 			MGR_U16 ;
#ifdef MGR_U8
#undef MGR_U8
#endif
typedef unsigned char 			MGR_U8 ;
#ifdef MGR_S64
#undef MGR_S64
#endif
typedef long long				MGR_S64 ;
#ifdef MGR_S32
#undef MGR_S32
#endif
typedef int 					MGR_S32 ;
#ifdef MGR_S16
#undef MGR_S16
#endif
typedef short 					MGR_S16 ;
#ifdef MGR_S8
#undef MGR_S8
#endif
typedef char 					MGR_S8 ;

#ifdef MGR_Ret
#undef MGR_Ret
#endif
typedef int						MGR_Ret ;

#ifdef MGR_Cmd
#undef MGR_Cmd
#endif
typedef MGR_U16					MGR_Cmd ;

#ifdef MGR_Mask
#undef MGR_Mask
#endif
typedef MGR_U16					MGR_Mask ;

#ifdef MGR_Task
#undef MGR_Task
#endif
typedef MGR_U16					MGR_Task ;

#ifdef MGR_Time
#undef MGR_Time
#endif
typedef MGR_U32					MGR_Time ;

#endif /* _MGR_TYPE_H_ */

