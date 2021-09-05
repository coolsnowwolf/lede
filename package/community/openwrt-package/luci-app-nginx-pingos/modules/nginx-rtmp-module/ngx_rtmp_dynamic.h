/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_RTMP_DYNAMIC_H_INCLUDED_
#define _NGX_RTMP_DYNAMIC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_map.h"
#include "ngx_dynamic_conf.h"
#include "ngx_rtmp.h"


typedef struct {
    void       *(*create_main_conf)(ngx_conf_t *cf);
    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);

    void       *(*create_srv_conf)(ngx_conf_t *cf);
    char       *(*init_srv_conf)(ngx_conf_t *cf, void *conf);

    void       *(*create_app_conf)(ngx_conf_t *cf);
    char       *(*init_app_conf)(ngx_conf_t *cf, void *conf);
} ngx_rtmp_dynamic_module_t;


void *ngx_rtmp_get_module_main_dconf(ngx_rtmp_session_t *s, ngx_module_t *m);
void *ngx_rtmp_get_module_srv_dconf(ngx_rtmp_session_t *s, ngx_module_t *m);
void *ngx_rtmp_get_module_app_dconf(ngx_rtmp_session_t *s, ngx_module_t *m);

void  ngx_rmtp_get_serverid_by_domain(ngx_str_t *serverid, ngx_str_t *domain);


#endif
