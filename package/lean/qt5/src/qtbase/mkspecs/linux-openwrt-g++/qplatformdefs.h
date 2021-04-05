#include "../linux-g++/qplatformdefs.h"

#undef QT_SOCKLEN_T

#if defined(__GLIBC__) && (__GLIBC__ < 2)
#define QT_SOCKLEN_T            int
#else
#define QT_SOCKLEN_T            socklen_t
#endif
