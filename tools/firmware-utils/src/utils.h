#include <stdint.h>
#include <string.h>

#pragma once

#define FW_MEMCPY_STR(dst, src)					\
       do {							\
               size_t slen = strlen(src);			\
               size_t dlen = sizeof(dst);			\
               memcpy(dst, src, slen > dlen ? dlen : slen);	\
       } while (0);
