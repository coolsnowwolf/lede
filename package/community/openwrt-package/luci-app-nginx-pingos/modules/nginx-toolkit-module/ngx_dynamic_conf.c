/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_dynamic_conf.h"
#include "ngx_toolkit_misc.h"
#include "ngx_event_timer_module.h"


static ngx_int_t ngx_dynamic_conf_process_init(ngx_cycle_t *cycle);

static void *ngx_dynamic_conf_module_create_conf(ngx_cycle_t *cycle);
static char *ngx_dynamic_conf_module_init_conf(ngx_cycle_t *cycle, void *conf);

static char *ngx_dynamic_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_dynamic_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


#define NGX_CONF_BUFFER  4096

ngx_uint_t  ngx_core_max_module;

static ngx_uint_t arg_number[] = {
    NGX_CONF_NOARGS,
    NGX_CONF_TAKE1,
    NGX_CONF_TAKE2,
    NGX_CONF_TAKE3,
    NGX_CONF_TAKE4,
    NGX_CONF_TAKE5,
    NGX_CONF_TAKE6,
    NGX_CONF_TAKE7
};

typedef struct {
    void                      **module_conf;    /* index is module index */
    ngx_pool_t                 *pool;           /* dynamic conf alloc pool */
} ngx_dynamic_conf_ctx_t;

typedef struct {
    ngx_dynamic_conf_ctx_t     *conf[2];
    unsigned                    used;   /* 0 and 1 for index of conf */

    ngx_str_t                   file;
    ngx_msec_t                  refresh;
    ngx_log_t                  *log;

    u_char                      md5key[NGX_MD5KEY_LEN + 1];
} ngx_dynamic_conf_conf_t;


static ngx_command_t  ngx_dynamic_conf_commands[] = {

    { ngx_string("dynamic_conf"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE2,
      ngx_dynamic_conf,
      0,
      0,
      NULL },

    { ngx_string("dynamic_log"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_1MORE,
      ngx_dynamic_log,
      0,
      0,
      NULL },

      ngx_null_command
};


ngx_core_module_t  ngx_dynamic_conf_module_ctx = {
    ngx_string("dynamic_conf"),
    ngx_dynamic_conf_module_create_conf,
    ngx_dynamic_conf_module_init_conf
};


ngx_module_t  ngx_dynamic_conf_module = {
    NGX_MODULE_V1,
    &ngx_dynamic_conf_module_ctx,           /* module context */
    ngx_dynamic_conf_commands,              /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    ngx_dynamic_conf_process_init,          /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_dynamic_conf_module_create_conf(ngx_cycle_t *cycle)
{
    ngx_dynamic_conf_conf_t    *conf;

    conf = ngx_pcalloc(cycle->pool, sizeof(ngx_dynamic_conf_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->refresh = NGX_CONF_UNSET_MSEC;

    return conf;
}

static char *
ngx_dynamic_conf_module_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_dynamic_conf_conf_t    *dccf;

    dccf = conf;

    ngx_conf_init_msec_value(dccf->refresh, 60000);

    if (dccf->log == NULL) {
        dccf->log = &cycle->new_log;
    }

    return NGX_CONF_OK;
}

static char *
ngx_dynamic_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_dynamic_conf_conf_t    *dccf;
    ngx_str_t                  *value;
    ngx_fd_t                    fd;

    dccf = conf;

    if (dccf->file.len) {
        return "is duplicate";
    }

    value = cf->args->elts;

    dccf->file = value[1];

    if (ngx_conf_full_name(cf->cycle, &dccf->file, 0)) {
        return NGX_CONF_ERROR;
    }

    /* test file, ngx_conf_full_name will fill '\0' at end of file name */
    fd = ngx_open_file(dccf->file.data, NGX_FILE_RDONLY, NGX_FILE_OPEN, 0);
    if (fd == NGX_INVALID_FILE) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, ngx_errno, "dynamic conf "
                ngx_open_file_n " \"%s\" failed", dccf->file.data);
        return NGX_CONF_ERROR;
    }
    ngx_close_file(fd);

    dccf->refresh = ngx_parse_time(&value[2], 0);
    if (dccf->refresh == (ngx_msec_t) NGX_ERROR) {
        return "invalid refresh";
    }

    dccf->used = 1;

    return NGX_CONF_OK;
}

static char *
ngx_dynamic_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_dynamic_conf_conf_t    *dccf = conf;

    return ngx_log_set_log(cf, &dccf->log);
}


static ngx_int_t
ngx_dynamic_conf_handler(ngx_conf_t *cf, ngx_int_t last)
{
    char                       *rv;
    ngx_uint_t                  i, found;
    ngx_str_t                  *name;
    ngx_command_t              *cmd;
    void                       *conf;

    name = cf->args->elts;

    found = 0;

    for (i = 0; cf->cycle->modules[i]; ++i) {

        cmd = (ngx_command_t *) cf->cycle->modules[i]->spare_hook1;
        if (cmd == NULL) {
            continue;
        }

        for (/* void */; cmd->name.len; ++cmd) {

            if (name->len != cmd->name.len) {
                continue;
            }

            if (ngx_strcmp(name->data, cmd->name.data) != 0) {
                continue;
            }

            found = 1;

            if (cf->cycle->modules[i]->type != NGX_CONF_MODULE
                && cf->cycle->modules[i]->type != cf->module_type)
            {
                continue;
            }

            /* is the directive's location right ? */

            if (!(cmd->type & cf->cmd_type)) {
                continue;
            }

            if (!(cmd->type & NGX_CONF_BLOCK) && last != NGX_OK) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                  "directive \"%s\" is not terminated by \";\"",
                                  name->data);
                return NGX_ERROR;
            }

            if ((cmd->type & NGX_CONF_BLOCK) && last != NGX_CONF_BLOCK_START) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "directive \"%s\" has no opening \"{\"",
                                   name->data);
                return NGX_ERROR;
            }

            /* is the directive's argument count right ? */

            if (!(cmd->type & NGX_CONF_ANY)) {

                if (cmd->type & NGX_CONF_FLAG) {

                    if (cf->args->nelts != 2) {
                        goto invalid;
                    }

                } else if (cmd->type & NGX_CONF_1MORE) {

                    if (cf->args->nelts < 2) {
                        goto invalid;
                    }

                } else if (cmd->type & NGX_CONF_2MORE) {

                    if (cf->args->nelts < 3) {
                        goto invalid;
                    }

                } else if (cf->args->nelts > NGX_CONF_MAX_ARGS) {

                    goto invalid;

                } else if (!(cmd->type & arg_number[cf->args->nelts - 1])) {
                    goto invalid;
                }
            }

            /* set up the directive's configuration context */

            conf = ((void **) cf->ctx)[cf->cycle->modules[i]->ctx_index];

            rv = cmd->set(cf, cmd, conf);

            if (rv == NGX_CONF_OK) {
                return NGX_OK;
            }

            if (rv == NGX_CONF_ERROR) {
                return NGX_ERROR;
            }

            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                    "\"%s\" directive %s", name->data, rv);

            return NGX_ERROR;
        }
    }

    if (found) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                "\"%s\" directive is not allowed here", name->data);

        return NGX_ERROR;
    }

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "unknown directive \"%s\"", name->data);

    return NGX_ERROR;

invalid:

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "invalid number of arguments in \"%s\" directive",
            name->data);

    return NGX_ERROR;
}

/* same as ngx_conf_read_token in ngx_conf_file.c */
static ngx_int_t
ngx_dynamic_conf_read_token(ngx_conf_t *cf)
{
    u_char                     *start, ch, *src, *dst;
    off_t                       file_size;
    size_t                      len;
    ssize_t                     n, size;
    ngx_uint_t                  found, need_space, last_space, sharp_comment,
                                variable;
    ngx_uint_t                  quoted, s_quoted, d_quoted, start_line;
    ngx_str_t                  *word;
    ngx_buf_t                  *b;

    found = 0;
    need_space = 0;
    last_space = 1;
    sharp_comment = 0;
    variable = 0;
    quoted = 0;
    s_quoted = 0;
    d_quoted = 0;

    cf->args->nelts = 0;
    b = cf->conf_file->buffer;
    start = b->pos;
    start_line = cf->conf_file->line;

    file_size = ngx_file_size(&cf->conf_file->file.info);

    for ( ;; ) {

        if (b->pos >= b->last) {

            if (cf->conf_file->file.offset >= file_size) {

                if (cf->args->nelts > 0 || !last_space) {

                    if (cf->conf_file->file.fd == NGX_INVALID_FILE) {
                        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                           "unexpected end of parameter, "
                                           "expecting \";\"");
                        return NGX_ERROR;
                    }

                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                  "unexpected end of file, "
                                  "expecting \";\" or \"}\"");
                    return NGX_ERROR;
                }

                return NGX_CONF_FILE_DONE;
            }

            len = b->pos - start;

            if (len == NGX_CONF_BUFFER) {
                cf->conf_file->line = start_line;

                if (d_quoted) {
                    ch = '"';

                } else if (s_quoted) {
                    ch = '\'';

                } else {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "too long parameter \"%*s...\" started",
                                       10, start);
                    return NGX_ERROR;
                }

                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "too long parameter, probably "
                                   "missing terminating \"%c\" character", ch);
                return NGX_ERROR;
            }

            if (len) {
                ngx_memmove(b->start, start, len);
            }

            size = (ssize_t) (file_size - cf->conf_file->file.offset);

            if (size > b->end - (b->start + len)) {
                size = b->end - (b->start + len);
            }

            n = ngx_read_file(&cf->conf_file->file, b->start + len, size,
                              cf->conf_file->file.offset);

            if (n == NGX_ERROR) {
                return NGX_ERROR;
            }

            if (n != size) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   ngx_read_file_n " returned "
                                   "only %z bytes instead of %z",
                                   n, size);
                return NGX_ERROR;
            }

            b->pos = b->start + len;
            b->last = b->pos + n;
            start = b->start;
        }

        ch = *b->pos++;

        if (ch == LF) {
            cf->conf_file->line++;

            if (sharp_comment) {
                sharp_comment = 0;
            }
        }

        if (sharp_comment) {
            continue;
        }

        if (quoted) {
            quoted = 0;
            continue;
        }

        if (need_space) {
            if (ch == ' ' || ch == '\t' || ch == CR || ch == LF) {
                last_space = 1;
                need_space = 0;
                continue;
            }

            if (ch == ';') {
                return NGX_OK;
            }

            if (ch == '{') {
                return NGX_CONF_BLOCK_START;
            }

            if (ch == ')') {
                last_space = 1;
                need_space = 0;

            } else {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "unexpected \"%c\"", ch);
                return NGX_ERROR;
            }
        }

        if (last_space) {
            if (ch == ' ' || ch == '\t' || ch == CR || ch == LF) {
                continue;
            }

            start = b->pos - 1;
            start_line = cf->conf_file->line;

            switch (ch) {

            case ';':
            case '{':
                if (cf->args->nelts == 0) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "unexpected \"%c\"", ch);
                    return NGX_ERROR;
                }

                if (ch == '{') {
                    return NGX_CONF_BLOCK_START;
                }

                return NGX_OK;

            case '}':
                if (cf->args->nelts != 0) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "unexpected \"}\"");
                    return NGX_ERROR;
                }

                return NGX_CONF_BLOCK_DONE;

            case '#':
                sharp_comment = 1;
                continue;

            case '\\':
                quoted = 1;
                last_space = 0;
                continue;

            case '"':
                start++;
                d_quoted = 1;
                last_space = 0;
                continue;

            case '\'':
                start++;
                s_quoted = 1;
                last_space = 0;
                continue;

            default:
                last_space = 0;
            }

        } else {
            if (ch == '{' && variable) {
                continue;
            }

            variable = 0;

            if (ch == '\\') {
                quoted = 1;
                continue;
            }

            if (ch == '$') {
                variable = 1;
                continue;
            }

            if (d_quoted) {
                if (ch == '"') {
                    d_quoted = 0;
                    need_space = 1;
                    found = 1;
                }

            } else if (s_quoted) {
                if (ch == '\'') {
                    s_quoted = 0;
                    need_space = 1;
                    found = 1;
                }

            } else if (ch == ' ' || ch == '\t' || ch == CR || ch == LF
                       || ch == ';' || ch == '{')
            {
                last_space = 1;
                found = 1;
            }

            if (found) {
                word = ngx_array_push(cf->args);
                if (word == NULL) {
                    return NGX_ERROR;
                }

                word->data = ngx_pnalloc(cf->pool, b->pos - 1 - start + 1);
                if (word->data == NULL) {
                    return NGX_ERROR;
                }

                for (dst = word->data, src = start, len = 0;
                     src < b->pos - 1;
                     len++)
                {
                    if (*src == '\\') {
                        switch (src[1]) {
                        case '"':
                        case '\'':
                        case '\\':
                            src++;
                            break;

                        case 't':
                            *dst++ = '\t';
                            src += 2;
                            continue;

                        case 'r':
                            *dst++ = '\r';
                            src += 2;
                            continue;

                        case 'n':
                            *dst++ = '\n';
                            src += 2;
                            continue;
                        }

                    }
                    *dst++ = *src++;
                }
                *dst = '\0';
                word->len = len;

                if (ch == ';') {
                    return NGX_OK;
                }

                if (ch == '{') {
                    return NGX_CONF_BLOCK_START;
                }

                found = 0;
            }
        }
    }
}

ngx_int_t
ngx_dynamic_conf_parse(ngx_conf_t *cf, unsigned init)
{
    ngx_int_t                   rc;
    enum {
        parse_init = 0,
        parse_block
    } type;

    type = init ? parse_init : parse_block;

    for (;;) {
        rc = ngx_dynamic_conf_read_token(cf);

		/*
         * ngx_conf_read_token() may return
         *
         *    NGX_ERROR             there is error
         *    NGX_OK                the token terminated by ";" was found
         *    NGX_CONF_BLOCK_START  the token terminated by "{" was found
         *    NGX_CONF_BLOCK_DONE   the "}" was found
         *    NGX_CONF_FILE_DONE    the configuration file is done
         */

        if (rc == NGX_ERROR) {
            return NGX_ERROR;
        }

        if (rc == NGX_CONF_BLOCK_DONE) {

            if (type != parse_block) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "unexpected \"}\"");
                return NGX_ERROR;
            }

            return NGX_OK;
        }

        if (rc == NGX_CONF_FILE_DONE) {

            if (type == parse_block) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                        "unexpected end of file, expecting \"}\"");
                return NGX_ERROR;
            }

            return NGX_OK;
        }

        rc = ngx_dynamic_conf_handler(cf, rc);

        if (rc == NGX_ERROR) {
            return NGX_ERROR;
        }
    }
}

static ngx_int_t
ngx_dynamic_conf_load_conf(ngx_fd_t fd, ngx_dynamic_conf_conf_t *dccf)
{
    ngx_dynamic_conf_ctx_t     *conf;
    ngx_dynamic_core_module_t  *module;
    ngx_pool_t                 *pool;
    ngx_conf_t                  cf;
    ngx_buf_t                   buf;
    ngx_conf_file_t             conf_file;
    u_char                      buffer[NGX_CONF_BUFFER];
    ngx_uint_t                  i;
    void                       *rv;

    ngx_memzero(&cf, sizeof(ngx_conf_t));

    pool = ngx_create_pool(NGX_CYCLE_POOL_SIZE, dccf->log);
    if (pool == NULL) {
        return NGX_ERROR;
    }
    pool->log = dccf->log;

    conf = ngx_pcalloc(pool, sizeof(ngx_dynamic_conf_ctx_t));
    if (conf == NULL) {
        goto failed;
    }
    conf->pool = pool;

    ngx_core_max_module = ngx_count_modules((ngx_cycle_t *) ngx_cycle,
                                            NGX_CORE_MODULE);

    conf->module_conf = ngx_pcalloc(pool, sizeof(void *) * ngx_core_max_module);
    if (conf->module_conf == NULL) {
        goto failed;
    }

    cf.args = ngx_array_create(pool, 10, sizeof(ngx_str_t));
    if (cf.args == NULL) {
        goto failed;
    }

    cf.temp_pool = ngx_create_pool(NGX_CYCLE_POOL_SIZE, dccf->log);
    if (cf.temp_pool == NULL) {
        goto failed;
    }

    cf.ctx = conf->module_conf;
    cf.cycle = (ngx_cycle_t *) ngx_cycle;
    cf.pool = pool;
    cf.log = dccf->log;
    cf.module_type = NGX_CORE_MODULE;
    cf.cmd_type = NGX_MAIN_CONF;

    cf.conf_file = &conf_file;

    if (ngx_fd_info(fd, &cf.conf_file->file.info) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_EMERG, cf.log, ngx_errno,
                ngx_fd_info_n " failed");
    }

    cf.conf_file->buffer = &buf;

    buf.pos = buf.last = buf.start = buffer;
    buf.end = buf.start + NGX_CONF_BUFFER;
    buf.temporary = 1;

    cf.conf_file->file.fd = fd;
    cf.conf_file->file.name.len = dccf->file.len;
    cf.conf_file->file.name.data = dccf->file.data;
    cf.conf_file->file.offset = 0;
    cf.conf_file->file.log = cf.log;
    cf.conf_file->line = 1;

    for (i = 0; ngx_cycle->modules[i]; ++i) {
        if (ngx_cycle->modules[i]->type != NGX_CORE_MODULE) {
            continue;
        }

        module = (ngx_dynamic_core_module_t *)
                    ngx_cycle->modules[i]->spare_hook0;
        if (module == NULL) { /* dctx not configured */
            continue;
        }

        if (module->create_conf) {
            rv = module->create_conf(&cf);
            if (rv == NULL) {
                goto failed;
            }
            conf->module_conf[ngx_cycle->modules[i]->ctx_index] = rv;
        }
    }

    if (ngx_dynamic_conf_parse(&cf, 1) != NGX_OK) {
        goto failed;
    }

    for (i = 0; ngx_cycle->modules[i]; ++i) {
        if (ngx_cycle->modules[i]->type != NGX_CORE_MODULE) {
            continue;
        }

        module = (ngx_dynamic_core_module_t *) ngx_cycle->modules[i]->
                                               spare_hook0;
        if (module == NULL) {
            continue;
        }

        if (module->init_conf) {

            rv = conf->module_conf[ngx_cycle->modules[i]->ctx_index];
            if (module->init_conf(&cf, rv) == NGX_CONF_ERROR) {
                goto failed;
            }
        }
    }

    /* load conf ok, switch conf to new conf */
    if (dccf->conf[dccf->used]) {
        pool = dccf->conf[dccf->used]->pool;
        dccf->conf[dccf->used] = NULL;
        ngx_destroy_pool(pool);
    }

    dccf->used = dccf->used ? 0 : 1;
    dccf->conf[dccf->used] = conf;

    return NGX_OK;

failed:
    if (cf.temp_pool) {
        ngx_destroy_pool(cf.temp_pool);
    }

    ngx_destroy_pool(pool);

    return NGX_ERROR;
}

static void
ngx_dynamic_conf_check_conf(void *data)
{
    ngx_dynamic_conf_conf_t    *dccf;
    ngx_fd_t                    fd;
    u_char                      md5key[NGX_MD5KEY_LEN];
    NGX_START_TIMING

    dccf = data;

    if (ngx_exiting) { /* avoid nginx reload worker hungup */
        return;
    }

    fd = ngx_open_file(dccf->file.data, NGX_FILE_RDONLY, NGX_FILE_OPEN, 0);
    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, dccf->log, ngx_errno,
                ngx_open_file_n "\"%V\" failed", &dccf->file);
        goto done;
    }

    /* check md5key for dynamic conf file */
    if (ngx_md5_file(fd, md5key) == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, dccf->log, 0, "md5 file \"%V\" failed",
                &dccf->file);
        goto done;
    }

    if (ngx_memcmp(dccf->md5key, md5key, NGX_MD5KEY_LEN) == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, dccf->log, 0,
                "\"%V\" md5key not change", &dccf->file);
        goto done;
    }

    /* parse dynamic conf */
    if (ngx_dynamic_conf_load_conf(fd, dccf) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, dccf->log, 0, "load file \"%V\" failed",
                &dccf->file);
        goto done;
    }

    ngx_memcpy(dccf->md5key, md5key, NGX_MD5KEY_LEN);

    ngx_log_error(NGX_LOG_INFO, dccf->log, 0,
            "dynamic conf load successd, md5key: \"%s\"", dccf->md5key);

done:
    ngx_close_file(fd);

    ngx_event_timer_add_timer(dccf->refresh, ngx_dynamic_conf_check_conf, dccf);

    NGX_STOP_TIMING(dccf->log, "ngx_dynamic_conf_check_conf")
}

static ngx_int_t ngx_dynamic_conf_process_init(ngx_cycle_t *cycle)
{
    ngx_dynamic_conf_conf_t    *dccf;
    ngx_fd_t                    fd;
    u_char                      md5key[NGX_MD5KEY_LEN];

    NGX_START_TIMING

    dccf = (ngx_dynamic_conf_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                                    ngx_dynamic_conf_module);

    if (dccf->file.len == 0) { /* dynamic conf not configured */
        return NGX_OK;
    }

    fd = ngx_open_file(dccf->file.data, NGX_FILE_RDONLY, NGX_FILE_OPEN, 0);
    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, dccf->log, ngx_errno,
                ngx_open_file_n "\"%V\" failed", &dccf->file);
        return NGX_ERROR;
    }

    /* parse dynamic conf */
    if (ngx_dynamic_conf_load_conf(fd, dccf) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, dccf->log, 0, "load file \"%V\" failed",
                &dccf->file);
        return NGX_ERROR;
    }

    /* calc md5key for dynamic conf file */
    if (ngx_md5_file(fd, md5key) == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, dccf->log, 0, "md5 file \"%V\" failed",
                &dccf->file);
        return NGX_ERROR;
    }

    ngx_close_file(fd);

    ngx_memcpy(dccf->md5key, md5key, NGX_MD5KEY_LEN);

    ngx_log_error(NGX_LOG_INFO, dccf->log, 0,
            "dynamic conf load successd, md5key: \"%s\"", dccf->md5key);

    /* add dynamic conf parse timer */
    ngx_event_timer_add_timer(dccf->refresh, ngx_dynamic_conf_check_conf, dccf);

    NGX_STOP_TIMING(dccf->log, "ngx_dynamic_conf_process_init")

    return NGX_OK;
}

#if (NGX_PCRE)
ngx_dynamic_regex_t *
ngx_dynamic_regex_compile(ngx_conf_t *cf, ngx_regex_compile_t *rc)
{
    ngx_dynamic_regex_t        *re;

    rc->pool = cf->pool;

    if (ngx_regex_compile(rc) != NGX_OK) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "%V", &rc->err);
        return NULL;
    }

    re = ngx_pcalloc(cf->pool, sizeof(ngx_regex_elt_t));
    if (re == NULL) {
        return NULL;
    }

    re->regex = rc->regex;
    re->name = rc->pattern;

    return re;
}
#endif

int
ngx_dynamic_cmp_dns_wildcards(const void *one, const void *two)
{
    ngx_hash_key_t  *first, *second;

    first = (ngx_hash_key_t *) one;
    second = (ngx_hash_key_t *) two;

    return ngx_dns_strcmp(first->key.data, second->key.data);
}

void *
ngx_get_dconf(ngx_module_t *m)
{
    ngx_dynamic_conf_conf_t    *dccf;

    dccf = (ngx_dynamic_conf_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                    ngx_dynamic_conf_module);

    if (dccf->conf[dccf->used] == 0) {  /* dynamic conf not configured */
        return NULL;
    }

    return dccf->conf[dccf->used]->module_conf[m->ctx_index];
}
