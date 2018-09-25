#ifndef CONV_H_INCLUDE
#define CONV_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

const char* g2u(const char *inb); // gbk to utf-8
const char* u2g(const char *inb); // utf-8 to gbk

#ifdef __cplusplus
}
#endif

#endif // CONV_H_INCLUDE
