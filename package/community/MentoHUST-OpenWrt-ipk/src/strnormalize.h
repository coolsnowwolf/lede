/**
 * Copyright(c) 2012-2013, All Rights Reserved.
 *
 * @file strnormalize.h
 * @details Check GBK character you could do
 *     code >= 0x8000 && _pGbk2Utf16[code - 0x8000] != 0
 * @author cnangel
 * @version 1.0.0
 * @date 2012/10/09 11:44:58
 */

#ifndef __STRNORMALIZE_H__
#define __STRNORMALIZE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SNO_TO_LOWER        1
#define SNO_TO_UPPER        2
#define SNO_TO_HALF         4
#define SNO_TO_SIMPLIFIED   8

void str_normalize_init();
void str_normalize_gbk(char *text, unsigned options);
void str_normalize_utf8(char *text, unsigned options);

int gbk_to_utf8(const char *from, unsigned int from_len, char **to, unsigned int *to_len);
int utf8_to_gbk(const char *from, unsigned int from_len, char **to, unsigned int *to_len);

#ifdef __cplusplus
}
#endif

#endif /* __STRNORMALIZE_H__ */

