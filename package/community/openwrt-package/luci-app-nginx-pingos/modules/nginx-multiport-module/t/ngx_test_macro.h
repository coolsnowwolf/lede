/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_TEST_MACRO_H_INCLUDE_
#define _NGX_TEST_MACRO_H_INCLUDE_


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_int_t count  = 0;
static ngx_int_t pass   = 0;

#define NGX_TEST_INIT       count = 0, pass = 0;

#define NGX_TEST_ISOK(testcase)                                             \
{                                                                           \
    ngx_int_t  __ret = testcase;                                            \
    ++count;                                                                \
    if (__ret) ++pass;                                                      \
    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "    TEST "#testcase"%s",\
            (__ret ? " ...OK" : " ...ERROR"));                              \
}

#define NGX_TEST_INT(di, si)                                                \
    (di == si)

static ngx_inline ngx_int_t
ngx_test_str(ngx_str_t *nstr, char *cstr)
{
    size_t                      len;

    len = ngx_strlen(cstr);

    return (nstr->len == len && ngx_memcmp(nstr->data, cstr, len) == 0);
}

#endif
