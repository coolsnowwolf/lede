#ifndef __MUSL_COMPAT_ASM_ERRNO_H
#define __MUSL_COMPAT_ASM_ERRNO_H

/* Avoid including different versions of errno.h, the defines (incorrectly)
 * cause a redefinition error on PowerPC */
#include <errno.h>

#endif
