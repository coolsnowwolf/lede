/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_HTTP_DYNAMIC_H_INCLUDED_
#define _NGX_HTTP_DYNAMIC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    void       *(*create_main_conf)(ngx_conf_t *cf);
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void       *(*create_srv_conf)(ngx_conf_t *cf);
    char       *(*init_srv_conf)(ngx_conf_t *cf, void *conf);

    void       *(*create_loc_conf)(ngx_conf_t *cf);
    char       *(*init_loc_conf)(ngx_conf_t *cf, void *conf);
} ngx_http_dynamic_module_t;


void *ngx_http_get_module_main_dconf(ngx_http_request_t *r, ngx_module_t *m);
void *ngx_http_get_module_srv_dconf(ngx_http_request_t *r, ngx_module_t *m);
void *ngx_http_get_module_loc_dconf(ngx_http_request_t *r, ngx_module_t *m);


#endif
