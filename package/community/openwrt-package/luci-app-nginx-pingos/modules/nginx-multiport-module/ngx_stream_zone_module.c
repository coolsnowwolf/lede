/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct ngx_stream_zone_hash_s ngx_stream_zone_hash_t;
typedef struct ngx_stream_zone_node_s ngx_stream_zone_node_t;
typedef struct ngx_stream_zone_conf_s ngx_stream_zone_conf_t;

static ngx_int_t
ngx_stream_zone_init_process(ngx_cycle_t *cycle);
static void
ngx_stream_zone_exit_process(ngx_cycle_t *cycle);
static void *
ngx_stream_zone_create_conf(ngx_cycle_t *cf);
static char *
ngx_stream_zone_init_conf(ngx_cycle_t *cycle, void *conf);
static char *
ngx_stream_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *
ngx_stream_zone_shm_init(ngx_shm_t *shm, ngx_stream_zone_conf_t *szcf,
    ngx_cycle_t *cycle);


#define NAME_LEN    1024

static ngx_str_t stream_zone_key = ngx_string("stream_zone");

struct ngx_stream_zone_node_s {
    u_char                              name[NAME_LEN];
    ngx_int_t                           slot; /* process slot */
    ngx_int_t                           idx;
    ngx_int_t                           next; /* idx of stream node */
};

struct ngx_stream_zone_hash_s {
    ngx_shmtx_t                         mutex;
    ngx_shmtx_sh_t                      lock;
    ngx_int_t                           node; /* idx of stream node */
};

struct ngx_stream_zone_conf_s {
    ngx_int_t                           nbuckets;
    ngx_int_t                           nstreams;

    ngx_pool_t                         *pool;

    ngx_shmtx_t                        *mutex;
    ngx_shmtx_sh_t                     *lock;
    ngx_stream_zone_hash_t             *hash;       /* hash in shm */
    ngx_stream_zone_node_t             *stream_node;/* node in shm */
    ngx_int_t                          *free_node;  /* free node chain */
    ngx_int_t                          *alloc;      /* node number in use*/
};


static ngx_command_t  ngx_stream_zone_commands[] = {

    { ngx_string("stream_zone"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE2,
      ngx_stream_zone,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_core_module_t  ngx_stream_zone_module_ctx = {
    ngx_string("rtmp_stream_zone"),
    ngx_stream_zone_create_conf,            /* create conf */
    ngx_stream_zone_init_conf               /* init conf */
};


ngx_module_t  ngx_stream_zone_module = {
    NGX_MODULE_V1,
    &ngx_stream_zone_module_ctx,            /* module context */
    ngx_stream_zone_commands,               /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    ngx_stream_zone_init_process,           /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    ngx_stream_zone_exit_process,           /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_stream_zone_node_t *
ngx_stream_zone_get_node(ngx_str_t *name, ngx_int_t pslot)
{
    ngx_stream_zone_conf_t             *szcf;
    ngx_stream_zone_node_t             *node;

    szcf = (ngx_stream_zone_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                      ngx_stream_zone_module);

    ngx_shmtx_lock(szcf->mutex);

    if (*szcf->free_node == -1) {
        ngx_shmtx_unlock(szcf->mutex);
        return NULL;
    }

    node = &szcf->stream_node[*szcf->free_node];
    *szcf->free_node = node->next;

    *ngx_copy(node->name, name->data, ngx_min(NAME_LEN - 1, name->len)) = '\0';
    node->slot = pslot;
    node->next = -1;

    ++*szcf->alloc;

    ngx_shmtx_unlock(szcf->mutex);

    return node;
}

static void
ngx_stream_zone_put_node(ngx_int_t idx)
{
    ngx_stream_zone_conf_t             *szcf;
    ngx_stream_zone_node_t             *node;

    szcf = (ngx_stream_zone_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                      ngx_stream_zone_module);

    ngx_shmtx_lock(szcf->mutex);

    node = &szcf->stream_node[idx];

    node->next = *szcf->free_node;
    *szcf->free_node = idx;

    --*szcf->alloc;

    ngx_shmtx_unlock(szcf->mutex);
}

static void *
ngx_stream_zone_create_conf(ngx_cycle_t *cf)
{
    ngx_stream_zone_conf_t             *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_zone_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->nbuckets = NGX_CONF_UNSET;
    conf->nstreams = NGX_CONF_UNSET;
    conf->pool = ngx_create_pool(4096, cf->log);

    return conf;
}

static char *
ngx_stream_zone_init_conf(ngx_cycle_t *cycle, void *conf)
{
    size_t                              len;
    ngx_shm_t                           shm;
    ngx_stream_zone_conf_t             *szcf = conf;

    ngx_conf_init_value(szcf->nbuckets, 512);
    ngx_conf_init_value(szcf->nstreams, 40960);

    /* create shm zone */
    len = sizeof(ngx_shmtx_t) + sizeof(ngx_shmtx_sh_t)
        + sizeof(ngx_stream_zone_hash_t) * szcf->nbuckets
        + sizeof(ngx_stream_zone_node_t) * szcf->nstreams
        + sizeof(ngx_int_t) + sizeof(ngx_int_t);

    shm.size = len;
    shm.name = stream_zone_key;
    shm.log = cycle->log;

    if (ngx_shm_alloc(&shm) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    return ngx_stream_zone_shm_init(&shm, szcf, cycle);
}

static void
ngx_stream_zone_clear(ngx_cycle_t *cycle)
{
    ngx_stream_zone_conf_t             *szcf;
    volatile ngx_int_t                  idx, cur, next;

    szcf = (ngx_stream_zone_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                      ngx_stream_zone_module);

    if (szcf->nbuckets <= 0 || szcf->nstreams <= 0) {
        return;
    }

    for (idx = 0; idx < szcf->nbuckets; ++idx) {

        ngx_shmtx_lock(&szcf->hash[idx].mutex);
        cur = -1;

        while (1) {
            if (cur == -1) {
                next = szcf->hash[idx].node;
            } else {
                next = szcf->stream_node[cur].next;
            }

            if (next == -1) {
                break;
            }

            if (szcf->stream_node[next].slot == ngx_process_slot) {
                if (cur == -1) {
                    szcf->hash[idx].node = szcf->stream_node[next].next;
                } else {
                    szcf->stream_node[cur].next = szcf->stream_node[next].next;
                }

                ngx_stream_zone_put_node(next);
                continue;
            }

            cur = next;
        }
        ngx_shmtx_unlock(&szcf->hash[idx].mutex);
    }
}

static ngx_int_t
ngx_stream_zone_init_process(ngx_cycle_t *cycle)
{
    ngx_stream_zone_clear(cycle);

    return NGX_OK;
}

static void
ngx_stream_zone_exit_process(ngx_cycle_t *cycle)
{
    ngx_stream_zone_clear(cycle);
}

static char *
ngx_stream_zone_shm_init(ngx_shm_t *shm, ngx_stream_zone_conf_t *szcf,
    ngx_cycle_t *cycle)
{
    u_char                             *p;
    ngx_int_t                           i, next;

    p = shm->addr;

    szcf->mutex = (ngx_shmtx_t *) p;
    p += sizeof(ngx_shmtx_t);

    szcf->lock = (ngx_shmtx_sh_t *) p;
    p += sizeof(ngx_shmtx_sh_t);

    szcf->hash = (ngx_stream_zone_hash_t *) p;
    p += sizeof(ngx_stream_zone_hash_t) * szcf->nbuckets;

    szcf->stream_node = (ngx_stream_zone_node_t *) p;
    p += sizeof(ngx_stream_zone_node_t) * szcf->nstreams;

    szcf->free_node = (ngx_int_t *) p;
    p += sizeof(ngx_int_t);

    szcf->alloc = (ngx_int_t *) p;

    /* init shm zone */
#if (NGX_HAVE_ATOMIC_OPS)

    p = NULL;

#else
        p = ngx_pnalloc(szcf->pool, cycle->lock_file.len
                + stream_zone_key.len);
        if (p == NULL) {
            return NGX_CONF_ERROR;
        }
        *ngx_sprintf(p, "%V%V", &cycle->lock_file, &stream_zone_key) = 0;

#endif

        if (ngx_shmtx_create(szcf->mutex, szcf->lock, p) != NGX_OK) {
            return NGX_CONF_ERROR;
        }

    for (i = 0; i < szcf->nbuckets; ++i) {
#if (NGX_HAVE_ATOMIC_OPS)

        p = NULL;

#else
        p = ngx_pnalloc(szcf->pool, cycle->lock_file.len + stream_zone_key.len
                + NGX_INT32_LEN);
        if (p == NULL) {
            return NGX_CONF_ERROR;
        }
        *ngx_sprintf(p, "%V%V%d", &cycle->lock_file,
                           &stream_zone_key, i) = 0;

#endif

        if (ngx_shmtx_create(&szcf->hash[i].mutex, &szcf->hash[i].lock, p)
                != NGX_OK)
        {
            return NGX_CONF_ERROR;
        }

        szcf->hash[i].node = -1;
    }

    next = -1;
    i = szcf->nstreams;

    do {
        --i;

        szcf->stream_node[i].slot = -1;
        szcf->stream_node[i].idx = i;
        szcf->stream_node[i].next = next;
        next = i;
    } while (i);

    *szcf->free_node = i;
    *szcf->alloc = 0;

    return NGX_CONF_OK;
}

static char *
ngx_stream_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_uint_t                          i;
    ngx_str_t                          *value;
    ngx_stream_zone_conf_t             *szcf = conf;

    value = cf->args->elts;

    for (i = 1; i < cf->args->nelts; ++i) {

        if (ngx_strncmp(value[i].data, "buckets=", 8) == 0) {
            szcf->nbuckets = ngx_atoi(value[i].data + 8, value[i].len - 8);

            if (szcf->nbuckets <= 0) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                        "invalid buckets \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "streams=", 8) == 0) {
            szcf->nstreams = ngx_atoi(value[i].data + 8, value[i].len - 8);

            if (szcf->nstreams <= 0) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                        "invalid streams \"%V\"", &value[i]);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                "invalid parameter \"%V\"", &value[i]);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


ngx_int_t
ngx_stream_zone_insert_stream(ngx_str_t *name)
{
    ngx_stream_zone_conf_t             *szcf;
    volatile ngx_uint_t                 idx;
    volatile ngx_int_t                  i, pslot;
    ngx_stream_zone_node_t             *node;

    szcf = (ngx_stream_zone_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                      ngx_stream_zone_module);

    if (szcf->nbuckets <= 0 || szcf->nstreams <= 0) {
        return NGX_ERROR;
    }

    if (name->len >= NAME_LEN) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "stream name(%V) too long", name);
        return NGX_ERROR;
    }

    idx = ngx_hash_key(name->data, name->len) % szcf->nbuckets;

    ngx_shmtx_lock(&szcf->hash[idx].mutex);
    i = szcf->hash[idx].node;
    pslot = -1;
    while (i != -1) {
        if (ngx_strlen(szcf->stream_node[i].name) == name->len
            && ngx_memcmp(szcf->stream_node[i].name, name->data, name->len)
                == 0)
        {
            pslot = szcf->stream_node[i].slot;
            break;
        }

        i = szcf->stream_node[i].next;
    }

    if (i == -1) { /* stream not in hash */
        node = ngx_stream_zone_get_node(name, ngx_process_slot);
        if (node == NULL) {
            ngx_shmtx_unlock(&szcf->hash[idx].mutex);
            ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                    "stream zone get node failed");
            return NGX_ERROR;
        }
        node->slot = ngx_process_slot;

        node->next = szcf->hash[idx].node;
        szcf->hash[idx].node = node->idx;

        pslot = ngx_process_slot;
    }
    ngx_shmtx_unlock(&szcf->hash[idx].mutex);

    return pslot;
}

void
ngx_stream_zone_delete_stream(ngx_str_t *name)
{
    ngx_stream_zone_conf_t             *szcf;
    volatile ngx_uint_t                 idx;
    volatile ngx_int_t                  cur, next;

    szcf = (ngx_stream_zone_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                      ngx_stream_zone_module);

    if (szcf->nbuckets <= 0 || szcf->nstreams <= 0) {
        return;
    }

    idx = ngx_hash_key(name->data, name->len) % szcf->nbuckets;

    ngx_shmtx_lock(&szcf->hash[idx].mutex);
    cur = -1;
    next = szcf->hash[idx].node;
    while (next != -1) {
        if (ngx_strlen(szcf->stream_node[next].name) == name->len
            && ngx_memcmp(szcf->stream_node[next].name, name->data, name->len)
                == 0)
        {
            if (szcf->stream_node[next].slot != ngx_process_slot) {
                break;
            }

            if (cur == -1) { /* link header */
                szcf->hash[idx].node = szcf->stream_node[next].next;
            } else {
                szcf->stream_node[cur].next = szcf->stream_node[next].next;
            }
            ngx_stream_zone_put_node(next);
            break;
        }

        cur = next;
        next = szcf->stream_node[next].next;
    }
    ngx_shmtx_unlock(&szcf->hash[idx].mutex);
}

ngx_chain_t *
ngx_stream_zone_state(ngx_http_request_t *r, ngx_flag_t detail)
{
    ngx_stream_zone_conf_t             *szcf;
    ngx_chain_t                        *cl;
    ngx_buf_t                          *b;
    size_t                              len;
    volatile ngx_int_t                  idx, next;

    szcf = (ngx_stream_zone_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                      ngx_stream_zone_module);

    if (szcf->nbuckets <= 0 || szcf->nstreams <= 0) {
        return NULL;
    }

    len = sizeof("##########stream zone state##########\n") - 1
        + sizeof("ngx_stream_zone buckets: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_stream_zone streams: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_stream_zone alloc: \n") - 1 + NGX_OFF_T_LEN;

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
            "##########stream zone state##########\n"
            "ngx_stream_zone buckets: %i\nngx_stream_zone streams: %i\n"
            "ngx_stream_zone alloc: %i\n",
            szcf->nbuckets, szcf->nstreams, *szcf->alloc);

    if (detail) {
        for (idx = 0; idx < szcf->nbuckets; ++idx) {
            ngx_shmtx_lock(&szcf->hash[idx].mutex);

            next = szcf->hash[idx].node;
            if (next == -1) {
                ngx_shmtx_unlock(&szcf->hash[idx].mutex);
                continue;
            }
            ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "slot: %i", idx);

            while (next != -1) {
                ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                        "\t\tname:%s, slot:%i, idx:%i, next:%i",
                        szcf->stream_node[next].name,
                        szcf->stream_node[next].slot,
                        szcf->stream_node[next].idx,
                        szcf->stream_node[next].next);

                next = szcf->stream_node[next].next;
            }

            ngx_shmtx_unlock(&szcf->hash[idx].mutex);
        }
    }

    return cl;
}
