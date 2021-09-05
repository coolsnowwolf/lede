/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */



#ifndef _NGX_DYNAMIC_COMMAND_H_INCLUDED_
#define _NGX_DYNAMIC_COMMAND_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>


typedef struct {
    ngx_str_t               name;
    void                 *(*create_conf)(ngx_conf_t *cf);
    char                 *(*init_conf)(ngx_conf_t *cf, void *conf);
} ngx_dynamic_core_module_t;


#if (NGX_PCRE)
typedef struct {
    ngx_regex_t            *regex;
    ngx_str_t               name;
} ngx_dynamic_regex_t;
#endif

#if 0
/*
 * if use dynamic conf, module define should use
 *  NGX_MODULE_V1_DYNAMIC_PADDING and set
 *  module dynamic context and
 *  module dynamic directives as bellow
 */
ngx_module_t  ngx_test_module = {
    NGX_MODULE_V1,
    &ngx_test_module_ctx,                   /* module context */
    ngx_test_commands,                      /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    &ngx_test_module_dctx,                  /* module dynamic context */
    ngx_test_dcommands,                     /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};
#endif
#define NGX_MODULE_V1_DYNAMIC_PADDING  0, 0, 0, 0, 0, 0

extern ngx_uint_t  ngx_core_max_module;

/*
 * return value:
 *      return NGX_OK for successd, NGX_ERROR for failed
 * paras:
 *      cf:   ngx_conf_t passed from ngx_dynamic_conf_load_conf
 *      init: only ngx_dynamic_conf_load_conf set 1, otherwise set 0
 */
ngx_int_t ngx_dynamic_conf_parse(ngx_conf_t *cf, unsigned init);

/*
 * return value:
 *      return regex context
 * paras:
 *      cf: ngx_conf_t passed in dynamic cmd handler
 *      rc: regex options
 */
#if (NGX_PCRE)
ngx_dynamic_regex_t *ngx_dynamic_regex_compile(ngx_conf_t *cf,
        ngx_regex_compile_t *rc);
#endif

int ngx_dynamic_cmp_dns_wildcards(const void *one, const void *two);

void *ngx_get_dconf(ngx_module_t *m);


#endif
