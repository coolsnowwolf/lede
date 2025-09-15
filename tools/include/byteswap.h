#if defined(__linux__) || defined(__CYGWIN__) 
#include_next <byteswap.h>
#else
#include <endian.h>
#endif
