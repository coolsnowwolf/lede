/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include "ngx_event_timer_module.h"
#include "ngx_event_resolver.h"
#include "ngx_dynamic_resolver.h"
#include "ngx_toolkit_misc.h"


static ngx_int_t ngx_dynamic_resolver_process_init(ngx_cycle_t *cycle);

static void *ngx_dynamic_resolver_create_conf(ngx_cycle_t *cycle);
static char *ngx_dynamic_resolver_init_conf(ngx_cycle_t *cycle, void *conf);


#define MAX_DOMAIN_LEN      128
#define MAX_ADDRS           8


typedef struct ngx_dynamic_resolver_ctx_s       ngx_dynamic_resolver_ctx_t;
typedef struct ngx_dynamic_resolver_domain_s    ngx_dynamic_resolver_domain_t;

struct ngx_dynamic_resolver_ctx_s {
    ngx_dynamic_resolver_handler_pt     h;
    void                               *data;

    ngx_dynamic_resolver_ctx_t         *next;
};

typedef struct {
    struct sockaddr                     sockaddr;
    socklen_t                           socklen;
    u_short                             priority;
    u_short                             weight;
} ngx_dynamic_resolver_addr_t;

struct ngx_dynamic_resolver_domain_s {
    ngx_str_t                           domain;
    u_char                              domain_cstr[MAX_DOMAIN_LEN];

    ngx_uint_t                          naddrs;
    ngx_dynamic_resolver_addr_t         addrs[MAX_ADDRS];

    ngx_dynamic_resolver_ctx_t         *ctx;

    ngx_dynamic_resolver_domain_t      *next;
};

typedef struct {
    ngx_msec_t                          refresh_interval;

    size_t                              domain_buckets;
    ngx_dynamic_resolver_domain_t     **resolver_hash;

    ngx_dynamic_resolver_ctx_t         *free_ctx;
    ngx_dynamic_resolver_domain_t      *free_domain;

    ngx_uint_t                          nalloc_ctx;
    ngx_uint_t                          nfree_ctx;
    ngx_uint_t                          nalloc_domain;
    ngx_uint_t                          nfree_domain;
} ngx_dynamic_resolver_conf_t;


static ngx_str_t dynamic_resolver_name = ngx_string("dynamic_resolver");


static ngx_command_t ngx_dynamic_resolver_commands[] = {

    { ngx_string("dynamic_refresh_interval"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      0,
      offsetof(ngx_dynamic_resolver_conf_t, refresh_interval),
      NULL },

    { ngx_string("dynamic_domain_buckets"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_dynamic_resolver_conf_t, domain_buckets),
      NULL },

      ngx_null_command
};


ngx_event_module_t  ngx_dynamic_resolver_module_ctx = {
    &dynamic_resolver_name,
    ngx_dynamic_resolver_create_conf,         /* create configuration */
    ngx_dynamic_resolver_init_conf,           /* init configuration */

    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


/* this module use ngx_cycle->log */
ngx_module_t  ngx_dynamic_resolver_module = {
    NGX_MODULE_V1,
    &ngx_dynamic_resolver_module_ctx,         /* module context */
    ngx_dynamic_resolver_commands,            /* module directives */
    NGX_EVENT_MODULE,                       /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    ngx_dynamic_resolver_process_init,        /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_dynamic_resolver_create_conf(ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t      *conf;

    conf = ngx_pcalloc(cycle->pool, sizeof(ngx_dynamic_resolver_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->refresh_interval = NGX_CONF_UNSET_MSEC;
    conf->domain_buckets = NGX_CONF_UNSET_UINT;

    return conf;
}

static char *
ngx_dynamic_resolver_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_dynamic_resolver_conf_t      *drcf = conf;

    ngx_conf_init_msec_value(drcf->refresh_interval, 5000);
    ngx_conf_init_uint_value(drcf->domain_buckets, 101);

    if (drcf->refresh_interval > 0 && drcf->domain_buckets > 0) {
        drcf->resolver_hash = ngx_pcalloc(cycle->pool,
            sizeof(ngx_dynamic_resolver_domain_t *) * drcf->domain_buckets);
    }

    return NGX_CONF_OK;
}

/* reuse for ngx_dynamic_resolver_ctx_t */
static ngx_dynamic_resolver_ctx_t *
ngx_dynamic_resolver_get_ctx(ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_ctx_t     *ctx;

    drcf = ngx_event_get_conf(cycle->conf_ctx, ngx_dynamic_resolver_module);

    ctx = drcf->free_ctx;

    if (ctx == NULL) {
        ctx = ngx_pcalloc(cycle->pool, sizeof(ngx_dynamic_resolver_ctx_t));

        if (ctx == NULL) {
            ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "dynamic resolver, "
                    "alloc memory dynamic resolver ctx failed");
            return NULL;
        }
        ++drcf->nalloc_ctx;
    } else {
        drcf->free_ctx = drcf->free_ctx->next;
        ngx_memzero(ctx, sizeof(ngx_dynamic_resolver_ctx_t));
        --drcf->nfree_ctx;
    }

    return ctx;
}

static void
ngx_dynamic_resolver_put_ctx(ngx_dynamic_resolver_ctx_t *ctx,
        ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t    *drcf;

    drcf = ngx_event_get_conf(cycle->conf_ctx, ngx_dynamic_resolver_module);

    ctx->next = drcf->free_ctx;
    drcf->free_ctx = ctx;
    ++drcf->nfree_ctx;
}

/* reuse for ngx_dynamic_resolver_domain_t */
static ngx_dynamic_resolver_domain_t *
ngx_dynamic_resolver_get_domain(ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_domain_t  *domain;

    drcf = ngx_event_get_conf(cycle->conf_ctx, ngx_dynamic_resolver_module);

    domain = drcf->free_domain;

    if (domain == NULL) {
        domain = ngx_pcalloc(cycle->pool,
                             sizeof(ngx_dynamic_resolver_domain_t));

        if (domain == NULL) {
            ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "dynamic resolver, "
                    "alloc memory dynamic resolver domain failed");
            return NULL;
        }
        ++drcf->nalloc_domain;
    } else {
        drcf->free_domain = drcf->free_domain->next;
        ngx_memzero(domain, sizeof(ngx_dynamic_resolver_domain_t));
        --drcf->nfree_domain;
    }

    return domain;
}

static void
ngx_dynamic_resolver_put_domain(ngx_dynamic_resolver_domain_t *domain,
        ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t    *drcf;

    drcf = ngx_event_get_conf(cycle->conf_ctx, ngx_dynamic_resolver_module);

    domain->next = drcf->free_domain;
    drcf->free_domain = domain;
    ++drcf->nfree_domain;
}


static void
ngx_dynamic_resolver_on_result(void *data, ngx_resolver_addr_t *addrs,
        ngx_uint_t naddrs)
{
    ngx_dynamic_resolver_domain_t  *domain;
    ngx_dynamic_resolver_ctx_t     *ctx;
    ngx_uint_t                      i, n;

    domain = data;

    if (domain == NULL) {
        ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "dynamic resolver, "
                "%V has been deleted", &domain->domain);
        return;
    }

    if (naddrs == 0) {
        ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "dynamic resolver, "
                "domain '%V' resolver failed", &domain->domain);

        while (domain->ctx) {
            ctx = domain->ctx;
            domain->ctx = ctx->next;

            ctx->h(ctx->data, NULL, 0);
            ngx_dynamic_resolver_put_ctx(ctx, (ngx_cycle_t *) ngx_cycle);
        }

        return;
    }

    domain->naddrs = ngx_min(naddrs, MAX_ADDRS);
    for (i = 0; i < domain->naddrs; ++i) {
        ngx_memcpy(&domain->addrs[i].sockaddr, addrs[i].sockaddr,
                   addrs[i].socklen);
        domain->addrs[i].socklen = addrs[i].socklen;
        domain->addrs[i].priority = addrs[i].priority;
        domain->addrs[i].weight = addrs[i].weight;

        n = ngx_random() % domain->naddrs;

        while (domain->ctx) {
            ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                    "dynamic resolver, resolver '%V' successd",
                    &domain->domain);
            ctx = domain->ctx;
            domain->ctx = ctx->next;

            ctx->h(ctx->data, &domain->addrs[n].sockaddr,
                   domain->addrs[n].socklen);
            ngx_dynamic_resolver_put_ctx(ctx, (ngx_cycle_t *) ngx_cycle);

            ++n;
            n %= domain->naddrs;
        }
    }
}

static void
ngx_dynamic_resolver_on_timer(void *data)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_domain_t  *domain;
    ngx_uint_t                      i;

    if (ngx_exiting) {
        return;
    }

    drcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_dynamic_resolver_module);

    for (i = 0; i < drcf->domain_buckets; ++i) {
        domain = drcf->resolver_hash[i];
        while (domain) {
            ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                    "dynamic resolver, on timer start resolver %V",
                    &domain->domain);

            ngx_event_resolver_start_resolver(&domain->domain,
                    ngx_dynamic_resolver_on_result, domain);
            domain = domain->next;
        }
    }

    ngx_event_timer_add_timer(drcf->refresh_interval,
            ngx_dynamic_resolver_on_timer, NULL);
}

static ngx_int_t
ngx_dynamic_resolver_process_init(ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t      *drcf;

    drcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_dynamic_resolver_module);
    if (drcf->refresh_interval == 0) {
        return NGX_OK;
    }

    ngx_event_timer_add_timer(0, ngx_dynamic_resolver_on_timer, NULL);

    return NGX_OK;
}


void
ngx_dynamic_resolver_add_domain(ngx_str_t *domain, ngx_cycle_t *cycle)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_domain_t  *d;
    ngx_uint_t                      idx;
    struct sockaddr                 sa;
    socklen_t                       len;
    u_char                          temp[MAX_DOMAIN_LEN];

    if (domain == NULL || domain->len == 0) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0,
                "dynamic resolver add, domain is NULL");
        return;
    }

    len = ngx_sock_pton(&sa, domain->data, domain->len);
    /* addr is IP address */
    if (len) {
        ngx_log_error(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                "dynamic resolver add, domain is ipv4/ipv6/unix address");

        return;
    }

    drcf = ngx_event_get_conf(cycle->conf_ctx, ngx_dynamic_resolver_module);
    if (drcf->refresh_interval == 0) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "dynamic resolver add, "
                "dynamic resolver closed when add domain");
        return;
    }

    if (domain->len > MAX_DOMAIN_LEN) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0, "dynamic resolver add, "
                "domain length(%z) is too long", domain->len);
        return;
    }

    ngx_memzero(temp, MAX_DOMAIN_LEN);
    idx = ngx_hash_strlow(temp, domain->data, domain->len);
    idx %= drcf->domain_buckets;

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, cycle->log, 0, "dynamic resolver add, "
            "prepare add %V in %d slot", domain, idx);

    for (d = drcf->resolver_hash[idx]; d; d = d->next) {

        if (d->domain.len == domain->len &&
            ngx_memcmp(d->domain.data, temp, domain->len) == 0)
        {
            ngx_log_debug1(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                    "dynamic resolver add, %V is in dynamic resolv hash table",
                    domain);
            return;
        }
    }

    d = ngx_dynamic_resolver_get_domain(cycle);
    if (d == NULL) {
        return;
    }

    /* add domain in dynamic resolver */
    d->next = drcf->resolver_hash[idx];
    drcf->resolver_hash[idx] = d;

    ngx_memcpy(d->domain_cstr, temp, MAX_DOMAIN_LEN);
    d->domain.data = d->domain_cstr;
    d->domain.len = domain->len;
}

void
ngx_dynamic_resolver_del_domain(ngx_str_t *domain)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_domain_t **pd, *d;
    ngx_dynamic_resolver_ctx_t     *ctx;
    ngx_uint_t                      idx;
    struct sockaddr                 sa;
    socklen_t                       len;
    u_char                          temp[MAX_DOMAIN_LEN];

    if (domain == NULL || domain->len == 0) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic resolver del, domain is NULL");
        return;
    }

    len = ngx_sock_pton(&sa, domain->data, domain->len);
    /* addr is IP address */
    if (len) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                "dynamic resolver del, domain is ipv4/ipv6/unix address");

        return;
    }

    drcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_dynamic_resolver_module);
    if (drcf->refresh_interval == 0) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic resolver del, "
                "dynamic resolver closed when del domain");
        return;
    }

    if (domain->len > MAX_DOMAIN_LEN) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic resolver del, "
                "domain length(%z) is too long", domain->len);
        return;
    }

    ngx_memzero(temp, MAX_DOMAIN_LEN);
    idx = ngx_hash_strlow(temp, domain->data, domain->len);
    idx %= drcf->domain_buckets;

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "dynamic resolver del, "
            "prepare del %V in %d slot", domain, idx);

    for (pd = &drcf->resolver_hash[idx]; *pd; pd = &(*pd)->next) {

        if ((*pd)->domain.len == domain->len &&
            ngx_memcmp((*pd)->domain.data, temp, domain->len) == 0)
        {
            d= *pd;
            *pd = (*pd)->next;

            while (d->ctx) {
                ctx = d->ctx;
                d->ctx = ctx->next;

                ctx->h(ctx->data, NULL, 0);
                ngx_dynamic_resolver_put_ctx(ctx, (ngx_cycle_t *) ngx_cycle);
            }

            ngx_dynamic_resolver_put_domain(d, (ngx_cycle_t *) ngx_cycle);

            return;
        }
    }

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "dynamic resolver del, "
                  "%V is not in dynamic resolv hash table", domain);
}

void
ngx_dynamic_resolver_start_resolver(ngx_str_t *domain,
    ngx_dynamic_resolver_handler_pt h, void *data)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_domain_t  *d;
    ngx_dynamic_resolver_ctx_t     *ctx;
    ngx_uint_t                      idx, n;
    struct sockaddr                 sa;
    socklen_t                       len;
    u_char                          temp[MAX_DOMAIN_LEN];

    if (domain == NULL || domain->len == 0) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic resolver async, domain is NULL");
        return;
    }

    len = ngx_sock_pton(&sa, domain->data, domain->len);
    /* addr is ipv4/ipv6/unix address */
    if (len) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                "dynamic resolver async, domain is ipv4/ipv6/unix address");

        h(data, &sa, len);

        return;
    }

    drcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_dynamic_resolver_module);
    if (drcf->refresh_interval == 0) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic resolver async, "
                "dynamic resolver closed when start resolver");
        goto failed;
    }

    if (domain->len > MAX_DOMAIN_LEN) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic resolver async, "
                "domain length(%z) is too long", domain->len);
        goto failed;
    }

    ngx_memzero(temp, MAX_DOMAIN_LEN);
    idx = ngx_hash_strlow(temp, domain->data, domain->len);
    idx %= drcf->domain_buckets;

    d = drcf->resolver_hash[idx];
    while (d) {
        if (d->domain.len == domain->len &&
            ngx_memcmp(d->domain.data, temp, domain->len) == 0)
        {
            break;
        }
        d = d->next;
    }

    if (d == NULL) { /* not found */
        d = ngx_dynamic_resolver_get_domain((ngx_cycle_t *) ngx_cycle);
        if (d == NULL) {
            goto failed;
        }

        /* add domain in dynamic resolver */
        d->next = drcf->resolver_hash[idx];
        drcf->resolver_hash[idx] = d;

        ngx_memcpy(d->domain_cstr, temp, MAX_DOMAIN_LEN);
        d->domain.data = d->domain_cstr;
        d->domain.len = domain->len;
    }

    /* domain is not resolved */
    if (d->naddrs == 0) {

        /* add call back in resolver list */
        ctx = ngx_dynamic_resolver_get_ctx((ngx_cycle_t *) ngx_cycle);
        if (ctx == NULL) {
            goto failed;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                "dynamic resolver async, domain '%V' is not resolved",
                &d->domain);
        ctx->h = h;
        ctx->data = data;

        ctx->next = d->ctx;
        d->ctx = ctx;

        ngx_event_resolver_start_resolver(&d->domain,
                ngx_dynamic_resolver_on_result, d);

        return;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
            "dynamic resolver async, domain '%V' is resolved, call directly",
            &d->domain);

    /* call callback */
    n = ngx_random() % d->naddrs;
    h(data, &d->addrs[n].sockaddr, d->addrs[n].socklen);

    return;

failed:

    h(data, NULL, 0);
}

socklen_t
ngx_dynamic_resolver_gethostbyname(ngx_str_t *domain, struct sockaddr *sa)
{
    ngx_dynamic_resolver_conf_t    *drcf;
    ngx_dynamic_resolver_domain_t  *d;
    ngx_uint_t                      idx, n;
    socklen_t                       len;
    u_char                          temp[MAX_DOMAIN_LEN];

    if (domain == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic resolver sync, domain is NULL");
        return 0;
    }

    len = ngx_sock_pton(sa, domain->data, domain->len);
    /* addr is ipv4/ipv6/unix address */
    if (len) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                "dynamic resolver sync, domain is ipv4/ipv6/unix address");

        return len;
    }

    drcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_dynamic_resolver_module);
    if (drcf->refresh_interval == 0) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic resolver sync, "
                "dynamic resolver closed when start resolver");
        return 0;
    }

    if (domain->len > MAX_DOMAIN_LEN) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic resolver sync, "
                "domain length(%z) is too long", domain->len);
        return 0;
    }

    ngx_memzero(temp, MAX_DOMAIN_LEN);
    idx = ngx_hash_strlow(temp, domain->data, domain->len);
    idx %= drcf->domain_buckets;

    d = drcf->resolver_hash[idx];
    while (d) {
        if (d->domain.len == domain->len &&
            ngx_memcmp(d->domain.data, temp, domain->len) == 0)
        {
            if (d->naddrs == 0) {
                ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0,
                        "dynamic resolver sync, domain '%V' is not resolved",
                        &d->domain);
                return 0;
            }

            n = ngx_random() % d->naddrs;
            ngx_memcpy(sa, &d->addrs[n].sockaddr, d->addrs[n].socklen);

            return d->addrs[n].socklen;
        }

        d = d->next;
    }

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "dynamic resolver sync, "
            "domain '%V' is not in dynamic resolver table", domain);

    return 0;
}

ngx_chain_t *
ngx_dynamic_resolver_state(ngx_http_request_t *r)
{
	ngx_dynamic_resolver_conf_t *drcf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    size_t                      len;

    drcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_dynamic_resolver_module);

    len = sizeof("##########dynamic resolver state##########\n") - 1
        + sizeof("ngx_dynamic_resolver alloc ctx: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_dynamic_resolver free ctx: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_dynamic_resolver alloc domain: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_dynamic_resolver free domain: \n") - 1 + NGX_OFF_T_LEN;

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NULL;
    }
    cl->next = NULL;

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NULL;
    }
    cl->buf = b;

    b->last = ngx_snprintf(b->last, len,
            "##########dynamic resolver state##########\n"
            "ngx_dynamic_resolver alloc ctx: %ui\n"
            "ngx_dynamic_resolver free ctx: %ui\n"
            "ngx_dynamic_resolver alloc domain: %ui\n"
            "ngx_dynamic_resolver free domain: %ui\n",
            drcf->nalloc_ctx, drcf->nfree_ctx,
            drcf->nalloc_domain, drcf->nfree_domain);

    return cl;
}
